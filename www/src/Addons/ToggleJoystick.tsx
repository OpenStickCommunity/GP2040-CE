import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import { DPAD_MODES } from '../Data/Addons';

export const toggleJoystickScheme = {
	ToggleJoystickAddonEnabled: yup.number().required().label('ToggleJoystick Addon Enabled'),
	toggleJoystickPrimaryToggle: yup
		.number()
		.label('Primary Toggle Mode')
		.validateSelectionWhenValue('ToggleJoystickAddonEnabled', DPAD_MODES),
	toggleJoystickSecondaryToggle: yup
		.number()
		.label('Secondary Toggle Mode')
		.validateSelectionWhenValue('ToggleJoystickAddonEnabled', DPAD_MODES),
	toggleJoystickTiltFactor: yup
		.number()
		.label('Tilt Factor')
		.validateRangeWhenValue('ToggleJoystickAddonEnabled', 0, 100),
};

export const toggleJoystickState = {
	ToggleJoystickAddonEnabled: 0,
	toggleJoystickPrimaryToggle: 0,
	toggleJoystickSecondaryToggle: 0,
	toggleJoystickTiltFactor: 0,
};

const ToggleJoystick = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:toggle-joystick-addon-header-text')}>
			<div id="ToggleJoystickAddonOptions" hidden={!values.ToggleJoystickAddonEnabled}>
				<Row className="mb-3">
					<p>{t('AddonsConfig:pin-config-moved-to-core-text')}</p>
					<FormSelect
						label={t('AddonsConfig:toggle-joystick-addon-primary-mode-label')}
						name="toggleJoystickPrimaryToggle"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.toggleJoystickPrimaryToggle}
						error={errors.toggleJoystickPrimaryToggle}
						isInvalid={errors.toggleJoystickPrimaryToggle}
						onChange={handleChange}
					>
						{DPAD_MODES.map((o, i) => (
							<option key={`toggleJoystickPrimaryToggle-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
				</Row>
				<Row className="mb-3">
					<FormSelect
						label={t('AddonsConfig:toggle-joystick-addon-secondary-mode-label')}
						name="toggleJoystickSecondaryToggle"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.toggleJoystickSecondaryToggle}
						error={errors.toggleJoystickSecondaryToggle}
						isInvalid={errors.toggleJoystickSecondaryToggle}
						onChange={handleChange}
					>
						{DPAD_MODES.map((o, i) => (
							<option key={`toggleJoystickSecondaryToggle-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:toggle-joystick-addon-tilt-factor-label')}
						name="toggleJoystickTiltFactor"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.toggleJoystickTiltFactor}
						error={errors.toggleJoystickTiltFactor}
						isInvalid={errors.toggleJoystickTilt}Factor
						onChange={handleChange}
						min={0}
						max={100}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="ToggleJoystickInputButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.ToggleJoystickAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('ToggleJoystickAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default ToggleJoystick;
