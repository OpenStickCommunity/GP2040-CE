import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';

export const psPassthroughScheme = {
	PSPassthroughAddonEnabled: yup
		.number()
		.required()
		.label('PS Passthrough Add-On Enabled'),
	psPassthroughPinDplus: yup
		.number()
		.label('PS Passthrough D+ Pin')
		.validatePinWhenValue('PSPassthroughAddonEnabled'),
	psPassthroughPin5V: yup
		.number()
		.label('PS Passthrough 5V Power Pin')
		.validatePinWhenValue('PSPassthroughAddonEnabled'),
};

export const psPassthroughState = {
	PSPassthroughAddonEnabled: -1,
	psPassthroughPinDplus: 0,
	psPassthroughPin5V: -1,
};

const PSPassthrough = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:pspassthrough-header-text')}>
			<div
				id="PSPassthroughAddonOptions"
				hidden={!values.PSPassthroughAddonEnabled}
			>
				<Row className="mb-3">
					<p>{t('AddonsConfig:pspassthrough-sub-header-text')}</p>
					<FormControl
						type="number"
						label={t('AddonsConfig:pspassthrough-d-plus-label')}
						name="psPassthroughPinDplus"
						className="form-select-sm"
						groupClassName="col-sm-1 mb-3"
						value={values.psPassthroughPinDplus}
						error={errors.psPassthroughPinDplus}
						isInvalid={errors.psPassthroughPinDplus}
						onChange={handleChange}
						min={-1}
						max={28}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:pspassthrough-d-minus-label')}
						disabled
						className="form-select-sm"
						groupClassName="col-sm-1 mb-3"
						value={
							values.psPassthroughPinDplus === -1
								? -1
								: values.psPassthroughPinDplus + 1
						}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:pspassthrough-five-v-label')}
						name="psPassthroughPin5V"
						className="form-select-sm"
						groupClassName="col-sm-auto mb-3"
						value={values.psPassthroughPin5V}
						error={errors.psPassthroughPin5V}
						isInvalid={errors.psPassthroughPin5V}
						onChange={handleChange}
						min={-1}
						max={28}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="PSPassthroughAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.PSPassthroughAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('PSPassthroughAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default PSPassthrough;
