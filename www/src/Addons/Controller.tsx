import React ,{ useContext } from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, FormLabel, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';
import { AppContext } from '../Contexts/AppContext';

export const controllerScheme = {
	ControllerHostAddonEnabled: yup
		.number()
		.required()
		.label('Controller Host Add-On Enabled'),
};

export const controllerState = {
	ControllerHostAddonEnabled: 0,
};

const Controller = ({ values, errors, handleChange, handleCheckbox }) => {
	const { getAvailablePeripherals } = useContext(AppContext);
	return (
		<Section title={"Controller Host Addon"}>
			{getAvailablePeripherals('usb') ?
			<FormCheck
				label="Enabled"
				type="switch"
				id="ControllerHostAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.ControllerHostAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('ControllerHostAddonEnabled', values);
					handleChange(e);
				}}
			/>
			: <FormLabel>USB host not enabled!</FormLabel>}
			Currently incompatible with Keyboard host addon.
		</Section>
	);
};

export default Controller;
