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
	factorTilt1LeftX: yup
		.number()
		.label('Tilt 1 Factor Left X')
		.validateRangeWhenValue('TiltInputEnabled', 0, 100),
	factorTilt1LeftY: yup
		.number()
		.label('Tilt 1 Factor Left Y')
		.validateRangeWhenValue('TiltInputEnabled', 0, 100),
	factorTilt1RightX: yup
		.number()
		.label('Tilt 1 Factor Right X')
		.validateRangeWhenValue('TiltInputEnabled', 0, 100),
	factorTilt1RightY: yup
		.number()
		.label('Tilt 1 Factor Right Y')
		.validateRangeWhenValue('TiltInputEnabled', 0, 100),
	tilt2Pin: yup
		.number()
		.label('Tilt 2 Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	factorTilt2LeftX: yup
		.number()
		.label('Tilt 2 Factor Left X')
		.validateRangeWhenValue('TiltInputEnabled', 0, 100),
	factorTilt2LeftY: yup
		.number()
		.label('Tilt 2 Factor Left Y')
		.validateRangeWhenValue('TiltInputEnabled', 0, 100),
	factorTilt2RightX: yup
		.number()
		.label('Tilt 2 Factor Right X')
		.validateRangeWhenValue('TiltInputEnabled', 0, 100),
	factorTilt2RightY: yup
		.number()
		.label('Tilt 2 Factor Right Y')
		.validateRangeWhenValue('TiltInputEnabled', 0, 100),
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
	rotate1Pin: yup
		.number()
		.label('Rotate 1 Pin')
		.validatePinWhenValue('TiltInputEnabled'),	
	rotate2Pin: yup
		.number()
		.label('Rotate 2 Pin')
		.validatePinWhenValue('TiltInputEnabled'),
	factorRotate1Left: yup
		.number()
		.label('Rotate 1 Degree for Left Stick')
		.validateNumberWhenValue('TiltInputEnabled', 0, 360),
	factorRotate2Left: yup
		.number()
		.label('Rotate 2 Degree for Left Stick')
		.validateNumberWhenValue('TiltInputEnabled', 0, 360),
	factorRotate3Left: yup
		.number()
		.label('Rotate 3 Degree for Left Stick')
		.validateNumberWhenValue('TiltInputEnabled', 0, 360),
	factorRotate4Left: yup
		.number()
		.label('Rotate 4 Degree for Left Stick')
		.validateNumberWhenValue('TiltInputEnabled', 0, 360),
	factorRotate5Left: yup
		.number()
		.label('Rotate 5 Degree for Left Stick')
		.validateNumberWhenValue('TiltInputEnabled', 0, 360),
	factorRotate6Left: yup
		.number()
		.label('Rotate 6 Degree for Left Stick')
		.validateNumberWhenValue('TiltInputEnabled', 0, 360),		
	factorRotate1Right: yup
		.number()
		.label('Rotate 1 Degree for Right Stick')
		.validateNumberWhenValue('TiltInputEnabled', 0, 360),
	factorRotate2Right: yup
		.number()
		.label('Rotate 2 Degree for Right Stick')
		.validateNumberWhenValue('TiltInputEnabled', 0, 360),
	factorRotate3Right: yup
		.number()
		.label('Rotate 3 Degree for Right Stick')
		.validateNumberWhenValue('TiltInputEnabled', 0, 360),
	factorRotate4Right: yup
		.number()
		.label('Rotate 4 Degree for Right Stick')
		.validateNumberWhenValue('TiltInputEnabled', 0, 360),
	factorRotate5Right: yup
		.number()
		.label('Rotate 5 Degree for Right Stick')
		.validateNumberWhenValue('TiltInputEnabled', 0, 360),
	factorRotate6Right: yup
		.number()
		.label('Rotate 6 Degree for Right Stick')
		.validateNumberWhenValue('TiltInputEnabled', 0, 360),
	tiltLeftSOCDMode: yup
		.number()
		.label('Tilt Left Stick SOCD Mode')
		.validateSelectionWhenValue('TiltInputEnabled', TILT_SOCD_MODES),
	tiltRightSOCDMode: yup
		.number()
		.label('Tilt Right Stick SOCD Mode')
		.validateSelectionWhenValue('TiltInputEnabled', TILT_SOCD_MODES),
};

