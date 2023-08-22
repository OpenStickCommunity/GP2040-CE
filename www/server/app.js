/**
 * GP2040-CE Configurator Development Server
 */

import express from 'express';
import cors from 'cors';
import mapValues from 'lodash/mapValues.js';
import { readFileSync } from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { DEFAULT_KEYBOARD_MAPPING } from '../src/Data/Keyboard.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const { pico: picoController } = JSON.parse(
	readFileSync(path.resolve(__dirname, '../src/Data/Controllers.json'), 'utf8'),
);

const port = process.env.PORT || 8080;

const app = express();
app.use(cors());
app.use(express.json());
app.use((req, res, next) => {
	console.log('Request:', req.method, req.url);
	next();
});

app.get('/api/getUsedPins', (req, res) => {
	return res.send({ usedPins: Object.values(picoController) });
});

app.get('/api/resetSettings', (req, res) => {
	return res.send({ success: true });
});

app.get('/api/getDisplayOptions', (req, res) => {
	const data = {
		enabled: 1,
		sdaPin: 0,
		sclPin: 1,
		i2cAddress: '0x3D',
		i2cBlock: 0,
		i2cSpeed: 400000,
		flipDisplay: 0,
		invertDisplay: 1,
		buttonLayout: 0,
		buttonLayoutRight: 3,
		splashMode: 3,
		splashChoice: 0,
		splashDuration: 0,
		splashImage: Array(16 * 64).fill(255),
		buttonLayoutCustomOptions: {
			params: {
				layout: 2,
				startX: 8,
				startY: 28,
				buttonRadius: 8,
				buttonPadding: 2,
			},
			paramsRight: {
				layout: 9,
				startX: 8,
				startY: 28,
				buttonRadius: 8,
				buttonPadding: 2,
			},
		},

		displaySaverTimeout: 0,
	};
	console.log('data', data);
	return res.send(data);
});

app.get('/api/getSplashImage', (req, res) => {
	const data = {
		splashImage: Array(16 * 64).fill(255),
	};
	console.log('data', data);
	return res.send(data);
});

app.get('/api/getGamepadOptions', (req, res) => {
	return res.send({
		dpadMode: 0,
		inputMode: 4,
		socdMode: 2,
		switchTpShareForDs4: 0,
		forcedSetupMode: 0,
		lockHotkeys: 0,
		fourWayMode: 0,
		fnButtonPin: -1,
		profileNumber: 1,
		ps4ControllerType: 0,
		hotkey01: {
			auxMask: 32768,
			buttonsMask: 66304,
			action: 4,
		},
		hotkey02: {
			auxMask: 0,
			buttonsMask: 131840,
			action: 1,
		},
		hotkey03: {
			auxMask: 0,
			buttonsMask: 262912,
			action: 2,
		},
		hotkey04: {
			auxMask: 0,
			buttonsMask: 525056,
			action: 3,
		},
		hotkey05: {
			auxMask: 0,
			buttonsMask: 70144,
			action: 6,
		},
		hotkey06: {
			auxMask: 0,
			buttonsMask: 135680,
			action: 7,
		},
		hotkey07: {
			auxMask: 0,
			buttonsMask: 266752,
			action: 8,
		},
		hotkey08: {
			auxMask: 0,
			buttonsMask: 528896,
			action: 10,
		},
		hotkey09: {
			auxMask: 0,
			buttonsMask: 0,
			action: 0,
		},
		hotkey10: {
			auxMask: 0,
			buttonsMask: 0,
			action: 0,
		},
		hotkey11: {
			auxMask: 0,
			buttonsMask: 0,
			action: 0,
		},
		hotkey12: {
			auxMask: 0,
			buttonsMask: 0,
			action: 0,
		},
	});
});

app.get('/api/getLedOptions', (req, res) => {
	return res.send({
		brightnessMaximum: 255,
		brightnessSteps: 5,
		dataPin: 15,
		ledFormat: 0,
		ledLayout: 1,
		ledsPerButton: 2,
		ledButtonMap: {
			Up: 3,
			Down: 1,
			Left: 0,
			Right: 2,
			B1: 8,
			B2: 9,
			B3: 4,
			B4: 5,
			L1: 7,
			R1: 6,
			L2: 11,
			R2: 10,
			S1: null,
			S2: null,
			L3: null,
			R3: null,
			A1: null,
			A2: null,
		},
		usedPins: Object.values(picoController),
		pledType: 1,
		pledPin1: 12,
		pledPin2: 13,
		pledPin3: 14,
		pledPin4: 15,
		pledColor: 65280,
	});
});

