import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';

import FormControl from '../Components/FormControl';
import { SOCD_MODES, ANALOGMOD_SOCD_MODES } from '../Data/Addons';

export const analogmodScheme = {
	AnalogModInputEnabled: yup.number().required().label('Analog Modification Input Enabled'),
	tilt1Pin: yup
		.number()
		.label('Tilt 1 Pin')
		.validatePinWhenValue('TiltInputEnabled'),	
	factorTilt1LeftX: yup
		.number()
		.label('Tilt 1 Factor Left X')
		.validateNumberWhenValue('TiltInputEnabled'),
	factorTilt1LeftY: yup
		.number()
		.label('Tilt 1 Factor Left Y')
		.validateNumberWhenValue('TiltInputEnabled'),
	factorTilt1RightX: yup
		.number()
		.label('Tilt 1 Factor Right X')
		.validateNumberWhenValue('TiltInputEnabled'),
	factorTilt1RightY: yup
		.number()
		.label('Tilt 1 Factor Right Y')
		.validateNumberWhenValue('TiltInputEnabled'),
	tilt2Pin: yup
		.number()
		.label('Tilt 2 Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	factorTilt2LeftX: yup
		.number()
		.label('Tilt 2 Factor Left X')
		.validateNumberWhenValue('TiltInputEnabled'),
	factorTilt2LeftY: yup
		.number()
		.label('Tilt 2 Factor Left Y')
		.validateNumberWhenValue('TiltInputEnabled'),
	factorTilt2RightX: yup
		.number()
		.label('Tilt 2 Factor Right X')
		.validateNumberWhenValue('TiltInputEnabled'),
	factorTilt2RightY: yup
		.number()
		.label('Tilt 2 Factor Right Y')
		.validateNumberWhenValue('TiltInputEnabled'),
	analogmodLeftAnalogUpPin: yup
		.number()
		.label('AnalogMod Left Analog Up Pin')
		.validatePinWhenValue('AnalogModInputEnabled'),
	analogmodLeftAnalogDownPin: yup
		.number()
		.label('AnalogMod Left Analog Down Pin')
		.validatePinWhenValue('AnalogModInputEnabled'),
	analogmodLeftAnalogLeftPin: yup
		.number()
		.label('AnalogMod Left Analog Left Pin')
		.validatePinWhenValue('AnalogModInputEnabled'),
	analogmodLeftAnalogRightPin: yup
		.number()
		.label('AnalogMod Left Analog Right Pin')
		.validatePinWhenValue('AnalogModInputEnabled'),
	analogmodRightAnalogUpPin: yup
		.number()
		.label('AnalogMod Right Analog Up Pin')
		.validatePinWhenValue('AnalogModInputEnabled'),
	analogmodRightAnalogDownPin: yup
		.number()
		.label('AnalogMod Right Analog Down Pin')
		.validatePinWhenValue('AnalogModInputEnabled'),
	analogmodRightAnalogLeftPin: yup
		.number()
		.label('AnalogMod Right Analog Left Pin')
		.validatePinWhenValue('AnalogModInputEnabled'),
	analogmodRightAnalogRightPin: yup
		.number()
		.label('AnalogMod Right Analog Right Pin')
		.validatePinWhenValue('AnalogModInputEnabled'),
	analogmodSOCDMode: yup
		.number()
		.label('AnalogMod SOCE Mode')
		.validateSelectionWhenValue('AnalogModInputEnabled', SOCD_MODES),
};

export const analogmodState = {
	AnalogModInputEnabled: 0,
	tilt1Pin: -1,
	factorTilt1LeftX: -1,
	factorTilt1LeftY: -1,
	factorTilt1RightX: -1,
	factorTilt1ightY: -1,
	tilt2Pin: -1,
	factorTilt2LeftX: -1,
	factorTilt2LeftY: -1,
	factorTilt2RightX: -1,
	factorTilt2RightY: -1,
	analogmodLeftAnalogUpPin: -1,
	analogmodLeftAnalogDownPin: -1,
	analogmodLeftAnalogLeftPin: -1,
	analogmodLeftAnalogRightPin: -1,
	analogmodRightAnalogUpPin: -1,
	analogmodRightAnalogDownPin: -1,
	analogmodRightAnalogLeftPin: -1,
	analogmodRightAnalogRightPin: -1,
};

const AnalogMod = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:analogmod-header-text')}>
			<div id="AnalogModOptions" hidden={!values.AnalogModInputEnabled}>
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
						name="factorTilt1LeftX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorTilt1LeftX}
						error={errors.factorTilt1LeftX}
						isInvalid={errors.factorTilt1LeftX}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-1-factor-left-y-label')}
						name="factorTilt1LeftY"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorTilt1LeftY}
						error={errors.factorTilt1LeftY}
						isInvalid={errors.factorTilt1LeftY}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-1-factor-right-x-label')}
						name="factorTilt1RightX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorTilt1RightX}
						error={errors.factorTilt1RightX}
						isInvalid={errors.factorTilt1RightX}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-1-factor-right-y-label')}
						name="factorTilt1RightY"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorTilt1RightY}
						error={errors.factorTilt1RightY}
						isInvalid={errors.factorTilt1RightY}
						onChange={handleChange}
						min={-1}
						max={200}
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
						name="factorTilt2LeftX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorTilt2LeftX}
						error={errors.factorTilt2LeftX}
						isInvalid={errors.factorTilt2LeftX}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-2-factor-left-y-label')}
						name="factorTilt2LeftY"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorTilt2LeftY}
						error={errors.factorTilt2LeftY}
						isInvalid={errors.factorTilt2LeftY}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-2-factor-right-x-label')}
						name="factorTilt2RightX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorTilt2RightX}
						error={errors.factorTilt2RightX}
						isInvalid={errors.factorTilt2RightX}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-2-factor-right-y-label')}
						name="factorTilt2RightY"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorTilt2RightY}
						error={errors.factorTilt2RightY}
						isInvalid={errors.factorTilt2RightY}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:analogmod-left-analog-up-pin-label')}
						name="analogmodLeftAnalogUpPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogmodLeftAnalogUpPin}
						error={errors.analogmodLeftAnalogUpPin}
						isInvalid={errors.analogmodLeftAnalogUpPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:analogmod-left-analog-down-pin-label')}
						name="analogmodLeftAnalogDownPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogmodLeftAnalogDownPin}
						error={errors.analogmodLeftAnalogDownPin}
						isInvalid={errors.analogmodLeftAnalogDownPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:analogmod-left-analog-left-pin-label')}
						name="analogmodLeftAnalogLeftPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogmodLeftAnalogLeftPin}
						error={errors.analogmodLeftAnalogLeftPin}
						isInvalid={errors.analogmodLeftAnalogLeftPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:analogmod-left-analog-right-pin-label')}
						name="analogmodLeftAnalogRightPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogmodLeftAnalogRightPin}
						error={errors.analogmodLeftAnalogRightPin}
						isInvalid={errors.analogmodLeftAnalogRightPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:analogmod-right-analog-up-pin-label')}
						name="analogmodRightAnalogUpPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogmodRightAnalogUpPin}
						error={errors.analogmodRightAnalogUpPin}
						isInvalid={errors.analogmodRightAnalogUpPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:analogmod-right-analog-down-pin-label')}
						name="analogmodRightAnalogDownPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogmodRightAnalogDownPin}
						error={errors.analogmodRightAnalogDownPin}
						isInvalid={errors.analogmodRightAnalogDownPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:analogmod-right-analog-left-pin-label')}
						name="analogmodRightAnalogLeftPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogmodRightAnalogLeftPin}
						error={errors.analogmodRightAnalogLeftPin}
						isInvalid={errors.analogmodRightAnalogLeftPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:analogmod-right-analog-right-pin-label')}
						name="analogmodRightAnalogRightPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogmodRightAnalogRightPin}
						error={errors.analogmodRightAnalogRightPin}
						isInvalid={errors.analogmodRightAnalogRightPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormSelect
						label={t('AddonsConfig:analogmod-socd-mode-label')}
						name="analogmodSOCDMode"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogmodSOCDMode}
						error={errors.analogmodSOCDMode}
						isInvalid={errors.analogmodSOCDMode}
						onChange={handleChange}
					>
						{ANALOGMOD_SOCD_MODES.map((o, i) => (
							<option key={`button-analogmodSOCDMode-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="AnalogModInputButton"
				reverse={true}
				isInvalid={false}
				checked={Boolean(values.AnalogModInputEnabled)}
				onChange={(e) => {
					handleCheckbox('AnalogModInputEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default AnalogMod;
