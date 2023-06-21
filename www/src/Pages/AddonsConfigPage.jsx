import React, { useContext, useEffect, useState } from 'react';
import { Button, Form, Row, FormCheck } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import * as yup from 'yup';
import { Trans, useTranslation } from 'react-i18next';

import { AppContext } from '../Contexts/AppContext';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import KeyboardMapper, { validateMappings } from '../Components/KeyboardMapper';
import Section from '../Components/Section';
import WebApi, { baseButtonMappings } from '../Services/WebApi';
import JSEncrypt from 'jsencrypt';
import CryptoJS from 'crypto-js';
import get from 'lodash/get'
import set from "lodash/set"

const I2C_BLOCKS = [
	{ label: 'i2c0', value: 0 },
	{ label: 'i2c1', value: 1 },
];

const ON_BOARD_LED_MODES = [
	{ label: 'Off', value: 0 },
	{ label: 'Mode Indicator', value: 1 },
	{ label: 'Input Test', value: 2 }
];

const DUAL_STICK_MODES = [
	{ label: 'D-Pad', value: 0 },
	{ label: 'Left Analog', value: 1 },
	{ label: 'Right Analog', value: 2 },
];

const DUAL_COMBINE_MODES = [
    { label: 'Mixed', value: 0 },
	{ label: 'Gamepad', value: 1},
	{ label: 'Dual Directional', value: 2 },
	{ label: 'None', value: 3 }
];

const SHMUP_MIXED_MODES = [
	{ label: 'Turbo Priority', value: 0 },
	{ label: 'Charge Priority', value: 1}
];

const ANALOG_PINS = [
	-1,26,27,28,29
];

const ANALOG_PIN_OPTIONS = ANALOG_PINS.map((i) => <option key={`analogPins-option-${i}`} value={i}>{i === -1 ? 'None' : i}</option>);

const BUTTON_MASKS = [
	{ label: 'None',  value:  0          },
	{ label: 'B1',    value:  (1 << 0)   },
	{ label: 'B2',    value:  (1 << 1)   },
	{ label: 'B3',    value:  (1 << 2)   },
	{ label: 'B4',    value:  (1 << 3)   },
	{ label: 'L1',    value:  (1 << 4)   },
	{ label: 'R1',    value:  (1 << 5)   },
	{ label: 'L2',    value:  (1 << 6)   },
	{ label: 'R2',    value:  (1 << 7)   },
	{ label: 'S1',    value:  (1 << 8)   },
	{ label: 'S2',    value:  (1 << 9)   },
	{ label: 'L3',    value:  (1 << 10)  },
	{ label: 'R3',    value:  (1 << 11)  },
	{ label: 'A1',    value:  (1 << 12)  },
	{ label: 'A2',    value:  (1 << 13)  },
	{ label: 'Up',    value:  (1 << 16)  },
	{ label: 'Down',  value:  (1 << 17)  },
	{ label: 'Left',  value:  (1 << 18)  },
	{ label: 'Right', value:  (1 << 19)  },
];

const TURBO_MASKS = [
	{ label: 'None',  value:  0          },
	{ label: 'B1',    value:  (1 << 0)   },
	{ label: 'B2',    value:  (1 << 1)   },
	{ label: 'B3',    value:  (1 << 2)   },
	{ label: 'B4',    value:  (1 << 3)   },
	{ label: 'L1',    value:  (1 << 4)   },
	{ label: 'R1',    value:  (1 << 5)   },
	{ label: 'L2',    value:  (1 << 6)   },
	{ label: 'R2',    value:  (1 << 7)   },
	{ label: 'L3',    value:  (1 << 10)  },
	{ label: 'R3',    value:  (1 << 11)  }
]

const REVERSE_ACTION = [
	{ label: 'Disable', value: 0 },
	{ label: 'Enable', value: 1 },
	{ label: 'Neutral', value: 2 },
];

const verifyAndSavePS4 = async () => {
	let PS4Key = document.getElementById("ps4key-input");
	let PS4Serial = document.getElementById("ps4serial-input");
	let PS4Signature = document.getElementById("ps4signature-input");

	let count = 0;
	var pem;
	var signature;
	var serial;

	const handlePEM = (e) => {
		pem = keyReader.result;
		count++;
	};

	const handleSignature = (e) => {
		signature = sigReader.result;
		count++;
	};

	const handleSerial = (e) => {
		serial = serialReader.result;
		count++;
	};

	let keyReader = new FileReader();
	keyReader.onloadend = handlePEM;
	keyReader.readAsText(PS4Key.files[0]);

	let serialReader = new FileReader();
	serialReader.onloadend = handleSerial;
	serialReader.readAsText(PS4Serial.files[0]);

	let sigReader = new FileReader();
	sigReader.onloadend = handleSignature;
	sigReader.readAsBinaryString(PS4Signature.files[0]);

	async function checkRead () {
		if ( count < 3 ) {
			setTimeout(checkRead, 1000);
		} else {
			// Make sure our signature is 256 bytes
			if ( signature.length !== 256 || serial.length !== 16) {
				throw new Error("Signature or serial is invalid");
			}
			try {
				serial = serial.padStart(32,'0'); // Add our padding

				const key = new JSEncrypt();
				key.setPrivateKey(pem);
				const bytes = new Uint8Array(256);
				for(let i = 0; i < 256; i++){
					bytes[i] = Math.random();
				}
				const hashed = CryptoJS.SHA256(bytes);
				const signNonce = key.sign(hashed, CryptoJS.SHA256, "sha256");

				if (signNonce === false) {
					throw new Error("Bad Private Key");
				}

				// Private key worked!
				var BigInteger = require('jsbn').BigInteger;

				// Translate these to BigInteger
				var N = new BigInteger(String(key.key.n));
				var E = new BigInteger(String(key.key.e));
				var D = new BigInteger(String(key.key.d));
				var P = new BigInteger(String(key.key.p));
				var Q = new BigInteger(String(key.key.q));
				var DP = new BigInteger(String(key.key.dmp1));
				var DQ = new BigInteger(String(key.key.dmq1));
				var constantR = new BigInteger('2').pow(4096); 	// constant R
				var QP = Q.modInverse(P); 						// qp = 1 / ( Q % P)
				var RN = constantR.mod(N); 						// rn = constant R mod N

				function int2mbedmpi(num) {
					var out = [];
					var mask = new BigInteger('4294967295');
					var zero = new BigInteger('0');
					while(!num.equals(zero)) {
						out.push(num.and(mask).toString(16).padStart(8, '0'));
						num = num.shiftRight(32);
					}
					return out;
				}

				function hexToBytes(hex) {
					let bytes = [];
					for (let c = 0; c < hex.length; c += 2)
						bytes.push(parseInt(hex.substr(c, 2), 16));
					return bytes;
				}

				function mbedmpi2b64(mpi) {
					var arr = new Uint8Array(mpi.length*4);
					var cnt = 0;
					for ( let i = 0; i < mpi.length; i++) {
						let bytes = hexToBytes(mpi[i]);
						for ( let j = 4; j > 0; j--) {
							//arr[cnt] = bytes[j];
							// TEST: re-order from LSB to MSB
							arr[cnt] = bytes[j-1];
							cnt++;
						}
					}

					return btoa(String.fromCharCode.apply(null, arr));
				}

				const sendPS4Chunks = async (chunks) => {
					for ( var i in chunks ) {
						if (await WebApi.setPS4Options(chunks[i]) === false ) {
							return false;
						}
					}
					return true;
				};


				let serialBin = hexToBytes(serial);

				let success = await sendPS4Chunks([{
					N: mbedmpi2b64(int2mbedmpi(N)),
					E: mbedmpi2b64(int2mbedmpi(E)),
					D: mbedmpi2b64(int2mbedmpi(D))
				}, {
					P: mbedmpi2b64(int2mbedmpi(P)),
					Q: mbedmpi2b64(int2mbedmpi(Q)),
					DP: mbedmpi2b64(int2mbedmpi(DP)),
					DQ: mbedmpi2b64(int2mbedmpi(DQ))
				}, {
					QP: mbedmpi2b64(int2mbedmpi(QP)),
					RN: mbedmpi2b64(int2mbedmpi(RN)),
					serial: btoa(String.fromCharCode(...new Uint8Array(serialBin)))
				}, {
					signature: btoa(signature)
				}]);

				if ( success ) {
					document.getElementById("ps4alert").textContent = 'Verified and Saved PS4 Mode! Reboot to take effect';
					document.getElementById("save").click();
				} else {
					throw Error("PS4 Chunks Error");
				}

			} catch (e) {
				document.getElementById("ps4alert").textContent = 'ERROR: Could not verify required files';
			}
		}
	};
	setTimeout(checkRead, 1000);
};

