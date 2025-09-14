import { useTranslation } from 'react-i18next';
import { FormCheck } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import { BUTTON_MASKS_OPTIONS } from '../Data/Buttons';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

export const bootselScheme = {
	BootselButtonAddonEnabled: yup
		.number()
		.required()
		.label('Boot Select Button Add-On Enabled'),
	bootselButtonMap: yup
		.number()
		.label('BOOTSEL Button Map')
		.validateSelectionWhenValue(
			'BootselButtonAddonEnabled',
			BUTTON_MASKS_OPTIONS,
		),
};

export const bootselState = {
	bootselButtonMap: 0,
	BootselButtonAddonEnabled: 0,
};

const Bootsel = ({ values, errors, handleChange, handleCheckbox }: AddonPropTypes) => {
	const { t } = useTranslation();
	return (
		<Section title={
			<a
				href="https://gp2040-ce.info/add-ons/bootsel-button"
				target="_blank"
				className="text-reset text-decoration-none"
			>
				{t('AddonsConfig:bootsel-header-text')}
			</a>
		}
		>
			<div
				id="BootselButtonAddonOptions"
				hidden={!values.BootselButtonAddonEnabled}
			>
				<div className="alert alert-info" role="alert">
					{t('AddonsConfig:bootsel-sub-header-text')}
				</div>
				<FormSelect
					label={t('AddonsConfig:bootsel-button-pin-label')}
					name="bootselButtonMap"
					className="form-select-sm"
					groupClassName="col-sm-3 mb-3"
					value={values.bootselButtonMap}
					error={errors.bootselButtonMap}
					isInvalid={Boolean(errors.bootselButtonMap)}
					onChange={handleChange}
				>
					{BUTTON_MASKS_OPTIONS.map((o, i) => (
						<option key={`bootselButtonMap-option-${i}`} value={o.value}>
							{o.label}
						</option>
					))}
				</FormSelect>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="BootselButtonAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.BootselButtonAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('BootselButtonAddonEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Bootsel;
