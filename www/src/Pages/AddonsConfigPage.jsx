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
import get from 'lodash/get';
import set from 'lodash/set';
import isNil from 'lodash/isNil';

const I2C_BLOCKS = [
	{ label: 'i2c0', value: 0 },
	{ label: 'i2c1', value: 1 },
];

const ON_BOARD_LED_MODES = [
	{ label: 'Off', value: 0 },
	{ label: 'Mode Indicator', value: 1 },
	{ label: 'Input Test', value: 2 },
	{ label: 'PS4/5 Authentication', value: 3},
];

const DUAL_STICK_MODES = [
	{ label: 'D-Pad', value: 0 },
	{ label: 'Left Analog', value: 1 },
	{ label: 'Right Analog', value: 2 },
];

const ANALOG_STICK_MODES = [
	{ label: 'Left Analog', value: 1 },
	{ label: 'Right Analog', value: 2 },
];

const DUAL_COMBINE_MODES = [
	{ label: 'Mixed', value: 0 },
	{ label: 'Gamepad', value: 1 },
	{ label: 'Dual Directional', value: 2 },
	{ label: 'None', value: 3 },
];

const SHMUP_MIXED_MODES = [
	{ label: 'Turbo Priority', value: 0 },
	{ label: 'Charge Priority', value: 1 },
];

const TILT_SOCD_MODES = [
	{ label: 'Up Priority', value: 0 },
	{ label: 'Neutral', value: 1 },
	{ label: 'Last Win', value: 2 },
];

const INVERT_MODES = [
	{ label: 'None', value: 0 },
	{ label: 'X Axis', value: 1 },
	{ label: 'Y Axis', value: 2 },
	{ label: 'X/Y Axis', value: 3 },
];

const ANALOG_PINS = [26, 27, 28, 29];

const BUTTON_MASKS = [
	{ label: 'None', value: 0 },
	{ label: 'B1', value: 1 << 0 },
	{ label: 'B2', value: 1 << 1 },
	{ label: 'B3', value: 1 << 2 },
	{ label: 'B4', value: 1 << 3 },
	{ label: 'L1', value: 1 << 4 },
	{ label: 'R1', value: 1 << 5 },
	{ label: 'L2', value: 1 << 6 },
	{ label: 'R2', value: 1 << 7 },
	{ label: 'S1', value: 1 << 8 },
	{ label: 'S2', value: 1 << 9 },
	{ label: 'L3', value: 1 << 10 },
	{ label: 'R3', value: 1 << 11 },
	{ label: 'A1', value: 1 << 12 },
	{ label: 'A2', value: 1 << 13 },
	{ label: 'Up', value: 1 << 16 },
	{ label: 'Down', value: 1 << 17 },
	{ label: 'Left', value: 1 << 18 },
	{ label: 'Right', value: 1 << 19 },
];

const TURBO_MASKS = [
	{ label: 'None', value: 0 },
	{ label: 'B1', value: 1 << 0 },
	{ label: 'B2', value: 1 << 1 },
	{ label: 'B3', value: 1 << 2 },
	{ label: 'B4', value: 1 << 3 },
	{ label: 'L1', value: 1 << 4 },
	{ label: 'R1', value: 1 << 5 },
	{ label: 'L2', value: 1 << 6 },
	{ label: 'R2', value: 1 << 7 },
	{ label: 'L3', value: 1 << 10 },
	{ label: 'R3', value: 1 << 11 },
];

const REVERSE_ACTION = [
	{ label: 'Disable', value: 0 },
	{ label: 'Enable', value: 1 },
	{ label: 'Neutral', value: 2 },
];

