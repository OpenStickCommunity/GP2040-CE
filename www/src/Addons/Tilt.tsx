import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';

import FormControl from '../Components/FormControl';
import { SOCD_MODES, TILT_SOCD_MODES } from '../Data/Addons';

export const tiltScheme = {
	TiltInputEnabled: yup.number().required().label('Tilt Input Enabled'),
	tilt1Pin: yup
		.number()
		.label('Tilt 1 Pin')
		.validatePinWhenValue('TiltInputEnabled'),	
	tilt1FactorLeftX: yup
		.number()
		.label('Tilt 1 Factor Left X')
		.validatePinWhenValue('TiltInputEnabled'),
	tilt1FactorLeftY: yup
		.number()
		.label('Tilt 1 Factor Left Y')
		.validatePinWhenValue('TiltInputEnabled'),
	tilt1FactorRightX: yup
		.number()
		.label('Tilt 1 Factor Right X')
		.validatePinWhenValue('TiltInputEnabled'),
	tilt1FactorRightY: yup
		.number()
		.label('Tilt 1 Factor Right Y')
		.validatePinWhenValue('TiltInputEnabled'),
	tilt2Pin: yup
		.number()
		.label('Tilt 2 Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tilt2FactorLeftX: yup
		.number()
		.label('Tilt 2 Factor Left X')
		.validatePinWhenValue('TiltInputEnabled'),
	tilt2FactorLeftY: yup
		.number()
		.label('Tilt 2 Factor Left Y')
		.validatePinWhenValue('TiltInputEnabled'),
	tilt2FactorRightX: yup
		.number()
		.label('Tilt 2 Factor Right X')
		.validatePinWhenValue('TiltInputEnabled'),
	tilt2FactorRightY: yup
		.number()
		.label('Tilt 2 Factor Right Y')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltLeftAnalogUpPin: yup
		.number()
		.label('Tilt Left Analog Up Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltLeftAnalogDownPin: yup
		.number()
		.label('Tilt Left Analog Down Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltLeftAnalogLeftPin: yup
		.number()
		.label('Tilt Left Analog Left Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltLeftAnalogRightPin: yup
		.number()
		.label('Tilt Left Analog Right Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltRightAnalogUpPin: yup
		.number()
		.label('Tilt Right Analog Up Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltRightAnalogDownPin: yup
		.number()
		.label('Tilt Right Analog Down Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltRightAnalogLeftPin: yup
		.number()
		.label('Tilt Right Analog Left Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltRightAnalogRightPin: yup
		.number()
		.label('Tilt Right Analog Right Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	tiltSOCDMode: yup
		.number()
		.label('Tilt SOCE Mode')
		.validateSelectionWhenValue('TiltInputEnabled', SOCD_MODES),
};

export const tiltState = {
	TiltInputEnabled: 0,
	tilt1Pin: -1,
	tilt1FactorLeftX: 0.35,
	tilt1FactorLeftY: 0.45,
	tilt1FactorRightX: 0.3,
	tilt1FactorRightY: 1.7,
	tilt2Pin: -1,
	tilt2FactorLeftX: 0.65,
	tilt2FactorLeftY: 0.35,
	tilt2FactorRightX: 0.3,
	tilt2FactorRightY: 0.3,
	tiltLeftAnalogUpPin: -1,
	tiltLeftAnalogDownPin: -1,
	tiltLeftAnalogLeftPin: -1,
	tiltLeftAnalogRightPin: -1,
	tiltRightAnalogUpPin: -1,
	tiltRightAnalogDownPin: -1,
	tiltRightAnalogLeftPin: -1,
	tiltRightAnalogRightPin: -1,
};

const Tilt = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:tilt-header-text')}>
			<div id="TiltOptions" hidden={!values.TiltInputEnabled}>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-1-pin-label')}
						name="tilt1Pin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tilt1Pin}
						error={errors.tilt1Pin}
						isInvalid={errors.tilt1Pin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-1-factor-left-x-label')}
						name="tilt1FactorLeftX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tilt1FactorLeftX}
						error={errors.tilt1FactorLeftX}
						isInvalid={errors.tilt1FactorLeftX}
						onChange={handleChange}
						min={0.00}
						max={2.00}
						step={0.01}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-1-factor-left-y-label')}
						name="tilt1FactorLeftY"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tilt1FactorLeftY}
						error={errors.tilt1FactorLeftY}
						isInvalid={errors.tilt1FactorLeftY}
						onChange={handleChange}
						min={0.00}
						max={2.00}
						step={0.01}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-1-factor-right-x-label')}
						name="tilt1FactorRightX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tilt1FactorRightX}
						error={errors.tilt1FactorRightX}
						isInvalid={errors.tilt1FactorRightX}
						onChange={handleChange}
						min={0.00}
						max={2.00}
						step={0.01}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-1-factor-right-y-label')}
						name="tilt1FactorRightY"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tilt1FactorRightY}
						error={errors.tilt1FactorRightY}
						isInvalid={errors.tilt1FactorRightY}
						onChange={handleChange}
						min={0.00}
						max={2.00}
						step={0.01}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-2-pin-label')}
						name="tilt2Pin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tilt2Pin}
						error={errors.tilt2Pin}
						isInvalid={errors.tilt2Pin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-2-factor-left-x-label')}
						name="tilt2FactorLeftX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tilt2FactorLeftX}
						error={errors.tilt2FactorLeftX}
						isInvalid={errors.tilt2FactorLeftX}
						onChange={handleChange}
						min={0.00}
						max={2.00}
						step={0.01}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-2-factor-left-y-label')}
						name="tilt2FactorLeftY"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tilt2FactorLeftY}
						error={errors.tilt2FactorLeftY}
						isInvalid={errors.tilt2FactorLeftY}
						onChange={handleChange}
						min={0.00}
						max={2.00}
						step={0.01}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-2-factor-right-x-label')}
						name="tilt2FactorRightX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tilt2FactorRightX}
						error={errors.tilt2FactorRightX}
						isInvalid={errors.tilt2FactorRightX}
						onChange={handleChange}
						min={0.00}
						max={2.00}
						step={0.01}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-2-factor-right-y-label')}
						name="tilt2FactorRightY"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tilt2FactorRightY}
						error={errors.tilt2FactorRightY}
						isInvalid={errors.tilt2FactorRightY}
						onChange={handleChange}
						min={0.00}
						max={2.00}
						step={0.01}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-left-analog-up-pin-label')}
						name="tiltLeftAnalogUpPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tiltLeftAnalogUpPin}
						error={errors.tiltLeftAnalogUpPin}
						isInvalid={errors.tiltLeftAnalogUpPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-left-analog-down-pin-label')}
						name="tiltLeftAnalogDownPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tiltLeftAnalogDownPin}
						error={errors.tiltLeftAnalogDownPin}
						isInvalid={errors.tiltLeftAnalogDownPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-left-analog-left-pin-label')}
						name="tiltLeftAnalogLeftPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tiltLeftAnalogLeftPin}
						error={errors.tiltLeftAnalogLeftPin}
						isInvalid={errors.tiltLeftAnalogLeftPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-left-analog-right-pin-label')}
						name="tiltLeftAnalogRightPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tiltLeftAnalogRightPin}
						error={errors.tiltLeftAnalogRightPin}
						isInvalid={errors.tiltLeftAnalogRightPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-right-analog-up-pin-label')}
						name="tiltRightAnalogUpPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tiltRightAnalogUpPin}
						error={errors.tiltRightAnalogUpPin}
						isInvalid={errors.tiltRightAnalogUpPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-right-analog-down-pin-label')}
						name="tiltRightAnalogDownPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tiltRightAnalogDownPin}
						error={errors.tiltRightAnalogDownPin}
						isInvalid={errors.tiltRightAnalogDownPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-right-analog-left-pin-label')}
						name="tiltRightAnalogLeftPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tiltRightAnalogLeftPin}
						error={errors.tiltRightAnalogLeftPin}
						isInvalid={errors.tiltRightAnalogLeftPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-right-analog-right-pin-label')}
						name="tiltRightAnalogRightPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tiltRightAnalogRightPin}
						error={errors.tiltRightAnalogRightPin}
						isInvalid={errors.tiltRightAnalogRightPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormSelect
						label={t('AddonsConfig:tilt-socd-mode-label')}
						name="tiltSOCDMode"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tiltSOCDMode}
						error={errors.tiltSOCDMode}
						isInvalid={errors.tiltSOCDMode}
						onChange={handleChange}
					>
						{TILT_SOCD_MODES.map((o, i) => (
							<option key={`button-tiltSOCDMode-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="TiltInputButton"
				reverse={true}
				isInvalid={false}
				checked={Boolean(values.TiltInputEnabled)}
				onChange={(e) => {
					handleCheckbox('TiltInputEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Tilt;
