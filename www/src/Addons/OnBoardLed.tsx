import React from 'react';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import { FormCheck } from 'react-bootstrap';

const ON_BOARD_LED_MODES = [
	{ label: 'Off', value: 0 },
	{ label: 'Mode Indicator', value: 1 },
	{ label: 'Input Test', value: 2 },
	{ label: 'PS4/5 Authentication', value: 3 },
];

export const onBoardLedScheme = {
	BoardLedAddonEnabled: yup
		.number()
		.required()
		.label('Board LED Add-On Enabled'),
	onBoardLedMode: yup
		.number()
		.label('On-Board LED Mode')
		.validateSelectionWhenValue('BoardLedAddonEnabled', ON_BOARD_LED_MODES),
};

export const onBoardLedState = {
	onBoardLedMode: 0,
	BoardLedAddonEnabled: 0,
};

const OnBoardLed = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:on-board-led-configuration-label')}>
			<div
				id="BoardLedAddonEnabledOptions"
				hidden={!values.BoardLedAddonEnabled}
			>
				<FormSelect
					label={t('AddonsConfig:on-board-led-mode-label')}
					name="onBoardLedMode"
					className="form-select-sm"
					groupClassName="col-sm-4 mb-3"
					value={values.onBoardLedMode}
					error={errors.onBoardLedMode}
					isInvalid={errors.onBoardLedMode}
					onChange={handleChange}
				>
					{ON_BOARD_LED_MODES.map((o, i) => (
						<option key={`onBoardLedMode-option-${i}`} value={o.value}>
							{o.label}
						</option>
					))}
				</FormSelect>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="BoardLedAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.BoardLedAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('BoardLedAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default OnBoardLed;