const SOCD_MODES = [
	{ label: 'Up Priority', value: 0 },
	{ label: 'Neutral', value: 1 },
	{ label: 'Last Win', value: 2 },
	{ label: 'First Win', value: 3 },
	{ label: 'SOCD Cleaning Off', value: 4 },
];

const schema = yup.object().shape({
	I2CAnalog1219InputEnabled:   yup.number().label('I2C Analog1219 Input Enabled'),
	i2cAnalog1219SDAPin:         yup.number().label('I2C Analog1219 SDA Pin').validatePinWhenValue('I2CAnalog1219InputEnabled'),
	i2cAnalog1219SCLPin:         yup.number().label('I2C Analog1219 SCL Pin').validatePinWhenValue('I2CAnalog1219InputEnabled'),
	i2cAnalog1219Block:          yup.number().label('I2C Analog1219 Block').validateSelectionWhenValue('I2CAnalog1219InputEnabled', I2C_BLOCKS),
	i2cAnalog1219Speed:          yup.number().label('I2C Analog1219 Speed').validateNumberWhenValue('I2CAnalog1219InputEnabled'),
	i2cAnalog1219Address:        yup.number().label('I2C Analog1219 Address').validateNumberWhenValue('I2CAnalog1219InputEnabled'),

	AnalogInputEnabled:          yup.number().required().label('Analog Input Enabled'),
	analogAdcPinX:               yup.number().label('Analog Stick Pin X').validatePinWhenValue('AnalogInputEnabled'),
 	analogAdcPinY:               yup.number().label('Analog Stick Pin Y').validatePinWhenValue('AnalogInputEnabled'),
	forced_circularity:          yup.number().label('Force Circularity').validateRangeWhenValue('AnalogInputEnabled', 0, 1),
	analog_deadzone:             yup.number().label('Deadzone Size (%)').validateRangeWhenValue('AnalogInputEnabled', 0, 100),

	BoardLedAddonEnabled:        yup.number().required().label('Board LED Add-On Enabled'),
	onBoardLedMode:              yup.number().label('On-Board LED Mode').validateSelectionWhenValue('BoardLedAddonEnabled', ON_BOARD_LED_MODES),

	BootselButtonAddonEnabled:   yup.number().required().label('Boot Select Button Add-On Enabled'),
	bootselButtonMap:            yup.number().label('BOOTSEL Button Map').validateSelectionWhenValue('BootselButtonAddonEnabled', BUTTON_MASKS),

	FocusModeAddonEnabled:       yup.number().required().label('Focus Mode Add-On Enabled'),
	focusModePin:                yup.number().label('Focus Mode Pin').validatePinWhenValue('FocusModeAddonEnabled'),
	focusModeButtonLockEnabled:  yup.number().label('Focus Mode Button Lock Enabled').validateRangeWhenValue('FocusModeAddonEnabled', 0, 1),
	focusModeOledLockEnabled:    yup.number().label('Focus Mode OLED Lock Enabled').validateRangeWhenValue('FocusModeAddonEnabled', 0, 1),
	focusModeRgbLockEnabled:     yup.number().label('Focus Mode RGB Lock Enabled').validateRangeWhenValue('FocusModeAddonEnabled', 0, 1),
	focusModeButtonLockMask:      yup.number().label('Focus Mode Button Lock Map').validateRangeWhenValue('FocusModeAddonEnabled', 0, (1<<20) - 1),

	BuzzerSpeakerAddonEnabled:   yup.number().required().label('Buzzer Speaker Add-On Enabled'),
	buzzerPin:                   yup.number().label('Buzzer Pin').validatePinWhenValue('BuzzerSpeakerAddonEnabled'),
	buzzerVolume:                yup.number().label('Buzzer Volume').validateRangeWhenValue('BuzzerSpeakerAddonEnabled', 0, 100),

	DualDirectionalInputEnabled: yup.number().required().label('Dual Directional Input Enabled'),
	dualDirUpPin:                yup.number().label('Dual Directional Up Pin').validatePinWhenValue('DualDirectionalInputEnabled')  ,
	dualDirDownPin:              yup.number().label('Dual Directional Down Pin').validatePinWhenValue('DualDirectionalInputEnabled'),
	dualDirLeftPin:              yup.number().label('Dual Directional Left Pin').validatePinWhenValue('DualDirectionalInputEnabled'),
	dualDirRightPin:             yup.number().label('Dual Directional Right Pin').validatePinWhenValue('DualDirectionalInputEnabled'),
	dualDirDpadMode:             yup.number().label('Dual Stick Mode').validateSelectionWhenValue('DualDirectionalInputEnabled', DUAL_STICK_MODES),
	dualDirCombineMode:          yup.number().label('Dual Combination Mode').validateSelectionWhenValue('DualDirectionalInputEnabled', DUAL_COMBINE_MODES),
	dualDirFourWayMode:          yup.number().label('Dual Directional 4-Way Joystick Mode').validateRangeWhenValue('DualDirectionalInputEnabled', 0, 1),

	ExtraButtonAddonEnabled:     yup.number().required().label('Extra Button Add-On Enabled'),
	extraButtonPin:              yup.number().label('Extra Button Pin').validatePinWhenValue('ExtraButtonAddonEnabled'),
	extraButtonMap:              yup.number().label('Extra Button Map').validateSelectionWhenValue('ExtraButtonAddonEnabled', BUTTON_MASKS),

	JSliderInputEnabled:         yup.number().required().label('JSlider Input Enabled'),
	sliderLSPin:                 yup.number().label('Slider LS Pin').validatePinWhenValue('JSliderInputEnabled'),
	sliderRSPin:                 yup.number().label('Slider RS Pin').validatePinWhenValue('JSliderInputEnabled'),

	KeyboardHostAddonEnabled:    yup.number().required().label('Keyboard Host Add-On Enabled'),
	keyboardHostPinDplus:        yup.number().label('Keyboard Host D+ Pin').validatePinWhenValue('KeyboardHostAddonEnabled'),

	PlayerNumAddonEnabled:       yup.number().required().label('Player Number Add-On Enabled'),
	playerNumber:                yup.number().label('Player Number').validateRangeWhenValue('PlayerNumAddonEnabled', 1, 4),

	PS4ModeAddonEnabled: yup.number().required().label('PS4 Mode Add-on Enabled'),

	ReverseInputEnabled:         yup.number().required().label('Reverse Input Enabled'),
	reversePin:                  yup.number().label('Reverse Pin').validatePinWhenValue('ReverseInputEnabled'),
	reversePinLED:               yup.number().label('Reverse Pin LED').validatePinWhenValue('ReverseInputEnabled'),

	SliderSOCDInputEnabled:      yup.number().required().label('Slider SOCD Input Enabled'),
	sliderSOCDModeOne:           yup.number().label('SOCD Slider Mode One').validateSelectionWhenValue('SliderSOCDInputEnabled', SOCD_MODES),
	sliderSOCDModeTwo:           yup.number().label('SOCD Slider Mode Two').validateSelectionWhenValue('SliderSOCDInputEnabled', SOCD_MODES),
	sliderSOCDModeDefault:       yup.number().label('SOCD Slider Mode Default').validateSelectionWhenValue('SliderSOCDInputEnabled', SOCD_MODES),
	sliderSOCDPinOne:            yup.number().label('Slider SOCD Up Priority Pin').validatePinWhenValue('SliderSOCDInputEnabled'),
	sliderSOCDPinTwo:            yup.number().label('Slider SOCD Second Priority Pin').validatePinWhenValue('SliderSOCDInputEnabled'),

	TurboInputEnabled:           yup.number().required().label('Turbo Input Enabled'),
	turboPin:                    yup.number().label('Turbo Pin').validatePinWhenValue('TurboInputEnabled'),
	turboPinLED:                 yup.number().label('Turbo Pin LED').validatePinWhenValue('TurboInputEnabled'),
	pinShmupBtn1:                yup.number().label('Charge Shot 1 Pin').validatePinWhenValue('TurboInputEnabled'),
	pinShmupBtn2:                yup.number().label('Charge Shot 2 Pin').validatePinWhenValue('TurboInputEnabled'),
	pinShmupBtn3:                yup.number().label('Charge Shot 3 Pin').validatePinWhenValue('TurboInputEnabled'),
	pinShmupBtn4:                yup.number().label('Charge Shot 4 Pin').validatePinWhenValue('TurboInputEnabled'),
	pinShmupDial:                yup.number().label('Shmup Dial Pin').validatePinWhenValue('TurboInputEnabled'),
	turboShotCount:              yup.number().label('Turbo Shot Count').validateRangeWhenValue('TurboInputEnabled', 5, 30),
	shmupMode:                   yup.number().label('Shmup Mode Enabled').validateRangeWhenValue('TurboInputEnabled', 0, 1),
	shmupMixMode:                yup.number().label('Shmup Mix Priority').validateSelectionWhenValue('TurboInputEnabled', DUAL_STICK_MODES),
	shmupAlwaysOn1:              yup.number().label('Turbo-Button 1 (Always On)').validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupAlwaysOn2:              yup.number().label('Turbo-Button 2 (Always On)').validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupAlwaysOn3:              yup.number().label('Turbo-Button 3 (Always On)').validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupAlwaysOn4:              yup.number().label('Turbo-Button 4 (Always On)').validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupBtnMask1:               yup.number().label('Charge Shot Button 1 Map').validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupBtnMask2:               yup.number().label('Charge Shot Button 2 Map').validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupBtnMask3:               yup.number().label('Charge Shot Button 3 Map').validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupBtnMask4:               yup.number().label('Charge Shot Button 4 Map').validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),

	WiiExtensionAddonEnabled:    yup.number().required().label('Wii Extensions Enabled'),
	wiiExtensionSDAPin:          yup.number().required().label('WiiExtension I2C SDA Pin').validatePinWhenValue('WiiExtensionAddonEnabled'),
	wiiExtensionSCLPin:          yup.number().required().label('WiiExtension I2C SCL Pin').validatePinWhenValue('WiiExtensionAddonEnabled'),
	wiiExtensionBlock:           yup.number().required().label('WiiExtension I2C Block').validateSelectionWhenValue('WiiExtensionAddonEnabled', I2C_BLOCKS),
	wiiExtensionSpeed:           yup.number().label('WiiExtension I2C Speed').validateNumberWhenValue('WiiExtensionAddonEnabled'),
});

