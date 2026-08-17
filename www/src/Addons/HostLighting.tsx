import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormSelect from '../Components/FormSelect';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

export const hostLightingScheme = {
	HostLightingAddonEnabled: yup
		.number()
		.required()
		.label('Host Lighting Add-On Enabled'),
	hostLightingXInputMode: yup
		.number()
		.label('Host Lighting XInput Mode')
		.validateRangeWhenValue('HostLightingAddonEnabled', 0, 2),
};

export const hostLightingState = {
	HostLightingAddonEnabled: 0,
	hostLightingXInputMode: 2,
};

const XINPUT_MODES = [
	{ labelKey: 'host-lighting-xinput-mode-off', value: 0 },
	{ labelKey: 'host-lighting-xinput-mode-on', value: 1 },
	{ labelKey: 'host-lighting-xinput-mode-auto', value: 2 },
];

const HostLighting = ({
	values,
	errors,
	handleChange,
	handleCheckbox,
}: AddonPropTypes) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:host-lighting-header-text')}>
			<div
				id="HostLightingAddonOptions"
				hidden={!values.HostLightingAddonEnabled}
			>
				<div className="alert alert-info" role="alert">
					{t('AddonsConfig:host-lighting-sub-header-text')}
				</div>
				<Row className="mb-3">
					<FormSelect
						label={t('AddonsConfig:host-lighting-xinput-mode-label')}
						name="hostLightingXInputMode"
						className="form-select-sm"
						groupClassName="col-sm-4 mb-3"
						value={values.hostLightingXInputMode}
						error={errors.hostLightingXInputMode}
						isInvalid={Boolean(errors.hostLightingXInputMode)}
						onChange={handleChange}
					>
						{XINPUT_MODES.map((mode, i) => (
							<option key={`hostLightingXInputMode-option-${i}`} value={mode.value}>
								{t(`AddonsConfig:${mode.labelKey}`)}
							</option>
						))}
					</FormSelect>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="HostLightingAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.HostLightingAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('HostLightingAddonEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default HostLighting;
