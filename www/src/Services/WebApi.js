import axios from 'axios';
import { hexToInt, rgbIntToHex } from './Utilities';

const baseUrl =
	process.env.NODE_ENV === 'production'
		? ''
		: import.meta.env.VITE_DEV_BASE_URL;

export const baseButtonMappings = {
	Up: { pin: -1, key: 0, error: null },
	Down: { pin: -1, key: 0, error: null },
	Left: { pin: -1, key: 0, error: null },
	Right: { pin: -1, key: 0, error: null },
	B1: { pin: -1, key: 0, error: null },
	B2: { pin: -1, key: 0, error: null },
	B3: { pin: -1, key: 0, error: null },
	B4: { pin: -1, key: 0, error: null },
	L1: { pin: -1, key: 0, error: null },
	R1: { pin: -1, key: 0, error: null },
	L2: { pin: -1, key: 0, error: null },
	R2: { pin: -1, key: 0, error: null },
	S1: { pin: -1, key: 0, error: null },
	S2: { pin: -1, key: 0, error: null },
	L3: { pin: -1, key: 0, error: null },
	R3: { pin: -1, key: 0, error: null },
	A1: { pin: -1, key: 0, error: null },
	A2: { pin: -1, key: 0, error: null },
	Fn: { pin: -1, key: 0, error: null },
};

export const baseProfileOptions = {
	alternativePinMappings: [
		{
			Up: { pin: -1, key: 0, error: null },
			Down: { pin: -1, key: 0, error: null },
			Left: { pin: -1, key: 0, error: null },
			Right: { pin: -1, key: 0, error: null },
			B1: { pin: -1, key: 0, error: null },
			B2: { pin: -1, key: 0, error: null },
			B3: { pin: -1, key: 0, error: null },
			B4: { pin: -1, key: 0, error: null },
			L1: { pin: -1, key: 0, error: null },
			R1: { pin: -1, key: 0, error: null },
			L2: { pin: -1, key: 0, error: null },
			R2: { pin: -1, key: 0, error: null },
		},
		{
			Up: { pin: -1, key: 0, error: null },
			Down: { pin: -1, key: 0, error: null },
			Left: { pin: -1, key: 0, error: null },
			Right: { pin: -1, key: 0, error: null },
			B1: { pin: -1, key: 0, error: null },
			B2: { pin: -1, key: 0, error: null },
			B3: { pin: -1, key: 0, error: null },
			B4: { pin: -1, key: 0, error: null },
			L1: { pin: -1, key: 0, error: null },
			R1: { pin: -1, key: 0, error: null },
			L2: { pin: -1, key: 0, error: null },
			R2: { pin: -1, key: 0, error: null },
		},
		{
			Up: { pin: -1, key: 0, error: null },
			Down: { pin: -1, key: 0, error: null },
			Left: { pin: -1, key: 0, error: null },
			Right: { pin: -1, key: 0, error: null },
			B1: { pin: -1, key: 0, error: null },
			B2: { pin: -1, key: 0, error: null },
			B3: { pin: -1, key: 0, error: null },
			B4: { pin: -1, key: 0, error: null },
			L1: { pin: -1, key: 0, error: null },
			R1: { pin: -1, key: 0, error: null },
			L2: { pin: -1, key: 0, error: null },
			R2: { pin: -1, key: 0, error: null },
		},
	],
};