const defaultValues = {
	turboPin: -1,
	turboPinLED: -1,
	sliderLSPin: -1,
	sliderRSPin: -1,
	sliderSOCDPinOne: -1,
	sliderSOCDPinTwo: -1,
	turboShotCount: 5,
	reversePin: -1,
	reversePinLED: -1,
	i2cAnalog1219SDAPin: -1,
	i2cAnalog1219SCLPin: -1,
	i2cAnalog1219Block: 0,
	i2cAnalog1219Speed: 400000,
	i2cAnalog1219Address: 0x40,
	onBoardLedMode: 0,
	dualDirUpPin: -1,
	dualDirDownPin: -1,
	dualDirLeftPin: -1,
	dualDirRightPin: -1,
	dualDirDpadMode: 0,
	dualDirCombineMode: 0,
	dualDirFourWayMode: 0,
	analogAdcPinX : -1,
 	analogAdcPinY : -1,
	forced_circularity : 0,
	analog_deadzone: 5,
	bootselButtonMap: 0,
	buzzerPin: -1,
	buzzerVolume: 100,
	extrabuttonPin: -1,
	extraButtonMap: 0,
	playerNumber: 1,
	shmupMode: 0,
	shmupMixMode: 0,
	shmupAlwaysOn1: 0,
	shmupAlwaysOn2: 0,
	shmupAlwaysOn3: 0,
	shmupAlwaysOn4: 0,
	pinShmupBtn1: -1,
	pinShmupBtn2: -1,
	pinShmupBtn3: -1,
	pinShmupBtn4: -1,
	shmupBtnMask1: 0,
	shmupBtnMask2: 0,
	shmupBtnMask3: 0,
	shmupBtnMask4: 0,
	pinShmupDial: -1,
	sliderSOCDModeOne: 0,
	sliderSOCDModeTwo: 2,
	sliderSOCDModeDefault: 1,
	wiiExtensionSDAPin: -1,
	wiiExtensionSCLPin: -1,
	wiiExtensionBlock: 0,
	wiiExtensionSpeed: 400000,
	snesPadClockPin: -1,
	snesPadLatchPin: -1,
	snesPadDataPin: -1,
	keyboardHostPinDplus: -1,
	keyboardHostMap: baseButtonMappings,
	AnalogInputEnabled: 0,
	BoardLedAddonEnabled: 0,
	FocusModeAddonEnabled: 0,
	BuzzerSpeakerAddonEnabled: 0,
	BootselButtonAddonEnabled: 0,
	DualDirectionalInputEnabled: 0,
	ExtraButtonAddonEnabled: 0,
	I2CAnalog1219InputEnabled: 0,
	JSliderInputEnabled: 0,
	KeyboardHostAddonEnabled: 0,
	SliderSOCDInputEnabled: 0,
	PlayerNumAddonEnabled: 0,
	PS4ModeAddonEnabled: 0,
	ReverseInputEnabled: 0,
	TurboInputEnabled: 0,
	WiiExtensionAddonEnabled: 0,
	SNESpadAddonEnabled: 0,
};

const FormContext = ({setStoredData}) => {
	const { values, setValues } = useFormikContext();

	useEffect(() => {
		async function fetchData() {
			const data = await WebApi.getAddonsOptions();

			setValues(data);
			setStoredData(JSON.parse(JSON.stringify(data))); // Do a deep copy to keep the original
		}
		fetchData();
	}, [setValues]);

	useEffect(() => {
		sanitizeData(values);
	}, [values, setValues]);

	return null;
};

