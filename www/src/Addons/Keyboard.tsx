import React, { useContext, useState } from 'react';
import { Trans, useTranslation } from 'react-i18next';
import { FormCheck, Row, FormLabel } from 'react-bootstrap';
import { NavLink } from 'react-router-dom';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';
import KeyboardMapper, { validateMappings } from '../Components/KeyboardMapper';
import { baseButtonMappings } from '../Services/WebApi';
import { AppContext } from '../Contexts/AppContext';

export const keyboardScheme = {
	KeyboardHostAddonEnabled: yup
		.number()
		.required()
		.label('Keyboard Host Add-On Enabled'),
};

export const keyboardState = {
	keyboardHostMap: baseButtonMappings,
	KeyboardHostAddonEnabled: 0,
};

const Keyboard = ({
	values,
	errors,
	handleChange,
	handleCheckbox,
	setFieldValue,
}) => {
	const { buttonLabels, getAvailablePeripherals } = useContext(AppContext);
	const { t } = useTranslation();
	const [validated, setValidated] = useState(false);

	const handleKeyChange = (values, setFieldValue) => (value, button) => {
		const newMappings = { ...values.keyboardHostMap };
		newMappings[button].key = value;
		const mappings = validateMappings(newMappings, t);
		setFieldValue('keyboardHostMap', mappings);
		setValidated(true);
	};

	const getKeyMappingForButton = (values) => (button) => {
		return values.keyboardHostMap[button];
	};

	return (
		<Section title={t('AddonsConfig:keyboard-host-header-text')}>
			<div
				id="KeyboardHostAddonOptions"
				hidden={!(values.KeyboardHostAddonEnabled && getAvailablePeripherals('usb'))}
			>
				<Row className="mb-3">
					<p>{t('KeyboardMapping:sub-header-text')}</p>
					<KeyboardMapper
						buttonLabels={buttonLabels}
						handleKeyChange={handleKeyChange(values, setFieldValue)}
						validated={validated}
						getKeyMappingForButton={getKeyMappingForButton(values)}
					/>
				</Row>
			</div>
            {getAvailablePeripherals('usb') ?
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="KeyboardHostAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.KeyboardHostAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('KeyboardHostAddonEnabled', values);
					handleChange(e);
				}}
			/>
            :
            <FormLabel><Trans ns="PeripheralMapping" i18nKey="peripheral-toggle-unavailable" values={{'name':'USB'}}><NavLink exact="true" to="/peripheral-mapping">{t('PeripheralMapping:header-text')}</NavLink></Trans></FormLabel>
            }
		</Section>
	);
};

export default Keyboard;