const verifyAndSavePS4 = async () => {
	let PS4Key = document.getElementById('ps4key-input');
	let PS4Serial = document.getElementById('ps4serial-input');
	let PS4Signature = document.getElementById('ps4signature-input');

	const loadFile = (file, text) => {
		return new Promise((resolve, reject) => {
			const keyReader = new FileReader();
			keyReader.onloadend = (e) => {
				if (!isNil(keyReader.error)) {
					reject(keyReader.error);
				} else {
					resolve(keyReader.result);
				}
			};
			if (text) {
				keyReader.readAsText(file);
			} else {
				keyReader.readAsBinaryString(file);
			}
		});
	};

	function int2mbedmpi(num) {
		var out = [];
		var mask = BigInt('4294967295');
		var zero = BigInt('0');
		while (num !== zero) {
			out.push((num & mask).toString(16).padStart(8, '0'));
			num = num >> BigInt(32);
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
		var arr = new Uint8Array(mpi.length * 4);
		var cnt = 0;
		for (let i = 0; i < mpi.length; i++) {
			let bytes = hexToBytes(mpi[i]);
			for (let j = 4; j > 0; j--) {
				//arr[cnt] = bytes[j];
				// TEST: re-order from LSB to MSB
				arr[cnt] = bytes[j - 1];
				cnt++;
			}
		}

		return btoa(String.fromCharCode.apply(null, arr));
	}

	try {
		const [pem, signature, serialFileContent] = await Promise.all([
			loadFile(PS4Key.files[0], true),
			loadFile(PS4Signature.files[0], false),
			loadFile(PS4Serial.files[0], true),
		]);

		// Make sure our signature is 256 bytes
		const serialNoPadding = serialFileContent.trimRight();
		if (signature.length !== 256 || serialNoPadding.length !== 16) {
			throw new Error('Signature or serial is invalid');
		}
		const serial = serialNoPadding.padStart(32, '0'); // Add our padding

		const key = new JSEncrypt();
		key.setPrivateKey(pem);
		const bytes = new Uint8Array(256);
		for (let i = 0; i < 256; i++) {
			bytes[i] = Math.random() * 255;
		}
		const hashed = CryptoJS.SHA256(bytes);
		const signNonce = key.sign(hashed, CryptoJS.SHA256, 'sha256');

		if (signNonce === false) {
			throw new Error('Bad Private Key');
		}

		// Private key worked!

		// Translate these to BigInteger
		var N = BigInt(String(key.key.n));
		var E = BigInt(String(key.key.e));
		var P = BigInt(String(key.key.p));
		var Q = BigInt(String(key.key.q));

		let serialBin = hexToBytes(serial);

		let success = await WebApi.setPS4Options({
			N: mbedmpi2b64(int2mbedmpi(N)),
			E: mbedmpi2b64(int2mbedmpi(E)),
			P: mbedmpi2b64(int2mbedmpi(P)),
			Q: mbedmpi2b64(int2mbedmpi(Q)),
			serial: btoa(String.fromCharCode(...new Uint8Array(serialBin))),
			signature: btoa(signature),
		});

		if (success) {
			document.getElementById('ps4alert').textContent =
				'Verified and Saved PS4 Mode! Reboot to take effect';
			document.getElementById('save').click();
		} else {
			throw Error('ERROR: Failed to upload the key to the board');
		}
	} catch (e) {
		document.getElementById('ps4alert').textContent =
			'ERROR: Could not verify required files: ${e}';
	}
};

const DPAD_MODES = [
	{ label: 'Digital', value: 0 },
	{ label: 'Left Analog', value: 1 },
	{ label: 'Right Analog', value: 2 },
];

const SOCD_MODES = [
	{ label: 'Up Priority', value: 0 },
	{ label: 'Neutral', value: 1 },
	{ label: 'Last Win', value: 2 },
	{ label: 'First Win', value: 3 },
	{ label: 'SOCD Cleaning Off', value: 4 },
];

const schema = yup.object().shape({
	I2CAnalog1219InputEnabled: yup.number().label('I2C Analog1219 Input Enabled'),
	i2cAnalog1219SDAPin: yup
		.number()
		.label('I2C Analog1219 SDA Pin')
		.validatePinWhenValue('I2CAnalog1219InputEnabled'),
	i2cAnalog1219SCLPin: yup
		.number()
		.label('I2C Analog1219 SCL Pin')
		.validatePinWhenValue('I2CAnalog1219InputEnabled'),
	i2cAnalog1219Block: yup
		.number()
		.label('I2C Analog1219 Block')
		.validateSelectionWhenValue('I2CAnalog1219InputEnabled', I2C_BLOCKS),
	i2cAnalog1219Speed: yup
		.number()
		.label('I2C Analog1219 Speed')
		.validateNumberWhenValue('I2CAnalog1219InputEnabled'),
	i2cAnalog1219Address: yup
		.number()
		.label('I2C Analog1219 Address')
		.validateNumberWhenValue('I2CAnalog1219InputEnabled'),

	AnalogInputEnabled: yup.number().required().label('Analog Input Enabled'),
	analogAdc1PinX: yup
		.number()
		.label('Analog Stick 1 Pin X')
		.validatePinWhenValue('AnalogInputEnabled'),
	analogAdc1PinY: yup
		.number()
		.label('Analog Stick 1 Pin Y')
		.validatePinWhenValue('AnalogInputEnabled'),
	analogAdc1Mode: yup
		.number()
		.label('Analog Stick 1 Mode')
		.validateSelectionWhenValue('AnalogInputEnabled', ANALOG_STICK_MODES),
	analogAdc1Invert: yup
		.number()
		.label('Analog Stick 1 Invert')
		.validateSelectionWhenValue('AnalogInputEnabled', INVERT_MODES),
	analogAdc2PinX: yup
		.number()
		.label('Analog Stick 2 Pin X')
		.validatePinWhenValue('AnalogInputEnabled'),
	analogAdc2PinY: yup
		.number()
		.label('Analog Stick 2 Pin Y')
		.validatePinWhenValue('AnalogInputEnabled'),
	analogAdc2Mode: yup
		.number()
		.label('Analog Stick 2 Mode')
		.validateSelectionWhenValue('AnalogInputEnabled', ANALOG_STICK_MODES),
	analogAdc2Invert: yup
		.number()
		.label('Analog Stick 2 Invert')
		.validateSelectionWhenValue('AnalogInputEnabled', INVERT_MODES),

	forced_circularity: yup
		.number()
		.label('Force Circularity')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 1),
	analog_deadzone: yup
		.number()
		.label('Deadzone Size (%)')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 100),
	auto_calibrate: yup
		.number()
		.label('Auto Calibration')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 1),

	BoardLedAddonEnabled: yup
		.number()
		.required()
		.label('Board LED Add-On Enabled'),
	onBoardLedMode: yup
		.number()
		.label('On-Board LED Mode')
		.validateSelectionWhenValue('BoardLedAddonEnabled', ON_BOARD_LED_MODES),

	BootselButtonAddonEnabled: yup
		.number()
		.required()
		.label('Boot Select Button Add-On Enabled'),
	bootselButtonMap: yup
		.number()
		.label('BOOTSEL Button Map')
		.validateSelectionWhenValue('BootselButtonAddonEnabled', BUTTON_MASKS),

	FocusModeAddonEnabled: yup
		.number()
		.required()
		.label('Focus Mode Add-On Enabled'),
	focusModePin: yup
		.number()
		.label('Focus Mode Pin')
		.validatePinWhenValue('FocusModeAddonEnabled'),
	focusModeButtonLockEnabled: yup
		.number()
		.label('Focus Mode Button Lock Enabled')
		.validateRangeWhenValue('FocusModeAddonEnabled', 0, 1),
	focusModeOledLockEnabled: yup
		.number()
		.label('Focus Mode OLED Lock Enabled')
		.validateRangeWhenValue('FocusModeAddonEnabled', 0, 1),
	focusModeRgbLockEnabled: yup
		.number()
		.label('Focus Mode RGB Lock Enabled')
		.validateRangeWhenValue('FocusModeAddonEnabled', 0, 1),
	focusModeButtonLockMask: yup
		.number()
		.label('Focus Mode Button Lock Map')
		.validateRangeWhenValue('FocusModeAddonEnabled', 0, (1 << 20) - 1),

	BuzzerSpeakerAddonEnabled: yup
		.number()
		.required()
		.label('Buzzer Speaker Add-On Enabled'),
	buzzerPin: yup
		.number()
		.label('Buzzer Pin')
		.validatePinWhenValue('BuzzerSpeakerAddonEnabled'),
	buzzerVolume: yup
		.number()
		.label('Buzzer Volume')
		.validateRangeWhenValue('BuzzerSpeakerAddonEnabled', 0, 100),

	DualDirectionalInputEnabled: yup
		.number()
		.required()
		.label('Dual Directional Input Enabled'),
	dualDirUpPin: yup
		.number()
		.label('Dual Directional Up Pin')
		.validatePinWhenValue('DualDirectionalInputEnabled'),
	dualDirDownPin: yup
		.number()
		.label('Dual Directional Down Pin')
		.validatePinWhenValue('DualDirectionalInputEnabled'),
	dualDirLeftPin: yup
		.number()
		.label('Dual Directional Left Pin')
		.validatePinWhenValue('DualDirectionalInputEnabled'),
	dualDirRightPin: yup
		.number()
		.label('Dual Directional Right Pin')
		.validatePinWhenValue('DualDirectionalInputEnabled'),
	dualDirDpadMode: yup
		.number()
		.label('Dual Stick Mode')
		.validateSelectionWhenValue(
			'DualDirectionalInputEnabled',
			DUAL_STICK_MODES,
		),
	dualDirCombineMode: yup
		.number()
		.label('Dual Combination Mode')
		.validateSelectionWhenValue(
			'DualDirectionalInputEnabled',
			DUAL_COMBINE_MODES,
		),
	dualDirFourWayMode: yup
		.number()
		.label('Dual Directional 4-Way Joystick Mode')
		.validateRangeWhenValue('DualDirectionalInputEnabled', 0, 1),

	TiltInputEnabled: yup.number().required().label('Tilt Input Enabled'),
	tilt1Pin: yup
		.number()
		.label('Tilt 1 Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tilt2Pin: yup
		.number()
		.label('Tilt 2 Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltLeftAnalogUpPin: yup
		.number()
		.label('Tilt Left Analog Up Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltLeftAnalogDownPin: yup
		.number()
		.label('Tilt Left Analog Down Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltLeftAnalogLeftPin: yup
		.number()
		.label('Tilt Left Analog Left Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltLeftAnalogRightPin: yup
		.number()
		.label('Tilt Left Analog Right Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltRightAnalogUpPin: yup
		.number()
		.label('Tilt Right Analog Up Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltRightAnalogDownPin: yup
		.number()
		.label('Tilt Right Analog Down Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltRightAnalogLeftPin: yup
		.number()
		.label('Tilt Right Analog Left Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltRightAnalogRightPin: yup
		.number()
		.label('Tilt Right Analog Right Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltSOCDMode: yup
		.number()
		.label('Tilt SOCE Mode')
		.validateSelectionWhenValue('TiltInputEnabled', SOCD_MODES),

	ExtraButtonAddonEnabled: yup
		.number()
		.required()
		.label('Extra Button Add-On Enabled'),
	extraButtonPin: yup
		.number()
		.label('Extra Button Pin')
		.validatePinWhenValue('ExtraButtonAddonEnabled'),
	extraButtonMap: yup
		.number()
		.label('Extra Button Map')
		.validateSelectionWhenValue('ExtraButtonAddonEnabled', BUTTON_MASKS),

	JSliderInputEnabled: yup.number().required().label('JSlider Input Enabled'),
	sliderPinOne: yup
		.number()
		.label('Slider Pin One')
		.validatePinWhenValue('JSliderInputEnabled'),
	sliderPinTwo: yup
		.number()
		.label('Slider Pin Two')
		.validatePinWhenValue('JSliderInputEnabled'),
	sliderModeZero: yup
		.number()
		.label('Slider Mode Zero')
		.validateSelectionWhenValue('JSliderInputEnabled', DPAD_MODES),
	sliderModeOne: yup
		.number()
		.label('Slider Mode One')
		.validateSelectionWhenValue('JSliderInputEnabled', DPAD_MODES),
	sliderModeTwo: yup
		.number()
		.label('Slider Mode Two')
		.validateSelectionWhenValue('JSliderInputEnabled', DPAD_MODES),

	KeyboardHostAddonEnabled: yup
		.number()
		.required()
		.label('Keyboard Host Add-On Enabled'),
	keyboardHostPinDplus: yup
		.number()
		.label('Keyboard Host D+ Pin')
		.validatePinWhenValue('KeyboardHostAddonEnabled'),
	keyboardHostPin5V: yup
		.number()
		.label('Keyboard Host 5V Power Pin')
		.validatePinWhenValue('KeyboardHostAddonEnabled'),

	PSPassthroughAddonEnabled: yup
		.number()
		.required()
		.label('PS Passthrough Add-On Enabled'),
	psPassthroughPinDplus: yup
		.number()
		.label('PS Passthrough D+ Pin')
		.validatePinWhenValue('PSPassthroughAddonEnabled'),
	psPassthroughPin5V: yup
		.number()
		.label('PS Passthrough 5V Power Pin')
		.validatePinWhenValue('PSPassthroughAddonEnabled'),

	PlayerNumAddonEnabled: yup
		.number()
		.required()
		.label('Player Number Add-On Enabled'),
	playerNumber: yup
		.number()
		.label('Player Number')
		.validateRangeWhenValue('PlayerNumAddonEnabled', 1, 4),

	PS4ModeAddonEnabled: yup.number().required().label('PS4 Mode Add-on Enabled'),

	ReverseInputEnabled: yup.number().required().label('Reverse Input Enabled'),
	reversePin: yup
		.number()
		.label('Reverse Pin')
		.validatePinWhenValue('ReverseInputEnabled'),
	reversePinLED: yup
		.number()
		.label('Reverse Pin LED')
		.validatePinWhenValue('ReverseInputEnabled'),

	SliderSOCDInputEnabled: yup
		.number()
		.required()
		.label('Slider SOCD Input Enabled'),
	sliderSOCDModeOne: yup
		.number()
		.label('SOCD Slider Mode One')
		.validateSelectionWhenValue('SliderSOCDInputEnabled', SOCD_MODES),
	sliderSOCDModeTwo: yup
		.number()
		.label('SOCD Slider Mode Two')
		.validateSelectionWhenValue('SliderSOCDInputEnabled', SOCD_MODES),
	sliderSOCDModeDefault: yup
		.number()
		.label('SOCD Slider Mode Default')
		.validateSelectionWhenValue('SliderSOCDInputEnabled', SOCD_MODES),
	sliderSOCDPinOne: yup
		.number()
		.label('Slider SOCD Up Priority Pin')
		.validatePinWhenValue('SliderSOCDInputEnabled'),
	sliderSOCDPinTwo: yup
		.number()
		.label('Slider SOCD Second Priority Pin')
		.validatePinWhenValue('SliderSOCDInputEnabled'),

	TurboInputEnabled: yup.number().required().label('Turbo Input Enabled'),
	turboPin: yup
		.number()
		.label('Turbo Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	turboPinLED: yup
		.number()
		.label('Turbo Pin LED')
		.validatePinWhenValue('TurboInputEnabled'),
	pinShmupBtn1: yup
		.number()
		.label('Charge Shot 1 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	pinShmupBtn2: yup
		.number()
		.label('Charge Shot 2 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	pinShmupBtn3: yup
		.number()
		.label('Charge Shot 3 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	pinShmupBtn4: yup
		.number()
		.label('Charge Shot 4 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	pinShmupDial: yup
		.number()
		.label('Shmup Dial Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	turboShotCount: yup
		.number()
		.label('Turbo Shot Count')
		.validateRangeWhenValue('TurboInputEnabled', 5, 30),
	shmupMode: yup
		.number()
		.label('Shmup Mode Enabled')
		.validateRangeWhenValue('TurboInputEnabled', 0, 1),
	shmupMixMode: yup
		.number()
		.label('Shmup Mix Priority')
		.validateSelectionWhenValue('TurboInputEnabled', DUAL_STICK_MODES),
	shmupAlwaysOn1: yup
		.number()
		.label('Turbo-Button 1 (Always On)')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupAlwaysOn2: yup
		.number()
		.label('Turbo-Button 2 (Always On)')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupAlwaysOn3: yup
		.number()
		.label('Turbo-Button 3 (Always On)')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupAlwaysOn4: yup
		.number()
		.label('Turbo-Button 4 (Always On)')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupBtnMask1: yup
		.number()
		.label('Charge Shot Button 1 Map')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupBtnMask2: yup
		.number()
		.label('Charge Shot Button 2 Map')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupBtnMask3: yup
		.number()
		.label('Charge Shot Button 3 Map')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),
	shmupBtnMask4: yup
		.number()
		.label('Charge Shot Button 4 Map')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS),

	WiiExtensionAddonEnabled: yup
		.number()
		.required()
		.label('Wii Extensions Enabled'),
	wiiExtensionSDAPin: yup
		.number()
		.required()
		.label('WiiExtension I2C SDA Pin')
		.validatePinWhenValue('WiiExtensionAddonEnabled'),
	wiiExtensionSCLPin: yup
		.number()
		.required()
		.label('WiiExtension I2C SCL Pin')
		.validatePinWhenValue('WiiExtensionAddonEnabled'),
	wiiExtensionBlock: yup
		.number()
		.required()
		.label('WiiExtension I2C Block')
		.validateSelectionWhenValue('WiiExtensionAddonEnabled', I2C_BLOCKS),
	wiiExtensionSpeed: yup
		.number()
		.label('WiiExtension I2C Speed')
		.validateNumberWhenValue('WiiExtensionAddonEnabled'),
});

const defaultValues = {
	turboPin: -1,
	turboPinLED: -1,
	sliderPinOne: -1,
	sliderPinTwo: -1,
	sliderSOCDPinOne: -1,
	sliderSOCDPinTwo: -1,
	sliderModeZero: 0,
	sliderModeOne: 1,
	sliderModeTwo: 2,
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
	tilt1Pin: -1,
	tilt2Pin: -1,
	tiltLeftAnalogUpPin: -1,
	tiltLeftAnalogDownPin: -1,
	tiltLeftAnalogLeftPin: -1,
	tiltLeftAnalogRightPin: -1,
	tiltRightAnalogUpPin: -1,
	tiltRightAnalogDownPin: -1,
	tiltRightAnalogLeftPin: -1,
	tiltRightAnalogRightPin: -1,
	analogAdc1PinX: -1,
	analogAdc1PinY: -1,
	analogAdc1Mode: 1,
	analogAdc1Invert: 0,
	analogAdc2PinX: -1,
	analogAdc2PinY: -1,
	analogAdc2Mode: 2,
	analogAdc2Invert: 0,
	forced_circularity: 0,
	analog_deadzone: 5,
	auto_calibrate: 0,
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
	keyboardHostPin5V: -1,
	keyboardHostMap: baseButtonMappings,
	AnalogInputEnabled: 0,
	BoardLedAddonEnabled: 0,
	FocusModeAddonEnabled: 0,
	BuzzerSpeakerAddonEnabled: 0,
	BootselButtonAddonEnabled: 0,
	DualDirectionalInputEnabled: 0,
	TiltInputEnabled: 0,
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

const FormContext = ({ setStoredData }) => {
	const { values, setValues } = useFormikContext();
	const { setLoading } = useContext(AppContext);

	useEffect(() => {
		async function fetchData() {
			const data = await WebApi.getAddonsOptions(setLoading);

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
	for (const prop in Object.keys(values).filter(
		(key) => !!!key.includes('keyboardHostMap'),
	)) {
		if (!!values[prop]) values[prop] = parseInt(values[prop]);
	}
};

function flattenObject(object) {
	var toReturn = {};

	for (var i in object) {
		if (!object.hasOwnProperty(i)) continue;

		if (typeof object[i] == 'object' && object[i] !== null) {
			var flatObject = flattenObject(object[i]);
			for (var x in flatObject) {
				if (!flatObject.hasOwnProperty(x)) continue;

				toReturn[i + '.' + x] = flatObject[x];
			}
		} else {
			toReturn[i] = object[i];
		}
	}
	return toReturn;
}

const AvailablePinOptions = ({ pins }) => {
	const { t } = useTranslation();
	return (
		<>
			<option value={-1}>
				{t('AddonsConfig:analog-available-pins-option-not-set')}
			</option>
			{pins.map((i) => (
				<option key={`analogPins-option-${i}`} value={i}>
					{i}
				</option>
			))}
		</>
	);
};

export default function AddonsConfigPage() {
	const { buttonLabels, updateUsedPins, usedPins } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');
	const [storedData, setStoredData] = useState({});
	const [validated, setValidated] = useState(false);

	const availableAnalogPins = ANALOG_PINS.filter(
		(pin) => !usedPins?.includes(pin),
	);

	const { t } = useTranslation();

	const translatedReverseAction = REVERSE_ACTION.map((option) => ({
		...option,
		label: t(`AddonsConfig:reverse-action-${option.label.toLowerCase()}-label`),
	}));

	const handleKeyChange = (values, setFieldValue) => (value, button) => {
		const newMappings = { ...values.keyboardHostMap };
		newMappings[button].key = value;
		const mappings = validateMappings(newMappings, t);
		setFieldValue('keyboardHostMap', mappings);
		setValidated(true);
	};

	const getKeyMappingForButton = (values) => (button) => {
		return values.keyboardHostMap[button];
	};

	const onSuccess = async (values) => {
		const flattened = flattenObject(storedData);
		const valuesCopy = schema.cast(values); // Strip invalid values

		// Compare what's changed and set it to resultObject
		let resultObject = {};
		Object.entries(flattened)?.map((entry) => {
			const [key, oldVal] = entry;
			const newVal = get(valuesCopy, key);
			if (newVal !== oldVal) {
				set(resultObject, key, newVal);
			}
		});
		sanitizeData(resultObject);
		const success = await WebApi.setAddonsOptions(resultObject);
		setStoredData(JSON.parse(JSON.stringify(values))); // Update to reflect saved data
		setSaveMessage(
			success
				? t('Common:saved-success-message')
				: t('Common:saved-error-message'),
		);
		if (success) updateUsedPins();
	};

	const handleCheckbox = async (name, values) => {
		values[name] = values[name] === 1 ? 0 : 1;
	};

	return (
		<Formik
			enableReinitialize={true}
			validationSchema={schema}
			onSubmit={onSuccess}
			initialValues={defaultValues}
		>
			{({ handleSubmit, handleChange, values, errors, setFieldValue }) => (
				<Form noValidate onSubmit={handleSubmit}>
					<Section title={t('AddonsConfig:header-text')}>
						<p>{t('AddonsConfig:sub-header-text')}</p>
					</Section>
					<Section title={t('AddonsConfig:bootsel-header-text')}>
						<div
							id="BootselButtonAddonOptions"
							hidden={!values.BootselButtonAddonEnabled}
						>
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
								{BUTTON_MASKS.map((o, i) => (
									<option key={`bootselButtonMap-option-${i}`} value={o.value}>
										{o.label}
									</option>
								))}
							</FormSelect>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="BootselButtonAddonButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.BootselButtonAddonEnabled)}
							onChange={(e) => {
								handleCheckbox('BootselButtonAddonEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:on-board-led-configuration-label')}>
						<div
							id="BoardLedAddonEnabledOptions"
							hidden={!values.BoardLedAddonEnabled}
						>
							<FormSelect
								label={t('AddonsConfig:on-board-led-mode-label')}
								name="onBoardLedMode"
								className="form-select-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.onBoardLedMode}
								error={errors.onBoardLedMode}
								isInvalid={errors.onBoardLedMode}
								onChange={handleChange}
							>
								{ON_BOARD_LED_MODES.map((o, i) => (
									<option key={`onBoardLedMode-option-${i}`} value={o.value}>
										{o.label}
									</option>
								))}
							</FormSelect>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="BoardLedAddonButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.BoardLedAddonEnabled)}
							onChange={(e) => {
								handleCheckbox('BoardLedAddonEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:analog-header-text')}>
						<div id="AnalogInputOptions" hidden={!values.AnalogInputEnabled}>
							<p>{t('AddonsConfig:analog-warning')}</p>
							<p>
								{t('AddonsConfig:analog-available-pins-text', {
									pins: availableAnalogPins.join(', '),
								})}
							</p>
							<Row className="mb-3">
								<FormSelect
									label={t('AddonsConfig:analog-adc-1-pin-x-label')}
									name="analogAdc1PinX"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdc1PinX}
									error={errors.analogAdc1PinX}
									isInvalid={errors.analogAdc1PinX}
									onChange={handleChange}
								>
									<AvailablePinOptions pins={ANALOG_PINS} />
								</FormSelect>
								<FormSelect
									label={t('AddonsConfig:analog-adc-1-pin-y-label')}
									name="analogAdc1PinY"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdc1PinY}
									error={errors.analogAdc1PinY}
									isInvalid={errors.analogAdc1PinY}
									onChange={handleChange}
								>
									<AvailablePinOptions pins={ANALOG_PINS} />
								</FormSelect>
								<FormSelect
									label={t('AddonsConfig:analog-adc-1-mode-label')}
									name="analogAdc1Mode"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdc1Mode}
									error={errors.analogAdc1Mode}
									isInvalid={errors.analogAdc1Mode}
									onChange={handleChange}
								>
									{ANALOG_STICK_MODES.map((o, i) => (
										<option
											key={`button-analogAdc1Mode-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
								<FormSelect
									label={t('AddonsConfig:analog-adc-1-invert-label')}
									name="analogAdc1Invert"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdc1Invert}
									error={errors.analogAdc1Invert}
									isInvalid={errors.analogAdc1Invert}
									onChange={handleChange}
								>
									{INVERT_MODES.map((o, i) => (
										<option
											key={`button-analogAdc1Invert-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
							</Row>
							<Row className="mb-3">
								<FormSelect
									label={t('AddonsConfig:analog-adc-2-pin-x-label')}
									name="analogAdc2PinX"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdc2PinX}
									error={errors.analogAdc2PinX}
									isInvalid={errors.analogAdc2PinX}
									onChange={handleChange}
								>
									<AvailablePinOptions pins={ANALOG_PINS} />
								</FormSelect>
								<FormSelect
									label={t('AddonsConfig:analog-adc-2-pin-y-label')}
									name="analogAdc2PinY"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdc2PinY}
									error={errors.analogAdc2PinY}
									isInvalid={errors.analogAdc2PinY}
									onChange={handleChange}
								>
									<AvailablePinOptions pins={ANALOG_PINS} />
								</FormSelect>
								<FormSelect
									label={t('AddonsConfig:analog-adc-2-mode-label')}
									name="analogAdc2Mode"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdc2Mode}
									error={errors.analogAdc2Mode}
									isInvalid={errors.analogAdc2Mode}
									onChange={handleChange}
								>
									{ANALOG_STICK_MODES.map((o, i) => (
										<option
											key={`button-analogAdc2Mode-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
								<FormSelect
									label={t('AddonsConfig:analog-adc-2-invert-label')}
									name="analogAdc2Invert"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdc2Invert}
									error={errors.analogAdc2Invert}
									isInvalid={errors.analogAdc2Invert}
									onChange={handleChange}
								>
									{INVERT_MODES.map((o, i) => (
										<option
											key={`button-analogAdc2Invert-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
							</Row>
							<Row className="mb-3">
								<FormControl
									type="number"
									label={t('AddonsConfig:analog-deadzone-size')}
									name="analog_deadzone"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analog_deadzone}
									error={errors.analog_deadzone}
									isInvalid={errors.analog_deadzone}
									onChange={handleChange}
									min={0}
									max={100}
								/>
								<FormCheck
									label={t('AddonsConfig:analog-force-circularity')}
									type="switch"
									id="Forced_circularity"
									className="col-sm-3 ms-2"
									isInvalid={false}
									checked={Boolean(values.forced_circularity)}
									onChange={(e) => {
										handleCheckbox('forced_circularity', values);
										handleChange(e);
									}}
								/>
								<FormCheck
									label={t('AddonsConfig:analog-auto-calibrate')}
									type="switch"
									id="Auto_calibrate"
									className="col-sm-3 ms-2"
									isInvalid={false}
									checked={Boolean(values.auto_calibrate)}
									onChange={(e) => {
										handleCheckbox('auto_calibrate', values);
										handleChange(e);
									}}
								/>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="AnalogInputButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.AnalogInputEnabled)}
							onChange={(e) => {
								handleCheckbox('AnalogInputEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:turbo-header-text')}>
						<div id="TurboInputOptions" hidden={!values.TurboInputEnabled}>
							<Row className="mb-3">
								<FormControl
									type="number"
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
								<FormControl
									type="number"
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
								<FormControl
									type="number"
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
									<AvailablePinOptions pins={ANALOG_PINS} />
								</FormSelect>
								<FormCheck
									label={t('AddonsConfig:turbo-shmup-mode-label')}
									type="switch"
									id="ShmupMode"
									className="col-sm-3 ms-2"
									isInvalid={false}
									checked={Boolean(values.shmupMode)}
									onChange={(e) => {
										handleCheckbox('shmupMode', values);
										handleChange(e);
									}}
								/>
								<div id="ShmupOptions" hidden={!values.shmupMode}>
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
											{TURBO_MASKS.map((o, i) => (
												<option
													key={`shmupAlwaysOn1-option-${i}`}
													value={o.value}
												>
													{o.label}
												</option>
											))}
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
											{TURBO_MASKS.map((o, i) => (
												<option
													key={`shmupAlwaysOn2-option-${i}`}
													value={o.value}
												>
													{o.label}
												</option>
											))}
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
											{TURBO_MASKS.map((o, i) => (
												<option
													key={`shmupAlwaysOn3-option-${i}`}
													value={o.value}
												>
													{o.label}
												</option>
											))}
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
											{TURBO_MASKS.map((o, i) => (
												<option
													key={`shmupAlwaysOn4-option-${i}`}
													value={o.value}
												>
													{o.label}
												</option>
											))}
										</FormSelect>
									</Row>
									<Row className="mb-3">
										<FormControl
											type="number"
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
										<FormControl
											type="number"
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
										<FormControl
											type="number"
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
										<FormControl
											type="number"
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
											{TURBO_MASKS.map((o, i) => (
												<option
													key={`shmupBtnMask1-option-${i}`}
													value={o.value}
												>
													{o.label}
												</option>
											))}
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
											{TURBO_MASKS.map((o, i) => (
												<option
													key={`shmupBtnMask2-option-${i}`}
													value={o.value}
												>
													{o.label}
												</option>
											))}
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
											{TURBO_MASKS.map((o, i) => (
												<option
													key={`shmupBtnMask3-option-${i}`}
													value={o.value}
												>
													{o.label}
												</option>
											))}
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
											{TURBO_MASKS.map((o, i) => (
												<option
													key={`shmupBtnMask4-option-${i}`}
													value={o.value}
												>
													{o.label}
												</option>
											))}
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
										{SHMUP_MIXED_MODES.map((o, i) => (
											<option
												key={`button-shmupMixedMode-option-${i}`}
												value={o.value}
											>
												{o.label}
											</option>
										))}
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
							onChange={(e) => {
								handleCheckbox('TurboInputEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section
						title={t('AddonsConfig:joystick-selection-slider-header-text')}
					>
						<div id="JSliderInputOptions" hidden={!values.JSliderInputEnabled}>
							<Row className="mb-3">
								<FormSelect
									label={t(
										'AddonsConfig:joystick-selection-slider-mode-zero-label',
									)}
									name="sliderModeZero"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.sliderModeZero}
									error={errors.sliderModeZero}
									isInvalid={errors.sliderModeZero}
									onChange={handleChange}
								>
									{DPAD_MODES.map((o, i) => (
										<option key={`sliderModeZero-option-${i}`} value={o.value}>
											{o.label}
										</option>
									))}
								</FormSelect>
								<FormSelect
									label={t(
										'AddonsConfig:joystick-selection-slider-mode-one-label',
									)}
									name="sliderModeOne"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.sliderModeOne}
									error={errors.sliderModeOne}
									isInvalid={errors.sliderModeOne}
									onChange={handleChange}
								>
									{DPAD_MODES.map((o, i) => (
										<option key={`sliderModeOne-option-${i}`} value={o.value}>
											{o.label}
										</option>
									))}
								</FormSelect>
								<FormControl
									type="number"
									label={t(
										'AddonsConfig:joystick-selection-slider-pin-one-label',
									)}
									name="sliderPinOne"
									className="form-select-sm"
									groupClassName="col-sm-1 mb-3"
									value={values.sliderPinOne}
									error={errors.sliderPinOne}
									isInvalid={errors.sliderPinOne}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormSelect
									label={t(
										'AddonsConfig:joystick-selection-slider-mode-two-label',
									)}
									name="sliderModeTwo"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.sliderModeTwo}
									error={errors.sliderModeTwo}
									isInvalid={errors.sliderModeTwo}
									onChange={handleChange}
								>
									{DPAD_MODES.map((o, i) => (
										<option key={`sliderModeTwo-option-${i}`} value={o.value}>
											{o.label}
										</option>
									))}
								</FormSelect>
								<FormControl
									type="number"
									label={t(
										'AddonsConfig:joystick-selection-slider-pin-two-label',
									)}
									name="sliderPinTwo"
									className="form-control-sm"
									groupClassName="col-sm-1 mb-3"
									value={values.sliderPinTwo}
									error={errors.sliderPinTwo}
									isInvalid={errors.sliderPinTwo}
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
							onChange={(e) => {
								handleCheckbox('JSliderInputEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:input-reverse-header-text')}>
						<div id="ReverseInputOptions" hidden={!values.ReverseInputEnabled}>
							<Row className="mb-3">
								<FormControl
									type="number"
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
								<FormControl
									type="number"
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
									{translatedReverseAction.map((o, i) => (
										<option key={`reverseActionUp-option-${i}`} value={o.value}>
											{o.label}
										</option>
									))}
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
									{translatedReverseAction.map((o, i) => (
										<option
											key={`reverseActionDown-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
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
									{translatedReverseAction.map((o, i) => (
										<option
											key={`reverseActionLeft-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
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
									{translatedReverseAction.map((o, i) => (
										<option
											key={`reverseActionRight-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
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
							onChange={(e) => {
								handleCheckbox('ReverseInputEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:i2c-analog-ads1219-header-text')}>
						<div
							id="I2CAnalog1219InputOptions"
							hidden={!values.I2CAnalog1219InputEnabled}
						>
							<Row className="mb-3">
								<FormControl
									type="number"
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
								<FormControl
									type="number"
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
									{I2C_BLOCKS.map((o, i) => (
										<option key={`i2cBlock-option-${i}`} value={o.value}>
											{o.label}
										</option>
									))}
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
							onChange={(e) => {
								handleCheckbox('I2CAnalog1219InputEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:dual-directional-input-header-text')}>
						<div
							id="DualDirectionalInputOptions"
							hidden={!values.DualDirectionalInputEnabled}
						>
							<Row className="mb-3">
								<FormControl
									type="number"
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
								<FormControl
									type="number"
									label={t(
										'AddonsConfig:dual-directional-input-down-pin-label',
									)}
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
								<FormControl
									type="number"
									label={t(
										'AddonsConfig:dual-directional-input-left-pin-label',
									)}
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
								<FormControl
									type="number"
									label={t(
										'AddonsConfig:dual-directional-input-right-pin-label',
									)}
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
									label={t(
										'AddonsConfig:dual-directional-input-dpad-mode-label',
									)}
									name="dualDirDpadMode"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.dualDirDpadMode}
									error={errors.dualDirDpadMode}
									isInvalid={errors.dualDirDpadMode}
									onChange={handleChange}
								>
									{DUAL_STICK_MODES.map((o, i) => (
										<option
											key={`button-dualDirDpadMode-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>

								<FormSelect
									label={t(
										'AddonsConfig:dual-directional-input-combine-mode-label',
									)}
									name="dualDirCombineMode"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.dualDirCombineMode}
									error={errors.dualDirCombineMode}
									isInvalid={errors.dualDirCombineMode}
									onChange={handleChange}
								>
									{DUAL_COMBINE_MODES.map((o, i) => (
										<option
											key={`button-dualDirCombineMode-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
								<FormCheck
									label={t(
										'AddonsConfig:dual-directional-input-four-way-joystick-mode-label',
									)}
									type="switch"
									id="DualDirFourWayMode"
									className="col-sm-3 ms-2"
									isInvalid={false}
									checked={Boolean(values.dualDirFourWayMode)}
									onChange={(e) => {
										handleCheckbox('dualDirFourWayMode', values);
										handleChange(e);
									}}
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
							onChange={(e) => {
								handleCheckbox('DualDirectionalInputEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:tilt-header-text')}>
						<div id="TiltOptions" hidden={!values.TiltInputEnabled}>
							<Row className="mb-3">
								<FormControl
									type="number"
									label={t('AddonsConfig:tilt-1-pin-label')}
									name="tilt1Pin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.tilt1Pin}
									error={errors.tilt1Pin}
									isInvalid={errors.tilt1Pin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl
									type="number"
									label={t('AddonsConfig:tilt-2-pin-label')}
									name="tilt2Pin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.tilt2Pin}
									error={errors.tilt2Pin}
									isInvalid={errors.tilt2Pin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
							</Row>
							<Row className="mb-3">
								<FormControl
									type="number"
									label={t('AddonsConfig:tilt-left-analog-up-pin-label')}
									name="tiltLeftAnalogUpPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.tiltLeftAnalogUpPin}
									error={errors.tiltLeftAnalogUpPin}
									isInvalid={errors.tiltLeftAnalogUpPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl
									type="number"
									label={t('AddonsConfig:tilt-left-analog-down-pin-label')}
									name="tiltLeftAnalogDownPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.tiltLeftAnalogDownPin}
									error={errors.tiltLeftAnalogDownPin}
									isInvalid={errors.tiltLeftAnalogDownPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl
									type="number"
									label={t('AddonsConfig:tilt-left-analog-left-pin-label')}
									name="tiltLeftAnalogLeftPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.tiltLeftAnalogLeftPin}
									error={errors.tiltLeftAnalogLeftPin}
									isInvalid={errors.tiltLeftAnalogLeftPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl
									type="number"
									label={t('AddonsConfig:tilt-left-analog-right-pin-label')}
									name="tiltLeftAnalogRightPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.tiltLeftAnalogRightPin}
									error={errors.tiltLeftAnalogRightPin}
									isInvalid={errors.tiltLeftAnalogRightPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
							</Row>
							<Row className="mb-3">
								<FormControl
									type="number"
									label={t('AddonsConfig:tilt-right-analog-up-pin-label')}
									name="tiltRightAnalogUpPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.tiltRightAnalogUpPin}
									error={errors.tiltRightAnalogUpPin}
									isInvalid={errors.tiltRightAnalogUpPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl
									type="number"
									label={t('AddonsConfig:tilt-right-analog-down-pin-label')}
									name="tiltRightAnalogDownPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.tiltRightAnalogDownPin}
									error={errors.tiltRightAnalogDownPin}
									isInvalid={errors.tiltRightAnalogDownPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl
									type="number"
									label={t('AddonsConfig:tilt-right-analog-left-pin-label')}
									name="tiltRightAnalogLeftPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.tiltRightAnalogLeftPin}
									error={errors.tiltRightAnalogLeftPin}
									isInvalid={errors.tiltRightAnalogLeftPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<FormControl
									type="number"
									label={t('AddonsConfig:tilt-right-analog-right-pin-label')}
									name="tiltRightAnalogRightPin"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.tiltRightAnalogRightPin}
									error={errors.tiltRightAnalogRightPin}
									isInvalid={errors.tiltRightAnalogRightPin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
							</Row>
							<Row className="mb-3">
								<FormSelect
									label={t('AddonsConfig:tilt-socd-mode-label')}
									name="tiltSOCDMode"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.tiltSOCDMode}
									error={errors.tiltSOCDMode}
									isInvalid={errors.tiltSOCDMode}
									onChange={handleChange}
								>
									{TILT_SOCD_MODES.map((o, i) => (
										<option
											key={`button-tiltSOCDMode-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="TiltInputButton"
							reverse={true}
							error={false}
							isInvalid={false}
							checked={Boolean(values.TiltInputEnabled)}
							onChange={(e) => {
								handleCheckbox('TiltInputEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:buzzer-speaker-header-text')}>
						<div
							id="BuzzerSpeakerAddonOptions"
							hidden={!values.BuzzerSpeakerAddonEnabled}
						>
							<Row className="mb-3">
								<FormControl
									type="number"
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
								<FormControl
									type="number"
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
							onChange={(e) => {
								handleCheckbox('BuzzerSpeakerAddonEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:extra-button-header-text')}>
						<div
							id="ExtraButtonAddonOptions"
							hidden={!values.ExtraButtonAddonEnabled}
						>
							<Row className="mb-3">
								<FormControl
									type="number"
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
									{BUTTON_MASKS.map((o, i) => (
										<option key={`extraButtonMap-option-${i}`} value={o.value}>
											{o.label}
										</option>
									))}
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
							onChange={(e) => {
								handleCheckbox('ExtraButtonAddonEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:player-number-header-text')}>
						<div
							id="PlayerNumAddonOptions"
							hidden={!values.PlayerNumAddonEnabled}
						>
							<p>
								<strong>
									{t('AddonsConfig:player-number-sub-header-text')}
								</strong>
							</p>
							<Row className="mb-3">
								<FormControl
									type="number"
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
							onChange={(e) => {
								handleCheckbox('PlayerNumAddonEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section
						title={t(
							'AddonsConfig:socd-cleaning-mode-selection-slider-header-text',
						)}
					>
						<div
							id="SliderSOCDInputOptions"
							hidden={!values.SliderSOCDInputEnabled}
						>
							<Row className="mb-3">
								<p>
									{t(
										'AddonsConfig:socd-cleaning-mode-selection-slider-sub-header-text',
									)}
								</p>
								<FormSelect
									label={t(
										'AddonsConfig:socd-cleaning-mode-selection-slider-mode-default-label',
									)}
									name="sliderSOCDModeDefault"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.sliderSOCDModeDefault}
									error={errors.sliderSOCDModeDefault}
									isInvalid={errors.sliderSOCDModeDefault}
									onChange={handleChange}
								>
									{SOCD_MODES.map((o, i) => (
										<option
											key={`sliderSOCDModeDefault-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
								<FormSelect
									label={t(
										'AddonsConfig:socd-cleaning-mode-selection-slider-mode-one-label',
									)}
									name="sliderSOCDModeOne"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.sliderSOCDModeOne}
									error={errors.sliderSOCDModeOne}
									isInvalid={errors.sliderSOCDModeOne}
									onChange={handleChange}
								>
									{SOCD_MODES.map((o, i) => (
										<option
											key={`sliderSOCDModeOne-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
								<FormControl
									type="number"
									label={t(
										'AddonsConfig:socd-cleaning-mode-selection-slider-pin-one-label',
									)}
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
									label={t(
										'AddonsConfig:socd-cleaning-mode-selection-slider-mode-two-label',
									)}
									name="sliderSOCDModeTwo"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.sliderSOCDModeTwo}
									error={errors.sliderSOCDModeTwo}
									isInvalid={errors.sliderSOCDModeTwo}
									onChange={handleChange}
								>
									{SOCD_MODES.map((o, i) => (
										<option
											key={`sliderSOCDModeTwo-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
								<FormControl
									type="number"
									label={t(
										'AddonsConfig:socd-cleaning-mode-selection-slider-pin-two-label',
									)}
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
							onChange={(e) => {
								handleCheckbox('SliderSOCDInputEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:ps4-mode-header-text')}>
						<div id="PS4ModeOptions" hidden={!values.PS4ModeAddonEnabled}>
							<Row>
								<Trans ns="AddonsConfig" i18nKey="ps4-mode-sub-header-text">
									<h2>
										!!!! DISCLAIMER: GP2040-CE WILL NEVER SUPPLY THESE FILES
										!!!!
									</h2>
									<p>
										Please upload the 3 required files and click the
										&quot;Verify & Save&quot; button to use PS4 Mode.
									</p>
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
									<Button type="button" onClick={verifyAndSavePS4}>
										{t('Common:button-verify-save-label')}
									</Button>
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
							onChange={(e) => {
								handleCheckbox('PS4ModeAddonEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:pspassthrough-header-text')}>
						<div
							id="PSPassthroughAddonOptions"
							hidden={!values.PSPassthroughAddonEnabled}
						>
							<Row className="mb-3">
								<p>{t('AddonsConfig:pspassthrough-sub-header-text')}</p>
								<FormControl
									type="number"
									label={t('AddonsConfig:pspassthrough-d-plus-label')}
									name="psPassthroughPinDplus"
									className="form-select-sm"
									groupClassName="col-sm-1 mb-3"
									value={values.psPassthroughPinDplus}
									error={errors.psPassthroughPinDplus}
									isInvalid={errors.psPassthroughPinDplus}
									onChange={handleChange}
									min={-1}
									max={28}
								/>
								<FormControl
									type="number"
									label={t('AddonsConfig:pspassthrough-d-minus-label')}
									disabled
									className="form-select-sm"
									groupClassName="col-sm-1 mb-3"
									value={
										values.psPassthroughPinDplus === -1
											? -1
											: values.psPassthroughPinDplus + 1
									}
								/>
								<FormControl
									type="number"
									label={t('AddonsConfig:pspassthrough-five-v-label')}
									name="psPassthroughPin5V"
									className="form-select-sm"
									groupClassName="col-sm-auto mb-3"
									value={values.psPassthroughPin5V}
									error={errors.psPassthroughPin5V}
									isInvalid={errors.psPassthroughPin5V}
									onChange={handleChange}
									min={-1}
									max={28}
								/>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="PSPassthroughAddonButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.PSPassthroughAddonEnabled)}
							onChange={(e) => {
								handleCheckbox('PSPassthroughAddonEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:wii-extension-header-text')}>
						<div
							id="WiiExtensionAddonOptions"
							hidden={!values.WiiExtensionAddonEnabled}
						>
							<Row>
								<Trans
									ns="AddonsConfig"
									i18nKey="wii-extension-sub-header-text"
								>
									<p>
										Note: If the Display is enabled at the same time, this Addon
										will be disabled.
									</p>
									<h3>Currently Supported Controllers</h3>
									<p>
										Classic/Classic Pro: Both Analogs and D-Pad Supported. B =
										B1, A = B2, Y = B3, X = B4, L = L1, ZL = L2, R = R1, ZR =
										R2, Minus = S1, Plus = S2, Home = A1
									</p>
									<p>Nunchuck: Analog Stick Supported. C = B1, Z = B2</p>
									<p>
										Guitar Hero Guitar: Analog Stick Supported. Green = B1, Red
										= B2, Blue = B3, Yellow = B4, Orange = L1, Strum Up = Up,
										Strum Down = Down, Minus = S1, Plus = S2
									</p>
								</Trans>
							</Row>
							<Row className="mb-3">
								<FormControl
									type="number"
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
								<FormControl
									type="number"
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
									{I2C_BLOCKS.map((o, i) => (
										<option
											key={`wiiExtensionI2cBlock-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
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
							reverse={true}
							error={undefined}
							isInvalid={false}
							checked={Boolean(values.WiiExtensionAddonEnabled)}
							onChange={(e) => {
								handleCheckbox('WiiExtensionAddonEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:snes-extension-header-text')}>
						<div id="SNESpadAddonOptions" hidden={!values.SNESpadAddonEnabled}>
							<Row>
								<Trans
									ns="AddonsConfig"
									i18nKey="snes-extension-sub-header-text"
								>
									<p>
										Note: If the Display is enabled at the same time, this Addon
										will be disabled.
									</p>
									<h3>Currently Supported Controllers</h3>
									<p>
										SNES pad: D-Pad Supported. B = B1, A = B2, Y = B3, X = B4, L
										= L1, R = R1, Select = S1, Start = S2
									</p>
									<p>
										SNES mouse: Analog Stick Supported. Left Click = B1, Right
										Click = B2
									</p>
									<p>
										NES: D-Pad Supported. B = B1, A = B2, Select = S1, Start =
										S2
									</p>
								</Trans>
							</Row>
							<Row className="mb-3">
								<FormControl
									type="number"
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
								<FormControl
									type="number"
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
								<FormControl
									type="number"
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
							reverse={true}
							error={undefined}
							isInvalid={false}
							checked={Boolean(values.SNESpadAddonEnabled)}
							onChange={(e) => {
								handleCheckbox('SNESpadAddonEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:focus-mode-header-text')}>
						<div
							id="FocusModeAddonOptions"
							hidden={!values.FocusModeAddonEnabled}
						>
							<Row className="mb-3">
								<FormControl
									type="number"
									label={t('AddonsConfig:focus-mode-pin-label')}
									name="focusModePin"
									className="form-select-sm col-3"
									groupClassName="col-sm-3 mb-3"
									value={values.focusModePin}
									error={errors.focusModePin}
									isInvalid={errors.focusModePin}
									onChange={handleChange}
									min={-1}
									max={29}
								/>
								<div className="col-sm-3">
									<FormCheck
										label={t('Common:lock-oled-screen')}
										className="form-check-sm"
										type="switch"
										reverse
										id="FocusModeAddonOLEDButton"
										isInvalid={false}
										checked={Boolean(values.focusModeOledLockEnabled)}
										onChange={(e) => {
											handleCheckbox('focusModeOledLockEnabled', values);
											handleChange(e);
										}}
									/>
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
										onChange={(e) => {
											handleCheckbox('focusModeRgbLockEnabled', values);
											handleChange(e);
										}}
									/>
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
										onChange={(e) => {
											handleCheckbox('focusModeButtonLockEnabled', values);
											handleChange(e);
										}}
									/>
								</div>
								<Row>
									{BUTTON_MASKS.map((mask) =>
										values.focusModeButtonLockMask & mask.value ? (
											<FormSelect
												name="focusModeButtonLockMask"
												className="form-select-sm"
												groupClassName="col-sm-3 mb-3"
												value={values.focusModeButtonLockMask & mask.value}
												error={errors.focusModeButtonLockMask}
												isInvalid={errors.focusModeButtonLockMask}
												onChange={(e) => {
													setFieldValue(
														'focusModeButtonLockMask',
														(values.focusModeButtonLockMask ^ mask.value) |
															e.target.value,
													);
												}}
											>
												{BUTTON_MASKS.map((o, i) => (
													<option
														key={`focusModeButtonLockMask-option-${i}`}
														value={o.value}
													>
														{o.label}
													</option>
												))}
											</FormSelect>
										) : (
											<></>
										),
									)}
									<FormSelect
										name="focusModeButtonLockMask"
										className="form-select-sm"
										groupClassName="col-sm-3 mb-3"
										value={0}
										error={errors.focusModeButtonLockMask}
										isInvalid={errors.focusModeButtonLockMask}
										onChange={(e) => {
											setFieldValue(
												'focusModeButtonLockMask',
												values.focusModeButtonLockMask | e.target.value,
											);
										}}
									>
										{BUTTON_MASKS.map((o, i) => (
											<option
												key={`focusModeButtonLockMask-option-${i}`}
												value={o.value}
											>
												{o.label}
											</option>
										))}
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
							onChange={(e) => {
								handleCheckbox('FocusModeAddonEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<Section title={t('AddonsConfig:keyboard-host-header-text')}>
						<div
							id="KeyboardHostAddonOptions"
							hidden={!values.KeyboardHostAddonEnabled}
						>
							<Row className="mb-3">
								<p>{t('AddonsConfig:keyboard-host-sub-header-text')}</p>
								<FormControl
									type="number"
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
								<FormControl
									type="number"
									label={t('AddonsConfig:keyboard-host-d-minus-label')}
									disabled
									className="form-select-sm"
									groupClassName="col-sm-1 mb-3"
									value={
										values.keyboardHostPinDplus === -1
											? -1
											: values.keyboardHostPinDplus + 1
									}
								/>
								<FormControl
									type="number"
									label={t('AddonsConfig:keyboard-host-five-v-label')}
									name="keyboardHostPin5V"
									className="form-select-sm"
									groupClassName="col-sm-auto mb-3"
									value={values.keyboardHostPin5V}
									error={errors.keyboardHostPin5V}
									isInvalid={errors.keyboardHostPin5V}
									onChange={handleChange}
									min={-1}
									max={28}
								/>
							</Row>
							<Row className="mb-3">
								<p>{t('KeyboardMapping:sub-header-text')}</p>
								<KeyboardMapper
									buttonLabels={buttonLabels}
									handleKeyChange={handleKeyChange(values, setFieldValue)}
									validated={validated}
									getKeyMappingForButton={getKeyMappingForButton(values)}
								/>
							</Row>
						</div>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="KeyboardHostAddonButton"
							reverse
							isInvalid={false}
							checked={Boolean(values.KeyboardHostAddonEnabled)}
							onChange={(e) => {
								handleCheckbox('KeyboardHostAddonEnabled', values);
								handleChange(e);
							}}
						/>
					</Section>
					<div className="mt-3">
						<Button type="submit" id="save">
							{t('Common:button-save-label')}
						</Button>
						{saveMessage ? <span className="alert">{saveMessage}</span> : null}
					</div>
					<FormContext setStoredData={setStoredData} />
				</Form>
			)}
		</Formik>
	);
}
