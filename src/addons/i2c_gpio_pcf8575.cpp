#include "addons/i2c_gpio_pcf8575.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

// アドオンが有効か確認し、I2Cデバイスをスキャンする
bool PCF8575Addon::available() {
    const PCF8575Options& options = Storage::getInstance().getAddonOptions().pcf8575Options;
    if (options.enabled) {
        pcf = new PCF8575();
        PeripheralI2CScanResult result = PeripheralManager::getInstance().scanForI2CDevice(pcf->getDeviceAddresses());
        if (result.address > -1) {
            pcf->setAddress(result.address);
            pcf->setI2C(PeripheralManager::getInstance().getI2C(result.block));
            return true;
        } else {
            delete pcf;
        }
    }
    return false;
}

// ピンの割り当て設定を読み込み、PCF8575を初期化する
void PCF8575Addon::setup() {
    const PCF8575Options& options = Storage::getInstance().getAddonOptions().pcf8575Options;
    const GpioMappingInfo* gpioMappings = options.pins;

    uint16_t pinMask = 0xFFFF;
    for (uint8_t i = 0; i < options.pins_count; i++) {
        GpioMappingInfo pin = gpioMappings[i];
        if ((pin.action != GpioAction::NONE) && (pin.action != GpioAction::RESERVED) && (pin.action != GpioAction::ASSIGNED_TO_ADDON)) {
            pinRef.insert({i,pin});
        }
    }

    if (pinRef.size() > 0) {
        pcf->begin();
        pcf->send(pinMask);
    }
}

