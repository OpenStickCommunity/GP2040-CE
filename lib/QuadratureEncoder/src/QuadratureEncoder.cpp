#include "QuadratureEncoder.h"

#include <string.h>

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "quadrature_capture.pio.h"

// ---------------------------------------------------------------------------
// Shared tables and pools
// ---------------------------------------------------------------------------

#define RING_SIZE (1u << QUADRATURE_ENCODER_RING_BITS)
#define RING_MASK (RING_SIZE - 1)
#define CAPTURE_EMPTY 0xFF   // states are 0-3, so 0xFF marks a slot DMA hasn't filled yet

#if PICO_RP2040
#define DMA_COUNT_MASK 0xFFFFFFFFu
#else
#define DMA_COUNT_MASK DMA_CH0_TRANS_COUNT_COUNT_BITS   // top 4 bits are the mode on RP2350
#endif
#define DMA_FULL_COUNT DMA_COUNT_MASK
#define DMA_REARM_BELOW (DMA_COUNT_MASK / 2)

// index = old << 2 | new, state = A | (B << 1); forward (A leads) is 0 -> 1 -> 3 -> 2 -> 0.
// Entries 3, 6, 9, 12 are "both pins changed" (a skipped state) and count nothing.
// Not const so it stays in RAM for the IRQ path.
static int8_t quadratureTable[16] = {
     0, +1, -1,  0,
    -1,  0,  0, +1,
    +1,  0,  0, -1,
     0, -1, +1,  0,
};

static inline bool isInvalidTransition(uint8_t prev, uint8_t next) {
    return (prev ^ next) == 3;
}

// One capture ring per instance slot, aligned to its size for the DMA ring wrap.
static uint8_t captureRing[QUADRATURE_ENCODER_MAX_INSTANCES][RING_SIZE] __attribute__((aligned(RING_SIZE)));
static bool slotInUse[QUADRATURE_ENCODER_MAX_INSTANCES] = {};

// The capture program is position-independent and shared by every encoder on a PIO.
static int8_t programOffset[NUM_PIOS] = {
    -1, -1,
#if NUM_PIOS > 2
    -1,
#endif
};
static uint8_t programUsers[NUM_PIOS] = {};

// GPIO IRQ backend state, written only by the ISR (counts) and read elsewhere;
// 32-bit aligned loads/stores are atomic on Cortex-M0+/M33. The ISR runs on the
// core that called begin().
static volatile uint32_t irqCount[QUADRATURE_ENCODER_MAX_INSTANCES] = {};
static volatile uint32_t irqInvalid[QUADRATURE_ENCODER_MAX_INSTANCES] = {};
static uint8_t irqPinA[QUADRATURE_ENCODER_MAX_INSTANCES] = {};
static uint8_t irqPinB[QUADRATURE_ENCODER_MAX_INSTANCES] = {};
static uint8_t irqPrevState[QUADRATURE_ENCODER_MAX_INSTANCES] = {};

// Services one IRQ-backed encoder. Each instance slot gets its own tiny handler so
// encoders can be registered and removed independently without the shared
// handler chain calling one function several times per interrupt.
static __force_inline void serviceIrqSlot(uint8_t i) {
    uint32_t eventsA = gpio_get_irq_event_mask(irqPinA[i]);
    uint32_t eventsB = gpio_get_irq_event_mask(irqPinB[i]);
    if (!(eventsA | eventsB)) return;

    // acknowledge before sampling so an edge that lands after the read re-fires
    gpio_acknowledge_irq(irqPinA[i], eventsA);
    gpio_acknowledge_irq(irqPinB[i], eventsB);

    uint8_t state = (gpio_get(irqPinA[i]) ? 1 : 0) | (gpio_get(irqPinB[i]) ? 2 : 0);
    if (isInvalidTransition(irqPrevState[i], state)) irqInvalid[i] += 1;
    irqCount[i] += (uint32_t)(int32_t)quadratureTable[(irqPrevState[i] << 2) | state];
    irqPrevState[i] = state;
}

static_assert((QUADRATURE_ENCODER_MAX_INSTANCES >= 1) && (QUADRATURE_ENCODER_MAX_INSTANCES <= 4),
              "QUADRATURE_ENCODER_MAX_INSTANCES must be 1-4 (add IRQ slot handlers below for more)");
static void __not_in_flash_func(quadratureIrqSlot0)(void) { serviceIrqSlot(0); }
#if QUADRATURE_ENCODER_MAX_INSTANCES > 1
static void __not_in_flash_func(quadratureIrqSlot1)(void) { serviceIrqSlot(1); }
#endif
#if QUADRATURE_ENCODER_MAX_INSTANCES > 2
static void __not_in_flash_func(quadratureIrqSlot2)(void) { serviceIrqSlot(2); }
#endif
#if QUADRATURE_ENCODER_MAX_INSTANCES > 3
static void __not_in_flash_func(quadratureIrqSlot3)(void) { serviceIrqSlot(3); }
#endif
static const irq_handler_t irqSlotHandlers[QUADRATURE_ENCODER_MAX_INSTANCES] = {
    quadratureIrqSlot0,
#if QUADRATURE_ENCODER_MAX_INSTANCES > 1
    quadratureIrqSlot1,
#endif
#if QUADRATURE_ENCODER_MAX_INSTANCES > 2
    quadratureIrqSlot2,
#endif
#if QUADRATURE_ENCODER_MAX_INSTANCES > 3
    quadratureIrqSlot3,
#endif
};

