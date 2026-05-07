import { useTranslation } from 'react-i18next';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import { Form, FormCheck, Row } from 'react-bootstrap';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

const ENCODER_MODES = [
	{ label: 'encoder-mode-none', value: 0 },
	{ label: 'encoder-mode-left-analog-x', value: 1 },
	{ label: 'encoder-mode-left-analog-y', value: 2 },
	{ label: 'encoder-mode-right-analog-x', value: 3 },
	{ label: 'encoder-mode-right-analog-y', value: 4 },
	{ label: 'encoder-mode-left-trigger', value: 5 },
	{ label: 'encoder-mode-right-trigger', value: 6 },
	{ label: 'encoder-mode-dpad-x', value: 7 },
	{ label: 'encoder-mode-dpad-y', value: 8 },
	{ label: 'encoder-mode-volume', value: 9 },
];

const ENCODER_TYPES = [
	{ label: 'encoder-type-mechanical-detented', value: 0 },
	{ label: 'encoder-type-optical-continuous', value: 1 },
];

const ENCODER_COUNTS_PER_DETENT = [
	{ label: 'counts-per-detent-1', value: 1 },
	{ label: 'counts-per-detent-2', value: 2 },
	{ label: 'counts-per-detent-4', value: 4 },
];

const ENCODER_MIN_PPR = 1;
const ENCODER_MAX_PPR = 4096;

const ENCODER_MIN_MULTIPLE = 0.1;
const ENCODER_MAX_MULTIPLE = 200;

const ENCODER_MIN_PULSE_HOLD = 0;
const ENCODER_MAX_PULSE_HOLD = 500;

// Validators only fire when (1) the rotary addon itself is enabled AND (2) the
// specific sub-encoder is enabled. Gating per-encoder fields (pin, mode, PPR,
// etc.) on the per-encoder flag matches the firmware's protobuf semantics in
// src/webconfig.cpp, which reads encoderOne.enabled / encoderTwo.enabled
// independently. Gating only on the addon flag would force the user to fill in
// pins for the unused encoder before they could save.
const numberWhenAddonEnabled = (label: string) =>
	yup
		.number()
		.label(label)
		.when('RotaryAddonEnabled', {
			is: 1,
			then: (s) => s.required(),
			otherwise: (s) => s.notRequired(),
		});

const numberWhenSubEncoderEnabled = (label: string, subFlag: string) =>
	yup
		.number()
		.label(label)
		.when(['RotaryAddonEnabled', subFlag], {
			is: (addon: number, sub: number) => addon === 1 && sub === 1,
			then: (s) => s.required(),
			otherwise: (s) => s.notRequired(),
		});

const pinWhenSubEncoderEnabled = (label: string, subFlag: string) =>
	yup
		.number()
		.label(label)
		.when(['RotaryAddonEnabled', subFlag], {
			is: (addon: number, sub: number) => addon === 1 && sub === 1,
			then: (s) => s.required().validatePinWhenValue('RotaryAddonEnabled'),
			otherwise: (s) => s.notRequired(),
		});

