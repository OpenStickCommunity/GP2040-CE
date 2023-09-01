import React, { useContext, useEffect, useState } from 'react';
import { Button, Form, Row, FormCheck } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import * as yup from 'yup';
import { Trans, useTranslation } from 'react-i18next';

import get from 'lodash/get';
import set from 'lodash/set';
import isNil from 'lodash/isNil';

import { AppContext } from '../Contexts/AppContext';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import KeyboardMapper, { validateMappings } from '../Components/KeyboardMapper';
import Section from '../Components/Section';

import WebApi, { baseButtonMappings } from '../Services/WebApi';
import Analog, { analogScheme, analogState } from '../Addons/Analog';
import Bootsel, { bootselScheme, bootselState } from '../Addons/Bootsel';
import Buzzer, { buzzerScheme, buzzerState } from '../Addons/Buzzer';
import DualDirection, {
	dualDirectionScheme,
	dualDirectionState,
} from '../Addons/DualDirection';
import ExtraButton, {
	extraButtonScheme,
	extraButtonState,
} from '../Addons/ExtraButton';
import I2c, { i2cScheme, i2cState } from '../Addons/I2c';
import Joystick, { joystickScheme, joystickState } from '../Addons/Joystick';
import OnBoardLed, {
	onBoardLedScheme,
	onBoardLedState,
} from '../Addons/OnboardLed';
import PlayerNumber, {
	playerNumberScheme,
	playerNumberState,
} from '../Addons/PlayerNumber';
import Reverse, { reverseScheme, reverseState } from '../Addons/Reverse';
import SOCD, { socdScheme, socdState } from '../Addons/SOCD';
import Tilt, { tiltScheme, tiltState } from '../Addons/Tilt';
import Turbo, { turboScheme, turboState } from '../Addons/Turbo';
import { BUTTON_MASKS } from '../Data/Buttons';
import { I2C_BLOCKS } from '../Data/Addons';
import Ps4, { ps4Scheme, ps4State } from '../Addons/Ps4';

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
	...ps4Scheme,
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
	...ps4State,
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
	Ps4,
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