export const tiltState = {
	TiltInputEnabled: 0,
	tilt1Pin: -1,
	factorTilt1LeftX: 35,
	factorTilt1LeftY: 35,
	factorTilt1RightX: 35,
	factorTilt1RightY: 35,
	tilt2Pin: -1,
	factorTilt2LeftX: 65,
	factorTilt2LeftY: 65,
	factorTilt2RightX: 65,
	factorTilt2RightY: 65,
	tiltLeftAnalogUpPin: -1,
	tiltLeftAnalogDownPin: -1,
	tiltLeftAnalogLeftPin: -1,
	tiltLeftAnalogRightPin: -1,
	tiltRightAnalogUpPin: -1,
	tiltRightAnalogDownPin: -1,
	tiltRightAnalogLeftPin: -1,
	tiltRightAnalogRightPin: -1,
	rotate1Pin: -1,
	rotate2Pin: -1,
	factorRotate1Left: 15,
	factorRotate2Left: 345,
	factorRotate3Left: 0,
	factorRotate4Left: 0,
	factorRotate5Left: 0,
	factorRotate6Left: 0,
	factorRotate1Right: 15,
	factorRotate2Right: 345,
	factorRotate3Right: 0,
	factorRotate4Right: 0,
	factorRotate5Right: 0,
	factorRotate6Right: 0,
};

const Tilt = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:tilt-header-text')}>
			<div id="TiltOptions" hidden={!values.TiltInputEnabled}>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:tilt-left-analog-up-pin-label')}
						name="tiltLeftAnalogUpPin"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-2 mb-3"
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
						groupClassName="col-sm-2 mb-3"
						value={values.tiltRightAnalogRightPin}
						error={errors.tiltRightAnalogRightPin}
						isInvalid={errors.tiltRightAnalogRightPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
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
				</Row>
				<Row className="mb-3">
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
						min={0}
						max={100}
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
						min={0}
						max={100}
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
						min={0}
						max={100}
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
						min={0}
						max={100}
					/>
				</Row>
				<Row className="mb-3">
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
						min={0}
						max={100}
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
						min={0}
						max={100}
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
						min={0}
						max={100}
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
						min={0}
						max={100}
					/>
				</Row>
				<Row className="mb-3">
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
						min={0}
						max={360}
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
						min={0}
						max={360}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-3-factor-left-label')}
						name="factorRotate3Left"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.factorRotate3Left}
						error={errors.factorRotate3Left}
						isInvalid={errors.factorRotate3Left}
						onChange={handleChange}
						min={0}
						max={360}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-4-factor-left-label')}
						name="factorRotate4Left"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.factorRotate4Left}
						error={errors.factorRotate4Left}
						isInvalid={errors.factorRotate4Left}
						onChange={handleChange}
						min={0}
						max={360}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-5-factor-left-label')}
						name="factorRotate5Left"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.factorRotate5Left}
						error={errors.factorRotate5Left}
						isInvalid={errors.factorRotate5Left}
						onChange={handleChange}
						min={0}
						max={360}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-6-factor-left-label')}
						name="factorRotate6Left"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.factorRotate6Left}
						error={errors.factorRotate6Left}
						isInvalid={errors.factorRotate6Left}
						onChange={handleChange}
						min={0}
						max={360}
					/>
				</Row>
				<Row className="mb-3">
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
						min={0}
						max={360}
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
						min={0}
						max={360}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-3-factor-right-label')}
						name="factorRotate3Right"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.factorRotate3Right}
						error={errors.factorRotate3Right}
						isInvalid={errors.factorRotate3Right}
						onChange={handleChange}
						min={0}
						max={360}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-4-factor-right-label')}
						name="factorRotate4Right"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.factorRotate4Right}
						error={errors.factorRotate4Right}
						isInvalid={errors.factorRotate4Right}
						onChange={handleChange}
						min={0}
						max={360}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-5-factor-right-label')}
						name="factorRotate5Right"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.factorRotate5Right}
						error={errors.factorRotate5Right}
						isInvalid={errors.factorRotate5Right}
						onChange={handleChange}
						min={0}
						max={360}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-6-factor-right-label')}
						name="factorRotate6Right"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.factorRotate6Right}
						error={errors.factorRotate6Right}
						isInvalid={errors.factorRotate6Right}
						onChange={handleChange}
						min={0}
						max={360}
					/>
				</Row>
				<Row className="mb-3">
					<FormSelect
						label={t('AddonsConfig:tilt-left-socd-mode-label')}
						name="tiltLeftSOCDMode"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tiltLeftSOCDMode}
						error={errors.tiltLeftSOCDMode}
						isInvalid={errors.tiltLeftSOCDMode}
						onChange={handleChange}
					>
						{TILT_SOCD_MODES.map((o, i) => (
							<option key={`button-tiltLeftSOCDMode-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
					<FormSelect
						label={t('AddonsConfig:tilt-right-socd-mode-label')}
						name="tiltRightSOCDMode"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.tiltRightSOCDMode}
						error={errors.tiltRightSOCDMode}
						isInvalid={errors.tiltRightSOCDMode}
						onChange={handleChange}
					>
						{TILT_SOCD_MODES.map((o, i) => (
							<option key={`button-tiltRightSOCDMode-option-${i}`} value={o.value}>
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
