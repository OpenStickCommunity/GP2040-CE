import { memo, useEffect, useState } from 'react';
import {
	Alert,
	Button,
	Col,
	Form,
	FormGroup,
	FormCheck,
	OverlayTrigger,
	Row,
	Tab,
	Tabs,
	Tooltip,
} from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import {
	FieldArray,
	FieldArrayRenderProps,
	Formik,
	FormikErrors,
	useFormikContext,
} from 'formik';
import * as yup from 'yup';

import useLedsPreview from '../../Hooks/useLedsPreview';
import useLedStore, {
	AnimationOptions,
	AnimationProfile,
	LedOptions,
	Light,
	MAX_ANIMATION_PROFILES,
	MAX_NON_BUTTON_LIGHT_COLOR_INDEXES,
} from '../../Store/useLedStore';
import useLightsPresetsStore from '../../Store/useLightsPresetsStore';
import Section from '../../Components/Section';
import FormControl from '../../Components/FormControl';
import FormSelect from '../../Components/FormSelect';
import InfoCircle from '../../Icons/InfoCircle';
import { hexToInt, rgbIntToHex } from '../../Services/Utilities';
import {
	ANIMATION_NON_PRESSED_EFFECTS,
	ANIMATION_PRESSED_EFFECTS,
} from '../../Data/Animations';
import boards from '../../Data/Boards.json';
import { LED_FORMATS } from '../../Data/Leds';

import LightCoordsSection from './LightCoordsSection';
import ButtonLayoutPreview from './ButtonLayoutPreview';
import ImportLayout from './ImportLayout';

const GPIO_PIN_LENGTH =
	boards[import.meta.env.VITE_GP2040_BOARD as keyof typeof boards].maxPin + 1;

const schema = yup.object({
	ledOptions: yup.object().shape({
		dataPin: yup.number().required().checkUsedPins(),
		ledFormat: yup
			.number()
			.required()
			.positive()
			.integer()
			.min(0)
			.max(3)
			.label('LED Format'),
		brightnessMaximum: yup
			.number()
			.required()
			.positive()
			.integer()
			.min(0)
			.max(100)
			.label('Max Brightness'),
		turnOffWhenSuspended: yup.number().label('Turn Off When Suspended'),
		pledType: yup.number().required().label('Player LED Type'),
		pledPin1: yup
			.number()
			.label('PLED 1')
			.validatePinWhenEqualTo('pledPin1', 'pledType', 0),
		pledPin2: yup
			.number()
			.label('PLED 2')
			.validatePinWhenEqualTo('pledPin2', 'pledType', 0),
		pledPin3: yup
			.number()
			.label('PLED 3')
			.validatePinWhenEqualTo('pledPin3', 'pledType', 0),
		pledPin4: yup
			.number()
			.label('PLED 4')
			.validatePinWhenEqualTo('pledPin4', 'pledType', 0),
	}),
	AnimationOptions: yup.object().shape({
		baseProfileIndex: yup.number().required('Selecting a profile is required'),
		brightness: yup
			.number()
			.min(0, 'Brightness must be at least 0')
			.max(10, 'Brightness cannot be more than 10')
			.required('Brightness is required'),
		idletimeout: yup
			.number()
			.min(0, 'Idle timout value must be at least 0')
			.max(300, 'Idle timout value cannot be more than 300 seconds')
			.required('Idle timout value is required'),
		profiles: yup.array().of(
			yup.object().shape({
				bEnabled: yup.number().required(),
				bUseCaseLightsInPressedAnimations: yup.number().required(),
				baseCaseEffect: yup.number().required(),
				baseNonPressedEffect: yup.number().required(),
				basePressedEffect: yup.number().required(),
				buttonPressFadeOutTimeInMs: yup.number().required(),
				buttonPressHoldTimeInMs: yup.number().required(),
				nonButtonStaticColors: yup.array().of(yup.number()).required(),
				nonPressedSpecialColor: yup.number().required(),
				notPressedStaticColors: yup.array().of(yup.number()).required(),
				pressedSpecialColor: yup.number().required(),
				pressedStaticColors: yup.array().of(yup.number()),
				caseSpecialColor: yup.number().required(),
				bNonPressedSpecialColorIsRainbow: yup.number().required(),
				bPressedSpecialColorIsRainbow: yup.number().required(),
				bCaseSpecialColorIsRainbow: yup.number().required(),
				nonPressedContextParam: yup.number().required(),
				pressedContextParam: yup.number().required(),
				caseContextParam: yup.number().required(),
			}),
		),
	}),
	Lights: yup
		.array()
		.of(
			yup.object({
				GPIOPinOrNonButtonIndex: yup.number().required(),
				firstLedIndex: yup
					.number()
					.min(0, 'First LED index must be at least 0')
					.required('First LED index is required'),
				lightType: yup.number().required(),
				numLedsOnLight: yup
					.number()
					.min(1, 'Number of LEDs on Light must be at least 1')
					.required('Number of LEDs on Light is required'),
				xCoord: yup.number().required(),
				yCoord: yup.number().required(),
			}),
		)
		// TODO: lookup how to make this better with yup
		.test('no-duplicate-coords', 'Overlapping light', function (lights) {
			if (!lights) return true;
			const coordMap = new Map();
			const overlaps = [];

			for (let i = 0; i < lights.length; i++) {
				const key = `${lights[i].xCoord},${lights[i].yCoord}`;
				if (coordMap.has(key)) {
					overlaps.push(coordMap.get(key), i);
				} else {
					coordMap.set(key, i);
				}
			}
			if (overlaps.length) {
				const errors = Array.from(new Set(overlaps)).reduce(
					(acc, i) => [
						...acc,
						new yup.ValidationError(
							'Overlapping light',
							null,
							`Lights.${i}.xCoord`,
						),
						new yup.ValidationError(
							'Overlapping light',
							null,
							`Lights.${i}.yCoord`,
						),
					],
					[] as yup.ValidationError[],
				);
				throw new yup.ValidationError(errors);
			}
			return true;
		}),
});

