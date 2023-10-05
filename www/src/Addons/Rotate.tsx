import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';

import FormControl from '../Components/FormControl';
import { SOCD_MODES, ROTATE_SOCD_MODES } from '../Data/Addons';

export const rotateScheme = {
	RotateInputEnabled: yup.number().required().label('Rotate Input Enabled'),
	rotate1Pin: yup
		.number()
		.label('Rotate 1 Pin')
		.validatePinWhenValue('RotateInputEnabled'),	
	factorRotate1LeftX: yup
		.number()
		.label('Rotate 1 Factor Left X')
		.validateNumberWhenValue('RotateInputEnabled'),
	factorRotate1LeftY: yup
		.number()
		.label('Rotate 1 Factor Left Y')
		.validateNumberWhenValue('RotateInputEnabled'),
	factorRotate1RightX: yup
		.number()
		.label('Rotate 1 Factor Right X')
		.validateNumberWhenValue('RotateInputEnabled'),
	factorRotate1RightY: yup
		.number()
		.label('Rotate 1 Factor Right Y')
		.validateNumberWhenValue('RotateInputEnabled'),
	rotate2Pin: yup
		.number()
		.label('Rotate 2 Pin')
		.validatePinWhenValue('RotateInputEnabled'),
	factorRotate2LeftX: yup
		.number()
		.label('Rotate 2 Factor Left X')
		.validateNumberWhenValue('RotateInputEnabled'),
	factorRotate2LeftY: yup
		.number()
		.label('Rotate 2 Factor Left Y')
		.validateNumberWhenValue('RotateInputEnabled'),
	factorRotate2RightX: yup
		.number()
		.label('Rotate 2 Factor Right X')
		.validateNumberWhenValue('RotateInputEnabled'),
	factorRotate2RightY: yup
		.number()
		.label('Rotate 2 Factor Right Y')
		.validateNumberWhenValue('RotateInputEnabled'),
	rotateLeftAnalogUpPin: yup
		.number()
		.label('Rotate Left Analog Up Pin')
		.validatePinWhenValue('RotateInputEnabled'),
	rotateLeftAnalogDownPin: yup
		.number()
		.label('Rotate Left Analog Down Pin')
		.validatePinWhenValue('RotateInputEnabled'),
	rotateLeftAnalogLeftPin: yup
		.number()
		.label('Rotate Left Analog Left Pin')
		.validatePinWhenValue('RotateInputEnabled'),
	rotateLeftAnalogRightPin: yup
		.number()
		.label('Rotate Left Analog Right Pin')
		.validatePinWhenValue('RotateInputEnabled'),
	rotateRightAnalogUpPin: yup
		.number()
		.label('Rotate Right Analog Up Pin')
		.validatePinWhenValue('RotateInputEnabled'),
	rotateRightAnalogDownPin: yup
		.number()
		.label('Rotate Right Analog Down Pin')
		.validatePinWhenValue('RotateInputEnabled'),
	rotateRightAnalogLeftPin: yup
		.number()
		.label('Rotate Right Analog Left Pin')
		.validatePinWhenValue('RotateInputEnabled'),
	rotateRightAnalogRightPin: yup
		.number()
		.label('Rotate Right Analog Right Pin')
		.validatePinWhenValue('RotateInputEnabled'),
	rotateSOCDMode: yup
		.number()
		.label('Rotate SOCE Mode')
		.validateSelectionWhenValue('RotateInputEnabled', SOCD_MODES),
};

export const rotateState = {
	RotateInputEnabled: 0,
	rotate1Pin: -1,
	factorRotate1LeftX: -1,
	factorRotate1LeftY: -1,
	factorRotate1RightX: -1,
	factorRotate1ightY: -1,
	rotate2Pin: -1,
	factorRotate2LeftX: -1,
	factorRotate2LeftY: -1,
	factorRotate2RightX: -1,
	factorRotate2RightY: -1,
	rotateLeftAnalogUpPin: -1,
	rotateLeftAnalogDownPin: -1,
	rotateLeftAnalogLeftPin: -1,
	rotateLeftAnalogRightPin: -1,
	rotateRightAnalogUpPin: -1,
	rotateRightAnalogDownPin: -1,
	rotateRightAnalogLeftPin: -1,
	rotateRightAnalogRightPin: -1,
};

