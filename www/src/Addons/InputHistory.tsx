import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';

export const inputHistoryScheme = {
	InputHistoryAddonEnabled: yup
		.number()
		.required()
		.label('Input History Enabled'),
	inputHistoryLength: yup
		.number()
		.label('Input History Length')
		.validateRangeWhenValue('InputHistoryAddonEnabled', 1, 21),
	inputHistoryCol: yup
		.number()
		.label('Col')
		.validateRangeWhenValue('InputHistoryAddonEnabled', 0, 20),
	inputHistoryRow: yup
		.number()
		.label('Row')
		.validateRangeWhenValue('InputHistoryAddonEnabled', 0, 7),
};

export const inputHistoryState = {
	InputHistoryAddonEnabled: 0,
	inputHistoryLength: 21,
	inputHistoryCol: 0,
	inputHistoryRow: 7,
};

const InputHistory = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:input-history-header-text')}>
			<div id="InputHistoryOptions" hidden={!values.InputHistoryAddonEnabled}>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:input-history-length-label')}
						name="inputHistoryLength"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.inputHistoryLength}
						error={errors.inputHistoryLength}
						isInvalid={errors.inputHistoryLength}
						onChange={handleChange}
						min={1}
						max={21}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:input-history-col-label')}
						name="inputHistoryCol"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.inputHistoryCol}
						error={errors.inputHistoryCol}
						isInvalid={errors.inputHistoryCol}
						onChange={handleChange}
						min={0}
						max={20}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:input-history-row-label')}
						name="inputHistoryRow"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.inputHistoryRow}
						error={errors.inputHistoryRow}
						isInvalid={errors.inputHistoryRow}
						onChange={handleChange}
						min={0}
						max={7}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="InputHistoryButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.InputHistoryAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('InputHistoryAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default InputHistory;
