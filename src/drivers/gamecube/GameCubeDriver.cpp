#include "drivers/gamecube/GameCubeDriver.h"
#include "storagemanager.h"
#include "hardware/pio.h"
#include "gamecube.h"

// GameCube protocol timing constants
#define GC_POLL_TIME_US 1000
#define GC_RESPONSE_TIME_US 850

void GameCubeDriver::initialize() {
    // Initialize PIO for GameCube protocol
    pio = pio0;  // Use PIO0 by default
    sm = pio_claim_unused_sm(pio, true);

    // Get data pin from storage configuration
    const GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    data_pin = pinMappings[0].pin;  // Use first available pin for now

    // Initialize the pin
    gpio_init(data_pin);
    gpio_set_dir(data_pin, GPIO_IN);
    gpio_pull_up(data_pin);

    // Load PIO program for GameCube protocol
    offset = pio_add_program(pio, &gamecube_program);
    gamecube_program_init(pio, sm, offset, data_pin);
}

void GameCubeDriver::initializeAux() {
    // No auxiliary initialization needed
}

bool GameCubeDriver::process(Gamepad* gamepad) {
    // Convert GP2040-CE gamepad state to GameCube report format
    report.a = gamepad->state.buttons & GAMEPAD_MASK_B2;
    report.b = gamepad->state.buttons & GAMEPAD_MASK_B1;
    report.x = gamepad->state.buttons & GAMEPAD_MASK_B4;
    report.y = gamepad->state.buttons & GAMEPAD_MASK_B3;
    report.start = gamepad->state.buttons & GAMEPAD_MASK_S2;

    report.dpadUp = gamepad->state.dpad & GAMEPAD_MASK_UP;
    report.dpadDown = gamepad->state.dpad & GAMEPAD_MASK_DOWN;
    report.dpadLeft = gamepad->state.dpad & GAMEPAD_MASK_LEFT;
    report.dpadRight = gamepad->state.dpad & GAMEPAD_MASK_RIGHT;

    report.z = gamepad->state.buttons & GAMEPAD_MASK_R1;
    report.r = gamepad->state.buttons & GAMEPAD_MASK_R2;
    report.l = gamepad->state.buttons & GAMEPAD_MASK_L2;

    // Convert analog values
    report.xAxis = gamepad->state.lx - 128;
    report.yAxis = gamepad->state.ly - 128;
    report.cxAxis = gamepad->state.rx - 128;
    report.cyAxis = gamepad->state.ry - 128;

    // Analog triggers
    report.left = gamepad->state.lt;
    report.right = gamepad->state.rt;

    // Send the report if we detect a poll from the console
    handlePoll();

    return true;
}

void GameCubeDriver::processAux() {
    // No auxiliary processing needed
}

void GameCubeDriver::handlePoll() {
    // Wait for poll from console and send response
    if (gamecube_wait_for_poll_start(pio, sm)) {
        sendReport();
    }
}

void GameCubeDriver::sendReport() {
    // Send the report using PIO
    gamecube_send_report(pio, sm, &report);
}

// USB descriptor callbacks - not used in GameCube mode
uint16_t GameCubeDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    return 0;
}

void GameCubeDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
}

bool GameCubeDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const* request) {
    return false;
}

const uint16_t* GameCubeDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
    return nullptr;
}

const uint8_t* GameCubeDriver::get_descriptor_device_cb() {
    return nullptr;
}

const uint8_t* GameCubeDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return nullptr;
}

const uint8_t* GameCubeDriver::get_descriptor_configuration_cb(uint8_t index) {
    return nullptr;
}

const uint8_t* GameCubeDriver::get_descriptor_device_qualifier_cb() {
    return nullptr;
}
