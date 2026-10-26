import { Col, Form, FormCheck, Row } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';

import FormSelect from './FormSelect';

// Up priority is up/down only; we cannot know which way a character faces
const SOCD_AXIS_MODES = [
	{ labelKey: 'advanced-socd-axis-neutral', value: 0 },
	{ labelKey: 'advanced-socd-axis-last-win', value: 1 },
	{ labelKey: 'advanced-socd-axis-first-win', value: 2 },
	{ labelKey: 'advanced-socd-axis-off', value: 3 },
];

const SOCD_UP_DOWN_MODES = [
	...SOCD_AXIS_MODES,
	{ labelKey: 'advanced-socd-axis-up-priority', value: 4 },
];

export const ADVANCED_SOCD_SLOTS = [
	{
		index: 1,
		value: 5,
		label: 'SOCD-1',
		enabledKey: 'advancedSOCD1Enabled',
		upDownKey: 'advancedSOCD1UpDown',
		leftRightKey: 'advancedSOCD1LeftRight',
		nameKey: 'advancedSOCD1Name',
	},
	{
		index: 2,
		value: 6,
		label: 'SOCD-2',
		enabledKey: 'advancedSOCD2Enabled',
		upDownKey: 'advancedSOCD2UpDown',
		leftRightKey: 'advancedSOCD2LeftRight',
		nameKey: 'advancedSOCD2Name',
	},
	{
		index: 3,
		value: 7,
		label: 'SOCD-3',
		enabledKey: 'advancedSOCD3Enabled',
		upDownKey: 'advancedSOCD3UpDown',
		leftRightKey: 'advancedSOCD3LeftRight',
		nameKey: 'advancedSOCD3Name',
	},
	{
		index: 4,
		value: 8,
		label: 'SOCD-4',
		enabledKey: 'advancedSOCD4Enabled',
		upDownKey: 'advancedSOCD4UpDown',
		leftRightKey: 'advancedSOCD4LeftRight',
		nameKey: 'advancedSOCD4Name',
	},
];

type AdvancedSOCDSlot = (typeof ADVANCED_SOCD_SLOTS)[number];

export const getAdvancedSOCDLabel = (
	values: Record<string, any>,
	slot: AdvancedSOCDSlot,
) => values?.[slot.nameKey] || slot.label;

export const getEnabledAdvancedSOCDSlots = (values: Record<string, any>) =>
	values?.advancedSOCDEnabled
		? ADVANCED_SOCD_SLOTS.filter((slot) => values[slot.enabledKey])
		: [];

export const advancedSOCDScheme = ADVANCED_SOCD_SLOTS.reduce(
	(scheme, { label, enabledKey, upDownKey, leftRightKey, nameKey }) => ({
		...scheme,
		[enabledKey]: yup.number().label(`${label} Enabled`),
		[nameKey]: yup.string().max(6).label(`${label} Name`),
		[upDownKey]: yup.number().min(0).max(4).label(`${label} Up + Down`),
		[leftRightKey]: yup.number().min(0).max(3).label(`${label} Left + Right`),
	}),
	{ advancedSOCDEnabled: yup.number().label('Advanced SOCD Enabled') },
);

const AdvancedSOCD = ({ values, errors, handleChange, setFieldValue }) => {
	const { t } = useTranslation('');

	return (
		<div className="mb-3">
			<p>{t('SettingsPage:advanced-socd-description')}</p>
			<Row className="g-3">
				{ADVANCED_SOCD_SLOTS.map(
					({ label, enabledKey, upDownKey, leftRightKey, nameKey }) => (
						<Col key={enabledKey} md={6} xxl={3}>
							<div className="border rounded p-3 h-100">
								<div className="d-flex justify-content-between align-items-center mb-3">
									<strong>{values[nameKey] || label}</strong>
									<FormCheck
										label={t('Common:switch-enabled')}
										type="switch"
										id={enabledKey}
										reverse
										isInvalid={false}
										checked={Boolean(values[enabledKey])}
										onChange={(e) =>
											setFieldValue(enabledKey, e.target.checked ? 1 : 0)
										}
									/>
								</div>
								<Form.Group className="mb-3">
									<Form.Label>
										{t('SettingsPage:advanced-socd-name-label')}
									</Form.Label>
									<Form.Control
										size="sm"
										type="text"
										name={nameKey}
										value={values[nameKey] ?? ''}
										placeholder={label}
										maxLength={6}
										disabled={!values[enabledKey]}
										isInvalid={Boolean(errors[nameKey])}
										onChange={(e) =>
											setFieldValue(
												nameKey,
												e.target.value.replace(/[^\x20-\x7e]/g, ''),
											)
										}
									/>
								</Form.Group>
								<FormSelect
									label={t('SettingsPage:advanced-socd-up-down-label')}
									name={upDownKey}
									className="form-select-sm"
									groupClassName="mb-3"
									value={values[upDownKey]}
									error={errors[upDownKey]}
									isInvalid={Boolean(errors[upDownKey])}
									disabled={!values[enabledKey]}
									onChange={handleChange}
								>
									{SOCD_UP_DOWN_MODES.map((o, i) => (
										<option key={`${upDownKey}-option-${i}`} value={o.value}>
											{t(`SettingsPage:${o.labelKey}`)}
										</option>
									))}
								</FormSelect>
								<FormSelect
									label={t('SettingsPage:advanced-socd-left-right-label')}
									name={leftRightKey}
									className="form-select-sm"
									value={values[leftRightKey]}
									error={errors[leftRightKey]}
									isInvalid={Boolean(errors[leftRightKey])}
									disabled={!values[enabledKey]}
									onChange={handleChange}
								>
									{SOCD_AXIS_MODES.map((o, i) => (
										<option key={`${leftRightKey}-option-${i}`} value={o.value}>
											{t(`SettingsPage:${o.labelKey}`)}
										</option>
									))}
								</FormSelect>
							</div>
						</Col>
					),
				)}
			</Row>
		</div>
	);
};

export default AdvancedSOCD;
