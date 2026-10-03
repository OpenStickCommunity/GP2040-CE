import { useContext, useEffect, useRef, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Button, FormCheck, Modal, Row, Tab, Tabs } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import AnalogPinOptions from '../Components/AnalogPinOptions';
import { AppContext } from '../Contexts/AppContext';
import FormControl from '../Components/FormControl';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

import useBoardDefinition from '../Store/useBoardDefinitionStore';

const ANALOG_STICK_MODES = [
	{ label: 'Left Analog', value: 1 },
	{ label: 'Right Analog', value: 2 },
];

const INVERT_MODES = [
	{ label: 'None', value: 0 },
	{ label: 'X Axis', value: 1 },
	{ label: 'Y Axis', value: 2 },
	{ label: 'X/Y Axis', value: 3 },
];

const ANALOG_ERROR_RATES = [
	{ label: '0%', value: 1000 },
	{ label: '1%', value: 990 },
	{ label: '2%', value: 979 },
	{ label: '3%', value: 969 },
	{ label: '4%', value: 958 },
	{ label: '5%', value: 946 },
	{ label: '6%', value: 934 },
	{ label: '7%', value: 922 },
	{ label: '8%', value: 911 },
	{ label: '9%', value: 900 },
	{ label: '10%', value: 890 },
	{ label: '11%', value: 876 },
	{ label: '12%', value: 863 },
	{ label: '13%', value: 848 },
	{ label: '14%', value: 834 },
	{ label: '15%', value: 821 },
];

export const analogScheme = {
	AnalogInputEnabled: yup.number().required().label('Analog Input Enabled'),
	analogAdc1PinX: yup
		.number()
		.label('Analog Stick 1 Pin X')
		.validatePinWhenValue('AnalogInputEnabled'),
	analogAdc1PinY: yup
		.number()
		.label('Analog Stick 1 Pin Y')
		.validatePinWhenValue('AnalogInputEnabled'),
	analogAdc1Mode: yup
		.number()
		.label('Analog Stick 1 Mode')
		.validateSelectionWhenValue('AnalogInputEnabled', ANALOG_STICK_MODES),
	analogAdc1Invert: yup
		.number()
		.label('Analog Stick 1 Invert')
		.validateSelectionWhenValue('AnalogInputEnabled', INVERT_MODES),
	analogAdc2PinX: yup
		.number()
		.label('Analog Stick 2 Pin X')
		.validatePinWhenValue('AnalogInputEnabled'),
	analogAdc2PinY: yup
		.number()
		.label('Analog Stick 2 Pin Y')
		.validatePinWhenValue('AnalogInputEnabled'),
	analogAdc2Mode: yup
		.number()
		.label('Analog Stick 2 Mode')
		.validateSelectionWhenValue('AnalogInputEnabled', ANALOG_STICK_MODES),
	analogAdc2Invert: yup
		.number()
		.label('Analog Stick 2 Invert')
		.validateSelectionWhenValue('AnalogInputEnabled', INVERT_MODES),

	forced_circularity: yup
		.number()
		.label('Force Circularity')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 1),
	forced_circularity2: yup
		.number()
		.label('Force Circularity')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 1),
	inner_deadzone: yup
		.number()
		.label('Inner Deadzone Size (%)')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 99),
	inner_deadzone2: yup
		.number()
		.label('Inner Deadzone Size (%)')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 99),
	outer_deadzone: yup
		.number()
		.label('Outer Deadzone Size (%)')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 100)
		.when('AnalogInputEnabled', {
			is: 1,
			then: (schema: yup.NumberSchema) =>
				schema.moreThan(yup.ref('inner_deadzone')),
		}),
	outer_deadzone2: yup
		.number()
		.label('Outer Deadzone Size (%)')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 100)
		.when('AnalogInputEnabled', {
			is: 1,
			then: (schema: yup.NumberSchema) =>
				schema.moreThan(yup.ref('inner_deadzone2')),
		}),
	auto_calibrate: yup
		.number()
		.label('Auto Calibration')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 1),
	auto_calibrate2: yup
		.number()
		.label('Auto Calibration')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 1),
	analog_smoothing: yup
		.number()
		.label('Analog Smoothing')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 1),
	analog_smoothing2: yup
		.number()
		.label('Analog Smoothing 2')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 1),
	smoothing_factor: yup
		.number()
		.label('Smoothing Factor')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 10),
	smoothing_factor2: yup
		.number()
		.label('Smoothing Factor 2')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 10),
	analog_error: yup
		.number()
		.label('Error Rate')
		.validateSelectionWhenValue('AnalogInputEnabled', ANALOG_ERROR_RATES),
	analog_error2: yup
		.number()
		.label('Error Rate 2')
		.validateSelectionWhenValue('AnalogInputEnabled', ANALOG_ERROR_RATES),
	joystickCenterX: yup
		.number()
		.label('Joystick Center X')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 4095),
	joystickCenterY: yup
		.number()
		.label('Joystick Center Y')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 4095),
	joystickCenterX2: yup
		.number()
		.label('Joystick Center X2')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 4095),
	joystickCenterY2: yup
		.number()
		.label('Joystick Center Y2')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 4095),
	joystickMinX: yup
		.number()
		.validateRangeWhenValue('AnalogInputEnabled', 0, 4095),
	joystickMaxX: yup
		.number()
		.validateRangeWhenValue('AnalogInputEnabled', 0, 4095),
	joystickMinY: yup
		.number()
		.validateRangeWhenValue('AnalogInputEnabled', 0, 4095),
	joystickMaxY: yup
		.number()
		.validateRangeWhenValue('AnalogInputEnabled', 0, 4095),
	joystickMinX2: yup
		.number()
		.validateRangeWhenValue('AnalogInputEnabled', 0, 4095),
	joystickMaxX2: yup
		.number()
		.validateRangeWhenValue('AnalogInputEnabled', 0, 4095),
	joystickMinY2: yup
		.number()
		.validateRangeWhenValue('AnalogInputEnabled', 0, 4095),
	joystickMaxY2: yup
		.number()
		.validateRangeWhenValue('AnalogInputEnabled', 0, 4095),
};

