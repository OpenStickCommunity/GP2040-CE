import { useContext, useEffect, useState, useRef } from 'react';
import { Button, Form, Row, Col, FormLabel, Tab, Tabs } from 'react-bootstrap';
import { Formik, useFormikContext, Field } from 'formik';
import chunk from 'lodash/chunk';
import * as yup from 'yup';
import { Trans, useTranslation } from 'react-i18next';
import { NavLink } from 'react-router-dom';

import { AppContext } from '../Contexts/AppContext';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import Section from '../Components/Section';
import JsonEditor from '../Components/JsonEditor';
import WebApi from '../Services/WebApi';
import { getButtonLabels } from '../Data/Buttons';
import { ButtonLayout, ButtonLayoutRight } from '@proto/enums';
import {
	parseLayoutCode,
	layoutCodeToBase64,
	base64ToLayoutCode,
} from '../Services/CustomLayout';

const ON_OFF_OPTIONS = [
	{ label: 'form.display-state.disabled', value: 0 },
	{ label: 'form.display-state.enabled', value: 1 },
];

const SPLASH_MODES = [
	{ label: 'form.splash-modes.enabled', value: 0 }, // STATICSPLASH
	{ label: 'form.splash-modes.disabled', value: 3 }, // NOSPLASH
];

const DISPLAY_FLIP_MODES = [
	{ label: 'form.flip-display.none', value: 0 },
	{ label: 'form.flip-display.flip', value: 1 },
	{ label: 'form.flip-display.mirror', value: 2 },
	{ label: 'form.flip-display.flip-mirror', value: 3 },
];

const DISPLAY_SAVER_MODES = [
	{ label: 'form.saver-modes.display-off', value: 0 },
	{ label: 'form.saver-modes.snow', value: 1 },
	{ label: 'form.saver-modes.bounce', value: 2 },
	{ label: 'form.saver-modes.pipes', value: 3 },
	{ label: 'form.saver-modes.toast', value: 4 },
];

const LAYOUT_ORIENTATION = [
	{ label: 'form.layout-modes.standard', value: 0 },
	{ label: 'form.layout-modes.southpaw', value: 1 },
	{ label: 'form.layout-modes.switched', value: 2 },
];

const defaultValues = {
	enabled: 0,
	flipDisplay: 0,
	invertDisplay: 0,
	buttonLayout: 0,
	buttonLayoutRight: 3,
	buttonLayoutOrientation: 0,
	splashDuration: 0,
	splashMode: 3,
	splashImage: Array(16 * 64).fill(0), // 128 columns represented by bytes so 16 and 64 rows
	displaySaverTimeout: 0,
	displaySaverMode: 0,
	inputMode: true,
	turboMode: true,
	dpadMode: true,
	socdMode: true,
	macroMode: true,
	profileMode: false,
	inputHistoryEnabled: false,
	inputHistoryLength: 21,
	inputHistoryCol: 0,
	inputHistoryRow: 7,
	turnOffWhenSuspended: 0,
	displayContrast: 255,
	customLayoutA: '',
	customLayoutB: '',
};

let buttonLayoutDefinitions = { buttonLayout: {}, buttonLayoutRight: {} };
let customLayoutMaxPin;

const buttonLayoutSchemaBase = yup.number().required();

let buttonLayoutSchema = buttonLayoutSchemaBase.label('Button Layout Left');
let buttonLayoutRightSchema = buttonLayoutSchemaBase.label(
	'Button Layout Right',
);

const customLayoutSchema = (layoutField, layout) =>
	yup.string().when(layoutField, {
		is: (value) => Number(value) === layout,
		then: (schema) =>
			schema.test('layout-code', (code, context) => {
				const { error } = parseLayoutCode(code, customLayoutMaxPin);
				return error === null || context.createError({ message: error });
			}),
	});

