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
		<Section title={
			<a
				href="https://gp2040-ce.info/add-ons/gamepad-usb-host"
				target="_blank"
				className="text-reset text-decoration-none"
			>
				{'Gamepad USB Host Addon'}
			</a>
		}
		>
			<div id="GamepadUSBHostOptions" hidden={!values.GamepadUSBHostAddonEnabled}>
				<div className="alert alert-info" role="alert">
					Currently incompatible with Keyboard/Mouse Host addon.
				</div>
			</div>
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
		</Section>
	);
};

export default GamepadUSBHost;