export const analogState = {
	AnalogInputEnabled: 0,
	analogAdc1PinX: -1,
	analogAdc1PinY: -1,
	analogAdc1Mode: 1,
	analogAdc1Invert: 0,
	analogAdc2PinX: -1,
	analogAdc2PinY: -1,
	analogAdc2Mode: 2,
	analogAdc2Invert: 0,
	forced_circularity: 0,
	forced_circularity2: 0,
	inner_deadzone: 5,
	inner_deadzone2: 5,
	outer_deadzone: 95,
	outer_deadzone2: 95,
	auto_calibrate: 0,
	auto_calibrate2: 0,
	joystickCenterX: 0,
	joystickCenterY: 0,
	joystickCenterX2: 0,
	joystickCenterY2: 0,
	joystickMinX: 0,
	joystickMaxX: 4095,
	joystickMinY: 0,
	joystickMaxY: 4095,
	joystickMinX2: 0,
	joystickMaxX2: 4095,
	joystickMinY2: 0,
	joystickMaxY2: 4095,
	analog_smoothing: 0,
	analog_smoothing2: 0,
	smoothing_factor: 2,
	smoothing_factor2: 2,
	analog_error: 1000,
	analog_error2: 1000,
};

const Analog = ({
	values,
	errors,
	handleChange,
	handleCheckbox,
	setFieldValue,
}: AddonPropTypes) => {
	const { usedPins } = useContext(AppContext);
	const { t } = useTranslation();
	const { boardDefinition } = useBoardDefinition();

	const ANALOG_PINS = boardDefinition.analogPins;

	const [calibrationStep, setCalibrationStep] = useState('');
	const calibration = useRef<{
		controller: AbortController;
		centerStarted: number;
	} | null>(null);

	useEffect(() => {
		return () => {
			calibration.current?.controller.abort('cancel');
			calibration.current = null;
		};
	}, []);

	const captureCenter = () => {
		if (!calibration.current || calibrationStep !== 'ready') {
			return;
		}
		calibration.current.centerStarted = performance.now();
		setCalibrationStep('center');
	};

	const calibrateStick = async (stick: 1 | 2) => {
		if (calibration.current) {
			return;
		}

		const suffix = stick === 1 ? '' : '2';
		const pins = {
			X: Number(values[`analogAdc${stick}PinX`]),
			Y: Number(values[`analogAdc${stick}PinY`]),
		};
		const axes = (['X', 'Y'] as const)
			.filter((axis) => pins[axis] >= 26 && pins[axis] <= 29)
			.map((axis) => ({ axis, low: [4095, 4095], high: [0, 0] }));
		const restSamples: { time: number; values: number[] }[] = [];
		const session = { controller: new AbortController(), centerStarted: 0 };
		const { signal } = session.controller;
		calibration.current = session;
		setCalibrationStep('rotation');

		try {
			if (!axes.length) {
				throw new Error(t('AddonsConfig:analog-calibration-select-pins'));
			}

			while (!signal.aborted) {
				const timeout = setTimeout(() => {
					session.controller.abort(
						new Error(t('AddonsConfig:analog-calibration-connection-timeout')),
					);
				}, 5000);
				let sample: {
					success: boolean;
					error?: string;
					pinX: number;
					pinY: number;
					x: number;
					y: number;
				};
				try {
					const response = await fetch(`/api/getJoystickCenter${suffix}`, {
						signal,
						cache: 'no-store',
					});
					if (!response.ok) {
						throw new Error(`HTTP ${response.status}`);
					}
					sample = await response.json();
				} finally {
					clearTimeout(timeout);
				}

				if (signal.aborted) {
					return;
				}
				if (!sample.success) {
					throw new Error(sample.error);
				}
				if (sample.pinX !== pins.X || sample.pinY !== pins.Y) {
					throw new Error(t('AddonsConfig:analog-calibration-save-pins'));
				}

				const readings = axes.map(
					({ axis }) => sample[axis === 'X' ? 'x' : 'y'],
				);
				if (
					readings.some(
						(value) => !Number.isInteger(value) || value < 0 || value > 4095,
					)
				) {
					throw new Error('Invalid ADC reading');
				}

				if (!session.centerStarted) {
					axes.forEach((axis, index) => {
						axis.low = [...axis.low, readings[index]]
							.sort((a, b) => a - b)
							.slice(0, 2);
						axis.high = [...axis.high, readings[index]]
							.sort((a, b) => b - a)
							.slice(0, 2);
					});
					const hasEnoughTravel = axes.every(
						({ low, high }) => high[1] - low[1] >= 64,
					);
					setCalibrationStep(hasEnoughTravel ? 'ready' : 'rotation');
				} else {
					const now = performance.now();
					if (now - session.centerStarted > 30000) {
						throw new Error(t('AddonsConfig:analog-calibration-rest-timeout'));
					}

					const nearTravelLimit = axes.some(
						({ low, high }, index) =>
							readings[index] - low[1] < 32 || high[1] - readings[index] < 32,
					);
					if (nearTravelLimit) {
						restSamples.length = 0;
					} else {
						restSamples.push({ time: now, values: readings });
						const moved = axes.some(({ low, high }, index) => {
							const samples = restSamples.map((sample) => sample.values[index]);
							const movement = Math.max(...samples) - Math.min(...samples);
							const tolerance = Math.max(4, (high[1] - low[1]) * 0.005);
							return movement > tolerance;
						});
						if (moved) {
							restSamples.splice(0, restSamples.length - 1);
						}
					}

					const centerIsStable =
						restSamples.length >= 10 && now - restSamples[0].time >= 1000;
					if (centerIsStable) {
						for (const axis of ['X', 'Y']) {
							const index = axes.findIndex((entry) => entry.axis === axis);
							const limits = axes[index];
							let center = 0;
							if (limits) {
								const total = restSamples.reduce(
									(sum, sample) => sum + sample.values[index],
									0,
								);
								center = Math.round(total / restSamples.length);
							}
							await setFieldValue(
								`joystickCenter${axis}${suffix}`,
								center,
								false,
							);
							await setFieldValue(
								`joystickMin${axis}${suffix}`,
								limits?.low[1] ?? 0,
								false,
							);
							await setFieldValue(
								`joystickMax${axis}${suffix}`,
								limits?.high[1] ?? 4095,
								false,
							);
						}
						alert(t('AddonsConfig:analog-calibration-save-notice'));
						return;
					}
				}
				await new Promise((resolve) => setTimeout(resolve, 25));
			}
		} catch (error) {
			if (signal.reason !== 'cancel') {
				alert(
					t('AddonsConfig:analog-calibration-failed', {
						error: String(signal.reason || error),
					}),
				);
			}
		} finally {
			if (calibration.current === session) {
				calibration.current = null;
				setCalibrationStep('');
			}
		}
	};

	const handlePinChange = (
		event: React.ChangeEvent<HTMLSelectElement>,
		axis: string,
	) => {
		setFieldValue(`joystickCenter${axis}`, 0, false);
		setFieldValue(`joystickMin${axis}`, 0, false);
		setFieldValue(`joystickMax${axis}`, 4095, false);
		handleChange(event);
	};

	const availableAnalogPins = ANALOG_PINS.filter(
		(pin) => !usedPins?.includes(pin),
	);

	return (
		<Section
			title={
				<a
					href="https://gp2040-ce.info/add-ons/analog"
					target="_blank"
					className="text-reset text-decoration-none"
				>
					{t('AddonsConfig:analog-header-text')}
				</a>
			}
		>
			<div id="AnalogInputOptions" hidden={!values.AnalogInputEnabled}>
				<div className="alert alert-info" role="alert">
					{t('AddonsConfig:analog-warning')}
				</div>
				<div className="alert alert-success" role="alert">
					{t('AddonsConfig:analog-available-pins-text', {
						pins: availableAnalogPins.join(', '),
					})}
				</div>
				<Tabs
					defaultActiveKey="analog1Config"
					id="analogConfigTabs"
					className="mb-3 pb-0"
					fill
				>
					<Tab
						key="analog1Config"
						eventKey="analog1Config"
						title={t('AddonsConfig:analog-adc-1')}
					>
						<Row className="mb-3">
							<FormSelect
								label={t('AddonsConfig:analog-adc-1-pin-x-label')}
								name="analogAdc1PinX"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.analogAdc1PinX}
								error={errors.analogAdc1PinX}
								isInvalid={Boolean(errors.analogAdc1PinX)}
								onChange={(event) => handlePinChange(event, 'X')}
							>
								<AnalogPinOptions />
							</FormSelect>
							<FormSelect
								label={t('AddonsConfig:analog-adc-1-pin-y-label')}
								name="analogAdc1PinY"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.analogAdc1PinY}
								error={errors.analogAdc1PinY}
								isInvalid={Boolean(errors.analogAdc1PinY)}
								onChange={(event) => handlePinChange(event, 'Y')}
							>
								<AnalogPinOptions />
							</FormSelect>
							<Row className="mb-3">
								<FormSelect
									label={t('AddonsConfig:analog-adc-1-mode-label')}
									name="analogAdc1Mode"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdc1Mode}
									error={errors.analogAdc1Mode}
									isInvalid={Boolean(errors.analogAdc1Mode)}
									onChange={handleChange}
								>
									{ANALOG_STICK_MODES.map((o, i) => (
										<option
											key={`button-analogAdc1Mode-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
								<FormSelect
									label={t('AddonsConfig:analog-adc-1-invert-label')}
									name="analogAdc1Invert"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdc1Invert}
									error={errors.analogAdc1Invert}
									isInvalid={Boolean(errors.analogAdc1Invert)}
									onChange={handleChange}
								>
									{INVERT_MODES.map((o, i) => (
										<option
											key={`button-analogAdc1Invert-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
							</Row>
							<Row className="mb-3">
								<FormControl
									type="number"
									label={t('AddonsConfig:inner-deadzone-size')}
									name="inner_deadzone"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.inner_deadzone}
									error={errors.inner_deadzone}
									isInvalid={Boolean(errors.inner_deadzone)}
									onChange={handleChange}
									min={0}
									max={99}
								/>
								<FormControl
									type="number"
									label={t('AddonsConfig:outer-deadzone-size')}
									name="outer_deadzone"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.outer_deadzone}
									error={errors.outer_deadzone}
									isInvalid={Boolean(errors.outer_deadzone)}
									onChange={handleChange}
									min={Math.min(Number(values.inner_deadzone) + 1, 100)}
									max={100}
								/>
							</Row>
							<Row className="mb-3">
								<FormCheck
									label={t('AddonsConfig:analog-smoothing')}
									type="switch"
									id="Analog_smoothing"
									className="col-sm-3 ms-3"
									isInvalid={false}
									checked={Boolean(values.analog_smoothing)}
									onChange={(e) => {
										handleCheckbox('analog_smoothing');
										handleChange(e);
									}}
								/>
								<FormControl
									hidden={!values.analog_smoothing}
									type="number"
									label={t('AddonsConfig:smoothing-factor')}
									name="smoothing_factor"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.smoothing_factor}
									error={errors.smoothing_factor}
									isInvalid={Boolean(errors.smoothing_factor)}
									onChange={handleChange}
									min={0}
									max={10}
								/>
							</Row>
							<Row className="mb-3">
								<FormCheck
									label={t('AddonsConfig:analog-force-circularity')}
									type="switch"
									id="Forced_circularity"
									className="col-sm-3 ms-3"
									isInvalid={false}
									checked={Boolean(values.forced_circularity)}
									onChange={(e) => {
										handleCheckbox('forced_circularity');
										handleChange(e);
									}}
								/>
								<FormSelect
									hidden={!values.forced_circularity}
									label={t('AddonsConfig:analog-error-label')}
									name="analog_error"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analog_error}
									onChange={handleChange}
								>
									{ANALOG_ERROR_RATES.map((o, i) => (
										<option key={`analog_error-option-${i}`} value={o.value}>
											{o.label}
										</option>
									))}
								</FormSelect>
							</Row>
							<Row className="align-items-center">
								<FormCheck
									label={t('AddonsConfig:analog-auto-calibrate')}
									type="switch"
									id="Auto_calibrate"
									className="col-sm-3 ms-3"
									isInvalid={false}
									checked={Boolean(values.auto_calibrate)}
									onChange={(e) => {
										handleCheckbox('auto_calibrate');
										handleChange(e);
									}}
								/>
								<button
									type="button"
									className="col-auto btn btn-sm btn-outline-secondary ms-2"
									disabled={Boolean(values.auto_calibrate)}
									onClick={() => calibrateStick(1)}
								>
									{t('AddonsConfig:analog-calibrate-stick-1-button')}
								</button>
								<div className="col-auto ms-3 small text-muted">
									{`Center: X=${values.joystickCenterX}, Y=${values.joystickCenterY}`}
								</div>
							</Row>
							{Boolean(values.auto_calibrate) && (
								<div className="alert alert-info mt-2 mb-3">
									<small>
										<strong>
											{t(
												'AddonsConfig:analog-auto-calibration-enabled-stick-1',
											)}
											：
										</strong>{' '}
										{t(
											'AddonsConfig:analog-calibration-auto-mode-instruction',
											{ stick: '1' },
										)}
									</small>
								</div>
							)}
							{!Boolean(values.auto_calibrate) && (
								<div className="alert alert-warning mt-2 mb-3">
									<small>
										<strong>
											{t('AddonsConfig:analog-manual-calibration-mode-stick-1')}
											：
										</strong>
										<br />
										{t('AddonsConfig:analog-calibration-rotation-overview')}
									</small>
								</div>
							)}
						</Row>
					</Tab>
					<Tab
						key="analog2Config"
						eventKey="analog2Config"
						title={t('AddonsConfig:analog-adc-2')}
					>
						<Row className="mb-3">
							<FormSelect
								label={t('AddonsConfig:analog-adc-2-pin-x-label')}
								name="analogAdc2PinX"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.analogAdc2PinX}
								error={errors.analogAdc2PinX}
								isInvalid={Boolean(errors.analogAdc2PinX)}
								onChange={(event) => handlePinChange(event, 'X2')}
							>
								<AnalogPinOptions />
							</FormSelect>
							<FormSelect
								label={t('AddonsConfig:analog-adc-2-pin-y-label')}
								name="analogAdc2PinY"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.analogAdc2PinY}
								error={errors.analogAdc2PinY}
								isInvalid={Boolean(errors.analogAdc2PinY)}
								onChange={(event) => handlePinChange(event, 'Y2')}
							>
								<AnalogPinOptions />
							</FormSelect>
							<Row className="mb-3">
								<FormSelect
									label={t('AddonsConfig:analog-adc-2-mode-label')}
									name="analogAdc2Mode"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdc2Mode}
									error={errors.analogAdc2Mode}
									isInvalid={Boolean(errors.analogAdc2Mode)}
									onChange={handleChange}
								>
									{ANALOG_STICK_MODES.map((o, i) => (
										<option
											key={`button-analogAdc2Mode-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
								<FormSelect
									label={t('AddonsConfig:analog-adc-2-invert-label')}
									name="analogAdc2Invert"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analogAdc2Invert}
									error={errors.analogAdc2Invert}
									isInvalid={Boolean(errors.analogAdc2Invert)}
									onChange={handleChange}
								>
									{INVERT_MODES.map((o, i) => (
										<option
											key={`button-analogAdc2Invert-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
							</Row>
							<Row className="mb-3">
								<FormControl
									type="number"
									label={t('AddonsConfig:inner-deadzone-size')}
									name="inner_deadzone2"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.inner_deadzone2}
									error={errors.inner_deadzone2}
									isInvalid={Boolean(errors.inner_deadzone2)}
									onChange={handleChange}
									min={0}
									max={99}
								/>
								<FormControl
									type="number"
									label={t('AddonsConfig:outer-deadzone-size')}
									name="outer_deadzone2"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.outer_deadzone2}
									error={errors.outer_deadzone2}
									isInvalid={Boolean(errors.outer_deadzone2)}
									onChange={handleChange}
									min={Math.min(Number(values.inner_deadzone2) + 1, 100)}
									max={100}
								/>
							</Row>
							<Row className="mb-3">
								<FormCheck
									label={t('AddonsConfig:analog-smoothing')}
									type="switch"
									id="Analog_smoothing2"
									className="col-sm-3 ms-3"
									isInvalid={false}
									checked={Boolean(values.analog_smoothing2)}
									onChange={(e) => {
										handleCheckbox('analog_smoothing2');
										handleChange(e);
									}}
								/>
								<FormControl
									hidden={!values.analog_smoothing2}
									type="number"
									label={t('AddonsConfig:smoothing-factor')}
									name="smoothing_factor2"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.smoothing_factor2}
									error={errors.smoothing_factor2}
									isInvalid={Boolean(errors.smoothing_factor2)}
									onChange={handleChange}
									min={0}
									max={10}
								/>
							</Row>
							<Row className="mb-3">
								<FormCheck
									label={t('AddonsConfig:analog-force-circularity')}
									type="switch"
									id="Forced_circularity2"
									className="col-sm-3 ms-3"
									isInvalid={false}
									checked={Boolean(values.forced_circularity2)}
									onChange={(e) => {
										handleCheckbox('forced_circularity2');
										handleChange(e);
									}}
								/>
								<FormSelect
									hidden={!values.forced_circularity2}
									label={t('AddonsConfig:analog-error-label')}
									name="analog_error2"
									className="form-control-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.analog_error2}
									onChange={handleChange}
								>
									{ANALOG_ERROR_RATES.map((o, i) => (
										<option key={`analog_error-option-${i}`} value={o.value}>
											{o.label}
										</option>
									))}
								</FormSelect>
							</Row>
							<Row className="align-items-center">
								<FormCheck
									label={t('AddonsConfig:analog-auto-calibrate')}
									type="switch"
									id="Auto_calibrate2"
									className="col-sm-3 ms-3"
									isInvalid={false}
									checked={Boolean(values.auto_calibrate2)}
									onChange={(e) => {
										handleCheckbox('auto_calibrate2');
										handleChange(e);
									}}
								/>
								<button
									type="button"
									className="col-auto btn btn-sm btn-outline-secondary ms-2"
									disabled={Boolean(values.auto_calibrate2)}
									onClick={() => calibrateStick(2)}
								>
									{t('AddonsConfig:analog-calibrate-stick-2-button')}
								</button>
								<div className="col-auto ms-3 small text-muted">
									{`Center: X=${values.joystickCenterX2}, Y=${values.joystickCenterY2}`}
								</div>
							</Row>
							{Boolean(values.auto_calibrate2) && (
								<div className="alert alert-info mt-2 mb-3">
									<small>
										<strong>
											{t(
												'AddonsConfig:analog-auto-calibration-enabled-stick-2',
											)}
											：
										</strong>{' '}
										{t(
											'AddonsConfig:analog-calibration-auto-mode-instruction',
											{ stick: '2' },
										)}
									</small>
								</div>
							)}
							{!Boolean(values.auto_calibrate2) && (
								<div className="alert alert-warning mt-2 mb-3">
									<small>
										<strong>
											{t('AddonsConfig:analog-manual-calibration-mode-stick-2')}
											：
										</strong>
										<br />
										{t('AddonsConfig:analog-calibration-rotation-overview')}
									</small>
								</div>
							)}
						</Row>
					</Tab>
				</Tabs>
			</div>
			<Modal
				show={!!calibrationStep}
				backdrop="static"
				onHide={() => calibration.current?.controller.abort('cancel')}
			>
				<Modal.Header closeButton>
					<Modal.Title>
						{t('AddonsConfig:analog-calibration-title')}
					</Modal.Title>
				</Modal.Header>
				<Modal.Body role="status">
					{t(
						calibrationStep === 'center'
							? 'AddonsConfig:analog-calibration-release'
							: 'AddonsConfig:analog-calibration-rotate',
					)}
				</Modal.Body>
				<Modal.Footer>
					<Button
						variant="secondary"
						onClick={() => calibration.current?.controller.abort('cancel')}
					>
						{t('AddonsConfig:analog-calibration-cancel')}
					</Button>
					{calibrationStep !== 'center' && (
						<Button
							disabled={calibrationStep !== 'ready'}
							onClick={captureCenter}
						>
							{t('AddonsConfig:analog-calibration-done-rotating')}
						</Button>
					)}
				</Modal.Footer>
			</Modal>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="AnalogInputButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.AnalogInputEnabled)}
				onChange={(e) => {
					handleCheckbox('AnalogInputEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Analog;
