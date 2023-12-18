import { AppContext } from '../Contexts/AppContext';
import React, { useContext } from 'react';
import { Trans, useTranslation } from 'react-i18next';
import { FormCheck, Row, FormLabel } from 'react-bootstrap';
import { NavLink } from 'react-router-dom';
import * as yup from 'yup';

import Section from '../Components/Section';

export const xbonePassthroughScheme = {
	XBOnePassthroughAddonEnabled: yup
		.number()
		.required()
		.label('Xbox One Passthrough Add-On Enabled')
};

export const xbonePassthroughState = {
	XBOnePassthroughAddonEnabled: -1
};

const XBOnePassthrough = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
    const { getAvailablePeripherals } = useContext(AppContext);
	return (
		<Section title={t('AddonsConfig:xbonepassthrough-header-text')}>
			<div
				id="XBOnePassthroughAddonOptions"
				hidden={!values.XBOnePassthroughAddonEnabled}
			>
			</div>
            {getAvailablePeripherals('usb') ?
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="XBOnePassthroughAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.XBOnePassthroughAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('XBOnePassthroughAddonEnabled', values);
					handleChange(e);
				}}
			/>
            :
            <FormLabel><Trans ns="PeripheralMapping" i18nKey="peripheral-toggle-unavailable" values={{'name':'USB'}}><NavLink exact="true" to="/peripheral-mapping">{t('PeripheralMapping:header-text')}</NavLink></Trans></FormLabel>
            }
		</Section>
	);
};

export default XBOnePassthrough;
