import React, { useContext, useEffect, useState } from 'react';
import { AppContext } from '../Contexts/AppContext';
import { Button, Form, Modal } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import { NavLink } from 'react-router-dom';
import * as yup from 'yup';
import { Trans, useTranslation } from 'react-i18next';

import Section from '../Components/Section';
import WebApi from '../Services/WebApi';
import { BUTTON_MASKS, getButtonLabels} from '../Data/Buttons';

const PS4Mode = 4;
const INPUT_MODES = [
	{ labelKey: 'input-mode-options.xinput', value: 0 },
	{ labelKey: 'input-mode-options.nintendo-switch', value: 1 },
	{ labelKey: 'input-mode-options.ps3', value: 2 },
	{ labelKey: 'input-mode-options.keyboard', value: 3 },
	{ labelKey: 'input-mode-options.ps4', value: PS4Mode },
];

const DPAD_MODES = [
	{ labelKey: 'd-pad-mode-options.d-pad', value: 0 },
	{ labelKey: 'd-pad-mode-options.left-analog', value: 1 },
	{ labelKey: 'd-pad-mode-options.right-analog', value: 2 },
];

const SOCD_MODES = [
	{ labelKey: 'socd-cleaning-mode-options.up-priority', value: 0 },
	{ labelKey: 'socd-cleaning-mode-options.neutral', value: 1 },
	{ labelKey: 'socd-cleaning-mode-options.last-win', value: 2 },
	{ labelKey: 'socd-cleaning-mode-options.first-win', value: 3 },
	{ labelKey: 'socd-cleaning-mode-options.off', value: 4 },
];

const PS4_MODES = [
	{ labelKey: 'ps4-mode-options.controller', value: 0 },
	{ labelKey: 'ps4-mode-options.arcadestick', value: 7 },
];

const HOTKEY_ACTIONS = [
	{ labelKey: 'hotkey-actions.no-action', value: 0 },
	{ labelKey: 'hotkey-actions.dpad-digital', value: 1 },
	{ labelKey: 'hotkey-actions.dpad-left-analog', value: 2 },
	{ labelKey: 'hotkey-actions.dpad-right-analog', value: 3 },
	{ labelKey: 'hotkey-actions.home-button', value: 4 },
	{ labelKey: 'hotkey-actions.capture-button', value: 5 },
	{ labelKey: 'hotkey-actions.socd-up-priority', value: 6 },
	{ labelKey: 'hotkey-actions.socd-neutral', value: 7 },
	{ labelKey: 'hotkey-actions.socd-last-win', value: 8 },
	{ labelKey: 'hotkey-actions.socd-first-win', value: 11 },
	{ labelKey: 'hotkey-actions.socd-off', value: 12 },
	{ labelKey: 'hotkey-actions.invert-x', value: 9 },
	{ labelKey: 'hotkey-actions.invert-y', value: 10 },
	{ labelKey: 'hotkey-actions.toggle-4way-joystick-mode', value: 13 },
	{ labelKey: 'hotkey-actions.toggle-ddi-4way-joystick-mode', value: 14 },
	{ labelKey: 'hotkey-actions.load-profile-1', value: 15 },
	{ labelKey: 'hotkey-actions.load-profile-2', value: 16 },
	{ labelKey: 'hotkey-actions.load-profile-3', value: 17 },
	{ labelKey: 'hotkey-actions.load-profile-4', value: 18 },
	{ labelKey: 'hotkey-actions.l3-button', value: 19 },
	{ labelKey: 'hotkey-actions.r3-button', value: 20 },
	{ labelKey: 'hotkey-actions.touchpad-button', value: 21 },
	{ labelKey: 'hotkey-actions.reboot-default', value: 22 },
];

const FORCED_SETUP_MODES = [
	{ labelKey: 'forced-setup-mode-options.off', value: 0 },
	{ labelKey: 'forced-setup-mode-options.disable-input-mode', value: 1 },
	{ labelKey: 'forced-setup-mode-options.disable-web-config', value: 2 },
	{ labelKey: 'forced-setup-mode-options.disable-both', value: 3 },
];

const hotkeySchema = {
	action: yup
		.number()
		.required()
		.oneOf(HOTKEY_ACTIONS.map((o) => o.value))
		.label('Hotkey Action'),
	buttonsMask: yup.number().required().label('Button Mask'),
	auxMask: yup.number().required().label('Function Key'),
};

