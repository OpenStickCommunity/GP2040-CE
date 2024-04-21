import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';

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
	return (
		<Section title={"Controller Host Addon"}>

			<FormCheck
				label={'Common:switch-enabled'}
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
		</Section>
	);
};

export default Controller;
