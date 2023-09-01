import React, { useContext, useEffect, useState } from 'react';
import { Button, Form, Row, FormCheck } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import * as yup from 'yup';
import { Trans, useTranslation } from 'react-i18next';
import JSEncrypt from 'jsencrypt';
import CryptoJS from 'crypto-js';
import get from 'lodash/get';
import set from 'lodash/set';
import isNil from 'lodash/isNil';

import { AppContext } from '../Contexts/AppContext';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import KeyboardMapper, { validateMappings } from '../Components/KeyboardMapper';
import Section from '../Components/Section';
import WebApi, { baseButtonMappings } from '../Services/WebApi';
import { BUTTON_MASKS } from '../Data/Buttons';

import Bootsel, { bootselScheme, bootselState } from '../Addons/Bootsel';
import OnBoardLed, {
	onBoardLedScheme,
	onBoardLedState,
} from '../Addons/OnBoardLed';
import Analog, { analogScheme, analogState } from '../Addons/Analog';
import Turbo, { turboScheme, turboState } from '../Addons/Turbo';
import { I2C_BLOCKS, SOCD_MODES } from '../Data/Addons';
import Joystick, { joystickScheme, joystickState } from '../Addons/Joystick';
import Reverse, { reverseScheme, reverseState } from '../Addons/Reverse';
import I2c, { i2cScheme, i2cState } from '../Addons/I2c';
import DualDirection, {
	dualDirectionScheme,
	dualDirectionState,
} from '../Addons/DualDirection';
import Tilt, { tiltScheme, tiltState } from '../Addons/Tilt';
import Buzzer, { buzzerScheme, buzzerState } from '../Addons/Buzzer';
import ExtraButton, {
	extraButtonScheme,
	extraButtonState,
} from '../Addons/ExtraButton';
import PlayerNumber, {
	playerNumberScheme,
	playerNumberState,
} from '../Addons/PlayerNumber';
import SOCD, { socdScheme, socdState } from '../Addons/SOCD';

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

const schema = yup.object().shape({
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

	PS4ModeAddonEnabled: yup.number().required().label('PS4 Mode Add-on Enabled'),

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
	...analogScheme,
	...bootselScheme,
	...onBoardLedScheme,
	...turboScheme,
	...joystickScheme,
	...reverseScheme,
	...i2cScheme,
	...dualDirectionScheme,
	...tiltScheme,
	...buzzerScheme,
	...extraButtonScheme,
	...playerNumberScheme,
	...socdScheme,
});

const defaultValues = {
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
	FocusModeAddonEnabled: 0,
	focusModeOledLockEnabled: 0,
	focusModeRgbLockEnabled: 0,

	KeyboardHostAddonEnabled: 0,

	PS4ModeAddonEnabled: 0,

	WiiExtensionAddonEnabled: 0,
	SNESpadAddonEnabled: 0,
	...analogState,
	...bootselState,
	...onBoardLedState,
	...turboState,
	...joystickState,
	...reverseState,
	...i2cState,
	...dualDirectionState,
	...tiltState,
	...buzzerState,
	...extraButtonState,
	...playerNumberState,
	...socdState,
};

const ADDONS = [
	Bootsel,
	OnBoardLed,
	Analog,
	Turbo,
	Joystick,
	Reverse,
	I2c,
	DualDirection,
	Tilt,
	Buzzer,
	ExtraButton,
	PlayerNumber,
	SOCD,
];

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

export default function AddonsConfigPage() {
	const { buttonLabels, updateUsedPins } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');
	const [storedData, setStoredData] = useState({});
	const [validated, setValidated] = useState(false);

	const { t } = useTranslation();

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
					<h1>{t('AddonsConfig:header-text')}</h1>
					<p>{t('AddonsConfig:sub-header-text')}</p>
					{ADDONS.map((Addon, index) => (
						<Addon
							key={`addon-${index}`}
							values={values}
							errors={errors}
							handleChange={handleChange}
							handleCheckbox={handleCheckbox}
						/>
					))}

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
												key={`focusModeButtonLockMask-${mask.label}`}
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