app.get('/api/getCustomTheme', (req, res) => {
	console.log('/api/getCustomTheme');
	return res.send({
		enabled: true,
		Up: { u: 16711680, d: 255 },
		Down: { u: 16711680, d: 255 },
		Left: { u: 16711680, d: 255 },
		Right: { u: 16711680, d: 255 },
		B1: { u: 65280, d: 16711680 },
		B2: { u: 65280, d: 16711680 },
		B3: { u: 255, d: 65280 },
		B4: { u: 255, d: 65280 },
		L1: { u: 255, d: 65280 },
		R1: { u: 255, d: 65280 },
		L2: { u: 65280, d: 16711680 },
		R2: { u: 65280, d: 16711680 },
		S1: { u: 65535, d: 16776960 },
		S2: { u: 65535, d: 16776960 },
		L3: { u: 65416, d: 16746496 },
		R3: { u: 65416, d: 16746496 },
		A1: { u: 8913151, d: 65416 },
		A2: { u: 8913151, d: 65416 },
	});
});

app.get('/api/getPinMappings', (req, res) => {
	return res.send(picoController);
});

app.get('/api/getKeyMappings', (req, res) =>
	res.send(mapValues(DEFAULT_KEYBOARD_MAPPING)),
);

app.get('/api/getProfileOptions', (req, res) => {
	return res.send({
		alternativePinMappings: [
			{
				B1: 10,
				B2: 6,
				B3: 11,
				B4: 12,
				L1: 13,
				R1: 9,
				L2: 7,
				R2: 8,
				Up: 2,
				Down: 3,
				Left: 5,
				Right: 4,
			},
			{
				B1: 10,
				B2: 11,
				B3: 12,
				B4: 13,
				L1: 6,
				R1: 8,
				L2: 7,
				R2: 9,
				Up: 3,
				Down: 2,
				Left: 4,
				Right: 5,
			},
			{
				B1: 6,
				B2: 7,
				B3: 8,
				B4: 9,
				L1: 10,
				R1: 12,
				L2: 11,
				R2: 13,
				Up: 3,
				Down: 5,
				Left: 4,
				Right: 2,
			},
		],
	});
});

app.get('/api/getAddonsOptions', (req, res) => {
	return res.send({
		turboPin: -1,
		turboPinLED: -1,
		sliderPinOne: -1,
		sliderPinTwo: -1,
		sliderModeZero: 0,
		sliderModeOne: 1,
		sliderModeTwo: 2,
		sliderSOCDPinOne: -1,
		sliderSOCDPinTwo: -1,
		turboShotCount: 20,
		reversePin: -1,
		reversePinLED: -1,
		reverseActionUp: 1,
		reverseActionDown: 1,
		reverseActionLeft: 1,
		reverseActionRight: 1,
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
		tiltSOCDMode: 0,
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
		extraButtonPin: -1,
		extraButtonMap: 0,
		focusModePin: -1,
		focusModeButtonLockMask: 0,
		focusModeButtonLockEnabled: 0,
		focusModeButtonLockOledLockEnabled: 0,
		focusModeButtonLockRgbLockEnabled: 0,
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
		keyboardHostPinDplus: 0,
		keyboardHostPin5V: -1,
		keyboardHostMap: DEFAULT_KEYBOARD_MAPPING,
		psPassthroughPinDplus: 0,
		psPassthroughPin5V: -1,
		AnalogInputEnabled: 1,
		BoardLedAddonEnabled: 1,
		FocusModeAddonEnabled: 1,
		BuzzerSpeakerAddonEnabled: 1,
		BootselButtonAddonEnabled: 1,
		DualDirectionalInputEnabled: 1,
		TiltInputEnabled: 1,
		ExtraButtonAddonEnabled: 1,
		I2CAnalog1219InputEnabled: 1,
		JSliderInputEnabled: 1,
		KeyboardHostAddonEnabled: 1,
		PlayerNumAddonEnabled: 1,
		PS4ModeAddonEnabled: 1,
		ReverseInputEnabled: 1,
		SliderSOCDInputEnabled: 1,
		TurboInputEnabled: 1,
		WiiExtensionAddonEnabled: 1,
		SNESpadAddonEnabled: 1,
		PSPassthroughAddonEnabled: 1,
		usedPins: Object.values(picoController),
	});
});

app.get('/api/getFirmwareVersion', (req, res) => {
	return res.send({
		version: process.env.VITE_CURRENT_VERSION,
	});
});

app.get('/api/getButtonLayoutCustomOptions', (req, res) => {
	return res.send({
		params: {
			layout: 2,
			startX: 8,
			startY: 28,
			buttonRadius: 8,
			buttonPadding: 2,
		},
		paramsRight: {
			layout: 9,
			startX: 8,
			startY: 28,
			buttonRadius: 8,
			buttonPadding: 2,
		},
	});
});

app.get('/api/reboot', (req, res) => {
	return res.send({});
});

app.get('/api/getMemoryReport', (req, res) => {
	return res.send({
		totalFlash: 2048,
		usedFlash: 1048,
		staticAllocs: 200,
		totalHeap: 2048,
		usedHeap: 1048,
	});
});

app.post('/api/*', (req, res) => {
	console.log(req.body);
	return res.send(req.body);
});

app.listen(port, () => {
	console.log(`Dev app listening at http://localhost:${port}`);
});
