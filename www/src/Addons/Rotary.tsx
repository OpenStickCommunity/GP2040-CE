import { useTranslation } from 'react-i18next';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import { FormCheck, Row } from 'react-bootstrap';
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

const ENCODER_MULTIPLES = [
	{ value: 0.25 },
	{ value: 0.5 },
	{ value: 0.75 },
	{ value: 1, default: true },
	{ value: 2 },
	{ value: 4 },
	{ value: 8 },
	{ value: 16 },
	{ value: 32 },
	{ value: 64 },
];

const ENCODER_MIN_PPR = 10;
const ENCODER_MAX_PPR = 800;

const ENCODER_MIN_MULTIPLE = 0.1;
const ENCODER_MAX_MULTIPLE = 200;

export const rotaryScheme = {
	RotaryAddonEnabled: yup
		.number()
		.required()
		.label('Rotary Encoder Add-On Enabled'),
	encoderOneEnabled: yup
		.number()
		.required()
		.label('Encoder One Enabled'),
	encoderOnePinA: yup
		.number()
		.label('Encoder One Pin A')
		.required()
		.validatePinWhenValue('RotaryAddonEnabled'),
	encoderOnePinB: yup
		.number()
		.label('Encoder One Pin B')
		.required()
		.validatePinWhenValue('RotaryAddonEnabled'),
	encoderOneMode: yup.number().required().label('Encoder One Mode'),
	encoderOnePPR: yup.number().label('Encoder One PPR').required(),
	encoderOneResetAfter: yup
		.number()
		.label('Encoder One Reset After')
		.required(),
	encoderOneAllowWrapAround: yup
		.boolean()
		.required()
		.label('Encoder One Allow Wrap Around'),
	encoderOneMultiplier: yup.number().label('Encoder One Multiplier').required(),
	encoderTwoEnabled: yup
		.number()
		.required()
		.label('Encoder Two Enabled'),
	encoderTwoPinA: yup
		.number()
		.label('Encoder Two Pin A')
		.required()
		.validatePinWhenValue('RotaryAddonEnabled'),
	encoderTwoPinB: yup
		.number()
		.label('Encoder Two Pin B')
		.required()
		.validatePinWhenValue('RotaryAddonEnabled'),
	encoderTwoMode: yup.number().required().label('Encoder Two Mode'),
	encoderTwoPPR: yup.number().label('Encoder Two PPR').required(),
	encoderTwoResetAfter: yup
		.number()
		.label('Encoder Two Reset After')
		.required(),
	encoderTwoAllowWrapAround: yup
		.boolean()
		.required()
		.label('Encoder Two Allow Wrap Around'),
	encoderTwoMultiplier: yup.number().label('Encoder Two Multiplier').required(),
};

export const rotaryState = {
	encoderOneEnabled: 0,
	encoderOnePinA: -1,
	encoderOnePinB: -1,
	encoderOneMode: 0,
	encoderOnePPR: 24,
	encoderOneResetAfter: 0,
	encoderOneAllowWrapAround: 0,
	encoderOneMultiplier: 0,
	encoderTwoEnabled: 0,
	encoderTwoPinA: -1,
	encoderTwoPinB: -1,
	encoderTwoMode: 0,
	encoderTwoPPR: 24,
	encoderTwoResetAfter: 0,
	encoderTwoAllowWrapAround: 0,
	encoderTwoMultiplier: 0,
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
							onChange={(e) => {
								handleCheckbox('encoderOneEnabled');
								handleChange(e);
							}}
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
								groupClassName="mb-3"
								value={values.encoderOneResetAfter}
								error={errors.encoderOneResetAfter}
								isInvalid={Boolean(errors.encoderOneResetAfter)}
								onChange={handleChange}
								min={0}
								max={500}
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
								id="encoderOneAllowWrapAround"
								isInvalid={false}
								checked={Boolean(values.encoderOneAllowWrapAround)}
								onChange={(e) => {
									handleCheckbox('encoderOneAllowWrapAround');
									handleChange(e);
								}}
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
							onChange={(e) => {
								handleCheckbox('encoderTwoEnabled');
								handleChange(e);
							}}
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
								groupClassName="mb-3"
								value={values.encoderTwoResetAfter}
								error={errors.encoderTwoResetAfter}
								isInvalid={Boolean(errors.encoderTwoResetAfter)}
								onChange={handleChange}
								min={0}
								max={500}
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
								id="encoderTwoAllowWrapAround"
								isInvalid={false}
								checked={Boolean(values.encoderTwoAllowWrapAround)}
								onChange={(e) => {
									handleCheckbox('encoderTwoAllowWrapAround');
									handleChange(e);
								}}
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
				onChange={(e) => {
					handleCheckbox('RotaryAddonEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Rotary;