export const baseWiiControls = {
    "nunchuk.analogStick.axisType": 1,
    "nunchuk.buttonC": 1,
    "nunchuk.buttonZ": 2,
    "classic.analogLeftStick.x.axisType": 1,
    "classic.analogLeftStick.y.axisType": 2,
    "classic.analogRightStick.x.axisType": 3,
    "classic.analogRightStick.y.axisType": 4,
    "classic.analogLeftTrigger.axisType": 7,
    "classic.analogRightTrigger.axisType": 8,
    "classic.buttonA": 2,
    "classic.buttonB": 1,
    "classic.buttonX": 8,
    "classic.buttonY": 4,
    "classic.buttonL": 64,
    "classic.buttonR": 128,
    "classic.buttonZL": 16,
    "classic.buttonZR": 32,
    "classic.buttonMinus": 256,
    "classic.buttonHome": 4096,
    "classic.buttonPlus": 512,
    "classic.buttonUp": 65536,
    "classic.buttonDown": 131072,
    "classic.buttonLeft": 262144,
    "classic.buttonRight": 524288,
    "guitar.analogStick.x.axisType": 1,
    "guitar.analogStick.y.axisType": 2,
    "guitar.analogWhammyBar.axisType": 14,
    "guitar.buttonOrange": 64,
    "guitar.buttonRed": 2,
    "guitar.buttonBlue": 4,
    "guitar.buttonGreen": 1,
    "guitar.buttonYellow": 8,
    "guitar.buttonPedal": 128,
    "guitar.buttonMinus": 256,
    "guitar.buttonPlus": 512,
    "guitar.buttonStrumUp": 65536,
    "guitar.buttonStrumDown": 131072,
    "drum.analogStick.x.axisType": 1,
    "drum.analogStick.y.axisType": 2,
    "drum.buttonOrange": 64,
    "drum.buttonRed": 2,
    "drum.buttonBlue": 8,
    "drum.buttonGreen": 1,
    "drum.buttonYellow": 4,
    "drum.buttonPedal": 128,
    "drum.buttonMinus": 256,
    "drum.buttonPlus": 512,
    "turntable.analogStick.x.axisType": 1,
    "turntable.analogStick.y.axisType": 2,
    "turntable.analogLeftTurntable.axisType": 13,
    "turntable.analogRightTurntable.axisType": 15,
    "turntable.analogFader.axisType": 7,
    "turntable.analogEffects.axisType": 8,
    "turntable.buttonLeftGreen": 262144,
    "turntable.buttonLeftRed": 65536,
    "turntable.buttonLeftBlue": 524288,
    "turntable.buttonRightGreen": 4,
    "turntable.buttonRightRed": 8,
    "turntable.buttonRightBlue": 2,
    "turntable.buttonEuphoria": 32,
    "turntable.buttonMinus": 256,
    "turntable.buttonPlus": 512,
    "taiko.buttonDonLeft": 262144,
    "taiko.buttonKatLeft": 64,
    "taiko.buttonDonRight": 1,
    "taiko.buttonKatRight": 128,
};

async function resetSettings() {
	return axios
		.get(`${baseUrl}/api/resetSettings`)
		.then((response) => response.data)
		.catch(console.error);
}

async function getDisplayOptions() {
	try {
		const response = await axios.get(`${baseUrl}/api/getDisplayOptions`);

		if (response.data.i2cAddress) {
			response.data.i2cAddress = '0x' + response.data.i2cAddress.toString(16);
		}
		response.data.splashDuration = response.data.splashDuration / 1000; // milliseconds to seconds
		response.data.displaySaverTimeout =
			response.data.displaySaverTimeout / 60000; // milliseconds to minutes

		return response.data;
	} catch (error) {
		console.error(error);
	}
}