export const rotaryScheme = {
	RotaryAddonEnabled: yup
		.number()
		.required()
		.label('Rotary Encoder Add-On Enabled'),
	encoderOneEnabled: numberWhenAddonEnabled('Encoder One Enabled'),
	encoderOnePinA: pinWhenSubEncoderEnabled('Encoder One Pin A', 'encoderOneEnabled'),
	encoderOnePinB: pinWhenSubEncoderEnabled('Encoder One Pin B', 'encoderOneEnabled'),
	encoderOneMode: numberWhenSubEncoderEnabled('Encoder One Mode', 'encoderOneEnabled'),
	encoderOnePPR: numberWhenSubEncoderEnabled('Encoder One PPR', 'encoderOneEnabled')
		.min(ENCODER_MIN_PPR)
		.max(ENCODER_MAX_PPR),
	encoderOneResetAfter: numberWhenSubEncoderEnabled('Encoder One Reset After', 'encoderOneEnabled').min(0),
	encoderOneAllowWrapAround: numberWhenSubEncoderEnabled('Encoder One Allow Wrap Around', 'encoderOneEnabled'),
	encoderOneMultiplier: numberWhenSubEncoderEnabled('Encoder One Multiplier', 'encoderOneEnabled')
		.min(ENCODER_MIN_MULTIPLE)
		.max(ENCODER_MAX_MULTIPLE),
	encoderOneCountsPerDetent: numberWhenSubEncoderEnabled('Encoder One Counts Per Detent', 'encoderOneEnabled').oneOf([1, 2, 4]),
	encoderOneType: numberWhenSubEncoderEnabled('Encoder One Type', 'encoderOneEnabled').oneOf([0, 1]),
	encoderOnePulseHoldMs: numberWhenSubEncoderEnabled('Encoder One Pulse Hold (ms)', 'encoderOneEnabled')
		.min(ENCODER_MIN_PULSE_HOLD)
		.max(ENCODER_MAX_PULSE_HOLD),
	encoderTwoEnabled: numberWhenAddonEnabled('Encoder Two Enabled'),
	encoderTwoPinA: pinWhenSubEncoderEnabled('Encoder Two Pin A', 'encoderTwoEnabled'),
	encoderTwoPinB: pinWhenSubEncoderEnabled('Encoder Two Pin B', 'encoderTwoEnabled'),
	encoderTwoMode: numberWhenSubEncoderEnabled('Encoder Two Mode', 'encoderTwoEnabled'),
	encoderTwoPPR: numberWhenSubEncoderEnabled('Encoder Two PPR', 'encoderTwoEnabled')
		.min(ENCODER_MIN_PPR)
		.max(ENCODER_MAX_PPR),
	encoderTwoResetAfter: numberWhenSubEncoderEnabled('Encoder Two Reset After', 'encoderTwoEnabled').min(0),
	encoderTwoAllowWrapAround: numberWhenSubEncoderEnabled('Encoder Two Allow Wrap Around', 'encoderTwoEnabled'),
	encoderTwoMultiplier: numberWhenSubEncoderEnabled('Encoder Two Multiplier', 'encoderTwoEnabled')
		.min(ENCODER_MIN_MULTIPLE)
		.max(ENCODER_MAX_MULTIPLE),
	encoderTwoCountsPerDetent: numberWhenSubEncoderEnabled('Encoder Two Counts Per Detent', 'encoderTwoEnabled').oneOf([1, 2, 4]),
	encoderTwoType: numberWhenSubEncoderEnabled('Encoder Two Type', 'encoderTwoEnabled').oneOf([0, 1]),
	encoderTwoPulseHoldMs: numberWhenSubEncoderEnabled('Encoder Two Pulse Hold (ms)', 'encoderTwoEnabled')
		.min(ENCODER_MIN_PULSE_HOLD)
		.max(ENCODER_MAX_PULSE_HOLD),
};

export const rotaryState = {
	encoderOneEnabled: 0,
	encoderOnePinA: -1,
	encoderOnePinB: -1,
	encoderOneMode: 0,
	encoderOnePPR: 24,
	encoderOneResetAfter: 0,
	encoderOneAllowWrapAround: 0,
	encoderOneMultiplier: 1,
	encoderOneCountsPerDetent: 4,
	encoderOneType: 0,
	encoderOnePulseHoldMs: 30,
	encoderTwoEnabled: 0,
	encoderTwoPinA: -1,
	encoderTwoPinB: -1,
	encoderTwoMode: 0,
	encoderTwoPPR: 24,
	encoderTwoResetAfter: 0,
	encoderTwoAllowWrapAround: 0,
	encoderTwoMultiplier: 1,
	encoderTwoCountsPerDetent: 4,
	encoderTwoType: 0,
	encoderTwoPulseHoldMs: 30,
	RotaryAddonEnabled: 0,
};

