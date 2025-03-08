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
	sliderSOCDModeDefault: yup
		.number()
		.label('SOCD Slider Mode Default')
		.validateSelectionWhenValue('SliderSOCDInputEnabled', SOCD_MODES),
};

export const socdState = {
	SliderSOCDInputEnabled: 0,
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
						<br />
						{t('AddonsConfig:pin-config-moved-to-core-text')}
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
								{t(`AddonsConfig:socd-slider-mode-${i}`)}
							</option>
						))}
					</FormSelect>
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