// 毎フレームの入力処理を行う（マクロエンジン実行前に呼ばれる）
void PCF8575Addon::preprocess() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    inputButtonMacro1 = inputButtonMacro2 = false;
    inputButtonMacro3 = inputButtonMacro4 = inputButtonMacro5 = inputButtonMacro6 = false;
    inputButtonUp = inputButtonDown = inputButtonLeft = inputButtonRight = false;
    inputButtonB1 = inputButtonB2 = inputButtonB3 = inputButtonB4 = false;
    inputButtonL1 = inputButtonR1 = inputButtonL2 = inputButtonR2 = false;
    inputButtonS1 = inputButtonS2 = inputButtonA1 = inputButtonA2 = false;
    inputButtonL3 = inputButtonR3 = inputButtonA3 = inputButtonA4 = false;
    inputButtonEXT1 = inputButtonEXT2 = inputButtonEXT3 = false;
    inputButtonEXT4 = inputButtonEXT5 = inputButtonEXT6 = false;
    inputButtonEXT7 = inputButtonEXT8 = inputButtonEXT9 = false;    
    inputButtonEXT10 = inputButtonEXT11 = inputButtonEXT12 = false;

    uint16_t allPins = pcf->receive(); 

    for (std::map<uint8_t, GpioMappingInfo>::iterator pin = pinRef.begin(); pin != pinRef.end(); ++pin) {
        if (pin->second.direction == GpioDirection::GPIO_DIRECTION_INPUT) {
            bool pinValue = !(allPins & (1 << pin->first));
            if (bootSkipCount < 100) {
                pinValue = false;
            }
            if (pinValue) {
                switch (pin->second.action) {
                    case GpioAction::BUTTON_PRESS_UP:    inputButtonUp = true; break;
                    case GpioAction::BUTTON_PRESS_DOWN:  inputButtonDown = true; break;
                    case GpioAction::BUTTON_PRESS_LEFT:  inputButtonLeft = true; break;
                    case GpioAction::BUTTON_PRESS_RIGHT: inputButtonRight = true; break;
                    case GpioAction::BUTTON_PRESS_B1:    inputButtonB1 = true; break;
                    case GpioAction::BUTTON_PRESS_B2:    inputButtonB2 = true; break;
                    case GpioAction::BUTTON_PRESS_B3:    inputButtonB3 = true; break;
                    case GpioAction::BUTTON_PRESS_B4:    inputButtonB4 = true; break;
                    case GpioAction::BUTTON_PRESS_L1:    inputButtonL1 = true; break;
                    case GpioAction::BUTTON_PRESS_R1:    inputButtonR1 = true; break;
                    case GpioAction::BUTTON_PRESS_L2:    inputButtonL2 = true; break;
                    case GpioAction::BUTTON_PRESS_R2:    inputButtonR2 = true; break;
		                case GpioAction::BUTTON_PRESS_S1:    inputButtonS1 = true; break;
                    case GpioAction::BUTTON_PRESS_S2:    inputButtonS2 = true; break;
                    case GpioAction::BUTTON_PRESS_L3:    inputButtonL3 = true; break;
                    case GpioAction::BUTTON_PRESS_R3:    inputButtonR3 = true; break;
                    case GpioAction::BUTTON_PRESS_A1:    inputButtonA1 = true; break;
                    case GpioAction::BUTTON_PRESS_A2:    inputButtonA2 = true; break;
                    case GpioAction::BUTTON_PRESS_A3:    inputButtonA3 = true; break;
                    case GpioAction::BUTTON_PRESS_A4:    inputButtonA4 = true; break;
                    case GpioAction::BUTTON_PRESS_E1:    inputButtonEXT1 = true; break;
                    case GpioAction::BUTTON_PRESS_E2:    inputButtonEXT2 = true; break;
                    case GpioAction::BUTTON_PRESS_E3:    inputButtonEXT3 = true; break;
                    case GpioAction::BUTTON_PRESS_E4:    inputButtonEXT4 = true; break;
                    case GpioAction::BUTTON_PRESS_E5:    inputButtonEXT5 = true; break;
                    case GpioAction::BUTTON_PRESS_E6:    inputButtonEXT6 = true; break;
                    case GpioAction::BUTTON_PRESS_E7:    inputButtonEXT7 = true; break;
                    case GpioAction::BUTTON_PRESS_E8:    inputButtonEXT8 = true; break;
                    case GpioAction::BUTTON_PRESS_E9:    inputButtonEXT9 = true; break;
                    case GpioAction::BUTTON_PRESS_E10:   inputButtonEXT10 = true; break;
                    case GpioAction::BUTTON_PRESS_E11:   inputButtonEXT11 = true; break;
                    case GpioAction::BUTTON_PRESS_E12:   inputButtonEXT12 = true; break;
                    case GpioAction::BUTTON_PRESS_MACRO_1: inputButtonMacro1 = true; break;
                    case GpioAction::BUTTON_PRESS_MACRO_2: inputButtonMacro2 = true; break;
                    case GpioAction::BUTTON_PRESS_MACRO_3: inputButtonMacro3 = true; break;
                    case GpioAction::BUTTON_PRESS_MACRO_4: inputButtonMacro4 = true; break;
                    case GpioAction::BUTTON_PRESS_MACRO_5: inputButtonMacro5 = true; break;
                    case GpioAction::BUTTON_PRESS_MACRO_6: inputButtonMacro6 = true; break;
                    default: break;
                }
            }
				} else if (pin->second.direction == GpioDirection::GPIO_DIRECTION_OUTPUT) {
            bool outState = true;
            switch (pin->second.action) {
                case GpioAction::BUTTON_PRESS_UP:    outState = !((gamepad->state.dpad & GAMEPAD_MASK_UP) == GAMEPAD_MASK_UP); break;
                case GpioAction::BUTTON_PRESS_DOWN:  outState = !((gamepad->state.dpad & GAMEPAD_MASK_DOWN) == GAMEPAD_MASK_DOWN); break;
                case GpioAction::BUTTON_PRESS_LEFT:  outState = !((gamepad->state.dpad & GAMEPAD_MASK_LEFT) == GAMEPAD_MASK_LEFT); break;
                case GpioAction::BUTTON_PRESS_RIGHT: outState = !((gamepad->state.dpad & GAMEPAD_MASK_RIGHT) == GAMEPAD_MASK_RIGHT); break;
                case GpioAction::BUTTON_PRESS_B1:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_B1) == GAMEPAD_MASK_B1); break;
                case GpioAction::BUTTON_PRESS_B2:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_B2) == GAMEPAD_MASK_B2); break;
                case GpioAction::BUTTON_PRESS_B3:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_B3) == GAMEPAD_MASK_B3); break;
                case GpioAction::BUTTON_PRESS_B4:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_B4) == GAMEPAD_MASK_B4); break;
                case GpioAction::BUTTON_PRESS_L1:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_L1) == GAMEPAD_MASK_L1); break;
                case GpioAction::BUTTON_PRESS_R1:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_R1) == GAMEPAD_MASK_R1); break;
                case GpioAction::BUTTON_PRESS_L2:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_L2) == GAMEPAD_MASK_L2); break;
                case GpioAction::BUTTON_PRESS_R2:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_R2) == GAMEPAD_MASK_R2); break;
                case GpioAction::BUTTON_PRESS_S1:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_S1) == GAMEPAD_MASK_S1); break;
                case GpioAction::BUTTON_PRESS_S2:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_S2) == GAMEPAD_MASK_S2); break;
                case GpioAction::BUTTON_PRESS_L3:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_L3) == GAMEPAD_MASK_L3); break;
                case GpioAction::BUTTON_PRESS_R3:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_R3) == GAMEPAD_MASK_R3); break;
                case GpioAction::BUTTON_PRESS_A1:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_A1) == GAMEPAD_MASK_A1); break;
                case GpioAction::BUTTON_PRESS_A2:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_A2) == GAMEPAD_MASK_A2); break;
                case GpioAction::BUTTON_PRESS_A3:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_A3) == GAMEPAD_MASK_A3); break;
                case GpioAction::BUTTON_PRESS_A4:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_A4) == GAMEPAD_MASK_A4); break;
                case GpioAction::BUTTON_PRESS_E1:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_E1) == GAMEPAD_MASK_E1); break;
                case GpioAction::BUTTON_PRESS_E2:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_E2) == GAMEPAD_MASK_E2); break;
                case GpioAction::BUTTON_PRESS_E3:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_E3) == GAMEPAD_MASK_E3); break;
                case GpioAction::BUTTON_PRESS_E4:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_E4) == GAMEPAD_MASK_E4); break;
                case GpioAction::BUTTON_PRESS_E5:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_E5) == GAMEPAD_MASK_E5); break;
                case GpioAction::BUTTON_PRESS_E6:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_E6) == GAMEPAD_MASK_E6); break;
                case GpioAction::BUTTON_PRESS_E7:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_E7) == GAMEPAD_MASK_E7); break;
                case GpioAction::BUTTON_PRESS_E8:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_E8) == GAMEPAD_MASK_E8); break;
                case GpioAction::BUTTON_PRESS_E9:    outState = !((gamepad->state.buttons & GAMEPAD_MASK_E9) == GAMEPAD_MASK_E9); break;
                case GpioAction::BUTTON_PRESS_E10:   outState = !((gamepad->state.buttons & GAMEPAD_MASK_E10) == GAMEPAD_MASK_E10); break;
                case GpioAction::BUTTON_PRESS_E11:   outState = !((gamepad->state.buttons & GAMEPAD_MASK_E11) == GAMEPAD_MASK_E11); break;
                case GpioAction::BUTTON_PRESS_E12:   outState = !((gamepad->state.buttons & GAMEPAD_MASK_E12) == GAMEPAD_MASK_E12); break;
                default: break;
            }
            pcf->setPin(pin->first, outState);
        }
    }