const toDisplayOptionsRequest = (values) => {
	const request = { ...values };
	['customLayoutA', 'customLayoutB'].forEach((field) => {
		const base64 = layoutCodeToBase64(values[field], customLayoutMaxPin);
		if (base64 === null) delete request[field];
		else request[field] = base64;
	});
	return request;
};

const schema = yup.object().shape({
	enabled: yup
		.number()
		.oneOf(ON_OFF_OPTIONS.map((o) => o.value))
		.label('Enabled?'),
	flipDisplay: yup
		.number()
		.oneOf(DISPLAY_FLIP_MODES.map((o) => o.value))
		.label('Flip Display'),
	invertDisplay: yup
		.number()
		.oneOf(ON_OFF_OPTIONS.map((o) => o.value))
		.label('Invert Display'),
	displayContrast: yup.number().min(38).max(255).label('Display Contrast'),
	turnOffWhenSuspended: yup.number().label('Turn Off When Suspended'),
	buttonLayout: buttonLayoutSchema,
	buttonLayoutRight: buttonLayoutRightSchema,
	buttonLayoutOrientation: yup.number().label('Layout Reversed'),
	splashMode: yup
		.number()
		.required()
		.oneOf(SPLASH_MODES.map((o) => o.value))
		.label('Splash Screen'),
	splashDuration: yup.number().required().min(0).label('Splash Duration'),
	displaySaverTimeout: yup
		.number()
		.required()
		.min(0)
		.label('Display Saver Timeout'),
	displaySaverMode: yup.number().required().min(0).label('Screen Saver'),
	inputMode: yup.number().label('Display Input Mode'),
	turboMode: yup.number().label('Display Turbo Mode'),
	dpadMode: yup.number().label('Display D-Pad Mode'),
	socdMode: yup.number().label('Display SOCD Mode'),
	macroMode: yup.number().label('Display Macro Mode'),
	profileMode: yup.number().label('Display Profile Mode'),
	inputHistoryEnabled: yup.number().label('Input History Enabled?'),
	inputHistoryLength: yup.number().label('Input History Length'),
	inputHistoryCol: yup.number().label('Input History Column Position'),
	inputHistoryRow: yup.number().label('Input History Row Position'),
	customLayoutA: customLayoutSchema(
		'buttonLayout',
		ButtonLayout.BUTTON_LAYOUT_CUSTOM_DEFINED_A,
	),
	customLayoutB: customLayoutSchema(
		'buttonLayoutRight',
		ButtonLayoutRight.BUTTON_LAYOUT_CUSTOM_DEFINED_B,
	),
});

const FormContext = () => {
	const { values, setValues } = useFormikContext();

	useEffect(() => {
		async function setDisplayOptions() {
			await WebApi.setDisplayOptions(toDisplayOptionsRequest(values), true);
		}

		setDisplayOptions();
	}, [values, setValues]);

	return null;
};

const isCustomLayoutUsed = (values) =>
	Number(values.buttonLayout) === ButtonLayout.BUTTON_LAYOUT_CUSTOM_DEFINED_A ||
	Number(values.buttonLayoutRight) ===
		ButtonLayoutRight.BUTTON_LAYOUT_CUSTOM_DEFINED_B;

const CustomLayoutCodeEditor = ({
	label,
	name,
	value,
	error,
	setFieldValue,
}) => {
	const { t } = useTranslation('');
	const editorRef = useRef(null);
	const { elements } = parseLayoutCode(value, customLayoutMaxPin);
	const elementCount = elements?.length ?? 0;
	const isJson = (() => {
		try {
			JSON.parse(value);
			return true;
		} catch {
			return false;
		}
	})();

	return (
		<Form.Group>
			<div className="d-flex justify-content-between align-items-end">
				<Form.Label>{label}</Form.Label>
				<Button
					variant="link"
					size="sm"
					className="p-0 mb-2"
					disabled={!isJson}
					onClick={() => editorRef.current?.format()}
				>
					{t('DisplayConfig:form.custom-layout-format-button')}
				</Button>
			</div>
			<JsonEditor
				ref={editorRef}
				name={name}
				value={value}
				onChange={(newValue) => setFieldValue(name, newValue)}
				isInvalid={Boolean(error)}
				isValid={!error && elementCount > 0}
			/>
			{error ? (
				<Form.Control.Feedback type="invalid" className="d-block">
					{error}
				</Form.Control.Feedback>
			) : (
				<Form.Text muted>
					{elementCount > 0
						? t('DisplayConfig:form.custom-layout-element-count', {
								count: elementCount,
							})
						: t('DisplayConfig:form.custom-layout-empty')}
				</Form.Text>
			)}
		</Form.Group>
	);
};

