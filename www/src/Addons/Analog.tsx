import { useContext } from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import { ANALOG_PINS } from '../Data/Buttons';
import AnalogPinOptions from '../Components/AnalogPinOptions';
import { AppContext } from '../Contexts/AppContext';
import FormControl from '../Components/FormControl';

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
	inner_deadzone: yup
		.number()
		.label('Inner Deadzone Size (%)')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 100),
	outer_deadzone: yup
		.number()
		.label('Outer Deadzone Size (%)')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 100),
	auto_calibrate: yup
		.number()
		.label('Auto Calibration')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 1),
	analog_smoothing: yup
		.number()
		.label('Analog Smoothing')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 1),
	smoothing_factor: yup
		.number()
		.label('Smoothing Factor')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 100),
	analog_error: yup
		.number()
		.label('Error Rate')
		.validateSelectionWhenValue('AnalogInputEnabled', ANALOG_ERROR_RATES),
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
	inner_deadzone: 5,
	outer_deadzone: 95,
	auto_calibrate: 0,
	analog_smoothing: 0,
	smoothing_factor: 5,
	analog_error: 1,
};

const Analog = ({ values, errors, handleChange, handleCheckbox }) => {
	const { usedPins } = useContext(AppContext);
	const { t } = useTranslation();
	const availableAnalogPins = ANALOG_PINS.filter(
		(pin) => !usedPins?.includes(pin),
	);

	return (
		<Section title={t('AddonsConfig:analog-header-text')}>
			<div id="AnalogInputOptions" hidden={!values.AnalogInputEnabled}>
				<p>{t('AddonsConfig:analog-warning')}</p>
				<p>
					{t('AddonsConfig:analog-available-pins-text', {
						pins: availableAnalogPins.join(', '),
					})}
				</p>
				<Row className="mb-3">
					<FormSelect
						label={t('AddonsConfig:analog-adc-1-pin-x-label')}
						name="analogAdc1PinX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogAdc1PinX}
						error={errors.analogAdc1PinX}
						isInvalid={errors.analogAdc1PinX}
						onChange={handleChange}
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
						isInvalid={errors.analogAdc1PinY}
						onChange={handleChange}
					>
						<AnalogPinOptions />
					</FormSelect>
					<FormSelect
						label={t('AddonsConfig:analog-adc-1-mode-label')}
						name="analogAdc1Mode"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogAdc1Mode}
						error={errors.analogAdc1Mode}
						isInvalid={errors.analogAdc1Mode}
						onChange={handleChange}
					>
						{ANALOG_STICK_MODES.map((o, i) => (
							<option key={`button-analogAdc1Mode-option-${i}`} value={o.value}>
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
						isInvalid={errors.analogAdc1Invert}
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
					<FormSelect
						label={t('AddonsConfig:analog-adc-2-pin-x-label')}
						name="analogAdc2PinX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogAdc2PinX}
						error={errors.analogAdc2PinX}
						isInvalid={errors.analogAdc2PinX}
						onChange={handleChange}
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
						isInvalid={errors.analogAdc2PinY}
						onChange={handleChange}
					>
						<AnalogPinOptions />
					</FormSelect>
					<FormSelect
						label={t('AddonsConfig:analog-adc-2-mode-label')}
						name="analogAdc2Mode"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogAdc2Mode}
						error={errors.analogAdc2Mode}
						isInvalid={errors.analogAdc2Mode}
						onChange={handleChange}
					>
						{ANALOG_STICK_MODES.map((o, i) => (
							<option key={`button-analogAdc2Mode-option-${i}`} value={o.value}>
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
						isInvalid={errors.analogAdc2Invert}
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
						name="inner_deadzone"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.inner_deadzone}
						error={errors.inner_deadzone}
						isInvalid={errors.inner_deadzone}
						onChange={handleChange}
						min={0}
						max={100}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:outer-deadzone-size')}
						name="outer_deadzone"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.outer_deadzone}
						error={errors.outer_deadzone}
						isInvalid={errors.outer_deadzone}
						onChange={handleChange}
						min={0}
						max={100}
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
					<FormControl
						hidden={!values.analog_smoothing}
						type="number"
						label={t('AddonsConfig:smoothing-factor')}
						name="smoothing_factor"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.smoothing_factor}
						error={errors.smoothing_factor}
						isInvalid={errors.smoothing_factor}
						onChange={handleChange}
						min={0}
						max={100}
					/>
					<FormCheck
						label={t('AddonsConfig:analog-force-circularity')}
						type="switch"
						id="Forced_circularity"
						className="col-sm-3 ms-2"
						isInvalid={false}
						checked={Boolean(values.forced_circularity)}
						onChange={(e) => {
							handleCheckbox('forced_circularity', values);
							handleChange(e);
						}}
					/>
					<FormCheck
						label={t('AddonsConfig:analog-auto-calibrate')}
						type="switch"
						id="Auto_calibrate"
						className="col-sm-3 ms-2"
						isInvalid={false}
						checked={Boolean(values.auto_calibrate)}
						onChange={(e) => {
							handleCheckbox('auto_calibrate', values);
							handleChange(e);
						}}
					/>
					<FormCheck
						label={t('AddonsConfig:analog-smoothing')}
						type="switch"
						id="Analog_smoothing"
						className="col-sm-3 ms-2"
						isInvalid={false}
						checked={Boolean(values.analog_smoothing)}
						onChange={(e) => {
							handleCheckbox('analog_smoothing', values);
							handleChange(e);
						}}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="AnalogInputButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.AnalogInputEnabled)}
				onChange={(e) => {
					handleCheckbox('AnalogInputEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Analog;
