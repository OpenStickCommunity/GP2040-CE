import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';

const REVERSE_ACTION = [
	{ label: 'Disable', value: 0 },
	{ label: 'Enable', value: 1 },
	{ label: 'Neutral', value: 2 },
];

export const reverseScheme = {
	ReverseInputEnabled: yup.number().required().label('Reverse Input Enabled'),
	reversePin: yup
		.number()
		.label('Reverse Pin')
		.validatePinWhenValue('ReverseInputEnabled'),
	reversePinLED: yup
		.number()
		.label('Reverse Pin LED')
		.validatePinWhenValue('ReverseInputEnabled'),
};

export const reverseState = {
	ReverseInputEnabled: 0,
	reversePin: -1,
	reversePinLED: -1,
	reverseActionUp: 0,
	reverseActionDown: 0,
	reverseActionLeft: 0,
	reverseActionRight: 0,
};

const Reverse = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	const translatedReverseAction = REVERSE_ACTION.map((option) => ({
		...option,
		label: t(`AddonsConfig:reverse-action-${option.label.toLowerCase()}-label`),
	}));
	return (
		<Section title={t('AddonsConfig:input-reverse-header-text')}>
			<div id="ReverseInputOptions" hidden={!values.ReverseInputEnabled}>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:input-reverse-button-pin-label')}
						name="reversePin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.reversePin}
						error={errors.reversePin}
						isInvalid={errors.reversePin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:input-reverse-led-pin-label')}
						name="reversePinLED"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.reversePinLED}
						error={errors.reversePinLED}
						isInvalid={errors.reversePinLED}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormSelect
						label={t('AddonsConfig:input-reverse-action-up-label')}
						name="reverseActionUp"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.reverseActionUp}
						error={errors.reverseActionUp}
						isInvalid={errors.reverseActionUp}
						onChange={handleChange}
					>
						{translatedReverseAction.map((o, i) => (
							<option key={`reverseActionUp-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
					<FormSelect
						label={t('AddonsConfig:input-reverse-action-down-label')}
						name="reverseActionDown"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.reverseActionDown}
						error={errors.reverseActionDown}
						isInvalid={errors.reverseActionDown}
						onChange={handleChange}
					>
						{translatedReverseAction.map((o, i) => (
							<option key={`reverseActionDown-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
					<FormSelect
						label={t('AddonsConfig:input-reverse-action-left-label')}
						name="reverseActionLeft"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.reverseActionLeft}
						error={errors.reverseActionLeft}
						isInvalid={errors.reverseActionLeft}
						onChange={handleChange}
					>
						{translatedReverseAction.map((o, i) => (
							<option key={`reverseActionLeft-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
					<FormSelect
						label={t('AddonsConfig:input-reverse-action-right-label')}
						name="reverseActionRight"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.reverseActionRight}
						error={errors.reverseActionRight}
						isInvalid={errors.reverseActionRight}
						onChange={handleChange}
					>
						{translatedReverseAction.map((o, i) => (
							<option key={`reverseActionRight-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="ReverseInputButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.ReverseInputEnabled)}
				onChange={(e) => {
					handleCheckbox('ReverseInputEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Reverse;
