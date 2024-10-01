import React, { useContext, useEffect, useState, useRef } from 'react';
import { Button, Form, Row, Col, FormLabel } from 'react-bootstrap';
import { Formik, useFormikContext, Field } from 'formik';
import chunk from 'lodash/chunk';
import * as yup from 'yup';
import { Trans, useTranslation } from 'react-i18next';
import { NavLink } from 'react-router-dom';

import { AppContext } from '../Contexts/AppContext';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import Section from '../Components/Section';
import WebApi from '../Services/WebApi';

const ON_OFF_OPTIONS = [
	{ label: 'Disabled', value: 0 },
	{ label: 'Enabled', value: 1 },
];

const SPLASH_MODES = [
	{ label: 'Enabled (Custom Splash Screen)', value: 0 }, // STATICSPLASH
	{ label: 'Logo Close In', value: 1 }, // CLOSEIN
	{ label: 'Logo Close In Custom', value: 2 }, // CLOSEINCUSTOM
	{ label: 'Disabled', value: 3 }, // NOSPLASH
];

const DISPLAY_FLIP_MODES = [
	{ label: 'None', value: 0 },
	{ label: 'Flip', value: 1 },
	{ label: 'Mirror', value: 2 },
	{ label: 'Flip and Mirror', value: 3 },
];

const defaultValues = {
	enabled: false,
	flipDisplay: false,
	invertDisplay: false,
	buttonLayout: 0,
	buttonLayoutRight: 3,
	splashDuration: 0,
	splashMode: 3,
	splashImage: Array(16 * 64).fill(0), // 128 columns represented by bytes so 16 and 64 rows
	invertSplash: false,
	buttonLayoutCustomOptions: {
		params: {
			layout: 0,
			startX: 8,
			startY: 28,
			buttonRadius: 8,
			buttonPadding: 2,
		},
		paramsRight: {
			layout: 3,
			startX: 8,
			startY: 28,
			buttonRadius: 8,
			buttonPadding: 2,
		},
	},
	displaySaverTimeout: 0,
};

let buttonLayoutDefinitions = { buttonLayout: {}, buttonLayoutRight: {} };

const buttonLayoutSchemaBase = yup.number().required();

let buttonLayoutSchema = buttonLayoutSchemaBase.label('Button Layout Left');
let buttonLayoutRightSchema = buttonLayoutSchemaBase.label(
	'Button Layout Right',
);

const schema = yup.object().shape({
	enabled: yup.number().label('Enabled?'),
	flipDisplay: yup
		.number()
		.oneOf(DISPLAY_FLIP_MODES.map((o) => o.value))
		.label('Flip Display'),
	invertDisplay: yup.number().label('Invert Display'),
	turnOffWhenSuspended: yup.number().label('Turn Off When Suspended'),
	buttonLayout: buttonLayoutSchema,
	buttonLayoutRight: buttonLayoutRightSchema,
	splashMode: yup
		.number()
		.required()
		.oneOf(SPLASH_MODES.map((o) => o.value))
		.label('Splash Screen'),
	buttonLayoutCustomOptions: yup.object().shape({
		params: yup.object().shape({
			layout: buttonLayoutSchema,
			startX: yup.number().required().min(0).max(128).label('Start X'),
			startY: yup.number().required().min(0).max(64).label('Start Y'),
			buttonRadius: yup
				.number()
				.required()
				.min(0)
				.max(20)
				.label('Button Radius'),
			buttonPadding: yup
				.number()
				.required()
				.min(0)
				.max(20)
				.label('Button Padding'),
		}),
		paramsRight: yup.object().shape({
			layout: buttonLayoutRightSchema,
			startX: yup.number().required().min(0).max(128).label('Start X'),
			startY: yup.number().required().min(0).max(64).label('Start Y'),
			buttonRadius: yup
				.number()
				.required()
				.min(0)
				.max(20)
				.label('Button Radius'),
			buttonPadding: yup
				.number()
				.required()
				.min(0)
				.max(20)
				.label('Button Padding'),
		}),
	}),
	splashDuration: yup.number().required().min(0).label('Splash Duration'),
	displaySaverTimeout: yup.number().required().min(0).label('Display Saver'),
});

