import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';

export const drv8833RumbleScheme = {
	DRV8833RumbleAddonEnabled: yup
		.number()
		.required()
		.label('DRV8833 Rumble Add-On Enabled'),
	drv8833RumbleLeftMotorPin: yup
		.number()
		.label('Left Motor Pin')
		.validatePinWhenValue('DRV8833RumbleAddonEnabled'),
	drv8833RumbleRightMotorPin: yup
		.number()
		.label('Right Motor Pin')
		.validatePinWhenValue('DRV8833RumbleAddonEnabled'),
	drv8833RumbleMotorSleepPin: yup
		.number()
		.label('Motor Sleep Pin')
		.validatePinWhenValue('DRV8833RumbleAddonEnabled'),
	drv8833RumblePWMFrequency: yup
		.number()
		.label('PWM Frequency')
		.validateRangeWhenValue('DRV8833RumbleAddonEnabled', 0, 50000),
	drv8833RumbleDutyMin: yup
		.number()
		.label('Minimum PWM Duty')
		.validateRangeWhenValue('DRV8833RumbleAddonEnabled', 0, 100),
	drv8833RumbleDutyMax: yup
		.number()
		.label('Maximum PWM Duty')
		.validateRangeWhenValue(
			'DRV8833RumbleAddonEnabled',
			yup.ref('drv8833RumbleDutyMin'),
			100,
		),
};

export const drv8833RumbleState = {
	DRV8833RumbleAddonEnabled: 0,
	drv8833RumbleLeftMotorPin: -1,
	drv8833RumbleRightMotorPin: -1,
	drv8833RumbleMotorSleepPin: -1,
	drv8833RumblePWMFrequency: 10000,
	drv8833RumbleDutyMin: 0,
	drv8833RumbleDutyMax: 100,
};

const DRV8833Rumble = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:drv8833-rumble-header-text')}>
			<div
				id="DRV8833RumbleAddonOptions"
				hidden={!values.DRV8833RumbleAddonEnabled}
			>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:drv8833-rumble-left-motor-pin-label')}
						name="drv8833RumbleLeftMotorPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.drv8833RumbleLeftMotorPin}
						error={errors.drv8833RumbleLeftMotorPin}
						isInvalid={errors.drv8833RumbleLeftMotorPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:drv8833-rumble-right-motor-pin-label')}
						name="drv8833RumbleRightMotorPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.drv8833RumbleRightMotorPin}
						error={errors.drv8833RumbleRightMotorPin}
						isInvalid={errors.drv8833RumbleRightMotorPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:drv8833-rumble-motor-sleep-pin-label')}
						name="drv8833RumbleMotorSleepPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.drv8833RumbleMotorSleepPin}
						error={errors.drv8833RumbleMotorSleepPin}
						isInvalid={errors.drv8833RumbleMotorSleepPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:drv8833-rumble-pwm-frequency-label')}
						name="drv8833RumblePWMFrequency"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.drv8833RumblePWMFrequency}
						error={errors.drv8833RumblePWMFrequency}
						isInvalid={errors.drv8833RumblePWMFrequency}
						onChange={handleChange}
						min={1}
						max={50000}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:drv8833-rumble-duty-min-label')}
						name="drv8833RumbleDutyMin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.drv8833RumbleDutyMin}
						error={errors.drv8833RumbleDutyMin}
						isInvalid={errors.drv8833RumbleDutyMin}
						onChange={handleChange}
						min={0}
						max={100}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:drv8833-rumble-duty-max-label')}
						name="drv8833RumbleDutyMax"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.drv8833RumbleDutyMax}
						error={errors.drv8833RumbleDutyMax}
						isInvalid={errors.drv8833RumbleDutyMax}
						onChange={handleChange}
						min={0}
						max={100}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="DRV8833RumbleAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.DRV8833RumbleAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('DRV8833RumbleAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default DRV8833Rumble;
