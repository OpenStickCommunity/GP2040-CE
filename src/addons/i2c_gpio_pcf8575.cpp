#include "addons/i2c_gpio_pcf8575.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

// available, setup, preprocess, process関数を含む完全な実装
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

/**
 * 【重要】マクロエンジンが判定を行う前に実行される preprocess() で全てを処理します
 */
void PCF8575Addon::preprocess() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    // 1. 変数リセット（各入力のフラグを毎フレーム false に初期化）
	  // 変数のリセット（これをしないとマクロが止まらなくなります）
    inputButtonMacro1 = inputButtonMacro2 = false;
    inputButtonMacro3 = inputButtonMacro4 = inputButtonMacro5 = inputButtonMacro6 = false;
    // ... 他のボタンのリセット ...
    inputButtonUp = inputButtonDown = inputButtonLeft = inputButtonRight = false;
    inputButtonB1 = inputButtonB2 = inputButtonB3 = inputButtonB4 = false;
    inputButtonL1 = inputButtonR1 = inputButtonL2 = inputButtonR2 = false;
    inputButtonS1 = inputButtonS2 = inputButtonA1 = inputButtonA2 = false;
    inputButtonL3 = inputButtonR3 = inputButtonA3 = inputButtonA4 = false;
    inputButtonEXT1 = inputButtonEXT2 = inputButtonEXT3 = false;
    inputButtonEXT4 = inputButtonEXT5 = inputButtonEXT6 = false;
	  inputButtonEXT7 = inputButtonEXT8 = inputButtonEXT9 = false;    
	  inputButtonEXT10 = inputButtonEXT11 = inputButtonEXT12 = false;
	
	  // -------------------------------------

       // --- 1. 全ポートの状態を一括取得 (ループの外で1回だけ行う) ---
    uint16_t allPins = pcf->receive(); 

    for (std::map<uint8_t, GpioMappingInfo>::iterator pin = pinRef.begin(); pin != pinRef.end(); ++pin) {
        if (pin->second.direction == GpioDirection::GPIO_DIRECTION_INPUT) {
            
            // --- 2. ビット演算で特定のピンの状態を抽出 (0なら押されている) ---
            bool pinValue = !(allPins & (1 << pin->first));
             
            // 起動直後のガード
            if (bootSkipCount < 100) {
                pinValue = false;
            }
            // --- 3. 押されている(true)の時だけ、各変数に反映させる ---
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
                case GpioAction::BUTTON_PRESS_FN:    inputButtonFN = true; break;
                // ... (既存のケース) ...
                // --- 追加分 ---
                // i2c_gpio_pcf8575.cpp の switch文の中身をこれに差し替え
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
								case GpioAction::BUTTON_PRESS_E10:    inputButtonEXT10 = true; break; 
								case GpioAction::BUTTON_PRESS_E11:    inputButtonEXT11 = true; break;
								case GpioAction::BUTTON_PRESS_E12:    inputButtonEXT12 = true; break;
							  // --- 必ずここ（pinValueが生きているスコープ）に追加してください ---
                case GpioAction::BUTTON_PRESS_MACRO:   inputButtonMacro = true; break;
                case GpioAction::BUTTON_PRESS_MACRO_1: inputButtonMacro1 = true; break;
                case GpioAction::BUTTON_PRESS_MACRO_2: inputButtonMacro2 = true; break;
                case GpioAction::BUTTON_PRESS_MACRO_3: inputButtonMacro3 = true; break;
                case GpioAction::BUTTON_PRESS_MACRO_4: inputButtonMacro4 = true; break;
                case GpioAction::BUTTON_PRESS_MACRO_5: inputButtonMacro5 = true; break;
                case GpioAction::BUTTON_PRESS_MACRO_6: inputButtonMacro6 = true; break;
                // ------------------------------------------------------------
							default:                             break;
}
       } else if (pin->second.direction == GpioDirection::GPIO_DIRECTION_OUTPUT) {
            switch (pin->second.action) {
                case GpioAction::BUTTON_PRESS_UP:    pcf->setPin(pin->first, !((gamepad->state.dpad & GAMEPAD_MASK_UP) == GAMEPAD_MASK_UP)); break;
                case GpioAction::BUTTON_PRESS_DOWN:  pcf->setPin(pin->first, !((gamepad->state.dpad & GAMEPAD_MASK_DOWN) == GAMEPAD_MASK_DOWN)); break;
                case GpioAction::BUTTON_PRESS_LEFT:  pcf->setPin(pin->first, !((gamepad->state.dpad & GAMEPAD_MASK_LEFT) == GAMEPAD_MASK_LEFT)); break;
                case GpioAction::BUTTON_PRESS_RIGHT: pcf->setPin(pin->first, !((gamepad->state.dpad & GAMEPAD_MASK_RIGHT) == GAMEPAD_MASK_RIGHT)); break;
                case GpioAction::BUTTON_PRESS_B1:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_B1) == GAMEPAD_MASK_B1)); break;
                case GpioAction::BUTTON_PRESS_B2:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_B2) == GAMEPAD_MASK_B2)); break;
                case GpioAction::BUTTON_PRESS_B3:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_B3) == GAMEPAD_MASK_B3)); break;
                case GpioAction::BUTTON_PRESS_B4:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_B4) == GAMEPAD_MASK_B4)); break;
                case GpioAction::BUTTON_PRESS_L1:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_L1) == GAMEPAD_MASK_L1)); break;
                case GpioAction::BUTTON_PRESS_R1:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_R1) == GAMEPAD_MASK_R1)); break;
                case GpioAction::BUTTON_PRESS_L2:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_L2) == GAMEPAD_MASK_L2)); break;
                case GpioAction::BUTTON_PRESS_R2:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_R2) == GAMEPAD_MASK_R2)); break;
                case GpioAction::BUTTON_PRESS_S1:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_S1) == GAMEPAD_MASK_S1)); break;
                case GpioAction::BUTTON_PRESS_S2:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_S2) == GAMEPAD_MASK_S2)); break;
                case GpioAction::BUTTON_PRESS_L3:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_L3) == GAMEPAD_MASK_L3)); break;
                case GpioAction::BUTTON_PRESS_R3:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_R3) == GAMEPAD_MASK_R3)); break;
                case GpioAction::BUTTON_PRESS_A1:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_A1) == GAMEPAD_MASK_A1)); break;
                case GpioAction::BUTTON_PRESS_A2:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_A2) == GAMEPAD_MASK_A2)); break;
                // ... (既存のケース) ...
                // --- 追加分 ---
                // i2c_gpio_pcf8575.cpp の switch文の中身をこれに差し替え
								// --- 出力(OUTPUT)側の追加分 ---
               case GpioAction::BUTTON_PRESS_A3:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_A3) == GAMEPAD_MASK_A3)); break;
							 case GpioAction::BUTTON_PRESS_A4:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_A4) == GAMEPAD_MASK_A4)); break;
               case GpioAction::BUTTON_PRESS_E1:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_E1) == GAMEPAD_MASK_E1)); break;
               case GpioAction::BUTTON_PRESS_E2:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_E2) == GAMEPAD_MASK_E2)); break;
               case GpioAction::BUTTON_PRESS_E3:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_E3) == GAMEPAD_MASK_E3)); break;
               case GpioAction::BUTTON_PRESS_E4:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_E4) == GAMEPAD_MASK_E4)); break;
               case GpioAction::BUTTON_PRESS_E5:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_E5) == GAMEPAD_MASK_E5)); break;
               case GpioAction::BUTTON_PRESS_E6:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_E6) == GAMEPAD_MASK_E6)); break;
               case GpioAction::BUTTON_PRESS_E7:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_E7) == GAMEPAD_MASK_E7)); break;
               case GpioAction::BUTTON_PRESS_E8:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_E8) == GAMEPAD_MASK_E8)); break;
               case GpioAction::BUTTON_PRESS_E9:    pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_E9) == GAMEPAD_MASK_E9)); break;
               case GpioAction::BUTTON_PRESS_E10:   pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_E10) == GAMEPAD_MASK_E10)); break;
               case GpioAction::BUTTON_PRESS_E11:   pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_E11) == GAMEPAD_MASK_E11)); break;
               case GpioAction::BUTTON_PRESS_E12:   pcf->setPin(pin->first, !((gamepad->state.buttons & GAMEPAD_MASK_E12) == GAMEPAD_MASK_E12)); break;
							default:                             break;
            }

        } else {
            // NYI
        }
    }
  // A3, A4, EXT1~12 の基本反映
    if (inputButtonA3)   gamepad->debouncedGpio |= GAMEPAD_MASK_A3;
    if (inputButtonA4)   gamepad->debouncedGpio |= GAMEPAD_MASK_A4;
    if (inputButtonEXT1) gamepad->debouncedGpio |= GAMEPAD_MASK_E1;
    if (inputButtonEXT2) gamepad->debouncedGpio |= GAMEPAD_MASK_E2;
    if (inputButtonEXT3) gamepad->debouncedGpio |= GAMEPAD_MASK_E3;
    if (inputButtonEXT4) gamepad->debouncedGpio |= GAMEPAD_MASK_E4;
    if (inputButtonEXT5) gamepad->debouncedGpio |= GAMEPAD_MASK_E5;
    if (inputButtonEXT6) gamepad->debouncedGpio |= GAMEPAD_MASK_E6;
    if (inputButtonEXT7) gamepad->debouncedGpio |= GAMEPAD_MASK_E7;
    if (inputButtonEXT8) gamepad->debouncedGpio |= GAMEPAD_MASK_E8;
    if (inputButtonEXT9) gamepad->debouncedGpio |= GAMEPAD_MASK_E9;
    if (inputButtonEXT10) gamepad->debouncedGpio |= GAMEPAD_MASK_E10;
    if (inputButtonEXT11) gamepad->debouncedGpio |= GAMEPAD_MASK_E11;
    if (inputButtonEXT12) gamepad->debouncedGpio |= GAMEPAD_MASK_E12;

    // マクロ専用ボタン入力を E7~E12 のビットに重ねて反映
    if (inputButtonMacro1) gamepad->debouncedGpio |= GAMEPAD_MASK_E7;
    if (inputButtonMacro2) gamepad->debouncedGpio |= GAMEPAD_MASK_E8;
    if (inputButtonMacro3) gamepad->debouncedGpio |= GAMEPAD_MASK_E9;
    if (inputButtonMacro4) gamepad->debouncedGpio |= GAMEPAD_MASK_E10;
    if (inputButtonMacro5) gamepad->debouncedGpio |= GAMEPAD_MASK_E11;
    if (inputButtonMacro6) gamepad->debouncedGpio |= GAMEPAD_MASK_E12;

    // これにより、Webコンフィグの Preview 上でもボタンが光るようになります
    if (inputButtonMacro1) gamepad->state.buttons |= GAMEPAD_MASK_E7;
    if (inputButtonMacro2) gamepad->state.buttons |= GAMEPAD_MASK_E8;
    if (inputButtonMacro3) gamepad->state.buttons |= GAMEPAD_MASK_E9;
    if (inputButtonMacro4) gamepad->state.buttons |= GAMEPAD_MASK_E10;
    if (inputButtonMacro5) gamepad->state.buttons |= GAMEPAD_MASK_E11;
    if (inputButtonMacro6) gamepad->state.buttons |= GAMEPAD_MASK_E12;
	
    // 2. PC送信用の状態 (gamepad->state) への反映
    // 十字キー
    if (inputButtonUp)    gamepad->state.dpad |= GAMEPAD_MASK_UP;
    if (inputButtonDown)  gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
    if (inputButtonLeft)  gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
    if (inputButtonRight) gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;

    // メインボタン B1~A2
    if (inputButtonB1) gamepad->state.buttons |= GAMEPAD_MASK_B1;
    if (inputButtonB2) gamepad->state.buttons |= GAMEPAD_MASK_B2;
    if (inputButtonB3) gamepad->state.buttons |= GAMEPAD_MASK_B3;
    if (inputButtonB4) gamepad->state.buttons |= GAMEPAD_MASK_B4;
    if (inputButtonL1) gamepad->state.buttons |= GAMEPAD_MASK_L1;
    if (inputButtonR1) gamepad->state.buttons |= GAMEPAD_MASK_R1;
    if (inputButtonL2) gamepad->state.buttons |= GAMEPAD_MASK_L2;
    if (inputButtonR2) gamepad->state.buttons |= GAMEPAD_MASK_R2;
    if (inputButtonS1) gamepad->state.buttons |= GAMEPAD_MASK_S1;
    if (inputButtonS2) gamepad->state.buttons |= GAMEPAD_MASK_S2;
    if (inputButtonL3) gamepad->state.buttons |= GAMEPAD_MASK_L3;
    if (inputButtonR3) gamepad->state.buttons |= GAMEPAD_MASK_R3;
    if (inputButtonA1) gamepad->state.buttons |= GAMEPAD_MASK_A1;
    if (inputButtonA2) gamepad->state.buttons |= GAMEPAD_MASK_A2;

    // 拡張ボタン A3, A4, E1~E12
    if (inputButtonA3)   gamepad->state.buttons |= GAMEPAD_MASK_A3;
    if (inputButtonA4)   gamepad->state.buttons |= GAMEPAD_MASK_A4;
    if (inputButtonEXT1) gamepad->state.buttons |= GAMEPAD_MASK_E1;
    if (inputButtonEXT2) gamepad->state.buttons |= GAMEPAD_MASK_E2;
    if (inputButtonEXT3) gamepad->state.buttons |= GAMEPAD_MASK_E3;
    if (inputButtonEXT4) gamepad->state.buttons |= GAMEPAD_MASK_E4;
    if (inputButtonEXT5) gamepad->state.buttons |= GAMEPAD_MASK_E5;
    if (inputButtonEXT6) gamepad->state.buttons |= GAMEPAD_MASK_E6;
    if (inputButtonEXT7) gamepad->state.buttons |= GAMEPAD_MASK_E7;
    if (inputButtonEXT8) gamepad->state.buttons |= GAMEPAD_MASK_E8;
    if (inputButtonEXT9) gamepad->state.buttons |= GAMEPAD_MASK_E9;
    if (inputButtonEXT10) gamepad->state.buttons |= GAMEPAD_MASK_E10;
    if (inputButtonEXT11) gamepad->state.buttons |= GAMEPAD_MASK_E11;
    if (inputButtonEXT12) gamepad->state.buttons |= GAMEPAD_MASK_E12;
    
	  // --- 修正案：Core 間のメモリ同期を確実にするための書き方 ---  
	  if (inputButtonMacro1) {
        gamepad->debouncedGpio |= GAMEPAD_MASK_E7;
        gamepad->state.buttons |= GAMEPAD_MASK_E7;
    }
    if (inputButtonMacro2) {
        gamepad->debouncedGpio |= GAMEPAD_MASK_E8;
        gamepad->state.buttons |= GAMEPAD_MASK_E8;
    }
	  if (inputButtonMacro3) {
        gamepad->debouncedGpio |= GAMEPAD_MASK_E9;
        gamepad->state.buttons |= GAMEPAD_MASK_E9;
    }
    if (inputButtonMacro4) {
        gamepad->debouncedGpio |= GAMEPAD_MASK_E10;
        gamepad->state.buttons |= GAMEPAD_MASK_E10;
    }	
	  if (inputButtonMacro5) {
        gamepad->debouncedGpio |= GAMEPAD_MASK_E11;
        gamepad->state.buttons |= GAMEPAD_MASK_E11;
    }
    if (inputButtonMacro6) {
        gamepad->debouncedGpio |= GAMEPAD_MASK_E12;
        gamepad->state.buttons |= GAMEPAD_MASK_E12;
    }	
	
    // 3. 起動時スキップカウント
    if (bootSkipCount < 100) {
        bootSkipCount++;
    }
} // preprocess の終了

// process 関数は空の状態で定義
void PCF8575Addon::process() {}