const FormContext = () => {
	const { values, setValues } = useFormikContext();

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
			setValues(data);
		}
		fetchData();
	}, [setValues]);

	useEffect(() => {
		async function setDisplayOptions() {
			if (!!values.enabled) values.enabled = parseInt(values.enabled);
			if (!!values.flipDisplay)
				values.flipDisplay = parseInt(values.flipDisplay);
			if (!!values.invertDisplay)
				values.invertDisplay = parseInt(values.invertDisplay);
			if (!!values.buttonLayout)
				values.buttonLayout = parseInt(values.buttonLayout);
			if (!!values.buttonLayoutRight)
				values.buttonLayoutRight = parseInt(values.buttonLayoutRight);
			if (!!values.splashMode) values.splashMode = parseInt(values.splashMode);
			if (!!values.splashChoice)
				values.splashChoice = parseInt(values.splashChoice);
			if (!!values.splashDuration)
				values.splashDuration = parseInt(values.splashDuration);
			if (!!values.turnOffWhenSuspended)
				values.turnOffWhenSuspended = parseInt(values.turnOffWhenSuspended);

			await WebApi.setDisplayOptions(values, true);
		}

		setDisplayOptions();
	}, [values, setValues]);

	useEffect(() => {
		async function setSplashImage() {
			if (!!values.enabled) values.enabled = parseInt(values.enabled);
			if (!!values.flipDisplay)
				values.flipDisplay = parseInt(values.flipDisplay);
			if (!!values.invertDisplay)
				values.invertDisplay = parseInt(values.invertDisplay);
			if (!!values.turnOffWhenSuspended)
				values.turnOffWhenSuspended = parseInt(values.turnOffWhenSuspended);
			if (!!values.buttonLayout)
				values.buttonLayout = parseInt(values.buttonLayout);
			if (!!values.buttonLayoutRight)
				values.buttonLayoutRight = parseInt(values.buttonLayoutRight);
			if (!!values.splashMode) values.splashMode = parseInt(values.splashMode);
			if (!!values.splashChoice)
				values.splashChoice = parseInt(values.splashChoice);

			await WebApi.setDisplayOptions(values, true);
		}

		setSplashImage();
	}, [values.splashImage]);

	return null;
};

const isButtonLayoutCustom = (values) =>
	values.buttonLayout === 12 || values.buttonLayoutRight === 16;

