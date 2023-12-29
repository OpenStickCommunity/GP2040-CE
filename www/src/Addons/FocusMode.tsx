import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import { BUTTON_MASKS } from '../Data/Buttons';

export const focusModeScheme = {
	FocusModeAddonEnabled: yup
		.number()
		.required()
		.label('Focus Mode Add-On Enabled'),
	focusModePin: yup
		.number()
		.label('Focus Mode Pin')
		.validatePinWhenValue('FocusModeAddonEnabled'),
	focusModeButtonLockEnabled: yup
		.number()
		.label('Focus Mode Button Lock Enabled')
		.validateRangeWhenValue('FocusModeAddonEnabled', 0, 1),
	focusModeMacroLockEnabled: yup
		.number()
		.label('Focus Mode Macro Lock Enabled')
		.validateRangeWhenValue('FocusModeAddonEnabled', 0, 1),
	focusModeButtonLockMask: yup
		.number()
		.label('Focus Mode Button Lock Map')
		.validateRangeWhenValue('FocusModeAddonEnabled', 0, (1 << 20) - 1),
};

export const focusModeState = {
	FocusModeAddonEnabled: 0,
	focusModeMacroLockEnabled: 0,
};

const FocusMode = ({
	values,
	errors,
	handleChange,
	handleCheckbox,
	setFieldValue,
}) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:focus-mode-header-text')}>
			<div id="FocusModeAddonOptions" hidden={!values.FocusModeAddonEnabled}>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:focus-mode-pin-label')}
						name="focusModePin"
						className="form-select-sm col-3"
						groupClassName="col-sm-3 mb-3"
						value={values.focusModePin}
						error={errors.focusModePin}
						isInvalid={errors.focusModePin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<div className="col-sm-3">
						<FormCheck
							label={t('Common:lock-macro')}
							className="form-check-sm"
							type="switch"
							reverse
							id="FocusModeAddonMacroButton"
							isInvalid={false}
							checked={Boolean(values.focusModeMacroLockEnabled)}
							onChange={(e) => {
								handleCheckbox('focusModeMacroLockEnabled', values);
								handleChange(e);
							}}
						/>
					</div>
					<div className="col-sm-3">
						<FormCheck
							label={t('Common:lock-buttons')}
							className="form-check-sm"
							type="switch"
							reverse
							id="FocusModeAddonButton"
							isInvalid={false}
							checked={Boolean(values.focusModeButtonLockEnabled)}
							onChange={(e) => {
								handleCheckbox('focusModeButtonLockEnabled', values);
								handleChange(e);
							}}
						/>
					</div>
					<Row>
						{BUTTON_MASKS.map((mask) =>
							values.focusModeButtonLockMask & mask.value ? (
								<FormSelect
									key={`focusModeButtonLockMask-${mask.label}`}
									name="focusModeButtonLockMask"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.focusModeButtonLockMask & mask.value}
									error={errors.focusModeButtonLockMask}
									isInvalid={errors.focusModeButtonLockMask}
									onChange={(e) => {
										setFieldValue(
											'focusModeButtonLockMask',
											(values.focusModeButtonLockMask ^ mask.value) |
												e.target.value,
										);
									}}
								>
									{BUTTON_MASKS.map((o, i) => (
										<option
											key={`focusModeButtonLockMask-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
							) : (
								<></>
							),
						)}
						<FormSelect
							name="focusModeButtonLockMask"
							className="form-select-sm"
							groupClassName="col-sm-3 mb-3"
							value={0}
							error={errors.focusModeButtonLockMask}
							isInvalid={errors.focusModeButtonLockMask}
							onChange={(e) => {
								setFieldValue(
									'focusModeButtonLockMask',
									values.focusModeButtonLockMask | e.target.value,
								);
							}}
						>
							{BUTTON_MASKS.map((o, i) => (
								<option
									key={`focusModeButtonLockMask-option-${i}`}
									value={o.value}
								>
									{o.label}
								</option>
							))}
						</FormSelect>
					</Row>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="FocusModeAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.FocusModeAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('FocusModeAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default FocusMode;
