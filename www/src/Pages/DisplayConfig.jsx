import React, { useContext, useEffect, useState, useRef, useMemo, useCallback } from 'react';
import { Button, Form, Row, Col, FormLabel } from 'react-bootstrap';
import { Formik, useFormikContext, Field } from 'formik';
import { parseGIF, decompressFrames } from 'gifuct-js'
import chunk from 'lodash/chunk';
import * as yup from 'yup';
import { Trans, useTranslation } from 'react-i18next';
import { NavLink } from 'react-router-dom';

import { AppContext } from '../Contexts/AppContext';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import Section from '../Components/Section';
import WebApi from '../Services/WebApi';

import { PERIPHERAL_DEVICES, I2C_BLOCKS } from '../Data/Peripherals';

const ON_OFF_OPTIONS = [
	{ label: 'Disabled', value: 0 },
	{ label: 'Enabled', value: 1 },
];

const BUTTON_LAYOUTS = [
	{ label: 'Stick', value: 0 }, // BUTTON_LAYOUT_STICK
	{ label: 'Stickless', value: 1 }, // BUTTON_LAYOUT_STICKLESS
	{ label: 'Buttons Angled', value: 2 }, // BUTTON_LAYOUT_BUTTONS_ANGLED
	{ label: 'Buttons Basic', value: 3 }, // BUTTON_LAYOUT_BUTTONS_BASIC
	{ label: 'Keyboard Angled', value: 4 }, // BUTTON_LAYOUT_KEYBOARD_ANGLED
	{ label: 'Keyboard', value: 5 }, // BUTTON_LAYOUT_KEYBOARDA
	{ label: 'Dancepad', value: 6 }, // BUTTON_LAYOUT_DANCEPADA
	{ label: 'Twinstick', value: 7 }, // BUTTON_LAYOUT_TWINSTICKA
	{ label: 'Blank', value: 8 }, // BUTTON_LAYOUT_BLANKA
	{ label: 'VLX', value: 9 }, // BUTTON_LAYOUT_VLXA
	{ label: 'Fightboard', value: 10 }, // BUTTON_LAYOUT_FIGHTBOARD_STICK
	{ label: 'Fightboard Mirrored', value: 11 }, // BUTTON_LAYOUT_FIGHTBOARD_MIRRORED
	{ label: 'Custom', value: 12 }, // BUTTON_LAYOUT_CUSTOM
	{ label: 'Open_Core0 WASD', value: 13 }, // BUTTON_LAYOUT_OPENCORE0WASDA
];

const BUTTON_LAYOUTS_RIGHT = [
	{ label: 'Arcade', value: 0 }, // BUTTON_LAYOUT_ARCADE
	{ label: 'Stickless', value: 1 }, // BUTTON_LAYOUT_STICKLESSB
	{ label: 'Buttons Angled', value: 2 }, // BUTTON_LAYOUT_BUTTONS_ANGLEDB
	{ label: 'Viewlix', value: 3 }, // BUTTON_LAYOUT_VEWLIX
	{ label: 'Viewlix 7', value: 4 }, // BUTTON_LAYOUT_VEWLIX7
	{ label: 'Capcom', value: 5 }, // BUTTON_LAYOUT_CAPCOM
	{ label: 'Capcom 6', value: 6 }, // BUTTON_LAYOUT_CAPCOM6
	{ label: 'Sega 2P', value: 7 }, // BUTTON_LAYOUT_SEGA2P
	{ label: 'Noir 8', value: 8 }, // BUTTON_LAYOUT_NOIR8
	{ label: 'Keyboard', value: 9 }, // BUTTON_LAYOUT_KEYBOARDB
	{ label: 'Dancepad', value: 10 }, // BUTTON_LAYOUT_DANCEPADB
	{ label: 'Twinstick', value: 11 }, // BUTTON_LAYOUT_TWINSTICKB
	{ label: 'Blank', value: 12 }, // BUTTON_LAYOUT_BLANKB
	{ label: 'VLX', value: 13 }, // BUTTON_LAYOUT_VLXB
	{ label: 'Fightboard', value: 14 }, // BUTTON_LAYOUT_FIGHTBOARD
	{ label: 'Fightboard Mirrored', value: 15 }, // BUTTON_LAYOUT_FIGHTBOARD_STICK_MIRRORED
	{ label: 'Custom', value: 16 }, // BUTTON_LAYOUT_CUSTOM
	{ label: 'Keyboard 8', value: 17 }, // BUTTON_LAYOUT_KEYBOARD8B
	{ label: 'Open_Core0 WASD', value: 18 }, // BUTTON_LAYOUT_OPENCORE0WASDB
];