static int8_t claimSlot() {
    uint32_t saved = save_and_disable_interrupts();
    int8_t slot = -1;
    for (uint8_t i = 0; i < QUADRATURE_ENCODER_MAX_INSTANCES; i++) {
        if (!slotInUse[i]) {
            slotInUse[i] = true;
            slot = (int8_t)i;
            break;
        }
    }
    restore_interrupts(saved);
    return slot;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool QuadratureEncoder::begin(const Config& config) {
    end();

    _slot = claimSlot();
    if (_slot < 0) return false;

    _pinA = config.pinA;
    _pinB = config.pinB;
    _count = 0;
    _overruns = 0;
    _invalid = 0;

    gpio_init(_pinA);
    gpio_set_dir(_pinA, GPIO_IN);
    gpio_init(_pinB);
    gpio_set_dir(_pinB, GPIO_IN);
    if (config.pullUp) {
        gpio_pull_up(_pinA);
        gpio_pull_up(_pinB);
    }

    if (beginPio(config)) return true;
    if (config.allowIrqFallback && beginIrq(config)) return true;

    slotInUse[_slot] = false;
    _slot = -1;
    return false;
}

void QuadratureEncoder::end() {
    if (_backend == Backend::PioCapture) {
        uint pioIndex = pio_get_index(_pio);
        pio_sm_set_enabled(_pio, (uint)_sm, false);
        dma_channel_abort((uint)_dma);
        dma_channel_unclaim((uint)_dma);
        pio_sm_unclaim(_pio, (uint)_sm);
        if (programUsers[pioIndex] && --programUsers[pioIndex] == 0) {
            pio_remove_program(_pio, &quadrature_capture_program, (uint)programOffset[pioIndex]);
            programOffset[pioIndex] = -1;
        }
        _pio = nullptr;
        _sm = -1;
        _dma = -1;
        _ring = nullptr;
    } else if (_backend == Backend::GpioIrq) {
        gpio_set_irq_enabled(_pinA, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
        gpio_set_irq_enabled(_pinB, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);
        gpio_remove_raw_irq_handler_masked64((1ull << _pinA) | (1ull << _pinB), irqSlotHandlers[_slot]);
    }

    if (_slot >= 0) slotInUse[_slot] = false;
    _slot = -1;
    _backend = Backend::None;
}

uint32_t QuadratureEncoder::getCount() {
    if (_backend == Backend::PioCapture) {
        drainCapture();
        return _count;
    }
    if (_backend == Backend::GpioIrq) {
        return irqCount[_slot];
    }
    return _count;
}

uint32_t QuadratureEncoder::invalidTransitions() const {
    if (_backend == Backend::GpioIrq) return irqInvalid[_slot];
    return _invalid;
}

// ---------------------------------------------------------------------------
// PIO capture backend
// ---------------------------------------------------------------------------

bool QuadratureEncoder::beginPio(const Config& config) {
    // `in pins, 2` reads two consecutive GPIOs
    if (!pinsAdjacent(_pinA, _pinB)) return false;
    uint pinLow = (_pinA < _pinB) ? _pinA : _pinB;

    for (uint8_t c = 0; c < config.pioCandidateCount; c++) {
        PIO pio = config.pioCandidates[c].pio;
        uint8_t reserved = config.pioCandidates[c].reservedSmMask;
        uint pioIndex = pio_get_index(pio);

#if PICO_PIO_USE_GPIO_BASE
        // don't move a PIO's GPIO window out from under other users; just skip it
        uint gpioBase = pio_get_gpio_base(pio);
        if ((pinLow < gpioBase) || ((pinLow + 1) >= (gpioBase + 32))) continue;
#endif

        if ((programOffset[pioIndex] < 0) && !pio_can_add_program(pio, &quadrature_capture_program)) continue;

        int sm = -1;
        for (uint s = 0; s < NUM_PIO_STATE_MACHINES; s++) {
            if (reserved & (1u << s)) continue;
            if (!pio_sm_is_claimed(pio, s)) {
                pio_sm_claim(pio, s);
                sm = (int)s;
                break;
            }
        }
        if (sm < 0) continue;

        int dma = dma_claim_unused_channel(false);
        if (dma < 0) {
            pio_sm_unclaim(pio, (uint)sm);
            return false; // DMA is global; no point trying another PIO
        }

        if (programOffset[pioIndex] < 0) {
            programOffset[pioIndex] = (int8_t)pio_add_program(pio, &quadrature_capture_program);
        }
        programUsers[pioIndex]++;

        _pio = pio;
        _sm = (int8_t)sm;
        _dma = (int8_t)dma;
        _swapBits = (_pinB < _pinA); // bit 0 of the captured state is the lower pin
        _ring = captureRing[_slot];
        _prevState = 0xFF;
        _entriesWritten = 0;
        _entriesConsumed = 0;
        _dmaLastRemaining = DMA_FULL_COUNT;
        memset(_ring, CAPTURE_EMPTY, RING_SIZE);

        uint32_t sampleHz = config.sampleHz ? config.sampleHz : QUADRATURE_ENCODER_DEFAULT_SAMPLE_HZ;
        float clkdiv = (float)clock_get_hz(clk_sys) / (4.0f * (float)sampleHz); // 4 cycles per sample
        if (clkdiv < 1.0f) clkdiv = 1.0f;
        quadrature_capture_program_init(pio, (uint)sm, (uint)programOffset[pioIndex], pinLow, clkdiv);

        // RX FIFO (low byte = 2-bit state) -> ring buffer, paced by the SM's RX DREQ
        dma_channel_config dc = dma_channel_get_default_config((uint)dma);
        channel_config_set_transfer_data_size(&dc, DMA_SIZE_8);
        channel_config_set_read_increment(&dc, false);
        channel_config_set_write_increment(&dc, true);
        channel_config_set_ring(&dc, true, QUADRATURE_ENCODER_RING_BITS);
        channel_config_set_dreq(&dc, pio_get_dreq(pio, (uint)sm, false));
        dma_channel_configure((uint)dma, &dc, _ring, &pio->rxf[sm],
                              dma_encode_transfer_count(DMA_FULL_COUNT), true);

        pio_sm_set_enabled(pio, (uint)sm, true);

        _backend = Backend::PioCapture;
        return true;
    }

    return false;
}

// Pull everything DMA has written since the last call and decode it.
void QuadratureEncoder::drainCapture() {
    uint dma = (uint)_dma;

    // How many entries has DMA written since we last looked?
    uint32_t remaining = dma_hw->ch[dma].transfer_count & DMA_COUNT_MASK;
    _entriesWritten += (_dmaLastRemaining - remaining) & DMA_COUNT_MASK;
    _dmaLastRemaining = remaining;

    // Top the transfer count back up long before it runs out. While the channel is
    // stopped the PIO keeps sampling into its 8-deep FIFO, so nothing is lost.
    if (remaining < DMA_REARM_BELOW) {
        dma_channel_abort(dma);
        remaining = dma_hw->ch[dma].transfer_count & DMA_COUNT_MASK;
        _entriesWritten += (_dmaLastRemaining - remaining) & DMA_COUNT_MASK;
        dma_channel_set_write_addr(dma, (void *)dma_hw->ch[dma].write_addr, false);
        dma_channel_set_trans_count(dma, dma_encode_transfer_count(DMA_FULL_COUNT), true);
        _dmaLastRemaining = DMA_FULL_COUNT;
    }

    uint32_t backlog = _entriesWritten - _entriesConsumed;

    // Lapped: the oldest entries were overwritten before we got to them. Those
    // counts are gone; jump to the newer half of the ring and resync from there.
    if (backlog > RING_SIZE) {
        _overruns++;
        _entriesConsumed += backlog - (RING_SIZE / 2);
        _prevState = 0xFF;
        backlog = RING_SIZE / 2;
    }

    while (backlog--) {
        uint32_t slot = _entriesConsumed & RING_MASK;
        uint8_t value = _ring[slot];
        if (value == CAPTURE_EMPTY) break;  // counted but not landed yet; take it next time
        _ring[slot] = CAPTURE_EMPTY;
        _entriesConsumed++;

        value &= 3;
        if (_swapBits) value = (uint8_t)(((value & 1) << 1) | (value >> 1));
        if (_prevState <= 3) {
            if (isInvalidTransition(_prevState, value)) _invalid++;
            _count += (uint32_t)(int32_t)quadratureTable[(_prevState << 2) | value];
        }
        _prevState = value; // first entry (or first after an overrun) is just the baseline
    }
}

// ---------------------------------------------------------------------------
// GPIO IRQ backend
// ---------------------------------------------------------------------------

bool QuadratureEncoder::beginIrq(const Config& config) {
    (void)config;
    uint8_t slot = (uint8_t)_slot;

    irqPinA[slot] = _pinA;
    irqPinB[slot] = _pinB;
    irqPrevState[slot] = (gpio_get(_pinA) ? 1 : 0) | (gpio_get(_pinB) ? 2 : 0);
    irqCount[slot] = 0;
    irqInvalid[slot] = 0;

    // Raw handlers coexist with other GPIO IRQ users (and the SDK's default
    // callback, which skips pins claimed here).
    gpio_add_raw_irq_handler_masked64((1ull << _pinA) | (1ull << _pinB), irqSlotHandlers[slot]);
    gpio_set_irq_enabled(_pinA, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(_pinB, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    irq_set_enabled(IO_IRQ_BANK0, true);

    _backend = Backend::GpioIrq;
    return true;
}
