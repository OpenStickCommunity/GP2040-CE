import React, { useContext } from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, FormLabel, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';
import { AppContext } from '../Contexts/AppContext';

export const gamepadUSBHostScheme = {
	GamepadUSBHostAddonEnabled: yup
		.number()
		.required()
		.label('Gamepad USB Host Add-On Enabled'),
};

export const gamepadUSBHostState = {
	GamepadUSBHostAddonEnabled: 0,
};

const GamepadUSBHost = ({ values, errors, handleChange, handleCheckbox }) => {
	const { getAvailablePeripherals } = useContext(AppContext);
	return (
		<Section title={'Gamepad USB Host Addon'}>
			{getAvailablePeripherals('usb') ? (
				<FormCheck
					label="Enabled"
					type="switch"
					id="GamepadUSBHostAddonButton"
					reverse
					isInvalid={false}
					checked={Boolean(values.GamepadUSBHostAddonEnabled)}
					onChange={(e) => {
						handleCheckbox('GamepadUSBHostAddonEnabled', values);
						handleChange(e);
					}}
				/>
			) : (
				<FormLabel>USB host not enabled!</FormLabel>
			)}
			Currently incompatible with Keyboard host addon.
		</Section>
	);
};

export default GamepadUSBHost;