const SPLASH_MODES = [
	{ label: 'Enabled', value: 0 }, // STATICSPLASH
	//	{ label: 'Close In', value: 1 },		 // CLOSEIN
	{ label: 'Disabled', value: 3 }, // NOSPLASH
];

const SPLASH_DURATION_CHOICES = [
	{ label: 'Default', value: 0 },
	{ label: '5 seconds', value: 5000 },
	{ label: '10 seconds', value: 10000 },
	{ label: '30 seconds', value: 30000 },
	{ label: 'Always ON', value: -1 },
];

const DISPLAY_SAVER_TIMEOUT_CHOICES = [
	{ label: 'Off', value: 0 },
	{ label: '1 minute', value: 1 },
	{ label: '2 minutes', value: 2 },
	{ label: '5 minutes', value: 5 },
	{ label: '10 minutes', value: 10 },
	{ label: '20 minutes', value: 20 },
	{ label: '30 minutes', value: 30 },
];

const DISPLAY_FLIP_MODES = [
	{ label: 'None', value: 0 },
	{ label: 'Flip', value: 1 },
	{ label: 'Mirror', value: 2 },
	{ label: 'Flip and Mirror', value: 3 },
];

const defaultValues = {
	enabled: false,
	sdaPin: -1,
	sclPin: -1,
	i2cAddress: '0x3C',
	i2cBlock: 0,
	i2cSpeed: 400000,
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

const buttonLayoutSchema = yup
	.number()
	.required()
	.oneOf(BUTTON_LAYOUTS.map((o) => o.value))
	.label('Button Layout Left');
const buttonLayoutRightSchema = yup
	.number()
	.required()
	.oneOf(BUTTON_LAYOUTS_RIGHT.map((o) => o.value))
	.label('Button Layout Right');

const schema = yup.object().shape({
	enabled: yup.number().label('Enabled?'),
	i2cAddress: yup.string().required().label('I2C Address'),
	sdaPin: yup.number().label('SDA Pin').validatePinWhenValue('sdaPin'),
	sclPin: yup.number().label('SCL Pin').validatePinWhenValue('sclPin'),
	i2cBlock: yup
		.number()
		.required()
		.oneOf(I2C_BLOCKS.map((o) => o.value))
		.label('I2C Block'),
	i2cSpeed: yup.number().required().label('I2C Speed'),
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
			setValues(data);
		}
		fetchData();
	}, [setValues]);

	useEffect(() => {
		async function setDisplayOptions() {
			if (!!values.enabled) values.enabled = parseInt(values.enabled);
			if (!!values.i2cBlock) values.i2cBlock = parseInt(values.i2cBlock);
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
			if (!!values.i2cBlock) values.i2cBlock = parseInt(values.i2cBlock);
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
	const { updateUsedPins, getAvailablePeripherals, getSelectedPeripheral } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');

	const { t } = useTranslation('');

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
			{({ handleSubmit, handleChange, handleBlur, values, touched, errors }) => {
                const handlePeripheralChange = (e) => {
                    let device = getSelectedPeripheral('i2c', e.target.value);
                    values.sdaPin = device.sda;
                    values.sclPin = device.scl;
                    values.i2cSpeed = device.speed;
                    handleChange(e);
                };

				return (
					<Section title={t('DisplayConfig:header-text')}>
                        {getAvailablePeripherals('i2c') ?
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
                                    <FormSelect
                                        label={t('DisplayConfig:form.i2c-block-label')}
                                        name="i2cBlock"
                                        className="form-select-sm"
                                        groupClassName="col-sm-3 mb-3"
                                        value={values.i2cBlock}
                                        error={errors.i2cBlock}
                                        isInvalid={errors.i2cBlock}
                                        onChange={handlePeripheralChange}
                                    >
                                        {getAvailablePeripherals('i2c').map((o, i) => (
                                            <option key={`i2cBlock-option-${i}`} value={o.value}>
                                                {o.label}
                                            </option>
                                        ))}
                                    </FormSelect>
                                    <FormControl
                                        type="text"
                                        label={t('DisplayConfig:form.i2c-address-label')}
                                        name="i2cAddress"
                                        className="form-control-sm"
                                        groupClassName="col-sm-3 mb-3"
                                        value={values.i2cAddress}
                                        error={errors.i2cAddress}
                                        isInvalid={errors.i2cAddress}
                                        onChange={handleChange}
                                        maxLength={4}
                                    />
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
                                            <option key={`flipDisplay-option-${i}`} value={o.value}>
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
                                            <option key={`invertDisplay-option-${i}`} value={o.value}>
                                                {o.label}
                                            </option>
                                        ))}
                                    </FormSelect>
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
                                        {BUTTON_LAYOUTS.map((o, i) => (
                                            <option key={`buttonLayout-option-${i}`} value={o.value}>
                                                {o.label}
                                            </option>
                                        ))}
                                    </FormSelect>
                                    <FormSelect
                                        label={t('DisplayConfig:form.button-layout-right-label')}
                                        name="buttonLayoutRight"
                                        className="form-select-sm"
                                        groupClassName="col-sm-3 mb-3"
                                        value={values.buttonLayoutRight}
                                        error={errors.buttonLayoutRight}
                                        isInvalid={errors.buttonLayoutRight}
                                        onChange={handleChange}
                                    >
                                        {BUTTON_LAYOUTS_RIGHT.map((o, i) => (
                                            <option
                                                key={`buttonLayoutRight-option-${i}`}
                                                value={o.value}
                                            >
                                                {o.label}
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
                                            <option key={`splashMode-option-${i}`} value={o.value}>
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
                                                    value={values.buttonLayoutCustomOptions.params.layout}
                                                    onChange={handleChange}
                                                >
                                                    {BUTTON_LAYOUTS.slice(0, -1).map((o, i) => (
                                                        <option
                                                            key={`buttonLayout-option-${i}`}
                                                            value={o.value}
                                                        >
                                                            {o.label}
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
                                                        values.buttonLayoutCustomOptions.paramsRight.layout
                                                    }
                                                    onChange={handleChange}
                                                >
                                                    {BUTTON_LAYOUTS_RIGHT.slice(0, -1).map((o, i) => (
                                                        <option
                                                            key={`buttonLayoutRight-option-${i}`}
                                                            value={o.value}
                                                        >
                                                            {o.label}
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
                                        label={t('DisplayConfig:form.display-saver-timeout-label')}
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
                                            meta,
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
                                    <Button type="submit">{t('Common:button-save-label')}</Button>
                                    {saveMessage ? (
                                        <span className="alert">{saveMessage}</span>
                                    ) : null}
                                </div>
                                <FormContext />
                            </Form>
                        </div>
                        : 
                        <FormLabel><Trans ns="PeripheralMapping" i18nKey="peripheral-toggle-unavailable" values={{'name':'I2C'}}><NavLink exact="true" to="/peripheral-mapping">{t('PeripheralMapping:header-text')}</NavLink></Trans></FormLabel>
                    }
					</Section>
				)}
			}
		</Formik>
	);
}

// TODO: Initialize canvas with value
const Canvas = ({ value: bitsArray, onChange }) => {
	const [frames, setFrames] = useState(null);
    const [framesImageData, setFramesImageData] = useState(null);
    const [currentFrame, setCurrentFrame] = useState(0)
	const [canvasContext, setCanvasContext] = useState(null);
	const [inverted, setInverted] = useState(false);
    const [invisibleCanvas, setInvisibleCanvas] = useState(null)
    const [invisibleCanvasContext, setInvisibleCanvasContext] = useState(null)
	const canvasRef = useRef();
	const { t } = useTranslation('');

    const resizeAndAdjustColorForFrame = useCallback((frame) => {
        const { patch } = frame
        const { width: canvasWidth, height: canvasHeight} = canvasContext.canvas
        const { width: imgWidth, height: imgHeight} = frame.dims
        const imgRatio = imgWidth / imgHeight
        const canvasRatio = canvasWidth / canvasHeight
        var offsetX = 0
        var offsetY = 0
        var finalImageWidth = canvasWidth
        var finalImageHeight = canvasHeight

        // Convert to monochrome
		for (var i = 0; i < patch.length; i = i + 4) {
			const avg = (patch[i] + patch[i + 1] + patch[i + 2]) / 3;
            const color = avg > 123 ? 0 : 255
 
			patch[i] = color;
			patch[i + 1] = color;
			patch[i + 2] = color;
		}

        // Resize to fit into canvas
        if (imgRatio < canvasRatio) {
            finalImageWidth = canvasWidth * imgRatio / canvasRatio
            offsetX = (canvasWidth - finalImageWidth) / 2
        }
        else if (imgRatio > canvasRatio) {
            finalImageHeight = canvasHeight - canvasHeight * canvasRatio / imgRatio
            offsetY = (canvasHeight - finalImageHeight) / 2
        }

        const imageData = new ImageData(patch, imgWidth, imgHeight)
        invisibleCanvasContext.putImageData(imageData, 0, 0)
        invisibleCanvasContext.drawImage(invisibleCanvas, 0, 0, imgWidth, imgHeight, offsetX, offsetY, finalImageWidth, finalImageHeight)
    
        return invisibleCanvasContext.getImageData(0, 0, canvasWidth, canvasHeight);
    }, [canvasContext, invisibleCanvasContext, invisibleCanvas, inverted])

	const createFrameObject = useCallback((imgData, delay) => {
        const bitsArray = [delay]
        chunk(
			[...new Uint8Array(imgData)].filter((_data, index) => index % 4 === 0),
			8,
		).forEach((chunks) =>{
            bitsArray.push(chunks.reduce((acc, curr, i) => {
				return acc + ((curr === 255 ? 1 : 0) << (7 - i));
			}, 0))
        })

        console.log('bits array', bitsArray)
        return bitsArray
	}, [])

    const toggleInverted = useCallback(() => {
		setInverted(prevState => !prevState);
	}, []);

    useEffect(() => {
        const invisibleCanvas = document.createElement('canvas');
        invisibleCanvas.width = canvasRef.width
        invisibleCanvas.height = canvasRef.height

		setCanvasContext(canvasRef.current.getContext('2d'));
        setInvisibleCanvas(invisibleCanvas)
        setInvisibleCanvasContext(invisibleCanvas.getContext('2d'))
	}, []);

    useEffect(() => {
        if (!canvasContext) return;
        if (!framesImageData) return;
        if (!frames) return;

        const { width, height}  = canvasContext.canvas
        const imageData = framesImageData[currentFrame]
        const frame = frames[currentFrame]

        if (!framesImageData) return;
        if (!frame)return;

        canvasContext.putImageData(imageData, 0, 0, 0, 0, width, height);
         
        const timeout = setTimeout(() => {
            const nextFrame = currentFrame === frames.length - 1
                ? 0
                : currentFrame + 1
            setCurrentFrame(nextFrame)
        }, frame.delay)

        return () => {
            clearTimeout(timeout)
        }
    }, [canvasContext, frames, framesImageData, currentFrame])

	useEffect(() => {
		if (canvasContext == null) return;
        if (frames == null) return;

        const framesData = []
        const framesArray = [frames.length]

        frames.forEach((frame) => {
            const frameData = resizeAndAdjustColorForFrame(frame)
            framesData.push(frameData)
            framesArray.push(...createFrameObject(frameData.data, frame.delay)) 
        })
        setCurrentFrame(0)
        setFramesImageData(framesData)
        onChange(framesArray);
        console.log("framesArray: ", framesArray)
	}, [frames, canvasContext, resizeAndAdjustColorForFrame, createFrameObject])

	const onImageAdd = (ev) => {
		var file = ev.target.files[0];
		const { type: file_type } = file
		var fr = new FileReader();
		fr.onload = () => {
			if (file_type == "image/gif") {
				var gif = parseGIF(fr.result)
				var frames = decompressFrames(gif, true)
                const { width, height } = frames[0].dims
                invisibleCanvas.width = Math.max(width, canvasRef.current.width)
                invisibleCanvas.height = Math.max(height, canvasRef.current.height)
                setFrames(frames)
			}
			else {
				const img = new Image();
                img.onload = () => {
                    const { width, height } = img
                    invisibleCanvas.width = Math.max(width, canvasRef.current.width)
                    invisibleCanvas.height = Math.max(height, canvasRef.current.height)
                    invisibleCanvasContext.drawImage(img, 0, 0, width, height)
                    const patch = invisibleCanvasContext.getImageData(0, 0, width, height).data
                    setFrames([{
                        patch,
                        dims: {
                            height,
                            width
                        },
                        delay: -1
                    }]);
                }			
				img.src = fr.result;
			}
		};
		if (file_type == "image/gif") fr.readAsArrayBuffer(file);
		else fr.readAsDataURL(file);
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
					accept="image/jpeg, image/png, image/jpg, image/gif"
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