export default function DisplayConfigPage() {
	const [loadingValues, setLoadingValues] = useState(true);
	const [values, setValues] = useState(defaultValues);

	const {
		buttonLabels,
		updateUsedPins,
		getAvailablePeripherals,
		updatePeripherals,
		boardDefinition,
	} = useContext(AppContext);
	customLayoutMaxPin = boardDefinition.maxPin;
	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const currentButtonLabels = getButtonLabels(
		buttonLabelType,
		swapTpShareLabels,
	);
	const [saveMessage, setSaveMessage] = useState('');
	const [activeTab, setActiveTab] = useState('defaultHardwareOptions');

	const { t } = useTranslation('');

	useEffect(() => {
		async function fetchData() {
			const data = await WebApi.getDisplayOptions();
			const splashImageResponse = await WebApi.getSplashImage();
			data.splashImage = splashImageResponse.splashImage;
			buttonLayoutDefinitions = await WebApi.getButtonLayoutDefs();
			buttonLayoutSchema = buttonLayoutSchema.oneOf(
				Object.values(buttonLayoutDefinitions.buttonLayout),
			);
			buttonLayoutRightSchema = buttonLayoutRightSchema.oneOf(
				Object.values(buttonLayoutDefinitions.buttonLayoutRight),
			);
			data.customLayoutA = base64ToLayoutCode(data.customLayoutA);
			data.customLayoutB = base64ToLayoutCode(data.customLayoutB);
			setValues(data);
			setLoadingValues(false);
		}
		updatePeripherals();
		fetchData();
	}, []);

	const onSuccess = async (values) => {
		const success = await WebApi.setDisplayOptions(
			toDisplayOptionsRequest(values),
			false,
		).then((saved) => saved && WebApi.setSplashImage(values));

		if (success) await updateUsedPins();

		setSaveMessage(
			success
				? t('Common:saved-success-message')
				: t('Common:saved-error-message'),
		);
	};

	const onChangeCanvas = (base64, form, field) => {
		return form.setFieldValue(field.name, base64);
	};

	if (loadingValues) {
		return (
			<div className="d-flex justify-content-center align-items-center loading">
				<div className="spinner-border" role="status">
					<span className="visually-hidden">{t('Common:loading-text')}</span>
				</div>
			</div>
		);
	}

	return (
		<Formik
			validationSchema={schema}
			onSubmit={onSuccess}
			initialValues={values}
		>
			{({ handleSubmit, handleChange, values, errors, setFieldValue }) => (
				<Section title={t('DisplayConfig:header-text')}>
					{getAvailablePeripherals('i2c') ? (
						<div>
							<p>{t('DisplayConfig:sub-header-text')}</p>
							<ul>
								<Trans ns="DisplayConfig" i18nKey="list-text">
									<li>Monochrome display with 128x64 resolution</li>
									<li>
										Uses I2C with a SSD1306, SH1106, SH1107 or other compatible
										display IC
									</li>
									<li>Supports 3.3v operation</li>
								</Trans>
							</ul>
							<Form noValidate onSubmit={handleSubmit}>
								<Tabs
									activeKey={activeTab}
									onSelect={(k) => setActiveTab(k)}
									id="displayConfigTabs"
									className="mb-3 pb-0"
									fill
								>
									<Tab
										key="defaultHardwareOptions"
										eventKey="defaultHardwareOptions"
										title={t('DisplayConfig:section.hardware-header')}
									>
										<Row className="mb-4">
											<FormSelect
												label={t('Common:switch-enabled')}
												name="enabled"
												className="form-select-sm"
												groupClassName="col-sm-3 mb-3"
												value={values.enabled}
												error={errors.enabled}
												isInvalid={errors.enabled}
												onChange={handleChange}
											>
												{ON_OFF_OPTIONS.map((o, i) => (
													<option key={`enabled-option-${i}`} value={o.value}>
														{t(`DisplayConfig:${o.label}`)}
													</option>
												))}
											</FormSelect>
										</Row>
									</Tab>
									<Tab
										key="displayScreenOptions"
										eventKey="displayScreenOptions"
										title={t('DisplayConfig:section.screen-header')}
									>
										<Row className="mb-4">
											<FormSelect
												label={t('DisplayConfig:form.flip-display-label')}
												name="flipDisplay"
												className="form-select-sm"
												groupClassName="col-sm-3 mb-3"
												value={values.flipDisplay}
												error={errors.flipDisplay}
												isInvalid={errors.flipDisplay}
												onChange={handleChange}
											>
												{DISPLAY_FLIP_MODES.map((o, i) => (
													<option
														key={`flipDisplay-option-${i}`}
														value={o.value}
													>
														{t(`DisplayConfig:${o.label}`)}
													</option>
												))}
											</FormSelect>
											<FormSelect
												label={t('DisplayConfig:form.invert-display-label')}
												name="invertDisplay"
												className="form-select-sm"
												groupClassName="col-sm-3 mb-3"
												value={values.invertDisplay}
												error={errors.invertDisplay}
												isInvalid={errors.invertDisplay}
												onChange={handleChange}
											>
												{ON_OFF_OPTIONS.map((o, i) => (
													<option
														key={`invertDisplay-option-${i}`}
														value={o.value}
													>
														{t(`DisplayConfig:${o.label}`)}
													</option>
												))}
											</FormSelect>
											<div className="col-sm-3 mb-3">
												<Form.Label>{`${t('DisplayConfig:form.display-contrast-label')}: ${Math.round((values.displayContrast / 255) * 100)}%`}</Form.Label>
												<Form.Range
													name="displayContrast"
													id="displayContrast"
													min={38}
													max={255}
													step={1}
													value={values.displayContrast}
													onChange={handleChange}
												/>
											</div>
											<div className="col-sm-3 mb-3">
												<label className="form-label">
													{t('DisplayConfig:form.power-management-header')}
												</label>
												<Form.Check
													label={t(
														'DisplayConfig:form.turn-off-when-suspended',
													)}
													type="switch"
													name="turnOffWhenSuspended"
													className="align-middle"
													isInvalid={false}
													checked={Boolean(values.turnOffWhenSuspended)}
													onChange={(e) => {
														setFieldValue(
															'turnOffWhenSuspended',
															e.target.checked ? 1 : 0,
														);
													}}
												/>
											</div>
										</Row>
									</Tab>
									<Tab
										key="displayLayoutOptions"
										eventKey="displayLayoutOptions"
										title={t('DisplayConfig:section.layout-header')}
									>
										<h1>{t('DisplayConfig:section.button-layout-header')}</h1>
										<Row className="mb-4">
											<FormSelect
												label={t('DisplayConfig:form.button-layout-label')}
												name="buttonLayout"
												className="form-select-sm"
												groupClassName="col-sm-3 mb-3"
												value={values.buttonLayout}
												error={errors.buttonLayout}
												isInvalid={errors.buttonLayout}
												onChange={handleChange}
											>
												{Object.keys(buttonLayoutDefinitions.buttonLayout).map(
													(o, i) => (
														<option
															key={`buttonLayout-option-${i}`}
															value={buttonLayoutDefinitions.buttonLayout[o]}
														>
															{o in ButtonLayout
																? t(`LayoutConfig:layouts.left.${o}`, {
																		defaultValue: o,
																	})
																: o}
														</option>
													),
												)}
											</FormSelect>
											<FormSelect
												label={t(
													'DisplayConfig:form.button-layout-right-label',
												)}
												name="buttonLayoutRight"
												className="form-select-sm"
												groupClassName="col-sm-3 mb-3"
												value={values.buttonLayoutRight}
												error={errors.buttonLayoutRight}
												isInvalid={errors.buttonLayoutRight}
												onChange={handleChange}
											>
												{Object.keys(
													buttonLayoutDefinitions.buttonLayoutRight,
												).map((o, i) => (
													<option
														key={`buttonLayoutRight-option-${i}`}
														value={buttonLayoutDefinitions.buttonLayoutRight[o]}
													>
														{o in ButtonLayoutRight
															? t(`LayoutConfig:layouts.right.${o}`, {
																	defaultValue: o,
																})
															: o}
													</option>
												))}
											</FormSelect>
											{!isCustomLayoutUsed(values) && (
												<FormSelect
													label={t(
														'DisplayConfig:form.button-layout-orientation',
													)}
													name="buttonLayoutOrientation"
													className="form-select-sm"
													groupClassName="col-sm-3 mb-3"
													value={values.buttonLayoutOrientation}
													error={errors.buttonLayoutOrientation}
													isInvalid={errors.buttonLayoutOrientation}
													onChange={handleChange}
												>
													{LAYOUT_ORIENTATION.map((o, i) => (
														<option
															key={`buttonLayoutOrientation-option-${i}`}
															value={o.value}
														>
															{t(`DisplayConfig:${o.label}`)}
														</option>
													))}
												</FormSelect>
											)}
										</Row>
										{isCustomLayoutUsed(values) && (
											<>
												<h1>
													{t('DisplayConfig:section.custom-layout-header')}
												</h1>
												<Row className="mb-4">
													<p>
														{t('DisplayConfig:form.custom-layout-description')}
													</p>
													<p>
														{t('DisplayConfig:form.custom-layout-preview-tip', {
															button: currentButtonLabels.B1,
														})}
													</p>
													{Number(values.buttonLayout) ===
														ButtonLayout.BUTTON_LAYOUT_CUSTOM_DEFINED_A && (
														<Col sm="6">
															<CustomLayoutCodeEditor
																label={t(
																	'DisplayConfig:form.custom-layout-left-label',
																)}
																name="customLayoutA"
																value={values.customLayoutA}
																error={errors.customLayoutA}
																setFieldValue={setFieldValue}
															/>
														</Col>
													)}
													{Number(values.buttonLayoutRight) ===
														ButtonLayoutRight.BUTTON_LAYOUT_CUSTOM_DEFINED_B && (
														<Col sm="6">
															<CustomLayoutCodeEditor
																label={t(
																	'DisplayConfig:form.custom-layout-right-label',
																)}
																name="customLayoutB"
																value={values.customLayoutB}
																error={errors.customLayoutB}
																setFieldValue={setFieldValue}
															/>
														</Col>
													)}
												</Row>
											</>
										)}
										<h1>{t('DisplayConfig:section.status-layout-header')}</h1>
										<Row className="mb-4">
											<div className="col-sm-2 mb-3">
												<Form.Check
													label={t(
														'DisplayConfig:form.status-header.input-mode',
													)}
													type="switch"
													name="inputMode"
													className="align-middle"
													isInvalid={false}
													checked={Boolean(values.inputMode)}
													onChange={(e) => {
														setFieldValue(
															'inputMode',
															e.target.checked ? 1 : 0,
														);
													}}
												/>
											</div>
											<div className="col-sm-2 mb-3">
												<Form.Check
													label={t(
														'DisplayConfig:form.status-header.turbo-mode',
													)}
													type="switch"
													name="turboMode"
													className="align-middle"
													isInvalid={false}
													checked={Boolean(values.turboMode)}
													onChange={(e) => {
														setFieldValue(
															'turboMode',
															e.target.checked ? 1 : 0,
														);
													}}
												/>
											</div>
											<div className="col-sm-2 mb-3">
												<Form.Check
													label={t(
														'DisplayConfig:form.status-header.dpad-mode',
													)}
													type="switch"
													name="dpadMode"
													className="align-middle"
													isInvalid={false}
													checked={Boolean(values.dpadMode)}
													onChange={(e) => {
														setFieldValue('dpadMode', e.target.checked ? 1 : 0);
													}}
												/>
											</div>
											<div className="col-sm-2 mb-3">
												<Form.Check
													label={t(
														'DisplayConfig:form.status-header.socd-mode',
													)}
													type="switch"
													name="displaySocdMode"
													className="align-middle"
													isInvalid={false}
													checked={Boolean(values.socdMode)}
													onChange={(e) => {
														setFieldValue('socdMode', e.target.checked ? 1 : 0);
													}}
												/>
											</div>
											<div className="col-sm-2 mb-3">
												<Form.Check
													label={t(
														'DisplayConfig:form.status-header.macro-mode',
													)}
													type="switch"
													name="macroMode"
													className="align-middle"
													isInvalid={false}
													checked={Boolean(values.macroMode)}
													onChange={(e) => {
														setFieldValue(
															'macroMode',
															e.target.checked ? 1 : 0,
														);
													}}
												/>
											</div>
											<div className="col-sm-2 mb-3">
												<Form.Check
													label={t(
														'DisplayConfig:form.status-header.profile-mode',
													)}
													type="switch"
													name="profileMode"
													className="align-middle"
													isInvalid={false}
													checked={Boolean(values.profileMode)}
													onChange={(e) => {
														setFieldValue(
															'profileMode',
															e.target.checked ? 1 : 0,
														);
													}}
												/>
											</div>
										</Row>
										<h1>{t('DisplayConfig:section.history-layout-header')}</h1>
										<Row className="mb-4">
											<div className="col-sm-2 mb-3">
												<Form.Check
													label={t('DisplayConfig:form.input-history-label')}
													type="switch"
													name="inputHistoryEnabled"
													className="align-middle mt-1"
													isInvalid={false}
													checked={Boolean(values.inputHistoryEnabled)}
													onChange={(e) => {
														setFieldValue(
															'inputHistoryEnabled',
															e.target.checked ? 1 : 0,
														);
													}}
												/>
											</div>
											<FormControl
												type="number"
												label={t('AddonsConfig:input-history-length-label')}
												name="inputHistoryLength"
												className="form-control-sm"
												groupClassName="col-sm-3 mb-3"
												value={values.inputHistoryLength}
												error={errors.inputHistoryLength}
												isInvalid={errors.inputHistoryLength}
												onChange={handleChange}
												min={1}
												max={21}
											/>
											<FormControl
												type="number"
												label={t('AddonsConfig:input-history-col-label')}
												name="inputHistoryCol"
												className="form-control-sm"
												groupClassName="col-sm-3 mb-3"
												value={values.inputHistoryCol}
												error={errors.inputHistoryCol}
												isInvalid={errors.inputHistoryCol}
												onChange={handleChange}
												min={0}
												max={20}
											/>
											<FormControl
												type="number"
												label={t('AddonsConfig:input-history-row-label')}
												name="inputHistoryRow"
												className="form-control-sm"
												groupClassName="col-sm-3 mb-3"
												value={values.inputHistoryRow}
												error={errors.inputHistoryRow}
												isInvalid={errors.inputHistoryRow}
												onChange={handleChange}
												min={0}
												max={7}
											/>
										</Row>
									</Tab>
									<Tab
										key="displayModeOptions"
										eventKey="displayModeOptions"
										title={t('DisplayConfig:section.mode-header')}
									>
										<Row className="mb-4">
											<FormSelect
												label={t('DisplayConfig:form.splash-mode-label')}
												name="splashMode"
												className="form-select-sm"
												groupClassName="col-sm-3 mb-3"
												value={values.splashMode}
												error={errors.splashMode}
												isInvalid={errors.splashMode}
												onChange={handleChange}
											>
												{SPLASH_MODES.map((o, i) => (
													<option
														key={`splashMode-option-${i}`}
														value={o.value}
													>
														{t(`DisplayConfig:${o.label}`)}
													</option>
												))}
											</FormSelect>
											<FormControl
												type="number"
												label={t('DisplayConfig:form.splash-duration-label')}
												name="splashDuration"
												className="form-select-sm"
												groupClassName="col-sm-3 mb-3"
												value={values.splashDuration}
												error={errors.splashDuration}
												isInvalid={errors.splashDuration}
												onChange={handleChange}
												min={0}
											/>
										</Row>
										<Row className="mb-3">
											<FormSelect
												label={t('DisplayConfig:form.screen-saver-mode-label')}
												name="displaySaverMode"
												className="form-select-sm"
												groupClassName="col-sm-3 mb-3"
												value={values.displaySaverMode}
												error={errors.displaySaverMode}
												isInvalid={errors.displaySaverMode}
												onChange={handleChange}
											>
												{DISPLAY_SAVER_MODES.map((o, i) => (
													<option
														key={`displaySaverMode-option-${i}`}
														value={o.value}
													>
														{t(`DisplayConfig:${o.label}`)}
													</option>
												))}
											</FormSelect>
											<FormControl
												type="number"
												label={t(
													'DisplayConfig:form.display-saver-timeout-label',
												)}
												name="displaySaverTimeout"
												className="form-select-sm"
												groupClassName="col-sm-3 mb-3"
												value={values.displaySaverTimeout}
												error={errors.displaySaverTimeout}
												isInvalid={errors.displaySaverTimeout}
												onChange={handleChange}
												min={0}
											/>
										</Row>
										<Row>
											<Field name="splashImage">
												{({
													field, // { name, value, onChange, onBlur }
													form, // also values, setXXXX, handleXXXX, dirty, isValid, status, etc.
												}) => (
													<div className="mt-3">
														<Canvas
															onChange={(base64) =>
																onChangeCanvas(base64, form, field)
															}
															value={field.value}
														/>
													</div>
												)}
											</Field>
										</Row>
									</Tab>
								</Tabs>
								<div className="mt-3">
									<Button type="submit">{t('Common:button-save-label')}</Button>
									{saveMessage ? (
										<span className="alert">{saveMessage}</span>
									) : null}
								</div>
								<FormContext />
							</Form>
						</div>
					) : (
						<FormLabel>
							<Trans
								ns="PeripheralMapping"
								i18nKey="peripheral-toggle-unavailable"
								values={{ name: 'I2C' }}
							>
								<NavLink exact="true" to="/peripheral-mapping">
									{t('PeripheralMapping:header-text')}
								</NavLink>
							</Trans>
						</FormLabel>
					)}
				</Section>
			)}
		</Formik>
	);
}

