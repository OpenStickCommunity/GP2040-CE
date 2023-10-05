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
		.label('Rotate 1 Pin (Clockwise Rotation)')
		.validatePinWhenValue('RotateInputEnabled'),	
	factorRotate1Left: yup
		.number()
		.label('Rotate 1 Degree for Left Stick')
		.validateNumberWhenValue('RotateInputEnabled'),
	factorRotate1Right: yup
		.number()
		.label('Rotate 1 Degree for Right Stick')
		.validateNumberWhenValue('RotateInputEnabled'),
	rotate2Pin: yup
		.number()
		.label('Rotate 2 Pin (Counter Clockwise Rotation)')
		.validatePinWhenValue('RotateInputEnabled'),
	factorRotate2Left: yup
		.number()
		.label('Rotate 2 Degree for Left Stick')
		.validateNumberWhenValue('RotateInputEnabled'),
	factorRotate2Right: yup
		.number()
		.label('Rotate 2 Degree for Right Stick')
		.validateNumberWhenValue('RotateInputEnabled'),
	rotateSOCDMode: yup
		.number()
		.label('Rotate SOCE Mode')
		.validateSelectionWhenValue('RotateInputEnabled', SOCD_MODES),
};

export const rotateState = {
	RotateInputEnabled: 0,
	rotate1Pin: -1,
	factorRotate1Left: 0,
	factorRotate1Right: 0,
	rotate2Pin: -1,
	factorRotate2Left: 0,
	factorRotate2Right: 0,
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
					<FormControl
						type="number"
						label={t('AddonsConfig:rotate-1-factor-left-label')}
						name="factorRotate1Left"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
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
						groupClassName="col-sm-3 mb-3"
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
						groupClassName="col-sm-3 mb-3"
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
						groupClassName="col-sm-3 mb-3"
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
						groupClassName="col-sm-3 mb-3"
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
