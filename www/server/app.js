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
		i2cAddress: 61,
		i2cBlock: 0,
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
		turnOffWhenSuspended: 0,
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
		debounceDelay: 5,
        inputModeB1: 1,
        inputModeB2: 0,
        inputModeB3: 2,
        inputModeB4: 4,
        inputModeL1: -1,
        inputModeL2: -1,
        inputModeR1: -1,
        inputModeR2: 3,
		ps4ReportHack: 0,
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
		hotkey13: {
			auxMask: 0,
			buttonsMask: 0,
			action: 0,
		},
		hotkey14: {
			auxMask: 0,
			buttonsMask: 0,
			action: 0,
		},
		hotkey15: {
			auxMask: 0,
			buttonsMask: 0,
			action: 0,
		},
		hotkey16: {
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
		dataPin: 22,
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
		pledIndex1: 12,
		pledIndex2: 13,
		pledIndex3: 14,
		pledIndex4: 15,
		pledColor: 65280,
		turnOffWhenSuspended: 0,
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

app.get('/api/getPeripheralOptions', (req, res) => {
	return res.send({
		peripheral: {
			i2c0: {
				enabled: 1,
				sda: 0,
				scl: 1,
				speed: 400000,
			},
			i2c1: {
				enabled: 0,
				sda: -1,
				scl: -1,
				speed: 400000,
			},
			spi0: {
				enabled: 0,
				rx: -1,
				cs: -1,
				sck: -1,
				tx: -1,
			},
			spi1: {
				enabled: 0,
				rx: -1,
				cs: -1,
				sck: -1,
				tx: -1,
			},
			usb0: {
				enabled: 1,
				dp: 28,
				enable5v: -1,
				order: 0,
			},
		},
	});
});

app.get('/api/getWiiControls', (req, res) =>
	res.send({
		'nunchuk.analogStick.x.axisType': 1,
		'nunchuk.analogStick.y.axisType': 2,
		'nunchuk.buttonC': 1,
		'nunchuk.buttonZ': 2,
		'classic.analogLeftStick.x.axisType': 1,
		'classic.analogLeftStick.y.axisType': 2,
		'classic.analogRightStick.x.axisType': 3,
		'classic.analogRightStick.y.axisType': 4,
		'classic.analogLeftTrigger.axisType': 7,
		'classic.analogRightTrigger.axisType': 8,
		'classic.buttonA': 2,
		'classic.buttonB': 1,
		'classic.buttonX': 8,
		'classic.buttonY': 4,
		'classic.buttonL': 64,
		'classic.buttonR': 128,
		'classic.buttonZL': 16,
		'classic.buttonZR': 32,
		'classic.buttonMinus': 256,
		'classic.buttonHome': 4096,
		'classic.buttonPlus': 512,
		'classic.buttonUp': 65536,
		'classic.buttonDown': 131072,
		'classic.buttonLeft': 262144,
		'classic.buttonRight': 524288,
		'guitar.analogStick.x.axisType': 1,
		'guitar.analogStick.y.axisType': 2,
		'guitar.analogWhammyBar.axisType': 14,
		'guitar.buttonOrange': 64,
		'guitar.buttonRed': 2,
		'guitar.buttonBlue': 4,
		'guitar.buttonGreen': 1,
		'guitar.buttonYellow': 8,
		'guitar.buttonPedal': 128,
		'guitar.buttonMinus': 256,
		'guitar.buttonPlus': 512,
		'guitar.buttonStrumUp': 65536,
		'guitar.buttonStrumDown': 131072,
		'drum.analogStick.x.axisType': 1,
		'drum.analogStick.y.axisType': 2,
		'drum.buttonOrange': 64,
		'drum.buttonRed': 2,
		'drum.buttonBlue': 8,
		'drum.buttonGreen': 1,
		'drum.buttonYellow': 4,
		'drum.buttonPedal': 128,
		'drum.buttonMinus': 256,
		'drum.buttonPlus': 512,
		'turntable.analogStick.x.axisType': 1,
		'turntable.analogStick.y.axisType': 2,
		'turntable.analogLeftTurntable.axisType': 13,
		'turntable.analogRightTurntable.axisType': 15,
		'turntable.analogFader.axisType': 7,
		'turntable.analogEffects.axisType': 8,
		'turntable.buttonLeftGreen': 262144,
		'turntable.buttonLeftRed': 65536,
		'turntable.buttonLeftBlue': 524288,
		'turntable.buttonRightGreen': 4,
		'turntable.buttonRightRed': 8,
		'turntable.buttonRightBlue': 2,
		'turntable.buttonEuphoria': 32,
		'turntable.buttonMinus': 256,
		'turntable.buttonPlus': 512,
		'taiko.buttonDonLeft': 262144,
		'taiko.buttonKatLeft': 64,
		'taiko.buttonDonRight': 1,
		'taiko.buttonKatRight': 128,
	}),
);

app.get('/api/getProfileOptions', (req, res) => {
	return res.send({
		alternativePinMappings: [picoController, picoController, picoController],
	});
});

app.get('/api/getAddonsOptions', (req, res) => {
	return res.send({
		turboPin: -1,
		turboPinLED: -1,
		sliderModeZero: 0,
		turboShotCount: 20,
		reversePin: -1,
		reversePinLED: -1,
		reverseActionUp: 1,
		reverseActionDown: 1,
		reverseActionLeft: 1,
		reverseActionRight: 1,
		i2cAnalog1219Block: 0,
		i2cAnalog1219Address: 0x40,
		onBoardLedMode: 0,
		dualDirDpadMode: 0,
		dualDirCombineMode: 0,
		dualDirFourWayMode: 0,
		tilt1Pin: -1,
		factorTilt1LeftX: 0,
		factorTilt1LeftY: 0,
		factorTilt1RightX: 0,
		factorTilt1RightY: 0,
		tilt2Pin: -1,
		factorTilt2LeftX: 0,
		factorTilt2LeftY: 0,
		factorTilt2RightX: 0,
		factorTilt2RightY: 0,
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
		focusModePin: -1,
		focusModeButtonLockMask: 0,
		focusModeButtonLockEnabled: 0,
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
		sliderSOCDModeDefault: 1,
		wiiExtensionBlock: 0,
		snesPadClockPin: -1,
		snesPadLatchPin: -1,
		snesPadDataPin: -1,
		keyboardHostMap: DEFAULT_KEYBOARD_MAPPING,
		AnalogInputEnabled: 1,
		BoardLedAddonEnabled: 1,
		FocusModeAddonEnabled: 1,
		focusModeMacroLockEnabled: 0,
		BuzzerSpeakerAddonEnabled: 1,
		BootselButtonAddonEnabled: 1,
		DualDirectionalInputEnabled: 1,
		TiltInputEnabled: 1,
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
		XBOnePassthroughAddonEnabled: 1,
		InputHistoryAddonEnabled: 1,
		inputHistoryLength: 21,
		inputHistoryCol: 0,
		inputHistoryRow: 7,
		usedPins: Object.values(picoController),
	});
});

app.get('/api/getMacroAddonOptions', (req, res) => {
	return res.send({
		macroList: [
			{
				enabled: 1,
				exclusive: 1,
				interruptible: 1,
				showFrames: 1,
				macroType: 1,
				useMacroTriggerButton: 0,
				macroTriggerPin: -1,
				macroTriggerButton: 0,
				macroLabel: 'Shoryuken',
				macroInputs: [
					{ buttonMask: 1 << 19, duration: 16666, waitDuration: 0 },
					{ buttonMask: 1 << 17, duration: 16666, waitDuration: 0 },
					{
						buttonMask: (1 << 17) | (1 << 19) | (1 << 3),
						duration: 16666,
						waitDuration: 0,
					},
				],
			},
		],
		macroPin: -1,
		macroBoardLedEnabled: 1,
		InputMacroAddonEnabled: 1,
	});
});

app.get('/api/getFirmwareVersion', (req, res) => {
	return res.send({
		boardConfigLabel: 'Pico',
		boardConfigFileName: `GP2040_local-dev-server_Pico`,
		boardConfig: 'Pico',
		version: 'local-dev-server',
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

app.get('/api/getHeldPins', async (req, res) => {
	await new Promise((resolve) => setTimeout(resolve, 2000));
	return res.send({
		heldPins: [7],
	});
});

app.get('/api/abortGetHeldPins', async (req, res) => {
	return res.send();
});

app.post('/api/*', (req, res) => {
	console.log(req.body);
	return res.send(req.body);
});

app.listen(port, () => {
	console.log(`Dev app listening at http://localhost:${port}`);
});
