import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormSelect from '../Components/FormSelect';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

export const lightingReportScheme = {
	LightingReportAddonEnabled: yup
		.number()
		.required()
		.label('Lighting Report Add-On Enabled'),
	lightingReportXInputMode: yup
		.number()
		.label('Lighting Report XInput Mode')
		.validateRangeWhenValue('LightingReportAddonEnabled', 0, 2),
};

export const lightingReportState = {
	LightingReportAddonEnabled: 0,
	lightingReportXInputMode: 2,
};

const XINPUT_MODES = [
	{ labelKey: 'lighting-report-xinput-mode-off', value: 0 },
	{ labelKey: 'lighting-report-xinput-mode-on', value: 1 },
	{ labelKey: 'lighting-report-xinput-mode-auto', value: 2 },
];

const LightingReport = ({
	values,
	errors,
	handleChange,
	handleCheckbox,
}: AddonPropTypes) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:lighting-report-header-text')}>
			<div
				id="LightingReportAddonOptions"
				hidden={!values.LightingReportAddonEnabled}
			>
				<div className="alert alert-info" role="alert">
					{t('AddonsConfig:lighting-report-sub-header-text')}
				</div>
				<Row className="mb-3">
					<FormSelect
						label={t('AddonsConfig:lighting-report-xinput-mode-label')}
						name="lightingReportXInputMode"
						className="form-select-sm"
						groupClassName="col-sm-4 mb-3"
						value={values.lightingReportXInputMode}
						error={errors.lightingReportXInputMode}
						isInvalid={Boolean(errors.lightingReportXInputMode)}
						onChange={handleChange}
					>
						{XINPUT_MODES.map((mode, i) => (
							<option
								key={`lightingReportXInputMode-option-${i}`}
								value={mode.value}
							>
								{t(`AddonsConfig:${mode.labelKey}`)}
							</option>
						))}
					</FormSelect>
					<div className="small text-muted">
						{t('AddonsConfig:lighting-report-xinput-mode-help')}
					</div>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="LightingReportAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.LightingReportAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('LightingReportAddonEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default LightingReport;