const emptyAnimationProfile = {
	bEnabled: 1,
	baseCaseEffect: 0,
	baseNonPressedEffect: 0,
	basePressedEffect: 0,
	buttonPressFadeOutTimeInMs: 0,
	buttonPressHoldTimeInMs: 0,
	bUseCaseLightsInPressedAnimations: 0,
	nonButtonStaticColors: Array.from(
		{ length: MAX_NON_BUTTON_LIGHT_COLOR_INDEXES },
		() => 1,
	),
	nonPressedSpecialColor: 0,
	pressedSpecialColor: 0,
	caseSpecialColor: 0,
	bNonPressedSpecialColorIsRainbow: 0,
	bPressedSpecialColorIsRainbow: 0,
	bCaseSpecialColorIsRainbow: 0,
	nonPressedContextParam: 0,
	pressedContextParam: 0,
	caseContextParam: 0,
	notPressedStaticColors: Array.from({ length: GPIO_PIN_LENGTH }, () => 0),
	pressedStaticColors: Array.from({ length: GPIO_PIN_LENGTH }, () => 1),
};

const ColorPickerList = memo(function ColorPickerList({
	colors,
	replace,
}: {
	colors: number[];
	replace: FieldArrayRenderProps['replace'];
}) {
	return (
		<div className="d-flex col gap-2 flex-wrap">
			{colors.map((color, index) => (
				<div
					key={`customColors-${index}}-${color}`}
					className="d-flex gap-1 flex-wrap"
				>
					<FormControl
						type="color"
						name={`customColors.${index}`}
						className="form-control-sm p-0 border-0"
						defaultValue={rgbIntToHex(color)}
						onBlur={(e) =>
							replace(index, hexToInt((e.target as HTMLInputElement).value))
						}
					/>
				</div>
			))}
		</div>
	);
});

const PreviewLedChanges = ({
	selectedProfile,
}: {
	selectedProfile: number;
}) => {
	const { values } = useFormikContext<{
		AnimationOptions: AnimationOptions;
		Lights: Light[];
	}>();
	const { activateLedsProfile } = useLedsPreview();

	useEffect(() => {
		const profile = values.AnimationOptions.profiles[selectedProfile];
		if (!profile) return;
		activateLedsProfile(profile);
	}, [values, selectedProfile]);

	return null;
};

