import { AppContext } from '../Contexts/AppContext';
import React, { useContext } from 'react';
import { Trans, useTranslation } from 'react-i18next';
import { FormCheck, Row, FormLabel } from 'react-bootstrap';
import { NavLink } from 'react-router-dom';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';

export const psPassthroughScheme = {
	PSPassthroughAddonEnabled: yup
		.number()
		.required()
		.label('PS Passthrough Add-On Enabled'),
};

export const psPassthroughState = {
	PSPassthroughAddonEnabled: -1,
};

const PSPassthrough = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
    const { getAvailablePeripherals } = useContext(AppContext);
	return (
		<Section title={t('AddonsConfig:pspassthrough-header-text')}>
			<div
				id="PSPassthroughAddonOptions"
				hidden={!values.PSPassthroughAddonEnabled}
			>
			</div>
            {getAvailablePeripherals('usb') ?
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
            :
            <FormLabel><Trans ns="PeripheralMapping" i18nKey="peripheral-toggle-unavailable" values={{'name':'USB'}}><NavLink exact="true" to="/peripheral-mapping">{t('PeripheralMapping:header-text')}</NavLink></Trans></FormLabel>
            }
		</Section>
	);
};

export default PSPassthrough;