// D-Pad とボタン入力の反映
    if (inputButtonUp)    gamepad->state.dpad |= GAMEPAD_MASK_UP;
    if (inputButtonDown)  gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
    if (inputButtonLeft)  gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
    if (inputButtonRight) gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
    
    // 通常ボタン
    if (inputButtonB1)    gamepad->state.buttons |= GAMEPAD_MASK_B1;
    if (inputButtonB2)    gamepad->state.buttons |= GAMEPAD_MASK_B2;
    if (inputButtonB3)    gamepad->state.buttons |= GAMEPAD_MASK_B3;
    if (inputButtonB4)    gamepad->state.buttons |= GAMEPAD_MASK_B4;
    if (inputButtonL1)    gamepad->state.buttons |= GAMEPAD_MASK_L1;
    if (inputButtonR1)    gamepad->state.buttons |= GAMEPAD_MASK_R1;
    if (inputButtonL2)    gamepad->state.buttons |= GAMEPAD_MASK_L2;
    if (inputButtonR2)    gamepad->state.buttons |= GAMEPAD_MASK_R2;
    if (inputButtonS1)    gamepad->state.buttons |= GAMEPAD_MASK_S1;
  	if (inputButtonS2)    gamepad->state.buttons |= GAMEPAD_MASK_S2;
    if (inputButtonL3)    gamepad->state.buttons |= GAMEPAD_MASK_L3;
    if (inputButtonR3)    gamepad->state.buttons |= GAMEPAD_MASK_R3;
    if (inputButtonA1)    gamepad->state.buttons |= GAMEPAD_MASK_A1;
    if (inputButtonA2)    gamepad->state.buttons |= GAMEPAD_MASK_A2;
    if (inputButtonA3)    gamepad->state.buttons |= GAMEPAD_MASK_A3;
    if (inputButtonA4)    gamepad->state.buttons |= GAMEPAD_MASK_A4;
    if (inputButtonEXT1)  gamepad->state.buttons |= GAMEPAD_MASK_E1;
  	if (inputButtonEXT2)  gamepad->state.buttons |= GAMEPAD_MASK_E2;
    if (inputButtonEXT3)  gamepad->state.buttons |= GAMEPAD_MASK_E3;
    if (inputButtonEXT4)  gamepad->state.buttons |= GAMEPAD_MASK_E4;
    if (inputButtonEXT5)  gamepad->state.buttons |= GAMEPAD_MASK_E5;
    if (inputButtonEXT6)  gamepad->state.buttons |= GAMEPAD_MASK_E6;
    if (inputButtonEXT7)  gamepad->state.buttons |= GAMEPAD_MASK_E7;
    if (inputButtonEXT8)  gamepad->state.buttons |= GAMEPAD_MASK_E8;
    if (inputButtonEXT9)  gamepad->state.buttons |= GAMEPAD_MASK_E9;
	  if (inputButtonEXT10) gamepad->state.buttons |= GAMEPAD_MASK_E10;
    if (inputButtonEXT11) gamepad->state.buttons |= GAMEPAD_MASK_E11;
    if (inputButtonEXT12) gamepad->state.buttons |= GAMEPAD_MASK_E12;
    if (inputButtonMacro1) gamepad->state.buttons |= GAMEPAD_MASK_E7;
    if (inputButtonMacro2) gamepad->state.buttons |= GAMEPAD_MASK_E8;
    if (inputButtonMacro3) gamepad->state.buttons |= GAMEPAD_MASK_E9;
    if (inputButtonMacro4) gamepad->state.buttons |= GAMEPAD_MASK_E10;
    if (inputButtonMacro5) gamepad->state.buttons |= GAMEPAD_MASK_E11;
  	if (inputButtonMacro6) gamepad->state.buttons |= GAMEPAD_MASK_E12;
    
    // 拡張ボタン/マクロ処理
    if (inputButtonEXT1)  gamepad->state.buttons |= GAMEPAD_MASK_E1;
    if (inputButtonEXT2)  gamepad->state.buttons |= GAMEPAD_MASK_E2;
    if (inputButtonEXT3)  gamepad->state.buttons |= GAMEPAD_MASK_E3;
    if (inputButtonEXT4)  gamepad->state.buttons |= GAMEPAD_MASK_E4;
    if (inputButtonEXT5)  gamepad->state.buttons |= GAMEPAD_MASK_E5;
    if (inputButtonEXT6)  gamepad->state.buttons |= GAMEPAD_MASK_E6;
    if (inputButtonEXT7)  gamepad->state.buttons |= GAMEPAD_MASK_E7;
    if (inputButtonEXT8)  gamepad->state.buttons |= GAMEPAD_MASK_E8;
    if (inputButtonEXT9)  gamepad->state.buttons |= GAMEPAD_MASK_E9;
  	if (inputButtonEXT10) gamepad->state.buttons |= GAMEPAD_MASK_E10;
    if (inputButtonEXT11) gamepad->state.buttons |= GAMEPAD_MASK_E11;
    if (inputButtonEXT12) gamepad->state.buttons |= GAMEPAD_MASK_E12;
    
    // --- 修正版：Macro 1〜6 を安全な空きビットに割り当て ---
    // state.buttons と debouncedGpio の両方にビットを立てることで、
    // マクロエンジンが確実に「押し下げイベント」を検知できるようになります。
    if (inputButtonMacro1) { gamepad->state.buttons |= (1ULL << 26); gamepad->debouncedGpio |= (1ULL << 26); }
    if (inputButtonMacro2) { gamepad->state.buttons |= (1ULL << 27); gamepad->debouncedGpio |= (1ULL << 27); }
    if (inputButtonMacro3) { gamepad->state.buttons |= (1ULL << 28); gamepad->debouncedGpio |= (1ULL << 28); }
    if (inputButtonMacro4) { gamepad->state.buttons |= (1ULL << 29); gamepad->debouncedGpio |= (1ULL << 29); }
    if (inputButtonMacro5) { gamepad->state.buttons |= (1ULL << 30); gamepad->debouncedGpio |= (1ULL << 30); }
    if (inputButtonMacro6) { gamepad->state.buttons |= (1ULL << 31); gamepad->debouncedGpio |= (1ULL << 31); }	
    
	  if (bootSkipCount < 100) {
        bootSkipCount++;
    }
}

void PCF8575Addon::process() {
}
