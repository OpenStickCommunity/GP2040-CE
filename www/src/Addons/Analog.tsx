import React, { useContext } from 'react';
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

const ANALOG_SENSITIVITY_MODES = [
	{ label: '0.25', value: 25 },
	{ label: '0.3', value: 30 },
	{ label: '0.4', value: 40 },
	{ label: '0.5', value: 50 },
	{ label: '0.6', value: 60 },
	{ label: '0.7', value: 70 },
	{ label: '0.8', value: 80 },
	{ label: '0.9', value: 90 },
	{ label: '1.0', value: 100 },
	{ label: '1.1', value: 110 },
	{ label: '1.2', value: 120 },
	{ label: '1.3', value: 130 },
	{ label: '1.4', value: 140 },
	{ label: '1.5', value: 150 },
	{ label: '1.6', value: 160 },
	{ label: '1.7', value: 170 },
	{ label: '1.8', value: 180 },
	{ label: '1.9', value: 190 },
	{ label: '2.0', value: 200 },
	{ label: '2.2', value: 220 },
	{ label: '2.4', value: 240 },
	{ label: '2.6', value: 260 },
	{ label: '2.8', value: 280 },
	{ label: '3.0', value: 300 },
	{ label: '3.2', value: 320 },
	{ label: '3.4', value: 340 },
	{ label: '3.6', value: 360 },
	{ label: '3.8', value: 380 },
	{ label: '4.0', value: 400 },
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

	analog_deadzone: yup
		.number()
		.label('Deadzone Size (%)')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 100),
	analog_sensitivity: yup
		.number()
		.label('ANALOG SENSITIVITY')
		.validateSelectionWhenValue('AnalogInputEnabled', ANALOG_SENSITIVITY_MODES),
	forced_circularity: yup
		.number()
		.label('Force Circularity')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 1),
	auto_calibrate: yup
		.number()
		.label('Auto Calibration')
		.validateRangeWhenValue('AnalogInputEnabled', 0, 1),
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
	analog_deadzone: 5,
	auto_calibrate: 0,
	analog_sensitivity: 100,
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
						label={t('AddonsConfig:analog-deadzone-size')}
						name="analog_deadzone"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analog_deadzone}
						error={errors.analog_deadzone}
						isInvalid={errors.analog_deadzone}
						onChange={handleChange}
						min={0}
						max={100}
					/>
					<FormSelect
						label={t('AddonsConfig:analog-sensitivity')}
						name="analog_sensitivity"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analog_sensitivity}
						error={errors.analog_sensitivity}
						isInvalid={errors.analog_sensitivity}
						onChange={handleChange}
					>
						{ANALOG_SENSITIVITY_MODES.map((o, i) => (
							<option key={`button-powtilt-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
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