const sanitizeData = (values) => {
	if (!!values.turboPin)
		values.turboPin = parseInt(values.turboPin);
	if (!!values.turboPinLED)
		values.turboPinLED = parseInt(values.turboPinLED);
	if (!!values.sliderLSPin)
		values.sliderLSPin = parseInt(values.sliderLSPin);
	if (!!values.sliderRSPin)
		values.sliderRSPin = parseInt(values.sliderRSPin);
	if (!!values.sliderSOCDPinOne)
		values.sliderSOCDPinOne = parseInt(values.sliderSOCDPinOne);
	if (!!values.sliderSOCDPinTwo)
		values.sliderSOCDPinTwo = parseInt(values.sliderSOCDPinTwo);
	if (!!values.turboShotCount)
		values.turboShotCount = parseInt(values.turboShotCount);
	if (!!values.reversePin)
		values.reversePin = parseInt(values.reversePin);
	if (!!values.reversePinLED)
		values.reversePinLED = parseInt(values.reversePinLED);
	if (!!values.reverseActionUp)
		values.reverseActionUp = parseInt(values.reverseActionUp);
	if (!!values.reverseActionDown)
		values.reverseActionDown = parseInt(values.reverseActionDown);
	if (!!values.reverseActionLeft)
		values.reverseActionLeft = parseInt(values.reverseActionLeft);
	if (!!values.reverseActionRight)
		values.reverseActionRight = parseInt(values.reverseActionRight);
	if (!!values.i2cAnalog1219SDAPin)
		values.i2cAnalog1219SDAPin = parseInt(values.i2cAnalog1219SDAPin);
	if (!!values.i2cAnalog1219SCLPin)
		values.i2cAnalog1219SCLPin = parseInt(values.i2cAnalog1219SCLPin);
	if (!!values.i2cAnalog1219Block)
		values.i2cAnalog1219Block = parseInt(values.i2cAnalog1219Block);
	if (!!values.i2cAnalog1219Speed)
		values.i2cAnalog1219Speed = parseInt(values.i2cAnalog1219Speed);
	if (!!values.i2cAnalog1219Address)
		values.i2cAnalog1219Address = parseInt(values.i2cAnalog1219Address);
	if (!!values.onBoardLedMode)
		values.onBoardLedMode = parseInt(values.onBoardLedMode);
	if (!!values.dualDirDownPin)
		values.dualDirDownPin = parseInt(values.dualDirDownPin);
	if (!!values.dualDirUpPin)
		values.dualDirUpPin = parseInt(values.dualDirUpPin);
	if (!!values.dualDirLeftPin)
		values.dualDirLeftPin = parseInt(values.dualDirLeftPin);
	if (!!values.dualDirRightPin)
		values.dualDirRightPin = parseInt(values.dualDirRightPin);
	if (!!values.dualDirMode)
		values.dualDirMode = parseInt(values.dualDirMode);
	if (!!values.analogAdcPinX)
		values.analogAdcPinX = parseInt(values.analogAdcPinX);
	if (!!values.analogAdcPinY)
		values.analogAdcPinY = parseInt(values.analogAdcPinY);
	if (!!values.bootselButtonMap)
		values.bootselButtonMap = parseInt(values.bootselButtonMap);
	if (!!values.focusModeButtonLockMask)
		values.focusModeButtonLockMask = parseInt(values.focusModeButtonLockMask);
	if (!!values.focusModePin)
		values.focusModePin = parseInt(values.focusModePin);
	if (!!values.buzzerPin)
		values.buzzerPin = parseInt(values.buzzerPin);
	if (!!values.buzzerVolume)
		values.buzzerVolume = parseInt(values.buzzerVolume);
	if (!!values.extraButtonMap)
		values.extraButtonMap = parseInt(values.extraButtonMap);
	if (!!values.extraButtonPin)
		values.extraButtonPin = parseInt(values.extraButtonPin);
	if (!!values.playerNumber)
		values.playerNumber = parseInt(values.playerNumber);
	if (!!values.shmupMode)
		values.shmupMode = parseInt(values.shmupMode);
	if (!!values.shmupMixMode)
		values.shmupMixMode = parseInt(values.shmupMixMode);
	if (!!values.shmupAlwaysOn1)
		values.shmupAlwaysOn1 = parseInt(values.shmupAlwaysOn1);
	if (!!values.shmupAlwaysOn2)
		values.shmupAlwaysOn2 = parseInt(values.shmupAlwaysOn2);
	if (!!values.shmupAlwaysOn3)
		values.shmupAlwaysOn3 = parseInt(values.shmupAlwaysOn3);
	if (!!values.shmupAlwaysOn4)
		values.shmupAlwaysOn4 = parseInt(values.shmupAlwaysOn4);
	if (!!values.pinShmupBtn1)
		values.pinShmupBtn1 = parseInt(values.pinShmupBtn1);
	if (!!values.pinShmupBtn2)
		values.pinShmupBtn2 = parseInt(values.pinShmupBtn2);
	if (!!values.pinShmupBtn3)
		values.pinShmupBtn3 = parseInt(values.pinShmupBtn3);
	if (!!values.pinShmupBtn4)
		values.pinShmupBtn4 = parseInt(values.pinShmupBtn4);
	if (!!values.shmupBtnMask1)
		values.shmupBtnMask1 = parseInt(values.shmupBtnMask1);
	if (!!values.shmupBtnMask2)
		values.shmupBtnMask2 = parseInt(values.shmupBtnMask2);
	if (!!values.shmupBtnMask3)
		values.shmupBtnMask3 = parseInt(values.shmupBtnMask3);
	if (!!values.shmupBtnMask4)
		values.shmupBtnMask4 = parseInt(values.shmupBtnMask4);
	if (!!values.pinShmupDial)
		values.pinShmupDial = parseInt(values.pinShmupDial);
	if (!!values.sliderSOCDModeOne)
		values.sliderSOCDModeOne = parseInt(values.sliderSOCDModeOne);
	if (!!values.sliderSOCDModeTwo)
		values.sliderSOCDModeTwo = parseInt(values.sliderSOCDModeTwo);
	if (!!values.sliderSOCDModeDefault)
		values.sliderSOCDModeDefault = parseInt(values.sliderSOCDModeDefault);
	if (!!values.wiiExtensionSDAPin)
		values.wiiExtensionSDAPin = parseInt(values.wiiExtensionSDAPin);
	if (!!values.wiiExtensionSCLPin)
		values.wiiExtensionSCLPin = parseInt(values.wiiExtensionSCLPin);
	if (!!values.wiiExtensionBlock)
		values.wiiExtensionBlock = parseInt(values.wiiExtensionBlock);
	if (!!values.wiiExtensionSpeed)
		values.wiiExtensionSpeed = parseInt(values.wiiExtensionSpeed);
	if (!!values.snesPadClockPin)
		values.snesPadClockPin = parseInt(values.snesPadClockPin);
	if (!!values.snesPadLatchPin)
		values.snesPadLatchPin = parseInt(values.snesPadLatchPin);
	if (!!values.snesPadDataPin)
		values.snesPadDataPin = parseInt(values.snesPadDataPin);
	if (!!values.AnalogInputEnabled)
		values.AnalogInputEnabled = parseInt(values.AnalogInputEnabled);
	if (!!values.BoardLedAddonEnabled)
		values.BoardLedAddonEnabled = parseInt(values.BoardLedAddonEnabled);
	if (!!values.BuzzerSpeakerAddonEnabled)
		values.BuzzerSpeakerAddonEnabled = parseInt(values.BuzzerSpeakerAddonEnabled);
	if (!!values.BootselButtonAddonEnabled)
		values.BootselButtonAddonEnabled = parseInt(values.BootselButtonAddonEnabled);
	if (!!values.DualDirectionalInputEnabled)
		values.DualDirectionalInputEnabled = parseInt(values.DualDirectionalInputEnabled);
	if (!!values.ExtraButtonAddonEnabled)
		values.ExtraButtonAddonEnabled = parseInt(values.ExtraButtonAddonEnabled);
	if (!!values.I2CAnalog1219InputEnabled)
		values.I2CAnalog1219InputEnabled = parseInt(values.I2CAnalog1219InputEnabled);
	if (!!values.JSliderInputEnabled)
		values.JSliderInputEnabled = parseInt(values.JSliderInputEnabled);
	if (!!values.SliderSOCDInputEnabled)
		values.SliderSOCDInputEnabled = parseInt(values.SliderSOCDInputEnabled);
	if (!!values.PlayerNumAddonEnabled)
		values.PlayerNumAddonEnabled = parseInt(values.PlayerNumAddonEnabled);
	if (!!values.PS4ModeAddonEnabled)
		values.PS4ModeAddonEnabled = parseInt(values.PS4ModeAddonEnabled);
	if (!!values.ReverseInputEnabled)
		values.ReverseInputEnabled = parseInt(values.ReverseInputEnabled);
	if (!!values.TurboInputEnabled)
		values.TurboInputEnabled = parseInt(values.TurboInputEnabled);
	if (!!values.WiiExtensionAddonEnabled)
		values.WiiExtensionAddonEnabled = parseInt(values.WiiExtensionAddonEnabled);
	if (!!values.SNESpadAddonEnabled)
		values.SNESpadAddonEnabled = parseInt(values.SNESpadAddonEnabled);
}

function flattenObject(object) {
	var toReturn = {};

	for (var i in object) {
		if (!object.hasOwnProperty(i)) continue;

		if (typeof object[i] == "object" && object[i] !== null) {
			var flatObject = flattenObject(object[i]);
			for (var x in flatObject) {
				if (!flatObject.hasOwnProperty(x)) continue;

				toReturn[i + "." + x] = flatObject[x];
			}
		} else {
			toReturn[i] = object[i];
		}
	}
	return toReturn;
}

