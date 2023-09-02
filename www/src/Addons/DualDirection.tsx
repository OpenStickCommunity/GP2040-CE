import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';

import FormControl from '../Components/FormControl';
import { DUAL_STICK_MODES } from '../Data/Addons';

const DUAL_COMBINE_MODES = [
	{ label: 'Mixed', value: 0 },
	{ label: 'Gamepad', value: 1 },
	{ label: 'Dual Directional', value: 2 },
	{ label: 'None', value: 3 },
];

export const dualDirectionScheme = {
	DualDirectionalInputEnabled: yup
		.number()
		.required()
		.label('Dual Directional Input Enabled'),
	dualDirUpPin: yup
		.number()
		.label('Dual Directional Up Pin')
		.validatePinWhenValue('DualDirectionalInputEnabled'),
	dualDirDownPin: yup
		.number()
		.label('Dual Directional Down Pin')
		.validatePinWhenValue('DualDirectionalInputEnabled'),
	dualDirLeftPin: yup
		.number()
		.label('Dual Directional Left Pin')
		.validatePinWhenValue('DualDirectionalInputEnabled'),
	dualDirRightPin: yup
		.number()
		.label('Dual Directional Right Pin')
		.validatePinWhenValue('DualDirectionalInputEnabled'),
	dualDirDpadMode: yup
		.number()
		.label('Dual Stick Mode')
		.validateSelectionWhenValue(
			'DualDirectionalInputEnabled',
			DUAL_STICK_MODES,
		),
	dualDirCombineMode: yup
		.number()
		.label('Dual Combination Mode')
		.validateSelectionWhenValue(
			'DualDirectionalInputEnabled',
			DUAL_COMBINE_MODES,
		),
	dualDirFourWayMode: yup
		.number()
		.label('Dual Directional 4-Way Joystick Mode')
		.validateRangeWhenValue('DualDirectionalInputEnabled', 0, 1),
};

export const dualDirectionState = {
	DualDirectionalInputEnabled: 0,
	dualDirUpPin: -1,
	dualDirDownPin: -1,
	dualDirLeftPin: -1,
	dualDirRightPin: -1,
	dualDirDpadMode: 0,
	dualDirCombineMode: 0,
	dualDirFourWayMode: 0,
};

const DualDirection = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:dual-directional-input-header-text')}>
			<div
				id="DualDirectionalInputOptions"
				hidden={!values.DualDirectionalInputEnabled}
			>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:dual-directional-input-up-pin-label')}
						name="dualDirUpPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.dualDirUpPin}
						error={errors.dualDirUpPin}
						isInvalid={errors.dualDirUpPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:dual-directional-input-down-pin-label')}
						name="dualDirDownPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.dualDirDownPin}
						error={errors.dualDirDownPin}
						isInvalid={errors.dualDirDownPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:dual-directional-input-left-pin-label')}
						name="dualDirLeftPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.dualDirLeftPin}
						error={errors.dualDirLeftPin}
						isInvalid={errors.dualDirLeftPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:dual-directional-input-right-pin-label')}
						name="dualDirRightPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.dualDirRightPin}
						error={errors.dualDirRightPin}
						isInvalid={errors.dualDirRightPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormSelect
						label={t('AddonsConfig:dual-directional-input-dpad-mode-label')}
						name="dualDirDpadMode"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.dualDirDpadMode}
						error={errors.dualDirDpadMode}
						isInvalid={errors.dualDirDpadMode}
						onChange={handleChange}
					>
						{DUAL_STICK_MODES.map((o, i) => (
							<option
								key={`button-dualDirDpadMode-option-${i}`}
								value={o.value}
							>
								{o.label}
							</option>
						))}
					</FormSelect>

					<FormSelect
						label={t('AddonsConfig:dual-directional-input-combine-mode-label')}
						name="dualDirCombineMode"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.dualDirCombineMode}
						error={errors.dualDirCombineMode}
						isInvalid={errors.dualDirCombineMode}
						onChange={handleChange}
					>
						{DUAL_COMBINE_MODES.map((o, i) => (
							<option
								key={`button-dualDirCombineMode-option-${i}`}
								value={o.value}
							>
								{o.label}
							</option>
						))}
					</FormSelect>
					<FormCheck
						label={t(
							'AddonsConfig:dual-directional-input-four-way-joystick-mode-label',
						)}
						type="switch"
						id="DualDirFourWayMode"
						className="col-sm-3 ms-2"
						isInvalid={false}
						checked={Boolean(values.dualDirFourWayMode)}
						onChange={(e) => {
							handleCheckbox('dualDirFourWayMode', values);
							handleChange(e);
						}}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="DualDirectionalInputButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.DualDirectionalInputEnabled)}
				onChange={(e) => {
					handleCheckbox('DualDirectionalInputEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default DualDirection;
