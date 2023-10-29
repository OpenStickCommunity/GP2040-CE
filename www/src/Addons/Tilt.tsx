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
	tiltSOCDMode: yup
		.number()
		.label('Tilt SOCE Mode')
		.validateSelectionWhenValue('TiltInputEnabled', SOCD_MODES),
};

export const tiltState = {
	TiltInputEnabled: 0,
	tilt1Pin: -1,
	factorTilt1LeftX: 0,
	factorTilt1LeftY: 0,
	factorTilt1RightX: 0,
	factorTilt1ightY: 0,
	tilt2Pin: -1,
	factorTilt2LeftX: 0,
	factorTilt2LeftY: 0,
	factorTilt2RightX: 0,
	factorTilt2RightY: 0,
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
