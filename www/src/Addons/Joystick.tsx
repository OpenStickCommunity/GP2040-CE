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
	sliderPinOne: yup
		.number()
		.label('Slider Pin One')
		.validatePinWhenValue('JSliderInputEnabled'),
	sliderPinTwo: yup
		.number()
		.label('Slider Pin Two')
		.validatePinWhenValue('JSliderInputEnabled'),
	sliderModeZero: yup
		.number()
		.label('Slider Mode Zero')
		.validateSelectionWhenValue('JSliderInputEnabled', DPAD_MODES),
	sliderModeOne: yup
		.number()
		.label('Slider Mode One')
		.validateSelectionWhenValue('JSliderInputEnabled', DPAD_MODES),
	sliderModeTwo: yup
		.number()
		.label('Slider Mode Two')
		.validateSelectionWhenValue('JSliderInputEnabled', DPAD_MODES),
};

export const joystickState = {
	JSliderInputEnabled: 0,
	sliderPinOne: -1,
	sliderPinTwo: -1,
	sliderModeZero: 0,
	sliderModeOne: 1,
	sliderModeTwo: 2,
};

const Joystick = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:joystick-selection-slider-header-text')}>
			<div id="JSliderInputOptions" hidden={!values.JSliderInputEnabled}>
				<Row className="mb-3">
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
					<FormSelect
						label={t('AddonsConfig:joystick-selection-slider-mode-one-label')}
						name="sliderModeOne"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.sliderModeOne}
						error={errors.sliderModeOne}
						isInvalid={errors.sliderModeOne}
						onChange={handleChange}
					>
						{DPAD_MODES.map((o, i) => (
							<option key={`sliderModeOne-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
					<FormControl
						type="number"
						label={t('AddonsConfig:joystick-selection-slider-pin-one-label')}
						name="sliderPinOne"
						className="form-select-sm"
						groupClassName="col-sm-1 mb-3"
						value={values.sliderPinOne}
						error={errors.sliderPinOne}
						isInvalid={errors.sliderPinOne}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormSelect
						label={t('AddonsConfig:joystick-selection-slider-mode-two-label')}
						name="sliderModeTwo"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.sliderModeTwo}
						error={errors.sliderModeTwo}
						isInvalid={errors.sliderModeTwo}
						onChange={handleChange}
					>
						{DPAD_MODES.map((o, i) => (
							<option key={`sliderModeTwo-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
					<FormControl
						type="number"
						label={t('AddonsConfig:joystick-selection-slider-pin-two-label')}
						name="sliderPinTwo"
						className="form-control-sm"
						groupClassName="col-sm-1 mb-3"
						value={values.sliderPinTwo}
						error={errors.sliderPinTwo}
						isInvalid={errors.sliderPinTwo}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
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