const hotkeyFields = Array(12)
	.fill(0)
	.reduce((acc, a, i) => {
		const number = String(i + 1).padStart(2, '0');
		const newSchema = yup
			.object()
			.label('Hotkey ' + number)
			.shape({ ...hotkeySchema });
		acc['hotkey' + number] = newSchema;
		return acc;
	}, {});

const schema = yup.object().shape({
	dpadMode: yup
		.number()
		.required()
		.oneOf(DPAD_MODES.map((o) => o.value))
		.label('D-Pad Mode'),
	...hotkeyFields,
	inputMode: yup
		.number()
		.required()
		.oneOf(INPUT_MODES.map((o) => o.value))
		.label('Input Mode'),
	socdMode: yup
		.number()
		.required()
		.oneOf(SOCD_MODES.map((o) => o.value))
		.label('SOCD Cleaning Mode'),
	switchTpShareForDs4: yup
		.number()
		.required()
		.label('Switch Touchpad and Share'),
	forcedSetupMode: yup
		.number()
		.required()
		.oneOf(FORCED_SETUP_MODES.map((o) => o.value))
		.label('SOCD Cleaning Mode'),
	lockHotkeys: yup.number().required().label('Lock Hotkeys'),
	fourWayMode: yup.number().required().label('4-Way Joystick Mode'),
	profileNumber: yup.number().required().label('Profile Number'),
	ps4ControllerType: yup
		.number()
		.required()
		.oneOf(PS4_MODES.map((o) => o.value))
		.label('PS4 Controller Type'),
});

const FormContext = ({ setButtonLabels }) => {
	const { values, setValues } = useFormikContext();
	const { setLoading } = useContext(AppContext);

	useEffect(() => {
		async function fetchData() {
			const options = await WebApi.getGamepadOptions(setLoading);
			setValues(options);
			setButtonLabels({
				swapTpShareLabels:
					options.switchTpShareForDs4 === 1 && options.inputMode === 4,
			});
		}
		fetchData();
	}, [setValues]);

	useEffect(() => {
		if (!!values.dpadMode) values.dpadMode = parseInt(values.dpadMode);
		if (!!values.inputMode) values.inputMode = parseInt(values.inputMode);
		if (!!values.socdMode) values.socdMode = parseInt(values.socdMode);
		if (!!values.switchTpShareForDs4)
			values.switchTpShareForDs4 = parseInt(values.switchTpShareForDs4);
		if (!!values.forcedSetupMode)
			values.forcedSetupMode = parseInt(values.forcedSetupMode);
		if (!!values.lockHotkeys) values.lockHotkeys = parseInt(values.lockHotkeys);
		if (!!values.fourWayMode) values.fourWayMode = parseInt(values.fourWayMode);
		if (!!values.profileNumber)
			values.profileNumber = parseInt(values.profileNumber);
		if (!!values.ps4ControllerType)
			values.ps4ControllerType = parseInt(values.ps4ControllerType);

		setButtonLabels({
			swapTpShareLabels:
				values.switchTpShareForDs4 === 1 && values.inputMode === 4,
		});

		Object.keys(hotkeyFields).forEach((a) => {
			const value = values[a];
			if (value) {
				values[a] = {
					action: parseInt(value.action),
					buttonsMask: parseInt(value.buttonsMask),
					auxMask: parseInt(value.auxMask),
				};
			}
		});
	}, [values, setValues]);

	return null;
};