export default function LedConfigPage() {
	const { t } = useTranslation('');
	const {
		fetchLedOptions,
		saveAnimationOptions,
		saveLightOptions,
		saveLedOptions,
	} = useLedStore();

	const { fetchPresets } = useLightsPresetsStore();
	const ledOptions = useLedStore((state) => state.ledOptions);
	const AnimationOptions = useLedStore((state) => state.AnimationOptions);
	const Lights = useLedStore((state) => state.Lights);
	const loading = useLedStore((state) => state.loading);
	const initialized = useLedStore((state) => state.initialized);
	const [selectedProfile, setSelectedProfile] = useState(
		AnimationOptions.baseProfileIndex,
	);
	const [layouteMode, setLayouteMode] = useState(false);
	const [saveMessage, setSaveMessage] = useState('');

	const onSuccess = async ({
		ledOptions,
		AnimationOptions,
		Lights,
	}: {
		ledOptions: LedOptions;
		AnimationOptions: AnimationOptions;
		Lights: Light[];
	}) => {
		try {
			await saveAnimationOptions(AnimationOptions);
			await saveLightOptions(Lights);
			await saveLedOptions(ledOptions);
			setSaveMessage(t('Common:saved-success-message'));
		} catch (error) {
			console.error(error);
		}
	};

	useEffect(() => {
		fetchLedOptions();
		fetchPresets();
	}, []);

	useEffect(() => {
		if (!initialized) return;
		if (Lights.length === 0) {
			setLayouteMode(true);
		}
	}, [Lights, initialized]);

	if (loading && !initialized) {
		return (
			<div className="d-flex justify-content-center">
				<span className="spinner-border"></span>
			</div>
		);
	}

	return (
		<Formik
			validationSchema={schema}
			onSubmit={onSuccess}
			initialValues={{ AnimationOptions, Lights, ledOptions }}
			validateOnChange={true}
		>
			{({
				handleSubmit,
				handleChange,
				// handleBlur,
				values,
				errors,
				setFieldValue,
				setValues,
			}) => (
				<Form onSubmit={handleSubmit}>
					<Section title={t('LedConfigPage:rgb.header-text')}>
						<Row>
							<FormControl
								type="number"
								label={t('LedConfigPage:rgb.data-pin-label')}
								name="ledOptions.dataPin"
								className="form-control-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.ledOptions.dataPin}
								error={errors.ledOptions?.dataPin}
								isInvalid={Boolean(errors.ledOptions?.dataPin)}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormSelect
								label={t('LedConfigPage:rgb.led-format-label')}
								name="ledOptions.ledFormat"
								className="form-select-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.ledOptions.ledFormat}
								error={errors.ledOptions?.ledFormat}
								isInvalid={Boolean(errors.ledOptions?.ledFormat)}
								onChange={(e) =>
									setFieldValue(
										'ledOptions.ledFormat',
										parseInt(e.target.value),
									)
								}
							>
								{LED_FORMATS.map((o, i) => (
									<option key={`ledFormat-option-${i}`} value={o.value}>
										{o.label}
									</option>
								))}
							</FormSelect>
							<div className="form-control-sm col-sm-4 mb-3">
								<Form.Label>{`${t('LedConfigPage:rgb.led-brightness-maximum-label')}: ${values.ledOptions.brightnessMaximum}%`}</Form.Label>
								<Form.Range
									name="ledOptions.brightnessMaximum"
									id={`ledOptions.brightnessMaximum`}
									min={0}
									max={100}
									step={1}
									value={values.ledOptions.brightnessMaximum}
									onChange={handleChange}
								/>
							</div>
						</Row>
						<Row>
							<div className="col-sm-4 mb-3">
								<FormCheck
									id="turnOffWhenSuspended"
									label={t('LedConfigPage:rgb.turn-off-when-suspended')}
									type="switch"
									isInvalid={false}
									checked={Boolean(values.ledOptions.turnOffWhenSuspended)}
									onChange={(e) => {
										setFieldValue(
											'ledOptions.turnOffWhenSuspended',
											e.target.checked ? 1 : 0,
										);
									}}
								/>
							</div>
						</Row>
						<Row className="mb-3">
							<FormSelect
								label={t('LedConfigPage:player.pled-type-label')}
								name="ledOptions.pledType"
								className="form-select-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.ledOptions.pledType}
								error={errors.ledOptions?.pledType}
								isInvalid={Boolean(errors.ledOptions?.pledType)}
								onChange={(e) =>
									setFieldValue('ledOptions.pledType', parseInt(e.target.value))
								}
							>
								<option value="-1">
									{t('LedConfigPage:player.pled-type-off')}
								</option>
								<option value="0">
									{t('LedConfigPage:player.pled-type-pwm')}
								</option>
								<option value="1">
									{t('LedConfigPage:player.pled-type-rgb')}
								</option>
							</FormSelect>
							<FormControl
								type="number"
								name="ledOptions.pledPin1"
								hidden={values.ledOptions.pledType !== 0}
								label={t('LedConfigPage:player.pled-pin-label', { pin: 1 })}
								className="form-control-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.ledOptions.pledPin1}
								error={errors.ledOptions?.pledPin1}
								isInvalid={Boolean(errors.ledOptions?.pledPin1)}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormControl
								type="number"
								name="ledOptions.pledPin2"
								hidden={values.ledOptions.pledType !== 0}
								label={t('LedConfigPage:player.pled-pin-label', { pin: 2 })}
								className="form-control-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.ledOptions.pledPin2}
								error={errors.ledOptions?.pledPin2}
								isInvalid={Boolean(errors.ledOptions?.pledPin2)}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormControl
								type="number"
								name="ledOptions.pledPin3"
								hidden={values.ledOptions.pledType !== 0}
								label={t('LedConfigPage:player.pled-pin-label', { pin: 3 })}
								className="form-control-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.ledOptions.pledPin3}
								error={errors.ledOptions?.pledPin3}
								isInvalid={Boolean(errors.ledOptions?.pledPin3)}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormControl
								type="number"
								name="ledOptions.pledPin4"
								hidden={values.ledOptions.pledType !== 0}
								label={t('LedConfigPage:player.pled-pin-label', { pin: 4 })}
								className="form-control-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.ledOptions.pledPin4}
								error={errors.ledOptions?.pledPin4}
								isInvalid={Boolean(errors.ledOptions?.pledPin4)}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
						</Row>
						{values.ledOptions.pledType === 0 && (
							<Alert variant="info">
								{t('LedConfigPage:player.pwm-sub-header-text')}
							</Alert>
						)}
						{values.ledOptions.pledType === 1 && (
							<Alert variant="info">
								{t('LedConfigPage:player.rgb-sub-header-text')}
							</Alert>
						)}
					</Section>
					<Section title="Custom LED Theme">
						<Row>
							<FormSelect
								label={t('LedConfigPage:theme.profile-label')}
								name="AnimationOptions.baseProfileIndex"
								className="form-select-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.AnimationOptions.baseProfileIndex}
								error={errors.AnimationOptions?.baseProfileIndex}
								isInvalid={Boolean(errors.AnimationOptions?.baseProfileIndex)}
								onChange={(e) =>
									setFieldValue(
										'AnimationOptions.baseProfileIndex',
										parseInt(e.target.value),
									)
								}
							>
								{values.AnimationOptions.profiles.map((_, profileIndex) => (
									<option
										key={`profile-select-${profileIndex}`}
										value={profileIndex}
									>
										{t('LedConfigPage:theme.profile-number', {
											profileNumber: profileIndex + 1,
										})}
									</option>
								))}
							</FormSelect>
							<FormControl
								type="number"
								label={t('LedConfigPage:theme.idle-timout-label')}
								name="AnimationOptions.idletimeout"
								className="form-control-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.AnimationOptions.idletimeout}
								error={errors.AnimationOptions?.idletimeout}
								isInvalid={Boolean(errors.AnimationOptions?.idletimeout)}
								onChange={handleChange}
								min={0}
								max={300}
							/>
							<div className="form-control-sm col-sm-4 mb-3">
								<Form.Label>{`${t('LedConfigPage:theme.current-led-brightness-label', { percent: values.AnimationOptions.brightness * 10 })}`}</Form.Label>
								<Form.Range
									name="AnimationOptions.brightness"
									id={`AnimationOptions.brightness`}
									min={0}
									max={10}
									step={1}
									value={values.AnimationOptions.brightness}
									onChange={handleChange}
								/>
							</div>
						</Row>
						<FormGroup>
							<Form.Label>
								{t('LedConfigPage:theme.custom-color-label')}
							</Form.Label>
							<FieldArray
								name="AnimationOptions.customColors"
								render={(arrayHelpers) => (
									<ColorPickerList
										colors={values.AnimationOptions.customColors}
										replace={arrayHelpers.replace}
									/>
								)}
							/>
						</FormGroup>

						<FieldArray
							name="AnimationOptions.profiles"
							render={(arrayHelpers) => (
								<Tabs
									activeKey={`profile-${selectedProfile}`}
									onSelect={(eventKey) => {
										if (!eventKey) return;
										if ('profile-add' === eventKey) {
											arrayHelpers.push(emptyAnimationProfile);
											setSelectedProfile(
												values.AnimationOptions.profiles.length,
											);
										} else {
											setSelectedProfile(
												parseInt(eventKey.replace('profile-', '')),
											);
										}
									}}
									className="my-3 pb-0"
								>
									{values.AnimationOptions.profiles.map(
										(profile, profileIndex) => (
											<Tab
												key={`profile-${profileIndex}`}
												eventKey={`profile-${profileIndex}`}
												title={t('LedConfigPage:theme.profile-number', {
													profileNumber: profileIndex + 1,
												})}
											>
												<FormCheck
													size={3}
													name={`AnimationOptions.profiles.${profileIndex}.bEnabled`}
													label={
														<OverlayTrigger
															overlay={
																<Tooltip>
																	{t(
																		'LedConfigPage:theme.switch-enabled-description',
																	)}
																</Tooltip>
															}
														>
															<div className="d-flex gap-1">
																<label>{t('Common:switch-enabled')}</label>
																<InfoCircle />
															</div>
														</OverlayTrigger>
													}
													type="switch"
													reverse
													checked={Boolean(profile.bEnabled)}
													onChange={() =>
														setFieldValue(
															`AnimationOptions.profiles.${profileIndex}.bEnabled`,
															Number(!profile.bEnabled),
														)
													}
												/>
												<Row>
													<FormSelect
														label={t(
															'LedConfigPage:theme.idle-animation-label',
														)}
														name={`AnimationOptions.profiles.${profileIndex}.baseNonPressedEffect`}
														className="form-select-sm"
														groupClassName="col-sm-4 mb-3"
														value={Number(profile.baseNonPressedEffect)}
														onChange={(e) =>
															setFieldValue(
																`AnimationOptions.profiles.${profileIndex}.baseNonPressedEffect`,
																parseInt(e.target.value),
															)
														}
													>
														{Object.entries(ANIMATION_NON_PRESSED_EFFECTS).map(
															([key, value]) => (
																<option
																	key={`baseNonPressedEffect-${key}`}
																	value={value}
																>
																	{t(`LedConfigPage:animations.${key}`)}
																</option>
															),
														)}
													</FormSelect>

													<FormSelect
														label={t(
															'LedConfigPage:theme.pressed-animation-label',
														)}
														name={`AnimationOptions.profiles.${profileIndex}.basePressedEffect`}
														className="form-select-sm"
														groupClassName="col-sm-4 mb-3"
														value={Number(profile.basePressedEffect)}
														onChange={(e) =>
															setFieldValue(
																`AnimationOptions.profiles.${profileIndex}.basePressedEffect`,
																parseInt(e.target.value),
															)
														}
													>
														{Object.entries(ANIMATION_PRESSED_EFFECTS).map(
															([key, value]) => (
																<option
																	key={`basePressedEffect-${key}`}
																	value={value}
																>
																	{t(`LedConfigPage:animations.${key}`)}
																</option>
															),
														)}
													</FormSelect>

													<FormSelect
														label={t(
															'LedConfigPage:theme.case-animation-label',
														)}
														name={`AnimationOptions.profiles.${profileIndex}.baseCaseEffect`}
														className="form-select-sm"
														groupClassName="col-sm-4 mb-3"
														value={Number(profile.baseCaseEffect)}
														onChange={(e) =>
															setFieldValue(
																`AnimationOptions.profiles.${profileIndex}.baseCaseEffect`,
																parseInt(e.target.value),
															)
														}
													>
														{Object.entries(ANIMATION_NON_PRESSED_EFFECTS).map(
															([key, value]) => (
																<option
																	key={`baseCaseEffect-${key}`}
																	value={value}
																>
																	{t(`LedConfigPage:animations.${key}`)}
																</option>
															),
														)}
													</FormSelect>
												</Row>
												<Row>
													<div className="d-flex align-items-center col-sm-4 mb-3">
														<FormCheck
															type="switch"
															name={`AnimationOptions.profiles.${profileIndex}.bUseCaseLightsInPressedAnimations`}
															label={
																<label>
																	{t(
																		`LedConfigPage:theme.switch-case-light-pressed-label`,
																	)}
																</label>
															}
															checked={Boolean(
																profile.bUseCaseLightsInPressedAnimations,
															)}
															onChange={() =>
																setFieldValue(
																	`AnimationOptions.profiles.${profileIndex}.bUseCaseLightsInPressedAnimations`,
																	Number(
																		!profile.bUseCaseLightsInPressedAnimations,
																	),
																)
															}
														/>
													</div>

													<FormControl
														type="number"
														label={t(
															`LedConfigPage:theme.pressed-fade-out-time-label`,
														)}
														name={`AnimationOptions.profiles.${profileIndex}.buttonPressFadeOutTimeInMs`}
														className="form-control-sm"
														groupClassName="col-sm-4 mb-3"
														value={profile.buttonPressFadeOutTimeInMs}
														onChange={handleChange}
													/>
													<FormControl
														type="number"
														label={t(
															'LedConfigPage:theme.pressed-hold-time-label',
														)}
														name={`AnimationOptions.profiles.${profileIndex}.buttonPressHoldTimeInMs`}
														className="form-control-sm"
														groupClassName="col-sm-4 mb-3"
														value={profile.buttonPressHoldTimeInMs}
														onChange={handleChange}
													/>
												</Row>
												<Row>
													<div className="d-flex align-items-center col-sm-4 mb-3">
														<FormCheck
															type="switch"
															name={`AnimationOptions.profiles.${profileIndex}.bNonPressedSpecialColorIsRainbow`}
															label={
																<label>
																	{t(
																		`LedConfigPage:theme.switch-specialnonpressed-rainbow-label`,
																	)}
																</label>
															}
															checked={Boolean(
																profile.bNonPressedSpecialColorIsRainbow,
															)}
															onChange={() =>
																setFieldValue(
																	`AnimationOptions.profiles.${profileIndex}.bNonPressedSpecialColorIsRainbow`,
																	Number(
																		!profile.bNonPressedSpecialColorIsRainbow,
																	),
																)
															}
														/>
													</div>
													{!profile.bNonPressedSpecialColorIsRainbow && (
														<FormControl
															type="color"
															label={t(
																`LedConfigPage:theme.idle-special-color-label`,
															)}
															name={`AnimationOptions.profiles.${profileIndex}.nonPressedSpecialColor`}
															groupClassName="col-sm-4 mb-3"
															className="form-control-sm p-0 border-0 mb-3"
															defaultValue={rgbIntToHex(
																profile.nonPressedSpecialColor,
															)}
															error={
																(
																	errors.AnimationOptions?.profiles?.[
																		profileIndex
																	] as FormikErrors<AnimationProfile>
																)?.nonPressedSpecialColor
															}
															isInvalid={Boolean(
																(
																	errors.AnimationOptions?.profiles?.[
																		profileIndex
																	] as FormikErrors<AnimationProfile>
																)?.nonPressedSpecialColor,
															)}
															onBlur={(e) =>
																setFieldValue(
																	`AnimationOptions.profiles.${profileIndex}.nonPressedSpecialColor`,
																	hexToInt(
																		(e.target as HTMLInputElement).value,
																	),
																)
															}
														/>
													)}
												</Row>
												<Row>
													<div className="d-flex align-items-center col-sm-4 mb-3">
														<FormCheck
															type="switch"
															name={`AnimationOptions.profiles.${profileIndex}.bPressedSpecialColorIsRainbow`}
															label={
																<label>
																	{t(
																		`LedConfigPage:theme.switch-specialpressed-rainbow-label`,
																	)}
																</label>
															}
															checked={Boolean(
																profile.bPressedSpecialColorIsRainbow,
															)}
															onChange={() =>
																setFieldValue(
																	`AnimationOptions.profiles.${profileIndex}.bPressedSpecialColorIsRainbow`,
																	Number(
																		!profile.bPressedSpecialColorIsRainbow,
																	),
																)
															}
														/>
													</div>
													{!profile.bPressedSpecialColorIsRainbow && (
														<FormControl
															type="color"
															label={t(
																`LedConfigPage:theme.pressed-special-color-label`,
															)}
															name={`AnimationOptions.profiles.${profileIndex}.pressedSpecialColor`}
															groupClassName="col-sm-4 mb-3"
															className="form-control-sm p-0 border-0 mb-3"
															defaultValue={rgbIntToHex(
																profile.pressedSpecialColor,
															)}
															error={
																(
																	errors.AnimationOptions?.profiles?.[
																		profileIndex
																	] as FormikErrors<AnimationProfile>
																)?.pressedSpecialColor
															}
															isInvalid={Boolean(
																(
																	errors.AnimationOptions?.profiles?.[
																		profileIndex
																	] as FormikErrors<AnimationProfile>
																)?.pressedSpecialColor,
															)}
															onBlur={(e) =>
																setFieldValue(
																	`AnimationOptions.profiles.${profileIndex}.pressedSpecialColor`,
																	hexToInt(
																		(e.target as HTMLInputElement).value,
																	),
																)
															}
														/>
													)}
												</Row>
												<Row>
													<div className="d-flex align-items-center col-sm-4 mb-3">
														<FormCheck
															type="switch"
															name={`AnimationOptions.profiles.${profileIndex}.bCaseSpecialColorIsRainbow`}
															label={
																<label>
																	{t(
																		`LedConfigPage:theme.switch-specialcase-rainbow-label`,
																	)}
																</label>
															}
															checked={Boolean(
																profile.bCaseSpecialColorIsRainbow,
															)}
															onChange={() =>
																setFieldValue(
																	`AnimationOptions.profiles.${profileIndex}.bCaseSpecialColorIsRainbow`,
																	Number(!profile.bCaseSpecialColorIsRainbow),
																)
															}
														/>
													</div>
													{!profile.bCaseSpecialColorIsRainbow && (
														<FormControl
															type="color"
															label={t(
																`LedConfigPage:theme.case-special-color-label`,
															)}
															name={`AnimationOptions.profiles.${profileIndex}.caseSpecialColor`}
															groupClassName="col-sm-4 mb-3"
															className="form-control-sm p-0 border-0 mb-3"
															defaultValue={rgbIntToHex(
																profile.caseSpecialColor,
															)}
															onBlur={(e) =>
																setFieldValue(
																	`AnimationOptions.profiles.${profileIndex}.caseSpecialColor`,
																	hexToInt(
																		(e.target as HTMLInputElement).value,
																	),
																)
															}
														/>
													)}
												</Row>
												<hr />

												<Row>
													<Col md={6}>
														<p>
															{t(`LedConfigPage:theme.layout-mode-description`)}
														</p>
														<Form.Check
															type="switch"
															label={t(`LedConfigPage:theme.layout-mode-label`)}
															className="mb-3"
															checked={layouteMode}
															disabled={!values.Lights.length}
															onChange={(e) => setLayouteMode(e.target.checked)}
														/>
													</Col>
													<Col md={6}>
														<ImportLayout
															setFieldValue={setFieldValue}
															lights={values.Lights}
														/>
													</Col>
												</Row>
												<hr />
												{layouteMode ? (
													<LightCoordsSection
														errors={errors}
														values={values}
														pressedStaticColors={profile.pressedStaticColors}
														notPressedStaticColors={
															profile.notPressedStaticColors
														}
														nonButtonStaticColors={
															profile.nonButtonStaticColors
														}
														profileIndex={profileIndex}
														handleChange={handleChange}
														setFieldValue={setFieldValue}
														setValues={setValues}
													/>
												) : (
													<ButtonLayoutPreview
														pressedStaticColors={profile.pressedStaticColors}
														notPressedStaticColors={
															profile.notPressedStaticColors
														}
														nonButtonStaticColors={
															profile.nonButtonStaticColors
														}
														profileIndex={profileIndex}
														customColors={values.AnimationOptions.customColors}
														setFieldValue={setFieldValue}
														Lights={values.Lights}
													/>
												)}
											</Tab>
										),
									)}

									{values.AnimationOptions.profiles.length !==
										MAX_ANIMATION_PROFILES && (
										<Tab
											key={`add-profile`}
											eventKey={`profile-add`}
											title={`+ Add Profile`}
										/>
									)}
								</Tabs>
							)}
						/>
					</Section>
					<Button className="mb-3" type="submit">
						{t('Common:button-save-label')}
					</Button>
					{saveMessage && <Alert variant="info">{saveMessage}</Alert>}
					<PreviewLedChanges selectedProfile={selectedProfile} />
				</Form>
			)}
		</Formik>
	);
}
