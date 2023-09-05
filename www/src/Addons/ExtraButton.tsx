import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import { BUTTON_MASKS } from '../Data/Buttons';

export const extraButtonScheme = {
	ExtraButtonAddonEnabled: yup
		.number()
		.required()
		.label('Extra Button Add-On Enabled'),
	extraButtonPin: yup
		.number()
		.label('Extra Button Pin')
		.validatePinWhenValue('ExtraButtonAddonEnabled'),
	extraButtonMap: yup
		.number()
		.label('Extra Button Map')
		.validateSelectionWhenValue('ExtraButtonAddonEnabled', BUTTON_MASKS),
};

export const extraButtonState = {
	ExtraButtonAddonEnabled: 0,
	extraButtonPin: -1,
	extraButtonMap: 0,
};

const ExtraButton = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:extra-button-header-text')}>
			<div
				id="ExtraButtonAddonOptions"
				hidden={!values.ExtraButtonAddonEnabled}
			>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:extra-button-pin-label')}
						name="extraButtonPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.extraButtonPin}
						error={errors.extraButtonPin}
						isInvalid={errors.extraButtonPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormSelect
						label={t('AddonsConfig:extra-button-map-label')}
						name="extraButtonMap"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.extraButtonMap}
						error={errors.extraButtonMap}
						isInvalid={errors.extraButtonMap}
						onChange={handleChange}
					>
						{BUTTON_MASKS.map((o, i) => (
							<option key={`extraButtonMap-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="ExtraButtonAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.ExtraButtonAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('ExtraButtonAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default ExtraButton;
