import axios from 'axios';
import { chunk } from 'lodash';

const baseUrl = process.env.NODE_ENV === 'production' ? '' : 'http://localhost:8080';

export const baseButtonMappings = {
	Up:    { pin: -1, error: null },
	Down:  { pin: -1, error: null },
	Left:  { pin: -1, error: null },
	Right: { pin: -1, error: null },
	B1:    { pin: -1, error: null },
	B2:    { pin: -1, error: null },
	B3:    { pin: -1, error: null },
	B4:    { pin: -1, error: null },
	L1:    { pin: -1, error: null },
	R1:    { pin: -1, error: null },
	L2:    { pin: -1, error: null },
	R2:    { pin: -1, error: null },
	S1:    { pin: -1, error: null },
	S2:    { pin: -1, error: null },
	L3:    { pin: -1, error: null },
	R3:    { pin: -1, error: null },
	A1:    { pin: -1, error: null },
	A2:    { pin: -1, error: null },
};

async function resetSettings() {
	return axios.get(`${baseUrl}/api/resetSettings`)
		.then((response) => response.data)
		.catch(console.error);
}

async function getDisplayOptions() {
	return axios.get(`${baseUrl}/api/getDisplayOptions`)
		.then((response) => {
			if (response.data.i2cAddress)
				response.data.i2cAddress = '0x' + response.data.i2cAddress.toString(16);

			return response.data;
		})
		.catch(console.error);
}

async function setDisplayOptions(options) {
	let newOptions = { ...options };
	newOptions.i2cAddress = parseInt(options.i2cAddress);
	newOptions.buttonLayout = parseInt(options.buttonLayout);
	newOptions.buttonLayoutRight = parseInt(options.buttonLayoutRight);
	newOptions.splashMode = parseInt(options.splashMode);
	newOptions.splashChoice = parseInt(options.splashChoice);
	newOptions.splashImage = '';
	return axios.post(`${baseUrl}/api/setDisplayOptions`, newOptions)
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
	return axios.get(`${baseUrl}/api/getSplashImage`)
		.then((response) => {
			return response.data;
		})
		.catch(console.error);
}

async function setSplashImage({splashImage}) {
	return await chunk(splashImage, 64).reduce(async (acc, chunk, index) => {
		return axios.post(`${baseUrl}/api/setSplashImage`, { splashImage: chunk, index})
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		}).then(acc);
	}, Promise.resolve({splashImage}));
}

async function getBuzzerOptions() {
	return axios.get(`${baseUrl}/api/getBuzzerOptions`)
		.then((response) => {

			return response.data;
		})
		.catch(console.error);
}

async function setBuzzerOptions(options) {
	let newOptions = { ...options };
	newOptions.buzzerPin = parseInt(options.buzzerPin);
	newOptions.buzzerVolume = parseInt(options.buzzerVolume);
	return axios.post(`${baseUrl}/api/setBuzzerOptions`, newOptions)
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getGamepadOptions() {
	return axios.get(`${baseUrl}/api/getGamepadOptions`)
		.then((response) => response.data)
		.catch(console.error);
}

async function setGamepadOptions(options) {
	return axios.post(`${baseUrl}/api/setGamepadOptions`, options)
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getLedOptions() {
	return axios.get(`${baseUrl}/api/getLedOptions`)
		.then((response) => response.data)
		.catch(console.error);
}

async function setLedOptions(options) {
	return axios.post(`${baseUrl}/api/setLedOptions`, options)
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getPinMappings() {
	return axios.get(`${baseUrl}/api/getPinMappings`)
		.then((response) => {
			let mappings = { ...baseButtonMappings };
			for (let prop of Object.keys(response.data))
				mappings[prop].pin = parseInt(response.data[prop]);

			return mappings;
		})
		.catch(console.error);
}

async function setPinMappings(mappings) {
	let data = {};
	Object.keys(mappings).map((button, i) => data[button] = mappings[button].pin);

	return axios.post(`${baseUrl}/api/setPinMappings`, data)
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getAddonsOptions() {
	return axios.get(`${baseUrl}/api/getAddonsOptions`)
		.then((response) => response.data)
		.catch(console.error);
}

async function setAddonsOptions(options) {
	return axios.post(`${baseUrl}/api/setAddonsOptions`, options)
		.then((response) => {
			console.log(response.data);
			return true;
		})
		.catch((err) => {
			console.error(err);
			return false;
		});
}

async function getFirmwareVersion() {
	return axios.get(`${baseUrl}/api/getFirmwareVersion`)
		.then((response) => response.data)
		.catch(console.error);
}

const WebApi = {
	resetSettings,
	getDisplayOptions,
	setDisplayOptions,
	getGamepadOptions,
	setGamepadOptions,
	getLedOptions,
	setLedOptions,
	getPinMappings,
	setPinMappings,
	getAddonsOptions,
	setAddonsOptions,
	getSplashImage,
	setSplashImage,
	getBuzzerOptions,
	setBuzzerOptions,
	getFirmwareVersion
};

export default WebApi;
