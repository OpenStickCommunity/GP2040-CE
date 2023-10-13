import React, { useContext, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
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
	keyboardHostPinDplus: yup
		.number()
		.label('Keyboard Host D+ Pin')
		.validatePinWhenValue('KeyboardHostAddonEnabled'),
	keyboardHostPin5V: yup
		.number()
		.label('Keyboard Host 5V Power Pin')
		.validatePinWhenValue('KeyboardHostAddonEnabled'),
};

export const keyboardState = {
	keyboardHostPinDplus: -1,
	keyboardHostPin5V: -1,
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
	const { buttonLabels } = useContext(AppContext);
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
				hidden={!values.KeyboardHostAddonEnabled}
			>
				<Row className="mb-3">
					<p>{t('AddonsConfig:keyboard-host-sub-header-text')}</p>
					<FormControl
						type="number"
						label={t('AddonsConfig:keyboard-host-d-plus-label')}
						name="keyboardHostPinDplus"
						className="form-select-sm"
						groupClassName="col-sm-1 mb-3"
						value={values.keyboardHostPinDplus}
						error={errors.keyboardHostPinDplus}
						isInvalid={Boolean(errors.keyboardHostPinDplus)}
						onChange={handleChange}
						min={-1}
						max={28}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:keyboard-host-d-minus-label')}
						disabled
						className="form-select-sm"
						groupClassName="col-sm-1 mb-3"
						value={
							values.keyboardHostPinDplus === -1
								? -1
								: values.keyboardHostPinDplus + 1
						}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:keyboard-host-five-v-label')}
						name="keyboardHostPin5V"
						className="form-select-sm"
						groupClassName="col-sm-auto mb-3"
						value={values.keyboardHostPin5V}
						error={errors.keyboardHostPin5V}
						isInvalid={Boolean(errors.keyboardHostPin5V)}
						onChange={handleChange}
						min={-1}
						max={28}
					/>
				</Row>
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
		</Section>
	);
};

export default Keyboard;
