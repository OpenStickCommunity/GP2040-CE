import { AppContext } from '../Contexts/AppContext';
import React, { useContext } from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row, FormLabel } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

export const xbonePassthroughScheme = {
	XBonePassthroughAddonEnabled: yup
		.number()
		.required()
		.label('Xbox One Passthrough Add-On Enabled')
};

export const xbonePassthroughState = {
	XBonePassthroughAddonEnabled: -1
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
            <FormLabel>{t('PeripheralMapping:peripheral-toggle-unavailable',{'name':'USB'})}</FormLabel>
            }
		</Section>
	);
};

export default XBOnePassthrough;
