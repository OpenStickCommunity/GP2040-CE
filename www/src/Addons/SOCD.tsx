import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import { SOCD_MODES } from '../Data/Addons';

export const socdScheme = {
	SliderSOCDInputEnabled: yup
		.number()
		.required()
		.label('Slider SOCD Input Enabled'),
	sliderSOCDModeOne: yup
		.number()
		.label('SOCD Slider Mode One')
		.validateSelectionWhenValue('SliderSOCDInputEnabled', SOCD_MODES),
	sliderSOCDModeTwo: yup
		.number()
		.label('SOCD Slider Mode Two')
		.validateSelectionWhenValue('SliderSOCDInputEnabled', SOCD_MODES),
	sliderSOCDModeDefault: yup
		.number()
		.label('SOCD Slider Mode Default')
		.validateSelectionWhenValue('SliderSOCDInputEnabled', SOCD_MODES),
	sliderSOCDPinOne: yup
		.number()
		.label('Slider SOCD Up Priority Pin')
		.validatePinWhenValue('SliderSOCDInputEnabled'),
	sliderSOCDPinTwo: yup
		.number()
		.label('Slider SOCD Second Priority Pin')
		.validatePinWhenValue('SliderSOCDInputEnabled'),
};

export const socdState = {
	SliderSOCDInputEnabled: 0,
	sliderSOCDPinOne: -1,
	sliderSOCDPinTwo: -1,

	sliderSOCDModeOne: 0,
	sliderSOCDModeTwo: 2,
	sliderSOCDModeDefault: 1,
};

const SOCD = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section
			title={t('AddonsConfig:socd-cleaning-mode-selection-slider-header-text')}
		>
			<div id="SliderSOCDInputOptions" hidden={!values.SliderSOCDInputEnabled}>
				<Row className="mb-3">
					<p>
						{t(
							'AddonsConfig:socd-cleaning-mode-selection-slider-sub-header-text',
						)}
					</p>
					<FormSelect
						label={t(
							'AddonsConfig:socd-cleaning-mode-selection-slider-mode-default-label',
						)}
						name="sliderSOCDModeDefault"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.sliderSOCDModeDefault}
						error={errors.sliderSOCDModeDefault}
						isInvalid={errors.sliderSOCDModeDefault}
						onChange={handleChange}
					>
						{SOCD_MODES.map((o, i) => (
							<option key={`sliderSOCDModeDefault-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
					<FormSelect
						label={t(
							'AddonsConfig:socd-cleaning-mode-selection-slider-mode-one-label',
						)}
						name="sliderSOCDModeOne"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.sliderSOCDModeOne}
						error={errors.sliderSOCDModeOne}
						isInvalid={errors.sliderSOCDModeOne}
						onChange={handleChange}
					>
						{SOCD_MODES.map((o, i) => (
							<option key={`sliderSOCDModeOne-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
					<FormControl
						type="number"
						label={t(
							'AddonsConfig:socd-cleaning-mode-selection-slider-pin-one-label',
						)}
						name="sliderSOCDPinOne"
						className="form-select-sm"
						groupClassName="col-sm-1 mb-3"
						value={values.sliderSOCDPinOne}
						error={errors.sliderSOCDPinOne}
						isInvalid={errors.sliderSOCDPinOne}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormSelect
						label={t(
							'AddonsConfig:socd-cleaning-mode-selection-slider-mode-two-label',
						)}
						name="sliderSOCDModeTwo"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.sliderSOCDModeTwo}
						error={errors.sliderSOCDModeTwo}
						isInvalid={errors.sliderSOCDModeTwo}
						onChange={handleChange}
					>
						{SOCD_MODES.map((o, i) => (
							<option key={`sliderSOCDModeTwo-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
					<FormControl
						type="number"
						label={t(
							'AddonsConfig:socd-cleaning-mode-selection-slider-pin-two-label',
						)}
						name="sliderSOCDPinTwo"
						className="form-control-sm"
						groupClassName="col-sm-1 mb-3"
						value={values.sliderSOCDPinTwo}
						error={errors.sliderSOCDPinTwo}
						isInvalid={errors.sliderSOCDPinTwo}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="SliderSOCDInputButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.SliderSOCDInputEnabled)}
				onChange={(e) => {
					handleCheckbox('SliderSOCDInputEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default SOCD;