export default function DisplayConfigPage() {
	const {
		updateUsedPins,
		getAvailablePeripherals,
		getSelectedPeripheral,
		updatePeripherals,
	} = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');

	const { t } = useTranslation('');

	DISPLAY_FLIP_MODES[0].label = t('DisplayConfig:form.flip-display-none');
	DISPLAY_FLIP_MODES[1].label = t('DisplayConfig:form.flip-display-flip');
	DISPLAY_FLIP_MODES[2].label = t('DisplayConfig:form.flip-display-mirror');
	DISPLAY_FLIP_MODES[3].label = t(
		'DisplayConfig:form.flip-display-flip-mirror',
	);

	useEffect(() => {
		updatePeripherals();
	}, []);

	const onSuccess = async (values) => {
		const success = await WebApi.setDisplayOptions(values, false).then(() =>
			WebApi.setSplashImage(values),
		);

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

	return (
		<Formik
			validationSchema={schema}
			onSubmit={onSuccess}
			initialValues={defaultValues}
		>
			{({ handleSubmit, handleChange, values, errors, setFieldValue }) => {
				const handlePeripheralChange = (e) => {
					let device = getSelectedPeripheral('i2c', e.target.value);
					handleChange(e);
				};

				return (
					console.log('errors', errors) ||
					console.log('values', values) || (
						<Section title={t('DisplayConfig:header-text')}>
							{getAvailablePeripherals('i2c') ? (
								<div>
									<p>{t('DisplayConfig:sub-header-text')}</p>
									<ul>
										<Trans ns="DisplayConfig" i18nKey="list-text">
											<li>Monochrome display with 128x64 resolution</li>
											<li>
												Uses I2C with a SSD1306, SH1106, SH1107 or other
												compatible display IC
											</li>
											<li>Supports 3.3v operation</li>
										</Trans>
									</ul>
									<Form noValidate onSubmit={handleSubmit}>
										<h1>{t('DisplayConfig:section.hardware-header')}</h1>
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
														{o.label}
													</option>
												))}
											</FormSelect>
										</Row>
										<h1>{t('DisplayConfig:section.screen-header')}</h1>
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
														{o.label}
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
														{o.label}
													</option>
												))}
											</FormSelect>
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
										<h1>{t('DisplayConfig:section.layout-header')}</h1>
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
															{t(`LayoutConfig:layouts.left.${o}`)}
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
														{t(`LayoutConfig:layouts.right.${o}`)}
													</option>
												))}
											</FormSelect>
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
														{o.label}
													</option>
												))}
											</FormSelect>
										</Row>
										{isButtonLayoutCustom(values) && (
											<Row className="mb-3">
												<FormLabel>
													{t('DisplayConfig:form.button-layout-custom-header')}
												</FormLabel>
												<Col sm="6">
													<Form.Group as={Row} name="buttonLayoutCustomOptions">
														<Form.Label column>
															{t(
																'DisplayConfig:form.button-layout-custom-left-label',
															)}
														</Form.Label>
														<FormSelect
															name="buttonLayoutCustomOptions.params.layout"
															className="form-select-sm"
															groupClassName="col-sm-10 mb-1"
															value={
																values.buttonLayoutCustomOptions.params.layout
															}
															onChange={handleChange}
														>
															{Object.keys(
																buttonLayoutDefinitions.buttonLayout,
															).map((o, i) => (
																<option
																	key={`buttonLayout-option-${i}`}
																	value={
																		buttonLayoutDefinitions.buttonLayout[o]
																	}
																>
																	{t(`LayoutConfig:layouts.left.${o}`)}
																</option>
															))}
														</FormSelect>
													</Form.Group>
													<Form.Group as={Row}>
														<Form.Label column>
															{t(
																'DisplayConfig:form.button-layout-custom-start-x-label',
															)}
														</Form.Label>
														<Col sm="10">
															<Field
																column
																className="mb-1"
																name="buttonLayoutCustomOptions.params.startX"
																type="number"
																as={Form.Control}
															/>
														</Col>
													</Form.Group>
													<Form.Group as={Row}>
														<Form.Label column>
															{t(
																'DisplayConfig:form.button-layout-custom-start-y-label',
															)}
														</Form.Label>
														<Col sm="10">
															<Field
																column
																className="mb-1"
																name="buttonLayoutCustomOptions.params.startY"
																type="number"
																as={Form.Control}
															/>
														</Col>
													</Form.Group>
													<Form.Group as={Row}>
														<Form.Label column>
															{t(
																'DisplayConfig:form.button-layout-custom-button-radius-label',
															)}
														</Form.Label>
														<Col sm="10">
															<Field
																column
																className="mb-1"
																name="buttonLayoutCustomOptions.params.buttonRadius"
																type="number"
																as={Form.Control}
															/>
														</Col>
													</Form.Group>
													<Form.Group as={Row}>
														<Form.Label column>
															{t(
																'DisplayConfig:form.button-layout-custom-button-padding-label',
															)}
														</Form.Label>
														<Col sm="10">
															<Field
																column
																className="mb-1"
																name="buttonLayoutCustomOptions.params.buttonPadding"
																type="number"
																as={Form.Control}
															/>
														</Col>
													</Form.Group>
												</Col>
												<Col sm="6">
													<Form.Group as={Row}>
														<Form.Label column>
															{t(
																'DisplayConfig:form.button-layout-custom-right-label',
															)}
														</Form.Label>
														<FormSelect
															name="buttonLayoutCustomOptions.paramsRight.layout"
															className="form-select-sm"
															groupClassName="col-sm-10 mb-1"
															value={
																values.buttonLayoutCustomOptions.paramsRight
																	.layout
															}
															onChange={handleChange}
														>
															{Object.keys(
																buttonLayoutDefinitions.buttonLayoutRight,
															).map((o, i) => (
																<option
																	key={`buttonLayoutRight-option-${i}`}
																	value={
																		buttonLayoutDefinitions.buttonLayoutRight[o]
																	}
																>
																	{t(`LayoutConfig:layouts.right.${o}`)}
																</option>
															))}
														</FormSelect>
													</Form.Group>
													<Form.Group as={Row}>
														<Form.Label column>
															{t(
																'DisplayConfig:form.button-layout-custom-start-x-label',
															)}
														</Form.Label>
														<Col sm="10">
															<Field
																column
																className="mb-1"
																name="buttonLayoutCustomOptions.paramsRight.startX"
																type="number"
																as={Form.Control}
															/>
														</Col>
													</Form.Group>
													<Form.Group as={Row}>
														<Form.Label column>
															{t(
																'DisplayConfig:form.button-layout-custom-start-y-label',
															)}
														</Form.Label>
														<Col sm="10">
															<Field
																column
																className="mb-1"
																name="buttonLayoutCustomOptions.paramsRight.startY"
																type="number"
																as={Form.Control}
															/>
														</Col>
													</Form.Group>
													<Form.Group as={Row}>
														<Form.Label column>
															{t(
																'DisplayConfig:form.button-layout-custom-button-radius-label',
															)}
														</Form.Label>
														<Col sm="10">
															<Field
																column
																className="mb-1"
																name="buttonLayoutCustomOptions.paramsRight.buttonRadius"
																type="number"
																as={Form.Control}
															/>
														</Col>
													</Form.Group>
													<Form.Group as={Row}>
														<Form.Label column>
															{t(
																'DisplayConfig:form.button-layout-custom-button-padding-label',
															)}
														</Form.Label>
														<Col sm="10">
															<Field
																column
																className="mb-1"
																name="buttonLayoutCustomOptions.paramsRight.buttonPadding"
																type="number"
																as={Form.Control}
															/>
														</Col>
													</Form.Group>
												</Col>
											</Row>
										)}
										<Row className="mb-3">
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
										<div className="mt-3">
											<Button type="submit">
												{t('Common:button-save-label')}
											</Button>
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
					)
				);
			}}
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