async function setDisplayOptions(options, isPreview) {
	let newOptions = sanitizeRequest(options);
	newOptions.i2cAddress = parseInt(options.i2cAddress);
	newOptions.buttonLayout = parseInt(options.buttonLayout);
	newOptions.buttonLayoutRight = parseInt(options.buttonLayoutRight);
	newOptions.splashMode = parseInt(options.splashMode);
	newOptions.splashDuration = parseInt(options.splashDuration) * 1000; // seconds to milliseconds
	newOptions.displaySaverTimeout =
		parseInt(options.displaySaverTimeout) * 60000; // minutes to milliseconds
	newOptions.splashChoice = parseInt(options.splashChoice);

	if (newOptions.buttonLayoutCustomOptions) {
		newOptions.buttonLayoutCustomOptions.params.layout = parseInt(
			options.buttonLayoutCustomOptions?.params?.layout,
		);
		newOptions.buttonLayoutCustomOptions.paramsRight.layout = parseInt(
			options.buttonLayoutCustomOptions?.paramsRight?.layout,
		);
	}

	delete newOptions.splashImage;
	const url = !isPreview
		? `${baseUrl}/api/setDisplayOptions`
		: `${baseUrl}/api/setPreviewDisplayOptions`;
	return axios
		.post(url, newOptions)
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getSplashImage() {
	try {
		const response = await axios.get(`${baseUrl}/api/getSplashImage`);
		return response.data;
	} catch (error) {
		console.error(error);
	}
}

async function setSplashImage({ splashImage }) {
	return axios
		.post(`${baseUrl}/api/setSplashImage`, {
			splashImage: btoa(
				String.fromCharCode.apply(null, new Uint8Array(splashImage)),
			),
		})
		.then((response) => {
			return response.data;
		})
		.catch(console.error);
}

async function getGamepadOptions(setLoading) {
	setLoading(true);

	try {
		const response = await axios.get(`${baseUrl}/api/getGamepadOptions`);
		setLoading(false);
		return response.data;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function setGamepadOptions(options) {
	return axios
		.post(`${baseUrl}/api/setGamepadOptions`, sanitizeRequest(options))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getLedOptions(setLoading) {
	setLoading(true);

	try {
		const response = await axios.get(`${baseUrl}/api/getLedOptions`);
		setLoading(false);

		response.data.pledColor = rgbIntToHex(response.data.pledColor) || '#ffffff';
		if (response.data.pledType === 1) {
			response.data.pledIndex1 = response.data.pledPin1;
			response.data.pledIndex2 = response.data.pledPin2;
			response.data.pledIndex3 = response.data.pledPin3;
			response.data.pledIndex4 = response.data.pledPin4;
		}

		return response.data;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function setLedOptions(options) {
	let data = sanitizeRequest(options);

	return axios
		.post(`${baseUrl}/api/setLedOptions`, sanitizeRequest(options))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getCustomTheme(setLoading) {
	setLoading(true);

	try {
		const response = await axios.get(`${baseUrl}/api/getCustomTheme`);
		setLoading(false);

		let data = { hasCustomTheme: response.data.enabled, customTheme: {} };

		// Transform ARGB int value to hex for easy use on frontend
		Object.keys(response.data)
			.filter((p) => p !== 'enabled')
			.forEach((button) => {
				data.customTheme[button] = {
					normal: rgbIntToHex(response.data[button].u),
					pressed: rgbIntToHex(response.data[button].d),
				};
			});

		console.log(data);
		return data;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function setCustomTheme(customThemeOptions) {
	let options = { enabled: customThemeOptions.hasCustomTheme };

	// Transform RGB hex values to ARGB int before sending back to API
	Object.keys(customThemeOptions.customTheme).forEach((p) => {
		options[p] = {
			u: hexToInt(customThemeOptions.customTheme[p].normal.replace('#', '')),
			d: hexToInt(customThemeOptions.customTheme[p].pressed.replace('#', '')),
		};
	});

	return axios
		.post(`${baseUrl}/api/setCustomTheme`, sanitizeRequest(options))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getPinMappings(setLoading) {
	setLoading(true);

	try {
		const response = await axios.get(`${baseUrl}/api/getPinMappings`);
		let mappings = { ...baseButtonMappings };
		for (let prop of Object.keys(response.data))
			mappings[prop].pin = parseInt(response.data[prop]);

		return mappings;
	} catch (error) {
		console.error(error);
		return false;
	}
}

async function setPinMappings(mappings) {
	let data = {};
	Object.keys(mappings).map(
		(button, i) => (data[button] = mappings[button].pin),
	);

	return axios
		.post(`${baseUrl}/api/setPinMappings`, sanitizeRequest(data))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getProfileOptions(setLoading) {
	setLoading(true);

	try {
		const response = await axios.get(`${baseUrl}/api/getProfileOptions`);
		let profileOptions = { ...baseProfileOptions };
		response.data['alternativePinMappings'].forEach((altButtons, index) => {
			for (let prop of Object.keys(altButtons))
				profileOptions['alternativePinMappings'][index][prop].pin = parseInt(
					response.data['alternativePinMappings'][index][prop],
				);
		});
		setLoading(false);
		return profileOptions;
	} catch (error) {
		console.error(error);
		return false;
	}
}

async function setProfileOptions(options) {
	let data = {};
	data['alternativePinMappings'] = [];
	options['alternativePinMappings'].forEach((altButtons, index) => {
		let altMapping = {};
		Object.keys(options['alternativePinMappings'][index]).map(
			(button, i) => (altMapping[button] = altButtons[button].pin),
		);
		data['alternativePinMappings'].push(altMapping);
	});

	return axios
		.post(`${baseUrl}/api/setProfileOptions`, sanitizeRequest(data))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getKeyMappings(setLoading) {
	setLoading(true);

	try {
		const response = await axios.get(`${baseUrl}/api/getKeyMappings`);
		setLoading(false);

		let mappings = { ...baseButtonMappings };
		for (let prop of Object.keys(response.data))
			mappings[prop].key = parseInt(response.data[prop]);

		return mappings;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function setKeyMappings(mappings) {
	let data = {};
	Object.keys(mappings).map(
		(button, i) => (data[button] = mappings[button].key),
	);

	return axios
		.post(`${baseUrl}/api/setKeyMappings`, sanitizeRequest(data))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getAddonsOptions(setLoading) {
	setLoading(true);

	try {
		const response = await axios.get(`${baseUrl}/api/getAddonsOptions`);
		const data = response.data;
		setLoading(false);

		let mappings = { ...baseButtonMappings };
		for (let prop of Object.keys(data.keyboardHostMap))
			mappings[prop].key = parseInt(data.keyboardHostMap[prop]);
		data.keyboardHostMap = mappings;
		return data;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function setAddonsOptions(options) {
	if (options.keyboardHostMap) {
		let data = {};
		Object.keys(options.keyboardHostMap).map(
			(button, i) => (data[button] = options.keyboardHostMap[button].key),
		);
		options.keyboardHostMap = data;
	}

	return axios
		.post(`${baseUrl}/api/setAddonsOptions`, sanitizeRequest(options))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getMacroAddonOptions(setLoading) {
	setLoading(true);

	try {
		const response = await axios.get(`${baseUrl}/api/getMacroAddonOptions`);
		const data = response.data;
		setLoading(false);

		return data;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function setMacroAddonOptions(options) {
	return axios
		.post(`${baseUrl}/api/setMacroAddonOptions`, sanitizeRequest(options))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function setPS4Options(options) {
	return axios
		.post(`${baseUrl}/api/setPS4Options`, options)
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getWiiControls(setLoading) {
	setLoading(true);

    try {
		const response = await axios.get(`${baseUrl}/api/getWiiControls`);
		setLoading(false);

		let mappings = { ...baseWiiControls, ...response.data };
		return mappings;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function setWiiControls(mappings) {
    console.dir(mappings);

	return axios
		.post(`${baseUrl}/api/setWiiControls`, sanitizeRequest(mappings))
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getFirmwareVersion(setLoading) {
	setLoading(true);

	try {
		const response = await axios.get(`${baseUrl}/api/getFirmwareVersion`);
		setLoading(false);
		return response.data;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function getMemoryReport(setLoading) {
	setLoading(true);

	try {
		const response = await axios.get(`${baseUrl}/api/getMemoryReport`);
		setLoading(false);
		return response.data;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function getUsedPins(setLoading) {
	setLoading(true);

	try {
		const response = await axios.get(`${baseUrl}/api/getUsedPins`);
		setLoading(false);
		return response.data;
	} catch (error) {
		setLoading(false);
		console.error(error);
	}
}

async function getHeldPins(setLoading, createAbortSignal) {
	setLoading && setLoading(true);

	try {
		const response = await axios.get(`${baseUrl}/api/getHeldPins`, {
			signal: createAbortSignal(),
		});
		setLoading && setLoading(false);
		return response.data;
	} catch (error) {
		setLoading && setLoading(false);
		if (error?.code === 'ERR_CANCELED') return { canceled: true };
		else console.error(error);
	}
}

async function abortGetHeldPins() {
	try {
		await axios.get(`${baseUrl}/api/abortGetHeldPins`);
	} catch (error) {
		// Expected to fail
	}
}

async function reboot(bootMode) {
	return axios
		.post(`${baseUrl}/api/reboot`, { bootMode })
		.then((response) => response.data)
		.catch(console.error);
}

function sanitizeRequest(request) {
	const newRequest = { ...request };
	delete newRequest.pledIndex1;
	delete newRequest.pledIndex2;
	delete newRequest.pledIndex3;
	delete newRequest.pledIndex4;
	delete newRequest.usedPins;
	return newRequest;
}

const WebApi = {
	resetSettings,
	getDisplayOptions,
	setDisplayOptions,
	getGamepadOptions,
	setGamepadOptions,
	getLedOptions,
	setLedOptions,
	getCustomTheme,
	setCustomTheme,
	getPinMappings,
	setPinMappings,
	getProfileOptions,
	setProfileOptions,
	getKeyMappings,
	setKeyMappings,
	getAddonsOptions,
	setAddonsOptions,
	getMacroAddonOptions,
	setMacroAddonOptions,
	setPS4Options,
	getWiiControls,
	setWiiControls,
	getSplashImage,
	setSplashImage,
	getFirmwareVersion,
	getMemoryReport,
	getUsedPins,
	getHeldPins,
	abortGetHeldPins,
	reboot,
};

export default WebApi;