const Rotate = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:rotate-header-text')}>
			<div id="RotateOptions" hidden={!values.RotateInputEnabled}>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-1-pin-label')}
						name="rotate1Pin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.rotate1Pin}
						error={errors.rotate1Pin}
						isInvalid={errors.rotate1Pin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-1-factor-left-x-label')}
						name="factorRotate1LeftX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorRotate1LeftX}
						error={errors.factorRotate1LeftX}
						isInvalid={errors.factorRotate1LeftX}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-1-factor-left-y-label')}
						name="factorRotate1LeftY"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorRotate1LeftY}
						error={errors.factorRotate1LeftY}
						isInvalid={errors.factorRotate1LeftY}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-1-factor-right-x-label')}
						name="factorRotate1RightX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorRotate1RightX}
						error={errors.factorRotate1RightX}
						isInvalid={errors.factorRotate1RightX}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-1-factor-right-y-label')}
						name="factorRotate1RightY"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorRotate1RightY}
						error={errors.factorRotate1RightY}
						isInvalid={errors.factorRotate1RightY}
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
						groupClassName="col-sm-3 mb-3"
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
						label={t('AddonsConfig:rotate-2-factor-left-x-label')}
						name="factorRotate2LeftX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorRotate2LeftX}
						error={errors.factorRotate2LeftX}
						isInvalid={errors.factorRotate2LeftX}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-2-factor-left-y-label')}
						name="factorRotate2LeftY"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorRotate2LeftY}
						error={errors.factorRotate2LeftY}
						isInvalid={errors.factorRotate2LeftY}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-2-factor-right-x-label')}
						name="factorRotate2RightX"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorRotate2RightX}
						error={errors.factorRotate2RightX}
						isInvalid={errors.factorRotate2RightX}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-2-factor-right-y-label')}
						name="factorRotate2RightY"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.factorRotate2RightY}
						error={errors.factorRotate2RightY}
						isInvalid={errors.factorRotate2RightY}
						onChange={handleChange}
						min={-1}
						max={200}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-left-analog-up-pin-label')}
						name="rotateLeftAnalogUpPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.rotateLeftAnalogUpPin}
						error={errors.rotateLeftAnalogUpPin}
						isInvalid={errors.rotateLeftAnalogUpPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-left-analog-down-pin-label')}
						name="rotateLeftAnalogDownPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.rotateLeftAnalogDownPin}
						error={errors.rotateLeftAnalogDownPin}
						isInvalid={errors.rotateLeftAnalogDownPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-left-analog-left-pin-label')}
						name="rotateLeftAnalogLeftPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.rotateLeftAnalogLeftPin}
						error={errors.rotateLeftAnalogLeftPin}
						isInvalid={errors.rotateLeftAnalogLeftPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-left-analog-right-pin-label')}
						name="rotateLeftAnalogRightPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.rotateLeftAnalogRightPin}
						error={errors.rotateLeftAnalogRightPin}
						isInvalid={errors.rotateLeftAnalogRightPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-right-analog-up-pin-label')}
						name="rotateRightAnalogUpPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.rotateRightAnalogUpPin}
						error={errors.rotateRightAnalogUpPin}
						isInvalid={errors.rotateRightAnalogUpPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-right-analog-down-pin-label')}
						name="rotateRightAnalogDownPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.rotateRightAnalogDownPin}
						error={errors.rotateRightAnalogDownPin}
						isInvalid={errors.rotateRightAnalogDownPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-right-analog-left-pin-label')}
						name="rotateRightAnalogLeftPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.rotateRightAnalogLeftPin}
						error={errors.rotateRightAnalogLeftPin}
						isInvalid={errors.rotateRightAnalogLeftPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-right-analog-right-pin-label')}
						name="rotateRightAnalogRightPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.rotateRightAnalogRightPin}
						error={errors.rotateRightAnalogRightPin}
						isInvalid={errors.rotateRightAnalogRightPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormSelect
						label={t('AddonsConfig:rotate-socd-mode-label')}
						name="rotateSOCDMode"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.rotateSOCDMode}
						error={errors.rotateSOCDMode}
						isInvalid={errors.rotateSOCDMode}
						onChange={handleChange}
					>
						{ROTATE_SOCD_MODES.map((o, i) => (
							<option key={`button-rotateSOCDMode-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="RotateInputButton"
				reverse={true}
				isInvalid={false}
				checked={Boolean(values.RotateInputEnabled)}
				onChange={(e) => {
					handleCheckbox('RotateInputEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Rotate;
