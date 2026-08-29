import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

export const analogTriggersScheme = {
	AnalogTriggersAddonEnabled: yup
		.number()
		.required()
		.label('Analog Triggers Add-On Enabled'),
	analogTriggersLeftPin: yup
		.number()
		.label('Left Trigger (L2) Pin')
		.validatePinWhenValue('AnalogTriggersAddonEnabled'),
	analogTriggersRightPin: yup
		.number()
		.label('Right Trigger (R2) Pin')
		.validatePinWhenValue('AnalogTriggersAddonEnabled'),
	analogTriggersInnerDeadzone: yup
		.number()
		.label('Inner Deadzone')
		.validateRangeWhenValue('AnalogTriggersAddonEnabled', 0, 100),
	analogTriggersOuterDeadzone: yup
		.number()
		.label('Outer Deadzone')
		.validateRangeWhenValue('AnalogTriggersAddonEnabled', 0, 100),
};

export const analogTriggersState = {
	AnalogTriggersAddonEnabled: 0,
	analogTriggersLeftPin: -1,
	analogTriggersRightPin: -1,
	analogTriggersLeftInvert: 0,
	analogTriggersRightInvert: 0,
	analogTriggersInnerDeadzone: 0,
	analogTriggersOuterDeadzone: 100,
};

const AnalogTriggers = ({ values, errors, handleChange, handleCheckbox }: AddonPropTypes) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:analog-triggers-header-text')}>
			<div
				id="AnalogTriggersAddonOptions"
				hidden={!values.AnalogTriggersAddonEnabled}
			>
				<p className="text-muted">
					{t('AddonsConfig:analog-triggers-sub-header-text')}
				</p>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:analog-triggers-left-pin-label')}
						name="analogTriggersLeftPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogTriggersLeftPin}
						error={errors.analogTriggersLeftPin}
						isInvalid={Boolean(errors.analogTriggersLeftPin)}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:analog-triggers-right-pin-label')}
						name="analogTriggersRightPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogTriggersRightPin}
						error={errors.analogTriggersRightPin}
						isInvalid={Boolean(errors.analogTriggersRightPin)}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:analog-triggers-inner-deadzone-label')}
						name="analogTriggersInnerDeadzone"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogTriggersInnerDeadzone}
						error={errors.analogTriggersInnerDeadzone}
						isInvalid={Boolean(errors.analogTriggersInnerDeadzone)}
						onChange={handleChange}
						min={0}
						max={100}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:analog-triggers-outer-deadzone-label')}
						name="analogTriggersOuterDeadzone"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analogTriggersOuterDeadzone}
						error={errors.analogTriggersOuterDeadzone}
						isInvalid={Boolean(errors.analogTriggersOuterDeadzone)}
						onChange={handleChange}
						min={0}
						max={100}
					/>
				</Row>
				<Row className="mb-3">
					<FormCheck
						label={t('AddonsConfig:analog-triggers-left-invert-label')}
						type="switch"
						id="analogTriggersLeftInvertButton"
						className="col-sm-3"
						reverse
						isInvalid={false}
						checked={Boolean(values.analogTriggersLeftInvert)}
						onChange={(e) => {
							handleCheckbox('analogTriggersLeftInvert');
							handleChange(e);
						}}
					/>
					<FormCheck
						label={t('AddonsConfig:analog-triggers-right-invert-label')}
						type="switch"
						id="analogTriggersRightInvertButton"
						className="col-sm-3"
						reverse
						isInvalid={false}
						checked={Boolean(values.analogTriggersRightInvert)}
						onChange={(e) => {
							handleCheckbox('analogTriggersRightInvert');
							handleChange(e);
						}}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="AnalogTriggersAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.AnalogTriggersAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('AnalogTriggersAddonEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default AnalogTriggers;
