import { useTranslation, Trans } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import { SOCD_MODES } from '../Data/Addons';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

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

const SOCD = ({ values, errors, handleChange, handleCheckbox }: AddonPropTypes) => {
	const { t } = useTranslation();
	return (
		<Section
			title={
			<a
				href="https://gp2040-ce.info/add-ons/socd-selection-slider"
				target="_blank"
				className="text-reset text-decoration-none"
			>
				{t('AddonsConfig:socd-cleaning-mode-selection-slider-header-text')}
			</a>
		}
		>
			<div id="SliderSOCDInputOptions" hidden={!values.SliderSOCDInputEnabled}>
				<div className="alert alert-info" role="alert">
					{t(
						'AddonsConfig:socd-cleaning-mode-selection-slider-sub-header-text',
					)}
				</div>
				<div className="alert alert-info" role="alert">
					<Trans
						ns="AddonsConfig"
						i18nKey='AddonsConfig:pin-config-moved-to-core-text'
						components={[
							<a
								key="0"
								href="../pin-mapping"
								className="alert-link"
								target="_blank"
							/>
						]}
					/>
				</div>
				<Row className="mb-3">
					<FormSelect
						label={t(
							'AddonsConfig:socd-cleaning-mode-selection-slider-mode-default-label',
						)}
						name="sliderSOCDModeDefault"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.sliderSOCDModeDefault}
						error={errors.sliderSOCDModeDefault}
						isInvalid={Boolean(errors.sliderSOCDModeDefault)}
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
					handleCheckbox('SliderSOCDInputEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default SOCD;