const Rotary = ({ values, errors, handleChange, handleCheckbox }: AddonPropTypes) => {
	const { t } = useTranslation();
	return (
		<Section title={
			<a
				href="https://gp2040-ce.info/add-ons/rotary-encoders"
				target="_blank"
				className="text-reset text-decoration-none"
			>
				{t('Rotary:header-text')}
			</a>
		}
		>
			<div id="RotaryAddonEnabledOptions" hidden={!values.RotaryAddonEnabled}>
				<Row className="mb-3">
					<div className="col-3">
						<label>{t('Rotary:encoder-one-label')}</label>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="EncoderOneEnabledButton"
							isInvalid={false}
							checked={Boolean(values.encoderOneEnabled)}
							onChange={() => handleCheckbox('encoderOneEnabled')}
						/>
						<div id="" hidden={!values.encoderOneEnabled}>
							<FormControl
								type="number"
								label={t('Rotary:encoder-pin-a-label')}
								name="encoderOnePinA"
								className="form-control-sm"
								groupClassName="mb-3"
								value={values.encoderOnePinA}
								error={errors.encoderOnePinA}
								isInvalid={Boolean(errors.encoderOnePinA)}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormControl
								type="number"
								label={t('Rotary:encoder-pin-b-label')}
								name="encoderOnePinB"
								className="form-control-sm"
								groupClassName="mb-3"
								value={values.encoderOnePinB}
								error={errors.encoderOnePinB}
								isInvalid={Boolean(errors.encoderOnePinB)}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormSelect
								label={t('Rotary:encoder-type-label')}
								name="encoderOneType"
								className="form-select-sm"
								groupClassName="mb-3"
								value={values.encoderOneType}
								error={errors.encoderOneType}
								isInvalid={Boolean(errors.encoderOneType)}
								onChange={handleChange}
							>
								{ENCODER_TYPES.map((o, i) => (
									<option key={`encoderOneType-option-${i}`} value={o.value}>
										{t(`Rotary:${o.label}`)}
									</option>
								))}
							</FormSelect>
							<FormControl
								type="number"
								label={t('Rotary:encoder-ppr-label')}
								name="encoderOnePPR"
								className="form-control-sm"
								groupClassName="mb-3"
								value={values.encoderOnePPR}
								error={errors.encoderOnePPR}
								isInvalid={Boolean(errors.encoderOnePPR)}
								onChange={handleChange}
								min={ENCODER_MIN_PPR}
								max={ENCODER_MAX_PPR}
							/>
							<FormSelect
								label={t('Rotary:encoder-counts-per-detent-label')}
								name="encoderOneCountsPerDetent"
								className="form-select-sm"
								groupClassName="mb-3"
								value={values.encoderOneCountsPerDetent}
								error={errors.encoderOneCountsPerDetent}
								isInvalid={Boolean(errors.encoderOneCountsPerDetent)}
								onChange={handleChange}
							>
								{ENCODER_COUNTS_PER_DETENT.map((o, i) => (
									<option key={`encoderOneCountsPerDetent-option-${i}`} value={o.value}>
										{t(`Rotary:${o.label}`)}
									</option>
								))}
							</FormSelect>
							<FormControl
								type="number"
								label={t('Rotary:encoder-multiplier-label')}
								name="encoderOneMultiplier"
								className="form-select-sm"
								groupClassName="mb-3"
								value={values.encoderOneMultiplier}
								error={errors.encoderOneMultiplier}
								isInvalid={Boolean(errors.encoderOneMultiplier)}
								onChange={handleChange}
								min={ENCODER_MIN_MULTIPLE}
								max={ENCODER_MAX_MULTIPLE}
							/>
							<FormControl
								type="number"
								label={t('Rotary:encoder-reset-after-label')}
								name="encoderOneResetAfter"
								className="form-control-sm"
								groupClassName="mb-1"
								value={values.encoderOneResetAfter}
								error={errors.encoderOneResetAfter}
								isInvalid={Boolean(errors.encoderOneResetAfter)}
								onChange={handleChange}
								min={0}
								max={500}
							/>
							<Form.Text className="text-muted d-block mb-3">
								{t('Rotary:encoder-reset-after-help')}
							</Form.Text>
							<FormControl
								type="number"
								label={t('Rotary:encoder-pulse-hold-ms-label')}
								name="encoderOnePulseHoldMs"
								className="form-control-sm"
								groupClassName="mb-3"
								value={values.encoderOnePulseHoldMs}
								error={errors.encoderOnePulseHoldMs}
								isInvalid={Boolean(errors.encoderOnePulseHoldMs)}
								onChange={handleChange}
								min={ENCODER_MIN_PULSE_HOLD}
								max={ENCODER_MAX_PULSE_HOLD}
							/>
							<FormSelect
								label={t('Rotary:encoder-mode-label')}
								name="encoderOneMode"
								className="form-select-sm"
								groupClassName="mb-3"
								value={values.encoderOneMode}
								error={errors.encoderOneMode}
								isInvalid={Boolean(errors.encoderOneMode)}
								onChange={handleChange}
							>
								{ENCODER_MODES.map((o, i) => (
									<option key={`encoderOneMode-option-${i}`} value={o.value}>
										{t(`Rotary:${o.label}`)}
									</option>
								))}
							</FormSelect>
							<FormCheck
								label={t('Rotary:encoder-allow-wrap-around-label')}
								type="switch"
								id="EncoderOneAllowWrapAround"
								isInvalid={false}
								checked={Boolean(values.encoderOneAllowWrapAround)}
								onChange={() => handleCheckbox('encoderOneAllowWrapAround')}
							/>
						</div>
					</div>
					<div className="col-3">
						<label>{t('Rotary:encoder-two-label')}</label>
						<FormCheck
							label={t('Common:switch-enabled')}
							type="switch"
							id="EncoderTwoEnabledButton"
							isInvalid={false}
							checked={Boolean(values.encoderTwoEnabled)}
							onChange={() => handleCheckbox('encoderTwoEnabled')}
						/>
						<div id="" hidden={!values.encoderTwoEnabled}>
							<FormControl
								type="number"
								label={t('Rotary:encoder-pin-a-label')}
								name="encoderTwoPinA"
								className="form-control-sm"
								groupClassName="mb-3"
								value={values.encoderTwoPinA}
								error={errors.encoderTwoPinA}
								isInvalid={Boolean(errors.encoderTwoPinA)}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormControl
								type="number"
								label={t('Rotary:encoder-pin-b-label')}
								name="encoderTwoPinB"
								className="form-control-sm"
								groupClassName="mb-3"
								value={values.encoderTwoPinB}
								error={errors.encoderTwoPinB}
								isInvalid={Boolean(errors.encoderTwoPinB)}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormSelect
								label={t('Rotary:encoder-type-label')}
								name="encoderTwoType"
								className="form-select-sm"
								groupClassName="mb-3"
								value={values.encoderTwoType}
								error={errors.encoderTwoType}
								isInvalid={Boolean(errors.encoderTwoType)}
								onChange={handleChange}
							>
								{ENCODER_TYPES.map((o, i) => (
									<option key={`encoderTwoType-option-${i}`} value={o.value}>
										{t(`Rotary:${o.label}`)}
									</option>
								))}
							</FormSelect>
							<FormControl
								type="number"
								label={t('Rotary:encoder-ppr-label')}
								name="encoderTwoPPR"
								className="form-control-sm"
								groupClassName="mb-3"
								value={values.encoderTwoPPR}
								error={errors.encoderTwoPPR}
								isInvalid={Boolean(errors.encoderTwoPPR)}
								onChange={handleChange}
								min={ENCODER_MIN_PPR}
								max={ENCODER_MAX_PPR}
							/>
							<FormSelect
								label={t('Rotary:encoder-counts-per-detent-label')}
								name="encoderTwoCountsPerDetent"
								className="form-select-sm"
								groupClassName="mb-3"
								value={values.encoderTwoCountsPerDetent}
								error={errors.encoderTwoCountsPerDetent}
								isInvalid={Boolean(errors.encoderTwoCountsPerDetent)}
								onChange={handleChange}
							>
								{ENCODER_COUNTS_PER_DETENT.map((o, i) => (
									<option key={`encoderTwoCountsPerDetent-option-${i}`} value={o.value}>
										{t(`Rotary:${o.label}`)}
									</option>
								))}
							</FormSelect>
							<FormControl
								type="number"
								label={t('Rotary:encoder-multiplier-label')}
								name="encoderTwoMultiplier"
								className="form-select-sm"
								groupClassName="mb-3"
								value={values.encoderTwoMultiplier}
								error={errors.encoderTwoMultiplier}
								isInvalid={Boolean(errors.encoderTwoMultiplier)}
								onChange={handleChange}
								min={ENCODER_MIN_MULTIPLE}
								max={ENCODER_MAX_MULTIPLE}
							/>
							<FormControl
								type="number"
								label={t('Rotary:encoder-reset-after-label')}
								name="encoderTwoResetAfter"
								className="form-control-sm"
								groupClassName="mb-1"
								value={values.encoderTwoResetAfter}
								error={errors.encoderTwoResetAfter}
								isInvalid={Boolean(errors.encoderTwoResetAfter)}
								onChange={handleChange}
								min={0}
								max={500}
							/>
							<Form.Text className="text-muted d-block mb-3">
								{t('Rotary:encoder-reset-after-help')}
							</Form.Text>
							<FormControl
								type="number"
								label={t('Rotary:encoder-pulse-hold-ms-label')}
								name="encoderTwoPulseHoldMs"
								className="form-control-sm"
								groupClassName="mb-3"
								value={values.encoderTwoPulseHoldMs}
								error={errors.encoderTwoPulseHoldMs}
								isInvalid={Boolean(errors.encoderTwoPulseHoldMs)}
								onChange={handleChange}
								min={ENCODER_MIN_PULSE_HOLD}
								max={ENCODER_MAX_PULSE_HOLD}
							/>
							<FormSelect
								label={t('Rotary:encoder-mode-label')}
								name="encoderTwoMode"
								className="form-select-sm"
								groupClassName="mb-3"
								value={values.encoderTwoMode}
								error={errors.encoderTwoMode}
								isInvalid={Boolean(errors.encoderTwoMode)}
								onChange={handleChange}
							>
								{ENCODER_MODES.map((o, i) => (
									<option key={`encoderTwoMode-option-${i}`} value={o.value}>
										{t(`Rotary:${o.label}`)}
									</option>
								))}
							</FormSelect>
							<FormCheck
								label={t('Rotary:encoder-allow-wrap-around-label')}
								type="switch"
								id="EncoderTwoAllowWrapAround"
								isInvalid={false}
								checked={Boolean(values.encoderTwoAllowWrapAround)}
								onChange={() => handleCheckbox('encoderTwoAllowWrapAround')}
							/>
						</div>
					</div>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="RotaryAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.RotaryAddonEnabled)}
				onChange={() => handleCheckbox('RotaryAddonEnabled')}
			/>
		</Section>
	);
};

export default Rotary;
