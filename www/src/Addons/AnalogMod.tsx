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
		.validatePinWhenValue('AnalogModInputEnabled'),	
	factorTilt1LeftX: yup
		.number()
		.label('Tilt 1 Factor Left X')
		.validateNumberWhenValue('AnalogModInputEnabled', 0, 200),
	factorTilt1LeftY: yup
		.number()
		.label('Tilt 1 Factor Left Y')
		.validateNumberWhenValue('AnalogModInputEnabled', 0, 200),
	factorTilt1RightX: yup
		.number()
		.label('Tilt 1 Factor Right X')
		.validateNumberWhenValue('AnalogModInputEnabled', 0, 200),
	factorTilt1RightY: yup
		.number()
		.label('Tilt 1 Factor Right Y')
		.validateNumberWhenValue('AnalogModInputEnabled', 0, 200),
	tilt2Pin: yup
		.number()
		.label('Tilt 2 Pin')
		.validatePinWhenValue('AnalogModInputEnabled'),
	factorTilt2LeftX: yup
		.number()
		.label('Tilt 2 Factor Left X')
		.validateNumberWhenValue('AnalogModInputEnabled', 0, 200),
	factorTilt2LeftY: yup
		.number()
		.label('Tilt 2 Factor Left Y')
		.validateNumberWhenValue('AnalogModInputEnabled', 0, 200),
	factorTilt2RightX: yup
		.number()
		.label('Tilt 2 Factor Right X')
		.validateNumberWhenValue('AnalogModInputEnabled', 0, 200),
	factorTilt2RightY: yup
		.number()
		.label('Tilt 2 Factor Right Y')
		.validateNumberWhenValue('AnalogModInputEnabled', 0, 200),
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
	rotate1Pin: yup
		.number()
		.label('Rotate 1 Pin (Clockwise Rotation)')
		.validatePinWhenValue('AnalogModInputEnabled'),	
	factorRotate1Left: yup
		.number()
		.label('Rotate 1 Degree for Left Stick')
		.validateNumberWhenValue('AnalogModInputEnabled'),
	factorRotate1Right: yup
		.number()
		.label('Rotate 1 Degree for Right Stick')
		.validateNumberWhenValue('AnalogModInputEnabled'),
	rotate2Pin: yup
		.number()
		.label('Rotate 2 Pin (Counter Clockwise Rotation)')
		.validatePinWhenValue('AnalogModInputEnabled'),
	factorRotate2Left: yup
		.number()
		.label('Rotate 2 Degree for Left Stick')
		.validateNumberWhenValue('AnalogModInputEnabled'),
	factorRotate2Right: yup
		.number()
		.label('Rotate 2 Degree for Right Stick')
		.validateNumberWhenValue('AnalogModInputEnabled'),
	analogmodSOCDMode: yup
		.number()
		.label('AnalogMod SOCE Mode')
		.validateSelectionWhenValue('AnalogModInputEnabled', SOCD_MODES),
};

export const analogmodState = {
	AnalogModInputEnabled: 0,
	tilt1Pin: -1,
	factorTilt1LeftX: 35,
	factorTilt1LeftY: 45,
	factorTilt1RightX: 35,
	factorTilt1RightY: 170,
	tilt2Pin: -1,
	factorTilt2LeftX: 65,
	factorTilt2LeftY: 35,
	factorTilt2RightX: 30,
	factorTilt2RightY: 30,
	analogmodLeftAnalogUpPin: -1,
	analogmodLeftAnalogDownPin: -1,
	analogmodLeftAnalogLeftPin: -1,
	analogmodLeftAnalogRightPin: -1,
	analogmodRightAnalogUpPin: -1,
	analogmodRightAnalogDownPin: -1,
	analogmodRightAnalogLeftPin: -1,
	analogmodRightAnalogRightPin: -1,
	rotate1Pin: -1,
	factorRotate1Left: 15,
	factorRotate1Right: 15,
	rotate2Pin: -1,
	factorRotate2Left: 15,
	factorRotate2Right: 15,
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
						groupClassName="col-sm-1 mb-3"
						value={values.tilt1Pin}
						error={errors.tilt1Pin}
						isInvalid={errors.tilt1Pin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-1-factor-left-x-label')}
						name="factorTilt1LeftX"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-1 mb-3"
						value={values.tilt2Pin}
						error={errors.tilt2Pin}
						isInvalid={errors.tilt2Pin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-2-factor-left-x-label')}
						name="factorTilt2LeftX"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-2 mb-3"
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
						label={t('AddonsConfig:rotate-1-pin-label')}
						name="rotate1Pin"
						className="form-select-sm"
						groupClassName="col-sm-1 mb-3"
						value={values.rotate1Pin}
						error={errors.rotate1Pin}
						isInvalid={errors.rotate1Pin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-1-factor-left-label')}
						name="factorRotate1Left"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.factorRotate1Left}
						error={errors.factorRotate1Left}
						isInvalid={errors.factorRotate1Left}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-1-factor-right-label')}
						name="factorRotate1Right"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.factorRotate1Right}
						error={errors.factorRotate1Right}
						isInvalid={errors.factorRotate1Right}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-2-pin-label')}
						name="rotate2Pin"
						className="form-select-sm"
						groupClassName="col-sm-1 mb-3"
						value={values.rotate2Pin}
						error={errors.rotate2Pin}
						isInvalid={errors.rotate2Pin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-2-factor-left-label')}
						name="factorRotate2Left"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.factorRotate2Left}
						error={errors.factorRotate2Left}
						isInvalid={errors.factorRotate2Left}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-2-factor-right-label')}
						name="factorRotate2Right"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.factorRotate2Right}
						error={errors.factorRotate2Right}
						isInvalid={errors.factorRotate2Right}
						onChange={handleChange}
						min={-1}
						max={200}
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