export default function AddonsConfigPage() {
	const { buttonLabels, updateUsedPins } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');
	const [storedData, setStoredData] = useState({});
	const [validated, setValidated] = useState(false);

	const { t } = useTranslation();

	const translatedReverseAction = REVERSE_ACTION.map((option) => ({
		...option,
		label: t(`AddonsConfig:reverse-action-${option.label.toLowerCase()}-label`),
	}));

	const handleKeyChange = (values, setFieldValue) => (value, button) => {
		const newMappings = {...values.keyboardHostMap};
		newMappings[button].key = value;
		const mappings = validateMappings(newMappings);
		setFieldValue('keyboardHostMap', mappings);
		setValidated(true);
	};

	// const handleSubmit = async (e) => {
	// 	e.preventDefault();
	// 	e.stopPropagation();

	// 	let mappings = {...keyMappings};
	// 	mappings = validateMappings(mappings);
	// 	setKeyMappings(mappings);
	// 	setValidated(true);

	// 	if (Object.keys(mappings).filter(p => !!mappings[p].error).length) {
	// 		setSaveMessage('Validation errors, see above');
	// 		return;
	// 	}

	// 	const success = await WebApi.setKeyMappings(mappings);
	// 	setSaveMessage(success ? 'Saved! Please Restart Your Device' : 'Unable to Save');
	// };

	const getKeyMappingForButton = (values) => (button) =>  {
		return values.keyboardHostMap[button];
	}

	const onSuccess = async (values) => {
		const flattened = flattenObject(storedData)
		const valuesCopy = schema.cast((values)) // Strip invalid values

		// Compare what's changed and set it to resultObject
		let resultObject = {}
		Object.entries(flattened)?.map(entry => {
			const [key, oldVal] = entry;
			const newVal = get(valuesCopy, key)
			if (newVal !== oldVal) {
				set(resultObject, key, newVal)
			}
		})
		sanitizeData(resultObject);
		const success = await WebApi.setAddonsOptions(resultObject);
		setStoredData(JSON.parse(JSON.stringify(values))); // Update to reflect saved data
		setSaveMessage(success ? t('Common:saved-success-message') : t('Common:saved-error-message'));
		if (success)
			updateUsedPins();
	};

	const handleCheckbox = async (name, values) => {
		values[name] = values[name] === 1 ? 0 : 1;
	};

	return (
		<Formik enableReinitialize={true} validationSchema={schema} onSubmit={onSuccess} initialValues={defaultValues}>
			{({
				handleSubmit,
				handleChange,
				values,
				errors,
				setFieldValue
			}) => (
				<Form noValidate onSubmit={handleSubmit}>
					<Section title={t('AddonsConfig:header-text')}>
						<p>{t('AddonsConfig:sub-header-text')}</p>
					</Section>
					<Section title={t('AddonsConfig:bootsel-header-text')}>
						<div
							id="BootselButtonAddonOptions"
							hidden={!values.BootselButtonAddonEnabled}>
							<p>{t('AddonsConfig:bootsel-sub-header-text')}</p>
							<FormSelect
								label={t('AddonsConfig:bootsel-button-pin-label')}
								name="bootselButtonMap"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.bootselButtonMap}
								error={errors.bootselButtonMap}
								isInvalid={errors.bootselButtonMap}
								onChange={handleChange}
							>
								{BUTTON_MASKS.map((o, i) => <option key={`bootselButtonMap-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="BootselButtonAddonButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.BootselButtonAddonEnabled)}
							onChange={(e) => { handleCheckbox("BootselButtonAddonEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:on-board-led-configuration-label')}>
						<div
							id="BoardLedAddonEnabledOptions"
							hidden={!values.BoardLedAddonEnabled}>
							<FormSelect
								label={t('AddonsConfig:on-board-led-mode-label')}
								name="onBoardLedMode"
								className="form-select-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.onBoardLedMode}
								error={errors.onBoardLedMode}
								isInvalid={errors.onBoardLedMode}
								onChange={handleChange}>
								{ON_BOARD_LED_MODES.map((o, i) => <option key={`onBoardLedMode-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="BoardLedAddonButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.BoardLedAddonEnabled)}
							onChange={(e) => {handleCheckbox("BoardLedAddonEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:analog-header-text')}>
						<div
							id="AnalogInputOptions"
							hidden={!values.AnalogInputEnabled}>
							<p>{t('AddonsConfig:analog-available-pins-text', {pins: ANALOG_PINS.filter(p => p !== -1).join(", ")})}</p>
							<Row className="mb-3">
								<FormSelect
									label={t('AddonsConfig:analog-adc-pin-x-label')}
									name="analogAdcPinX"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdcPinX}
									error={errors.analogAdcPinX}
									isInvalid={errors.analogAdcPinX}
									onChange={handleChange}
								>
									{ANALOG_PIN_OPTIONS}
								</FormSelect>
								<FormSelect
									label={t('AddonsConfig:analog-adc-pin-y-label')}
									name="analogAdcPinY"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdcPinY}
									error={errors.analogAdcPinY}
									isInvalid={errors.analogAdcPinY}
									onChange={handleChange}
								>
									{ANALOG_PIN_OPTIONS}
								</FormSelect>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="AnalogInputButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.AnalogInputEnabled)}
							onChange={(e) => {handleCheckbox("AnalogInputEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:turbo-header-text')}>
						<div
							id="TurboInputOptions"
							hidden={!values.TurboInputEnabled}>
							<Row className="mb-3">
								<FormControl type="number"
									label={t('AddonsConfig:turbo-button-pin-label')}
									name="turboPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.turboPin}
									error={errors.turboPin}
									isInvalid={errors.turboPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl type="number"
									label={t('AddonsConfig:turbo-led-pin-label')}
									name="turboPinLED"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.turboPinLED}
									error={errors.turboPinLED}
									isInvalid={errors.turboPinLED}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl type="number"
									label={t('AddonsConfig:turbo-shot-count-label')}
									name="turboShotCount"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.turboShotCount}
									error={errors.turboShotCount}
									isInvalid={errors.turboShotCount}
									onChange={handleChange}
									min={2}
									max={30}
								/>
								<FormSelect
									label={t('AddonsConfig:turbo-shmup-dial-pin-label')}
									name="pinShmupDial"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.pinShmupDial}
									error={errors.pinShmupDial}
									isInvalid={errors.pinShmupDial}
									onChange={handleChange}
								>
									{ANALOG_PIN_OPTIONS}
								</FormSelect>
								<FormCheck
									label={t('AddonsConfig:turbo-shmup-mode-label')}
									type="switch"
									id="ShmupMode"
								className="col-sm-3 ms-2"
								isInvalid={false}
								checked={Boolean(values.shmupMode)}
								onChange={(e) => {handleCheckbox("shmupMode", values); handleChange(e);}}
							/>
							<div
								id="ShmupOptions"
									hidden={!values.shmupMode}>
									<Row className="mb-3">
										<FormSelect
											label={t('AddonsConfig:turbo-shmup-always-on-1-label')}
											name="shmupAlwaysOn1"
											className="form-select-sm"
											groupClassName="col-sm-3 mb-3"
											value={values.shmupAlwaysOn1}
											error={errors.shmupAlwaysOn1}
											isInvalid={errors.shmupAlwaysOn1}
											onChange={handleChange}
										>
											{TURBO_MASKS.map((o, i) => <option key={`shmupAlwaysOn1-option-${i}`} value={o.value}>{o.label}</option>)}
										</FormSelect>
										<FormSelect
											label={t('AddonsConfig:turbo-shmup-always-on-2-label')}
											name="shmupAlwaysOn2"
											className="form-select-sm"
											groupClassName="col-sm-3 mb-3"
											value={values.shmupAlwaysOn2}
											error={errors.shmupAlwaysOn2}
											isInvalid={errors.shmupAlwaysOn2}
											onChange={handleChange}
										>
											{TURBO_MASKS.map((o, i) => <option key={`shmupAlwaysOn2-option-${i}`} value={o.value}>{o.label}</option>)}
										</FormSelect>
										<FormSelect
											label={t('AddonsConfig:turbo-shmup-always-on-3-label')}
											name="shmupAlwaysOn3"
											className="form-select-sm"
											groupClassName="col-sm-3 mb-3"
											value={values.shmupAlwaysOn3}
											error={errors.shmupAlwaysOn3}
											isInvalid={errors.shmupAlwaysOn3}
											onChange={handleChange}
										>
											{TURBO_MASKS.map((o, i) => <option key={`shmupAlwaysOn3-option-${i}`} value={o.value}>{o.label}</option>)}
										</FormSelect>
										<FormSelect
											label={t('AddonsConfig:turbo-shmup-always-on-4-label')}
											name="shmupAlwaysOn4"
											className="form-select-sm"
											groupClassName="col-sm-3 mb-3"
											value={values.shmupAlwaysOn4}
											error={errors.shmupAlwaysOn4}
											isInvalid={errors.shmupAlwaysOn4}
											onChange={handleChange}
										>
											{TURBO_MASKS.map((o, i) => <option key={`shmupAlwaysOn4-option-${i}`} value={o.value}>{o.label}</option>)}
										</FormSelect>
									</Row>
									<Row className="mb-3">
										<FormControl type="number"
											label={t('AddonsConfig:turbo-shmup-button-1-label')}
											name="pinShmupBtn1"
											className="form-control-sm"
											groupClassName="col-sm-3 mb-3"
											value={values.pinShmupBtn1}
											error={errors.pinShmupBtn1}
											isInvalid={errors.pinShmupBtn1}
											onChange={handleChange}
											min={-1}
											max={29}
										/>
										<FormControl type="number"
											label={t('AddonsConfig:turbo-shmup-button-2-label')}
											name="pinShmupBtn2"
											className="form-control-sm"
											groupClassName="col-sm-3 mb-3"
											value={values.pinShmupBtn2}
											error={errors.pinShmupBtn2}
											isInvalid={errors.pinShmupBtn2}
											onChange={handleChange}
											min={-1}
											max={29}
										/>
										<FormControl type="number"
											label={t('AddonsConfig:turbo-shmup-button-3-label')}
											name="pinShmupBtn3"
											className="form-control-sm"
											groupClassName="col-sm-3 mb-3"
											value={values.pinShmupBtn3}
											error={errors.pinShmupBtn3}
											isInvalid={errors.pinShmupBtn3}
											onChange={handleChange}
											min={-1}
											max={29}
										/>
										<FormControl type="number"
											label={t('AddonsConfig:turbo-shmup-button-4-label')}
											name="pinShmupBtn4"
											className="form-control-sm"
											groupClassName="col-sm-3 mb-3"
											value={values.pinShmupBtn4}
											error={errors.pinShmupBtn4}
											isInvalid={errors.pinShmupBtn4}
											onChange={handleChange}
											min={-1}
											max={29}
										/>
									</Row>
									<Row className="mb-3">
										<FormSelect
											label={t('AddonsConfig:turbo-shmup-button-mask-1-label')}
											name="shmupBtnMask1"
											className="form-select-sm"
											groupClassName="col-sm-3 mb-3"
											value={values.shmupBtnMask1}
											error={errors.shmupBtnMask1}
											isInvalid={errors.shmupBtnMask1}
											onChange={handleChange}
										>
											{TURBO_MASKS.map((o, i) => <option key={`shmupBtnMask1-option-${i}`} value={o.value}>{o.label}</option>)}
										</FormSelect>
										<FormSelect
											label={t('AddonsConfig:turbo-shmup-button-mask-2-label')}
											name="shmupBtnMask2"
											className="form-select-sm"
											groupClassName="col-sm-3 mb-3"
											value={values.shmupBtnMask2}
											error={errors.shmupBtnMask2}
											isInvalid={errors.shmupBtnMask2}
											onChange={handleChange}
										>
											{TURBO_MASKS.map((o, i) => <option key={`shmupBtnMask2-option-${i}`} value={o.value}>{o.label}</option>)}
										</FormSelect>
										<FormSelect
											label={t('AddonsConfig:turbo-shmup-button-mask-3-label')}
											name="shmupBtnMask3"
											className="form-select-sm"
											groupClassName="col-sm-3 mb-3"
											value={values.shmupBtnMask3}
											error={errors.shmupBtnMask3}
											isInvalid={errors.shmupBtnMask3}
											onChange={handleChange}
										>
											{TURBO_MASKS.map((o, i) => <option key={`shmupBtnMask3-option-${i}`} value={o.value}>{o.label}</option>)}
										</FormSelect>
										<FormSelect
											label={t('AddonsConfig:turbo-shmup-button-mask-4-label')}
											name="shmupBtnMask4"
											className="form-select-sm"
											groupClassName="col-sm-3 mb-3"
											value={values.shmupBtnMask4}
											error={errors.shmupBtnMask4}
											isInvalid={errors.shmupBtnMask4}
											onChange={handleChange}
										>
											{TURBO_MASKS.map((o, i) => <option key={`shmupBtnMask4-option-${i}`} value={o.value}>{o.label}</option>)}
										</FormSelect>
									</Row>
									<FormSelect
										label={t('AddonsConfig:turbo-shmup-mix-mode-label')}
										name="shmupMixMode"
										className="form-select-sm"
										groupClassName="col-sm-3 mb-3"
										value={values.shmupMixMode}
										error={errors.shmupMixMode}
										isInvalid={errors.shmupMixMode}
										onChange={handleChange}
									>
										{SHMUP_MIXED_MODES.map((o, i) => <option key={`button-shmupMixedMode-option-${i}`} value={o.value}>{o.label}</option>)}
									</FormSelect>
								</div>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="TurboInputButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.TurboInputEnabled)}
							onChange={(e) => {handleCheckbox("TurboInputEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:joystick-selection-slider-header-text')}>
						<div
							id="JSliderInputOptions"
							hidden={!values.JSliderInputEnabled}>
							<Row className="mb-3">
								<FormControl type="number"
									label={t('AddonsConfig:joystick-selection-slider-ls-pin-label')}
									name="sliderLSPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.sliderLSPin}
									error={errors.sliderLSPin}
									isInvalid={errors.sliderLSPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl type="number"
									label={t('AddonsConfig:joystick-selection-slider-rs-pin-label')}
									name="sliderRSPin"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.sliderRSPin}
									error={errors.sliderRSPin}
									isInvalid={errors.sliderRSPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="JSliderInputButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.JSliderInputEnabled)}
							onChange={(e) => {handleCheckbox("JSliderInputEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:input-reverse-header-text')}>
						<div
							id="ReverseInputOptions"
							hidden={!values.ReverseInputEnabled}>
							<Row className="mb-3">
								<FormControl type="number"
									label={t('AddonsConfig:input-reverse-button-pin-label')}
									name="reversePin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.reversePin}
									error={errors.reversePin}
									isInvalid={errors.reversePin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl type="number"
									label="Reverse Input Pin LED"
									name={t('AddonsConfig:input-reverse-led-pin-label')}
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.reversePinLED}
									error={errors.reversePinLED}
									isInvalid={errors.reversePinLED}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
							</Row>
							<Row className="mb-3">
								<FormSelect
									label={t('AddonsConfig:input-reverse-action-up-label')}
									name="reverseActionUp"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.reverseActionUp}
									error={errors.reverseActionUp}
									isInvalid={errors.reverseActionUp}
									onChange={handleChange}
								>
									{translatedReverseAction.map((o, i) => <option key={`reverseActionUp-option-${i}`} value={o.value}>{o.label}</option>)}
								</FormSelect>
								<FormSelect
									label={t('AddonsConfig:input-reverse-action-down-label')}
									name="reverseActionDown"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.reverseActionDown}
									error={errors.reverseActionDown}
									isInvalid={errors.reverseActionDown}
									onChange={handleChange}
								>
									{translatedReverseAction.map((o, i) => <option key={`reverseActionDown-option-${i}`} value={o.value}>{o.label}</option>)}
								</FormSelect>
								<FormSelect
									label={t('AddonsConfig:input-reverse-action-left-label')}
									name="reverseActionLeft"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.reverseActionLeft}
									error={errors.reverseActionLeft}
									isInvalid={errors.reverseActionLeft}
									onChange={handleChange}
								>
									{translatedReverseAction.map((o, i) => <option key={`reverseActionLeft-option-${i}`} value={o.value}>{o.label}</option>)}
								</FormSelect>
								<FormSelect
									label={t('AddonsConfig:input-reverse-action-right-label')}
									name="reverseActionRight"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.reverseActionRight}
									error={errors.reverseActionRight}
									isInvalid={errors.reverseActionRight}
									onChange={handleChange}
								>
									{translatedReverseAction.map((o, i) => <option key={`reverseActionRight-option-${i}`} value={o.value}>{o.label}</option>)}
								</FormSelect>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="ReverseInputButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.ReverseInputEnabled)}
							onChange={(e) => {handleCheckbox("ReverseInputEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:i2c-analog-ads1219-header-text')}>
						<div
							id="I2CAnalog1219InputOptions"
							hidden={!values.I2CAnalog1219InputEnabled}>
							<Row className="mb-3">
								<FormControl type="number"
									label={t('AddonsConfig:i2c-analog-ads1219-sda-pin-label')}
									name="i2cAnalog1219SDAPin"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.i2cAnalog1219SDAPin}
									error={errors.i2cAnalog1219SDAPin}
									isInvalid={errors.i2cAnalog1219SDAPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl type="number"
									label={t('AddonsConfig:i2c-analog-ads1219-scl-pin-label')}
									name="i2cAnalog1219SCLPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.i2cAnalog1219SCLPin}
									error={errors.i2cAnalog1219SCLPin}
									isInvalid={errors.i2cAnalog1219SCLPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormSelect
									label={t('AddonsConfig:i2c-analog-ads1219-block-label')}
									name="i2cAnalog1219Block"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.i2cAnalog1219Block}
									error={errors.i2cAnalog1219Block}
									isInvalid={errors.i2cAnalog1219Block}
									onChange={handleChange}
								>
									{I2C_BLOCKS.map((o, i) => <option key={`i2cBlock-option-${i}`} value={o.value}>{o.label}</option>)}
								</FormSelect>
								<FormControl
									label={t('AddonsConfig:i2c-analog-ads1219-speed-label')}
									name="i2cAnalog1219Speed"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.i2cAnalog1219Speed}
									error={errors.i2cAnalog1219Speed}
									isInvalid={errors.i2cAnalog1219Speed}
									onChange={handleChange}
									min={100000}
								/>
							</Row>
							<Row className="mb-3">
								<FormControl
									label={t('AddonsConfig:i2c-analog-ads1219-address-label')}
									name="i2cAnalog1219Address"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.i2cAnalog1219Address}
									error={errors.i2cAnalog1219Address}
									isInvalid={errors.i2cAnalog1219Address}
									onChange={handleChange}
									maxLength={4}
								/>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="I2CAnalog1219InputButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.I2CAnalog1219InputEnabled)}
							onChange={(e) => {handleCheckbox("I2CAnalog1219InputEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:dual-directional-input-header-text')}>
						<div
							id="DualDirectionalInputOptions"
							hidden={!values.DualDirectionalInputEnabled}>
							<Row className="mb-3">
								<FormControl type="number"
									label={t('AddonsConfig:dual-directional-input-up-pin-label')}
									name="dualDirUpPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.dualDirUpPin}
									error={errors.dualDirUpPin}
									isInvalid={errors.dualDirUpPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl type="number"
									label={t('AddonsConfig:dual-directional-input-down-pin-label')}
									name="dualDirDownPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.dualDirDownPin}
									error={errors.dualDirDownPin}
									isInvalid={errors.dualDirDownPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl type="number"
									label={t('AddonsConfig:dual-directional-input-left-pin-label')}
									name="dualDirLeftPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.dualDirLeftPin}
									error={errors.dualDirLeftPin}
									isInvalid={errors.dualDirLeftPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl type="number"
									label={t('AddonsConfig:dual-directional-input-right-pin-label')}
									name="dualDirRightPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.dualDirRightPin}
									error={errors.dualDirRightPin}
									isInvalid={errors.dualDirRightPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
							</Row>
							<Row className="mb-3">
								<FormSelect
									label={t('AddonsConfig:dual-directional-input-dpad-mode-label')}
									name="dualDirDpadMode"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.dualDirDpadMode}
									error={errors.dualDirDpadMode}
									isInvalid={errors.dualDirDpadMode}
									onChange={handleChange}
								>
									{DUAL_STICK_MODES.map((o, i) => <option key={`button-dualDirDpadMode-option-${i}`} value={o.value}>{o.label}</option>)}
								</FormSelect>

							<FormSelect
								label={t('AddonsConfig:dual-directional-input-combine-mode-label')}
								name="dualDirCombineMode"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.dualDirCombineMode}
								error={errors.dualDirCombineMode}
								isInvalid={errors.dualDirCombineMode}
								onChange={handleChange}
							>
								{DUAL_COMBINE_MODES.map((o, i) => <option key={`button-dualDirCombineMode-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
							<FormCheck
								label={t('AddonsConfig:dual-directional-input-four-way-joystick-mode-label')}
								type="switch"
								id="DualDirFourWayMode"
								className="col-sm-3 ms-2"
								isInvalid={false}
								checked={Boolean(values.dualDirFourWayMode)}
								onChange={(e) => {handleCheckbox("dualDirFourWayMode", values); handleChange(e);}}
							/>
						</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="DualDirectionalInputButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.DualDirectionalInputEnabled)}
							onChange={(e) => {handleCheckbox("DualDirectionalInputEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:buzzer-speaker-header-text')}>
						<div
							id="BuzzerSpeakerAddonOptions"
							hidden={!values.BuzzerSpeakerAddonEnabled}>
							<Row className="mb-3">
								<FormControl type="number"
									label={t('AddonsConfig:buzzer-speaker-pin-label')}
									name="buzzerPin"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.buzzerPin}
									error={errors.buzzerPin}
									isInvalid={errors.buzzerPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl type="number"
									label={t('AddonsConfig:buzzer-speaker-volume-label')}
									name="buzzerVolume"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.buzzerVolume}
									error={errors.buzzerVolume}
									isInvalid={errors.buzzerVolume}
									onChange={handleChange}
									min={0}
									max={100}
								/>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="BuzzerSpeakerAddonButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.BuzzerSpeakerAddonEnabled)}
							onChange={(e) => {handleCheckbox("BuzzerSpeakerAddonEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:extra-button-header-text')}>
						<div
							id="ExtraButtonAddonOptions"
							hidden={!values.ExtraButtonAddonEnabled}>
							<Row className="mb-3">
								<FormControl type="number"
									label={t('AddonsConfig:extra-button-pin-label')}
									name="extraButtonPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.extraButtonPin}
									error={errors.extraButtonPin}
									isInvalid={errors.extraButtonPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormSelect
									label={t('AddonsConfig:extra-button-map-label')}
									name="extraButtonMap"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.extraButtonMap}
									error={errors.extraButtonMap}
									isInvalid={errors.extraButtonMap}
									onChange={handleChange}
								>
									{BUTTON_MASKS.map((o, i) => <option key={`extraButtonMap-option-${i}`} value={o.value}>{o.label}</option>)}
								</FormSelect>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="ExtraButtonAddonButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.ExtraButtonAddonEnabled)}
							onChange={(e) => { handleCheckbox("ExtraButtonAddonEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:player-number-header-text')}>
						<div
							id="PlayerNumAddonOptions"
							hidden={!values.PlayerNumAddonEnabled}>
							<p><strong>{t('AddonsConfig:player-number-sub-header-text')}</strong></p>
							<Row className="mb-3">
								<FormControl type="number"
									label={t('AddonsConfig:player-number-label')}
									name="playerNumber"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.playerNumber}
									error={errors.playerNumber}
									isInvalid={errors.playerNumber}
									onChange={handleChange}
									min={1}
									max={4}
								/>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="PlayerNumAddonButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.PlayerNumAddonEnabled)}
							onChange={(e) => {handleCheckbox("PlayerNumAddonEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:socd-cleaning-mode-selection-slider-header-text')}>
						<div
							id="SliderSOCDInputOptions"
							hidden={!values.SliderSOCDInputEnabled}>
							<Row className="mb-3">
								<p>{t('AddonsConfig:socd-cleaning-mode-selection-slider-sub-header-text')}</p>
								<FormSelect
									label={t('AddonsConfig:socd-cleaning-mode-selection-slider-mode-default-label')}
									name="sliderSOCDModeDefault"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.sliderSOCDModeDefault}
									error={errors.sliderSOCDModeDefault}
									isInvalid={errors.sliderSOCDModeDefault}
									onChange={handleChange}
								>
									{SOCD_MODES.map((o, i) => <option key={`sliderSOCDModeDefault-option-${i}`} value={o.value}>{o.label}</option>)}
								</FormSelect>
								<FormSelect
									label={t('AddonsConfig:socd-cleaning-mode-selection-slider-mode-one-label')}
									name="sliderSOCDModeOne"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.sliderSOCDModeOne}
									error={errors.sliderSOCDModeOne}
									isInvalid={errors.sliderSOCDModeOne}
									onChange={handleChange}
								>
									{SOCD_MODES.map((o, i) => <option key={`sliderSOCDModeOne-option-${i}`} value={o.value}>{o.label}</option>)}
								</FormSelect>
								<FormControl type="number"
									label={t('AddonsConfig:socd-cleaning-mode-selection-slider-pin-one-label')}
									name="sliderSOCDPinOne"
									className="form-select-sm"
									groupClassName="col-sm-1 mb-3"
									value={values.sliderSOCDPinOne}
									error={errors.sliderSOCDPinOne}
									isInvalid={errors.sliderSOCDPinOne}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormSelect
									label={t('AddonsConfig:socd-cleaning-mode-selection-slider-mode-two-label')}
									name="sliderSOCDModeTwo"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.sliderSOCDModeTwo}
									error={errors.sliderSOCDModeTwo}
									isInvalid={errors.sliderSOCDModeTwo}
									onChange={handleChange}
								>
									{SOCD_MODES.map((o, i) => <option key={`sliderSOCDModeTwo-option-${i}`} value={o.value}>{o.label}</option>)}
								</FormSelect>
								<FormControl type="number"
									label={t('AddonsConfig:socd-cleaning-mode-selection-slider-pin-two-label')}
									name="sliderSOCDPinTwo"
									className="form-control-sm"
									groupClassName="col-sm-1 mb-3"
									value={values.sliderSOCDPinTwo}
									error={errors.sliderSOCDPinTwo}
									isInvalid={errors.sliderSOCDPinTwo}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="SliderSOCDInputButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.SliderSOCDInputEnabled)}
							onChange={(e) => {handleCheckbox("SliderSOCDInputEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:ps4-mode-header-text')}>
						<div
							id="PS4ModeOptions"
							hidden={!values.PS4ModeAddonEnabled}>
							<Row>
								<Trans ns="AddonsConfig" i18nKey="ps4-mode-sub-header-text">
									<h2>!!!! DISCLAIMER: GP2040-CE WILL NEVER SUPPLY THESE FILES !!!!</h2>
									<p>Please upload the 3 required files and click the &quot;Verify & Save&quot; button to use PS4 Mode.</p>
								</Trans>
							</Row>
							<Row className="mb-3">
								<div className="col-sm-3 mb-3">
									{t('AddonsConfig:ps4-mode-private-key-label')}:
									<input type="file" id="ps4key-input" accept="*/*" />
								</div>
								<div className="col-sm-3 mb-3">
									{t('AddonsConfig:ps4-mode-serial-number-label')}:
									<input type="file" id="ps4serial-input" accept="*/*" />
								</div>
								<div className="col-sm-3 mb-3">
									{t('AddonsConfig:ps4-mode-signature-label')}:
									<input type="file" id="ps4signature-input" accept="*/*" />
								</div>
							</Row>
							<Row className="mb-3">
								<div className="col-sm-3 mb-3">
									<Button type="button" onClick={verifyAndSavePS4}>{t('Common:button-verify-save-label')}</Button>
								</div>
							</Row>
							<Row className="mb-3">
								<div className="col-sm-3 mb-3">
									<span id="ps4alert"></span>
								</div>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="PS4ModeAddonEnabledButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.PS4ModeAddonEnabled)}
							onChange={(e) => {handleCheckbox("PS4ModeAddonEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:wii-extension-header-text')}>
						<div
							id="WiiExtensionAddonOptions"
							hidden={!values.WiiExtensionAddonEnabled}>
							<Row>
								<Trans ns="AddonsConfig" i18nKey="wii-extension-sub-header-text">
									<p>Note: If the Display is enabled at the same time, this Addon will be disabled.</p>
									<h3>Currently Supported Controllers</h3>
									<p>Classic/Classic Pro: Both Analogs and D-Pad Supported. B = B1, A = B2, Y = B3, X = B4, L = L1, ZL = L2, R = R1, ZR = R2, Minus = S1, Plus = S2, Home = A1</p>
									<p>Nunchuck: Analog Stick Supported. C = B1, Z = B2</p>
									<p>Guitar Hero Guitar: Analog Stick Supported. Green = B1, Red = B2, Blue = B3, Yellow = B4, Orange = L1, Strum Up = Up, Strum Down = Down, Minus = S1, Plus = S2</p>
								</Trans>
							</Row>
							<Row className="mb-3">
								<FormControl type="number"
									label={t('AddonsConfig:wii-extension-sda-pin-label')}
									name="wiiExtensionSDAPin"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.wiiExtensionSDAPin}
									error={errors.wiiExtensionSDAPin}
									isInvalid={errors.wiiExtensionSDAPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl type="number"
									label={t('AddonsConfig:wii-extension-scl-pin-label')}
									name="wiiExtensionSCLPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.wiiExtensionSCLPin}
									error={errors.wiiExtensionSCLPin}
									isInvalid={errors.wiiExtensionSCLPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormSelect
									label={t('AddonsConfig:wii-extension-block-label')}
									name="wiiExtensionBlock"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.wiiExtensionBlock}
									error={errors.wiiExtensionBlock}
									isInvalid={errors.wiiExtensionBlock}
									onChange={handleChange}
								>
									{I2C_BLOCKS.map((o, i) => <option key={`wiiExtensionI2cBlock-option-${i}`} value={o.value}>{o.label}</option>)}
								</FormSelect>
								<FormControl
									label={t('AddonsConfig:wii-extension-speed-label')}
									name="wiiExtensionSpeed"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.wiiExtensionSpeed}
									error={errors.wiiExtensionSpeed}
									isInvalid={errors.wiiExtensionSpeed}
									onChange={handleChange}
									min={100000}
								/>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="WiiExtensionButton"
							reverse="true"
							error={undefined}
							isInvalid={false}
							checked={Boolean(values.WiiExtensionAddonEnabled)}
							onChange={(e) => {handleCheckbox("WiiExtensionAddonEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:snes-extension-header-text')}>
						<div
							id="SNESpadAddonOptions"
							hidden={!values.SNESpadAddonEnabled}>
							<Row>
								<Trans ns="AddonsConfig" i18nKey="snes-extension-sub-header-text">
									<p>Note: If the Display is enabled at the same time, this Addon will be disabled.</p>
									<h3>Currently Supported Controllers</h3>
									<p>SNES pad: D-Pad Supported. B = B1, A = B2, Y = B3, X = B4, L = L1, R = R1, Select = S1, Start = S2</p>
									<p>SNES mouse: Analog Stick Supported. Left Click = B1, Right Click = B2</p>
									<p>NES: D-Pad Supported. B = B1, A = B2, Select = S1, Start = S2</p>
								</Trans>
							</Row>
							<Row className="mb-3">
								<FormControl type="number"
									label={t('AddonsConfig:snes-extension-clock-pin-label')}
									name="snesPadClockPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.snesPadClockPin}
									error={errors.snesPadClockPin}
									isInvalid={errors.snesPadClockPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl type="number"
									label={t('AddonsConfig:snes-extension-latch-pin-label')}
									name="snesPadLatchPin"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.snesPadLatchPin}
									error={errors.snesPadLatchPin}
									isInvalid={errors.snesPadLatchPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl type="number"
									label={t('AddonsConfig:snes-extension-data-pin-label')}
									name="snesPadDataPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.snesPadDataPin}
									error={errors.snesPadDataPin}
									isInvalid={errors.snesPadDataPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="SNESpadButton"
							reverse="true"
							error={undefined}
							isInvalid={false}
							checked={Boolean(values.SNESpadAddonEnabled)}
							onChange={(e) => {handleCheckbox("SNESpadAddonEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:focus-mode-header-text')}>
						<div
							id="FocusModeAddonOptions"
							hidden={!values.FocusModeAddonEnabled}>
							<Row className="mb-3">
								<FormControl type="number"
									label={t('AddonsConfig:focus-mode-pin-label')}
									name="focusModePin"
									className="form-select-sm col-3"
									groupClassName="col-sm-3 mb-3"
									value={values.focusModePin}
									error={errors.focusModePin}
									isInvalid={errors.focusModePin}
									onChange={handleChange}
									min={-1}
									max={29} />
								<div className="col-sm-3">
									<FormCheck
										label={t('Common:lock-oled-screen')}
										className="form-check-sm"
										type="switch"
										reverse
										id="FocusModeAddonOLEDButton"
										isInvalid={false}
										checked={Boolean(values.focusModeOledLockEnabled)}
										onChange={(e) => { handleCheckbox("focusModeOledLockEnabled", values); handleChange(e); }} />
								</div>
								<div className="col-sm-3">
									<FormCheck
										label={t('Common:lock-rgb-led')}
										className="form-check-sm"
										type="switch"
										reverse
										id="FocusModeAddonButton"
										isInvalid={false}
										checked={Boolean(values.focusModeRgbLockEnabled)}
										onChange={(e) => { handleCheckbox("focusModeRgbLockEnabled", values); handleChange(e); }} />
								</div>
								<div className="col-sm-3">
									<FormCheck
										label={t('Common:lock-buttons')}
										className="form-check-sm"
										type="switch"
										reverse
										id="FocusModeAddonButton"
										isInvalid={false}
										checked={Boolean(values.focusModeButtonLockEnabled)}
										onChange={(e) => { handleCheckbox("focusModeButtonLockEnabled", values); handleChange(e); }}
									/>
								</div>
								<Row>
									{BUTTON_MASKS.map(mask => (values.focusModeButtonLockMask & mask.value) ? <FormSelect
										name="focusModeButtonLockMask"
										className="form-select-sm"
										groupClassName="col-sm-3 mb-3"
										value={values.focusModeButtonLockMask & mask.value}
										error={errors.focusModeButtonLockMask}
										isInvalid={errors.focusModeButtonLockMask}
										onChange={(e) => { setFieldValue("focusModeButtonLockMask", values.focusModeButtonLockMask ^ mask.value | e.target.value); }}
									>
										{BUTTON_MASKS.map((o, i) => <option key={`focusModeButtonLockMask-option-${i}`} value={o.value}>{o.label}</option>)}
									</FormSelect> : <></>)}
									<FormSelect
										name="focusModeButtonLockMask"
										className="form-select-sm"
										groupClassName="col-sm-3 mb-3"
										value={0}
										error={errors.focusModeButtonLockMask}
										isInvalid={errors.focusModeButtonLockMask}
										onChange={(e) => { setFieldValue("focusModeButtonLockMask", values.focusModeButtonLockMask | e.target.value); }}
									>
										{BUTTON_MASKS.map((o, i) => <option key={`focusModeButtonLockMask-option-${i}`} value={o.value}>{o.label}</option>)}
									</FormSelect>
								</Row>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="FocusModeAddonButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.FocusModeAddonEnabled)}
							onChange={(e) => { handleCheckbox("FocusModeAddonEnabled", values); handleChange(e);}}
						/>
					</Section>
					<Section title={t('AddonsConfig:keyboard-host-header-text')}>
						<div
							id="KeyboardHostAddonOptions"
							hidden={!values.KeyboardHostAddonEnabled}>
							<Row className="mb-3">
								<p>{t('AddonsConfig:keyboard-host-sub-header-text')}</p>
								<FormControl type="number"
									label={t('AddonsConfig:keyboard-host-d-plus-label')}
									name="keyboardHostPinDplus"
									className="form-select-sm"
									groupClassName="col-sm-1 mb-3"
									value={values.keyboardHostPinDplus}
									error={errors.keyboardHostPinDplus}
									isInvalid={errors.keyboardHostPinDplus}
									onChange={handleChange}
									min={-1}
									max={28}
								/>
								<FormControl type="number"
									label={t('AddonsConfig:keyboard-host-d-minus-label')}
									disabled
									className="form-select-sm"
									groupClassName="col-sm-1 mb-3"
									value={values.keyboardHostPinDplus === -1 ? -1 : values.keyboardHostPinDplus + 1}
								/>
							</Row>
							<Row className="mb-3">
								<p>{t('KeyboardMapping:sub-header-text')}</p>
								<KeyboardMapper buttonLabels={buttonLabels}
									handleKeyChange={handleKeyChange(values, setFieldValue)}
									validated={validated}
									getKeyMappingForButton={getKeyMappingForButton(values)} />
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="KeyboardHostAddonButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.KeyboardHostAddonEnabled)}
							onChange={(e) => { handleCheckbox("KeyboardHostAddonEnabled", values); handleChange(e);}}
						/>
					</Section>
					<div className="mt-3">
						<Button type="submit" id="save">{t('Common:button-save-label')}</Button>
						{saveMessage ? <span className="alert">{saveMessage}</span> : null}
					</div>
					<FormContext setStoredData={setStoredData}/>
				</Form>
			)}
		</Formik>
	);
}
