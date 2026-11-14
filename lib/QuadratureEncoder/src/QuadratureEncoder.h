#ifndef _QUADRATURE_ENCODER_H_
#define _QUADRATURE_ENCODER_H_

// QuadratureEncoder: counts every edge of a quadrature encoder (4 counts per
// cycle) without depending on how often the application polls it.
//
// Backends:
//   PioCapture - a 6-instruction PIO program samples the two pins and pushes
//                each change; DMA streams the changes into a RAM ring buffer;
//                getCount() decodes them. Needs adjacent GPIOs, one state
//                machine and one DMA channel. Program is shared per PIO.
//   GpioIrq    - edge interrupts on both pins decode in the ISR. Any pins, no
//                PIO or DMA, but edges can be lost while interrupts are held off.
//
// The library has no placement policy of its own: the caller says which PIO
// blocks it may use and which state machines on them are off limits, so the
// application (or a future PIO manager) decides who gets PIO.

#include <stdint.h>
#include "hardware/pio.h"

// Changes buffered per encoder between getCount() calls: 2^bits bytes.
// 10 bits = 1 KB = ~40 ms of continuous spinning at 24k changes/s.
#ifndef QUADRATURE_ENCODER_RING_BITS
#define QUADRATURE_ENCODER_RING_BITS 10
#endif

// Maximum encoders that can be active at once (sizes the static ring/IRQ pools).
#ifndef QUADRATURE_ENCODER_MAX_INSTANCES
#define QUADRATURE_ENCODER_MAX_INSTANCES 2
#endif

#define QUADRATURE_ENCODER_DEFAULT_SAMPLE_HZ 2000000u

class QuadratureEncoder {
public:
    enum class Backend : uint8_t {
        None = 0,
        PioCapture,
        GpioIrq,
    };

    // A PIO block the caller allows, and state machines on it that must not be
    // used (bit n = SM n), e.g. ones another driver uses without claiming.
    struct PioCandidate {
        PIO pio;
        uint8_t reservedSmMask;
    };

    struct Config {
        uint8_t pinA = 0;
        uint8_t pinB = 0;
        bool pullUp = true;

        // Tried in order. Empty list = never use PIO.
        const PioCandidate * pioCandidates = nullptr;
        uint8_t pioCandidateCount = 0;

        bool allowIrqFallback = true;

        // PIO sample rate. Lower ignores shorter glitches/bounce; edges must stay
        // more than ~1.5 sample periods apart to be counted.
        uint32_t sampleHz = QUADRATURE_ENCODER_DEFAULT_SAMPLE_HZ;
    };

    QuadratureEncoder() = default;
    ~QuadratureEncoder() { end(); }
    QuadratureEncoder(const QuadratureEncoder&) = delete;
    QuadratureEncoder& operator=(const QuadratureEncoder&) = delete;

    // Configures pins and picks a backend. Returns false if neither backend could
    // be set up (no free instance slot, no PIO/DMA and IRQ fallback disallowed).
    bool begin(const Config& config);

    // Releases the state machine, DMA channel, IRQ slot and pins' interrupts.
    // The PIO program is removed once no encoder on that PIO uses it.
    void end();

    // Running count of edges, 4 per quadrature cycle, positive when A leads B.
    // Free-running and wraps: use (uint32_t) differences between calls.
    // For the PIO backend this also drains the capture buffer, so call it at
    // least once per buffer's worth of changes (see QUADRATURE_ENCODER_RING_BITS).
    uint32_t getCount();

    Backend backend() const { return _backend; }
    PIO pio() const { return _pio; }
    int stateMachine() const { return _sm; }
    int dmaChannel() const { return _dma; }

    // Times the capture buffer was lapped before being read (counts were lost).
    uint32_t overruns() const { return _overruns; }
    // Samples where both pins changed at once (a state was skipped; count lost).
    uint32_t invalidTransitions() const;

    static bool pinsAdjacent(uint8_t a, uint8_t b) { return (a + 1 == b) || (b + 1 == a); }

private:
    bool beginPio(const Config& config);
    bool beginIrq(const Config& config);
    void drainCapture();

    Backend _backend = Backend::None;
    int8_t _slot = -1;          // index into the static ring / IRQ pools
    uint8_t _pinA = 0;
    uint8_t _pinB = 0;

    // PIO capture
    PIO _pio = nullptr;
    int8_t _sm = -1;
    int8_t _dma = -1;
    bool _swapBits = false;     // pin B is the lower-numbered pin
    uint8_t * _ring = nullptr;
    uint32_t _dmaLastRemaining = 0;
    uint32_t _entriesWritten = 0;
    uint32_t _entriesConsumed = 0;
    uint8_t _prevState = 0xFF;  // 0xFF = not yet known
    uint32_t _count = 0;
    uint32_t _overruns = 0;
    uint32_t _invalid = 0;
};

#endif
