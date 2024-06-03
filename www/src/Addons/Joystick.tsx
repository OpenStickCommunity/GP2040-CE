import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import { DPAD_MODES } from '../Data/Addons';

export const joystickScheme = {
	JSliderInputEnabled: yup.number().required().label('JSlider Input Enabled'),
	sliderModeZero: yup
		.number()
		.label('Default Mode')
		.validateSelectionWhenValue('JSliderInputEnabled', DPAD_MODES),
};

export const joystickState = {
	JSliderInputEnabled: 0,
	sliderModeZero: 0,
};

const Joystick = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:joystick-selection-slider-header-text')}>
			<div id="JSliderInputOptions" hidden={!values.JSliderInputEnabled}>
				<Row className="mb-3">
					<p>{t('AddonsConfig:pin-config-moved-to-core-text')}</p>
					<FormSelect
						label={t('AddonsConfig:joystick-selection-slider-mode-zero-label')}
						name="sliderModeZero"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.sliderModeZero}
						error={errors.sliderModeZero}
						isInvalid={errors.sliderModeZero}
						onChange={handleChange}
					>
						{DPAD_MODES.map((o, i) => (
							<option key={`sliderModeZero-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="JSliderInputButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.JSliderInputEnabled)}
				onChange={(e) => {
					handleCheckbox('JSliderInputEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Joystick;