const Canvas = ({ value: bitsArray, onChange }) => {
	const [image, setImage] = useState(null);
	const [canvasContext, setCanvasContext] = useState(null);
	const [inverted, setInverted] = useState(false);
	const canvasRef = useRef();

	const { t } = useTranslation('');

	useEffect(() => {
		setCanvasContext(canvasRef.current.getContext('2d'));
	}, []);

	// image to bitsArray (binary)
	useEffect(() => {
		if (canvasContext == null || image == null) return;

		const ctxWidth = canvasContext.canvas.width,
			ctxHeight = canvasContext.canvas.height;
		const imgWidth = image.width,
			imgHeight = image.height;
		const ratioWidth = imgWidth / ctxWidth,
			ratioHeight = imgHeight / ctxHeight,
			ratioAspect =
				ratioWidth > 1 ? ratioWidth : ratioHeight > 1 ? ratioHeight : 1;
		const newWidth = imgWidth / ratioAspect,
			newHeight = imgHeight / ratioAspect;
		const offsetX = ctxWidth / 2 - newWidth / 2,
			offsetY = ctxHeight / 2 - newHeight / 2;
		canvasContext.clearRect(
			0,
			0,
			canvasRef.current.width,
			canvasRef.current.height,
		);
		canvasContext.drawImage(image, offsetX, offsetY, newWidth, newHeight);

		var imgPixels = canvasContext.getImageData(
			0,
			0,
			canvasContext.canvas.width,
			canvasContext.canvas.height,
		);

		// Convert to monochrome
		for (var i = 0; i < imgPixels.data.length; i = i + 4) {
			var avg =
				(imgPixels.data[i] + imgPixels.data[i + 1] + imgPixels.data[i + 2]) / 3;
			if (avg > 123) avg = 255;
			else avg = 0;
			imgPixels.data[i] = avg;
			imgPixels.data[i + 1] = avg;
			imgPixels.data[i + 2] = avg;
		}

		// Pick only first channel because all of them are same
		const bitsArray = chunk(
			[...new Uint8Array(imgPixels.data)].filter((x, y) => y % 4 === 0),
			8,
		).map((chunks) =>
			chunks.reduce((acc, curr, i) => {
				return acc + ((curr === 255 ? 1 : 0) << (7 - i));
			}, 0),
		);

		onChange(bitsArray.map((a) => (inverted ? 255 - a : a)));
	}, [image, canvasContext]);

	// binary to RGBA
	useEffect(() => {
		if (canvasContext == null) return;

		const w = canvasContext.canvas.width;
		const h = canvasContext.canvas.height;
		const rgbToRgba = [];

		// expand bytes to individual binary bits and then bits to 255 or 0, because monochrome
		const bitsArrayArray = bitsArray.flatMap((a) => {
			const bits = a.toString(2).split('').map(Number);
			const full = Array(8 - bits.length)
				.fill(0)
				.concat(bits);
			return full.map((a) => (a === 1 ? 255 : 0));
		});

		// fill up the new array as RGBA
		bitsArrayArray.forEach((x) => {
			rgbToRgba.push(x);
			rgbToRgba.push(x);
			rgbToRgba.push(x);
			rgbToRgba.push(255);
		});
		const imageDataCopy = new ImageData(new Uint8ClampedArray(rgbToRgba), w, h);
		canvasContext.putImageData(imageDataCopy, 0, 0, 0, 0, w, h);
	}, [bitsArray, canvasContext]);

	const onImageAdd = (ev) => {
		var file = ev.target.files[0];
		var fr = new FileReader();
		fr.onload = () => {
			const img = new Image();
			img.onload = () => {
				setImage(img);
			};
			img.src = fr.result;
		};
		fr.readAsDataURL(file);
	};

	const toggleInverted = () => {
		onChange(bitsArray.map((a) => 255 - a));
		setInverted(!inverted);
	};

	return (
		<div style={{ display: 'flex', alignItems: 'center' }}>
			<canvas
				ref={canvasRef}
				width="128"
				height="64"
				style={{ background: 'black' }}
			/>
			<div style={{ marginLeft: '11px' }}>
				<input
					type="file"
					id="image-input"
					accept="image/jpeg, image/png, image/jpg"
					onChange={onImageAdd}
				/>
				<br />
				<input
					type="checkbox"
					checked={inverted}
					onChange={toggleInverted}
				/>{' '}
				{t('DisplayConfig:form.inverted-label')}
				{/* <ErrorMessage name="splashImage" /> */}
			</div>
		</div>
	);
};
