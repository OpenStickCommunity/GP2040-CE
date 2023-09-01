import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import { BUTTON_MASKS } from '../Data/Buttons';

export const bootselScheme = {
	BootselButtonAddonEnabled: yup
		.number()
		.required()
		.label('Boot Select Button Add-On Enabled'),
	bootselButtonMap: yup
		.number()
		.label('BOOTSEL Button Map')
		.validateSelectionWhenValue('BootselButtonAddonEnabled', BUTTON_MASKS),
};

export const bootselState = {
	bootselButtonMap: 0,
	BootselButtonAddonEnabled: 0,
};

const Bootsel = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:bootsel-header-text')}>
			<div
				id="BootselButtonAddonOptions"
				hidden={!values.BootselButtonAddonEnabled}
			>
				<p>{t('AddonsConfig:bootsel-sub-header-text')}</p>
				<FormSelect
					label={t('AddonsConfig:bootsel-button-pin-label')}
					name="bootselButtonMap"
					className="form-select-sm"
					groupClassName="col-sm-3 mb-3"
					value={values.bootselButtonMap}
					error={errors.bootselButtonMap}
					isInvalid={errors.bootselButtonMap}
					onChange={handleChange}
				>
					{BUTTON_MASKS.map((o, i) => (
						<option key={`bootselButtonMap-option-${i}`} value={o.value}>
							{o.label}
						</option>
					))}
				</FormSelect>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="BootselButtonAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.BootselButtonAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('BootselButtonAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Bootsel;
