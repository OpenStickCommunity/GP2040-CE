import React from 'react';
import { Trans, useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';
import * as yup from 'yup';

// TODO: add scheme for validation
export const psxScheme = {
	psxEnabled: yup.number().required().label('PSX Enabled'),
	psxDataPin: yup
		.number()
		.label('Validation message PSX Data')
		.validatePinWhenValue('psxEnabled'),
	psxCommandPin: yup
		.number()
		.min(-1)
		.max(4)
		.label('Validation message PSX Command')
		.validatePinWhenValue('psxEnabled'),
	psxAttentionPin: yup
		.number()
		.label('Validation message PSX Attention')
		.validatePinWhenValue('psxEnabled'),
	psxClockPin: yup
		.number()
		.label('Validation message PSX Clock')
		.validatePinWhenValue('psxEnabled'),
	psxAcknowledgePin: yup
		.number()
		.label('Validation message PSX Acknowledge')
		.validatePinWhenValue('psxEnabled'),
};

export const psxState = {
	psxEnabled: 0,
	psxDataPin: -1,
	psxCommandPin: -1,
	psxAttentionPin: -1,
	psxClockPin: -1,
	psxAcknowledgePin: -1,
};

const PSX = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('PSX:header')}>
			<div
				id="PsxOptions"
				hidden={!values.psxEnabled}
			>
				<Row>
					{t('PSX:text')}
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('PSX:data-pin')}
						name="psxDataPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.psxDataPin}
						error={errors.psxDataPin}
						isInvalid={errors.psxDataPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('PSX:command-pin')}
						name="psxCommandPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.psxCommandPin}
						error={errors.psxCommandPin}
						isInvalid={errors.psxCommandPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('PSX:attention-pin')}
						name="psxAttentionPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.psxAttentionPin}
						error={errors.psxAttentionPin}
						isInvalid={errors.psxAttentionPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('PSX:clock-pin')}
						name="psxClockPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.psxClockPin}
						error={errors.psxClockPin}
						isInvalid={errors.psxClockPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('PSX:acknowledge-pin')}
						name="psxAcknowledgePin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.psxAcknowledgePin}
						error={errors.psxAcknowledgePin}
						isInvalid={errors.psxAcknowledgePin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="PsxButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.psxEnabled)}
				onChange={(e) => {
					handleCheckbox('psxEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default PSX;