export default function SettingsPage() {
	const { buttonLabels, setButtonLabels } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');
	const [warning, setWarning] = useState({ show: false, acceptText: '' });

	const WARNING_CHECK_TEXT = 'GP2040-CE';

	const handleWarningClose = async (accepted, values, setFieldValue) => {
		setWarning({ show: false, acceptText: '' });
		if (accepted) await saveSettings(values);
		else setFieldValue('forcedSetupMode', 0);
	};

	const setWarningAcceptText = (e) => {
		setWarning({ ...warning, acceptText: e.target.value });
	};

	const saveSettings = async (values) => {
		const success = await WebApi.setGamepadOptions(values);
		setSaveMessage(
			success
				? t('Common:saved-success-message')
				: t('Common:saved-error-message'),
		);
	};

	const onSuccess = async (values) => {
		if (values.forcedSetupMode > 1) {
			setWarning({ show: true, acceptText: '' });
		} else {
			await saveSettings(values);
		}
	};

	const translateArray = (array) => {
		return array.map(({ labelKey, value }) => {
			return { label: t(`SettingsPage:${labelKey}`), value };
		});
	};

	const { buttonLabelType, swapTpShareLabels } = buttonLabels;

	const currentButtonLabels= getButtonLabels(buttonLabelType, swapTpShareLabels);

	const { t } = useTranslation('');

	const translatedInputModes = translateArray(INPUT_MODES);
	const translatedDpadModes = translateArray(DPAD_MODES);
	const translatedSocdModes = translateArray(SOCD_MODES);
	const translatedHotkeyActions = translateArray(HOTKEY_ACTIONS);
	const translatedForcedSetupModes = translateArray(FORCED_SETUP_MODES);
	const translatedPS4ControllerTypeModes = translateArray(PS4_MODES);

	return (
		<Formik validationSchema={schema} onSubmit={onSuccess} initialValues={{}}>
			{({ handleSubmit, handleChange, values, errors, setFieldValue }) =>
				console.log('errors', errors) || (
					<div>
						<Form noValidate onSubmit={handleSubmit}>
							<Section title={t('SettingsPage:settings-header-text')}>
								<Form.Group className="row mb-3">
									<Form.Label>{t('SettingsPage:input-mode-label')}</Form.Label>
									<div className="col-sm-3">
										<Form.Select
											name="inputMode"
											className="form-select-sm"
											value={values.inputMode}
											onChange={handleChange}
											isInvalid={errors.inputMode}
										>
											{translatedInputModes.map((o, i) => (
												<option
													key={`button-inputMode-option-${i}`}
													value={o.value}
												>
													{o.label}
												</option>
											))}
										</Form.Select>
										<Form.Control.Feedback type="invalid">
											{errors.inputMode}
										</Form.Control.Feedback>
									</div>
									{values.inputMode === PS4Mode && (
										<div className="col-sm-3">
											<Form.Check
												label={t('SettingsPage:input-mode-extra-label')}
												type="switch"
												name="switchTpShareForDs4"
												isInvalid={false}
												checked={Boolean(values.switchTpShareForDs4)}
												onChange={(e) => {
													setFieldValue(
														'switchTpShareForDs4',
														e.target.checked ? 1 : 0,
													);
												}}
											/>
										</div>
									)}
									{values.inputMode === PS4Mode && (
										<div className="col-sm-3">
											<Form.Select
												name="ps4ControllerType"
												className="form-select-sm"
												value={values.ps4ControllerType}
												onChange={handleChange}
												isInvalid={errors.ps4ControllerType}
											>
												{translatedPS4ControllerTypeModes.map((o, i) => (
													<option
														key={`button-ps4ControllerType-option-${i}`}
														value={o.value}
													>
														{o.label}
													</option>
												))}
											</Form.Select>
										</div>
									)}
									{values.inputMode === PS4Mode && (
										<div className="mb-3">
											<Trans
												ns="SettingsPage"
												i18nKey="ps4-compatibility-label"
											>
												For <strong>PS5 compatibility</strong>, use "Arcade
												Stick" and enable PS Passthrough add-on
												<br />
												For <strong>PS4 support</strong>, use "Controller" and
												enable PS4 Mode add-on if you have the necessary files
											</Trans>
										</div>
									)}
								</Form.Group>
								<Form.Group className="row mb-3">
									<Form.Label>{t('SettingsPage:d-pad-mode-label')}</Form.Label>
									<div className="col-sm-3">
										<Form.Select
											name="dpadMode"
											className="form-select-sm"
											value={values.dpadMode}
											onChange={handleChange}
											isInvalid={errors.dpadMode}
										>
											{translatedDpadModes.map((o, i) => (
												<option
													key={`button-dpadMode-option-${i}`}
													value={o.value}
												>
													{o.label}
												</option>
											))}
										</Form.Select>
										<Form.Control.Feedback type="invalid">
											{errors.dpadMode}
										</Form.Control.Feedback>
									</div>
								</Form.Group>
								<Form.Group className="row mb-3">
									<Form.Label>
										{t('SettingsPage:socd-cleaning-mode-label')}
									</Form.Label>
									<div className="col-sm-3">
										<Form.Select
											name="socdMode"
											className="form-select-sm"
											value={values.socdMode}
											onChange={handleChange}
											isInvalid={errors.socdMode}
										>
											{translatedSocdModes.map((o, i) => (
												<option
													key={`button-socdMode-option-${i}`}
													value={o.value}
												>
													{o.label}
												</option>
											))}
										</Form.Select>
										<Form.Control.Feedback type="invalid">
											{errors.socdMode}
										</Form.Control.Feedback>
									</div>
								</Form.Group>
								<p>{t('SettingsPage:socd-cleaning-mode-note')}</p>
								<Form.Group className="row mb-3">
									<Form.Label>
										{t('SettingsPage:forced-setup-mode-label')}
									</Form.Label>
									<div className="col-sm-3">
										<Form.Select
											name="forcedSetupMode"
											className="form-select-sm"
											value={values.forcedSetupMode}
											onChange={handleChange}
											isInvalid={errors.forcedSetupMode}
										>
											{translatedForcedSetupModes.map((o, i) => (
												<option
													key={`button-forcedSetupMode-option-${i}`}
													value={o.value}
												>
													{o.label}
												</option>
											))}
										</Form.Select>
										<Form.Control.Feedback type="invalid">
											{errors.forcedSetupMode}
										</Form.Control.Feedback>
									</div>
								</Form.Group>
								<div className="row mb-3">
									<div className="col-sm-3">
										<Form.Check
											label={t('SettingsPage:4-way-joystick-mode-label')}
											type="switch"
											id="fourWayMode"
											isInvalid={false}
											checked={Boolean(values.fourWayMode)}
											onChange={(e) => {
												setFieldValue('fourWayMode', e.target.checked ? 1 : 0);
											}}
										/>
									</div>
								</div>
								<Form.Group className="row mb-3">
									<Form.Label>
										{t('SettingsPage:profile-number-label')}
									</Form.Label>
									<div className="col-sm-3">
										<Form.Select
											name="profileNumber"
											className="form-select-sm"
											value={values.profileNumber}
											onChange={handleChange}
											isInvalid={errors.profileNumber}
										>
											{[1, 2, 3, 4].map((i) => (
												<option
													key={`button-profileNumber-option-${i}`}
													value={i}
												>
													{i}
												</option>
											))}
										</Form.Select>
									</div>
								</Form.Group>
							</Section>
							<Section title={t('SettingsPage:hotkey-settings-label')}>
								<div className="mb-3">
									<Trans ns="SettingsPage" i18nKey="hotkey-settings-sub-header">
										The <strong>Fn</strong> slider provides a mappable Function
										button in the{' '}
										<NavLink exact="true" to="/pin-mapping">
											Pin Mapping
										</NavLink>{' '}
										page. By selecting the Fn slider option, the Function button
										must be held along with the selected hotkey settings.
										<br />
										Additionally, select <strong>None</strong> from the dropdown
										to unassign any button.
									</Trans>
								</div>
								{values.fnButtonPin === -1 && (
									<div className="alert alert-warning">
										{t('SettingsPage:hotkey-settings-warning')}
									</div>
								)}
								<div id="Hotkeys" hidden={values.lockHotkeys}>
									{Object.keys(hotkeyFields).map((o, i) => (
										<Form.Group key={`hotkey-${i}`} className="row mb-3">
											<div className="col-sm-auto">
												<Form.Check
													name={`${o}.auxMask`}
													label="&nbsp;&nbsp;Fn"
													type="switch"
													className="form-select-sm"
													disabled={values.fnButtonPin === -1}
													checked={values[o] && !!values[o]?.auxMask}
													onChange={(e) => {
														setFieldValue(
															`${o}.auxMask`,
															e.target.checked ? 32768 : 0,
														);
													}}
													isInvalid={errors[o] && errors[o]?.auxMask}
												/>
												<Form.Control.Feedback type="invalid">
													{errors[o] && errors[o]?.action}
												</Form.Control.Feedback>
											</div>
											<span className="col-sm-auto">+</span>
											{BUTTON_MASKS.map((mask) =>
												values[o] && values[o]?.buttonsMask & mask.value ? (
													[
														<div className="col-sm-auto">
															<Form.Select
																name={`${o}.buttonsMask`}
																className="form-select-sm sm-1"
																groupClassName="mb-3"
																value={
																	values[o] &&
																	values[o]?.buttonsMask & mask.value
																}
																error={errors[o] && errors[o]?.buttonsMask}
																isInvalid={errors[o] && errors[o]?.buttonsMask}
																onChange={(e) => {
																	setFieldValue(
																		`${o}.buttonsMask`,
																		(values[o] &&
																			values[o]?.buttonsMask ^ mask.value) |
																			e.target.value,
																	);
																}}
															>
																{BUTTON_MASKS.map((o, i2) => (
																	<option
																		key={`hotkey-${i}-button${i2}`}
																		value={o.value}
																	>
																		{ (o.label in currentButtonLabels)? currentButtonLabels[o.label]:o.label}
																	</option>
																))}
															</Form.Select>
														</div>,
														<span className="col-sm-auto">+</span>,
													]
												) : (
													<></>
												),
											)}
											<div className="col-sm-auto">
												<Form.Select
													name={`${o}.buttonsMask`}
													className="form-select-sm sm-1"
													groupClassName="mb-3"
													value={0}
													onChange={(e) => {
														setFieldValue(
															`${o}.buttonsMask`,
															(values[o] && values[o]?.buttonsMask) |
																e.target.value,
														);
													}}
												>
													{BUTTON_MASKS.map((o, i2) => (
														<option
															key={`hotkey-${i}-buttonZero-${i2}`}
															value={o.value}
														>
														{ (o.label in currentButtonLabels)? currentButtonLabels[o.label]:o.label}
														</option>
													))}
												</Form.Select>
											</div>
											<span className="col-sm-auto">=</span>
											<div className="col-sm-auto">
												<Form.Select
													name={`${o}.action`}
													className="form-select-sm"
													value={values[o] && values[o]?.action}
													onChange={handleChange}
													isInvalid={errors[o] && errors[o]?.action}
												>
													{translatedHotkeyActions.map((o, i) => (
														<option key={`hotkey-action-${i}`} value={o.value}>
															{o.label}
														</option>
													))}
												</Form.Select>
												<Form.Control.Feedback type="invalid">
													{errors[o] && errors[o]?.action}
												</Form.Control.Feedback>
											</div>
										</Form.Group>
									))}
								</div>
								<Form.Check
									label={t('SettingsPage:lock-hotkeys-label')}
									type="switch"
									id="LockHotkeys"
									reverse
									isInvalid={false}
									checked={Boolean(values.lockHotkeys)}
									onChange={(e) => {
										setFieldValue('lockHotkeys', e.target.checked ? 1 : 0);
									}}
								/>
							</Section>
							<Button type="submit">{t('Common:button-save-label')}</Button>
							{saveMessage ? (
								<span className="alert">{saveMessage}</span>
							) : null}
							<FormContext setButtonLabels={setButtonLabels} />
						</Form>
						<Modal size="lg" show={warning.show} onHide={handleWarningClose}>
							<Modal.Header closeButton>
								<Modal.Title>
									{t('SettingsPage:forced-setup-mode-modal-title')}
								</Modal.Title>
							</Modal.Header>
							<Modal.Body>
								<div className="mb-3">
									<Trans
										ns="SettingsPage"
										i18nKey="forced-setup-mode-modal-body"
										components={{ strong: <strong /> }}
										values={{ warningCheckText: WARNING_CHECK_TEXT }}
									/>
								</div>
								<Form.Control
									value={warning.acceptText}
									onChange={setWarningAcceptText}
								></Form.Control>
							</Modal.Body>
							<Modal.Footer>
								<Button
									disabled={warning.acceptText != WARNING_CHECK_TEXT}
									variant="warning"
									onClick={() => handleWarningClose(true, values)}
								>
									{t('Common:button-save-label')}
								</Button>
								<Button
									variant="primary"
									onClick={() =>
										handleWarningClose(false, values, setFieldValue)
									}
								>
									{t('Common:button-dismiss-label')}
								</Button>
							</Modal.Footer>
						</Modal>
					</div>
				)
			}
		</Formik>
	);
}
