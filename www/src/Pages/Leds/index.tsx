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
	useFormikContext,
} from 'formik';
import * as yup from 'yup';

import useLedsPreview from '../../Hooks/useLedsPreview';
import useLedStore, {
	AnimationOptions,
	Light,
	MAX_ANIMATION_PROFILES,
	MAX_CASE_LIGHTS,
} from '../../Store/useLedStore';
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

import LightCoordsSection from './LightCoordsSection';
import ButtonLayoutPreview from './ButtonLayoutPreview';

const GPIO_PIN_LENGTH =
	boards[import.meta.env.VITE_GP2040_BOARD as keyof typeof boards].maxPin + 1;

const schema = yup.object({
	AnimationOptions: yup.object().shape({
		baseProfileIndex: yup.number().required('Selecting a profile is required'),
		brightness: yup
			.number()
			.min(0, 'Brightness must be at least 0')
			.max(100, 'Brightness cannot be more than 100')
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
				caseStaticColors: yup.array().of(yup.number()).required(),
				nonPressedSpecialColor: yup.number().required(),
				notPressedStaticColors: yup.array().of(yup.number()).required(),
				pressedSpecialColor: yup.number().required(),
				pressedStaticColors: yup.array().of(yup.number()),
			}),
		),
	}),
	Lights: yup
		.array()
		.of(
			yup.object({
				GPIOPinorCaseChainIndex: yup.number().required(),
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
	caseStaticColors: Array.from({ length: MAX_CASE_LIGHTS }, () => 1),
	nonPressedSpecialColor: 0,
	pressedSpecialColor: 0,
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

const PreviewLedChanges = ({ advancedMode }: { advancedMode: boolean }) => {
	const { values } = useFormikContext<{
		AnimationOptions: AnimationOptions;
		Lights: Light[];
	}>();
	const { activateLedsProfile } = useLedsPreview();

	useEffect(() => {
		if (advancedMode) return;
		activateLedsProfile(
			values.AnimationOptions.profiles[
				values.AnimationOptions.baseProfileIndex
			],
		);
	}, [values, advancedMode]);
	return null;
};

export default function Leds() {
	const { t } = useTranslation('');
	const { fetchLedOptions, saveAnimationOptions, saveLightOptions } =
		useLedStore();
	// const { activateLedsProfile, turnOffLeds } = useLedsPreview();

	const AnimationOptions = useLedStore((state) => state.AnimationOptions);
	const Lights = useLedStore((state) => state.Lights);
	const loading = useLedStore((state) => state.loading);
	const initialized = useLedStore((state) => state.initialized);
	// const [previewProfileIndex, setPreviewProfileIndex] = useState(
	// 	AnimationOptions.baseProfileIndex,
	// );
	const [activeTab, setActiveTab] = useState(
		`profile-${AnimationOptions.baseProfileIndex}`,
	);

	const [advancedMode, setAdvancedMode] = useState(false);

	const [saveMessage, setSaveMessage] = useState('');

	const onSuccess = async ({
		AnimationOptions,
		Lights,
	}: {
		AnimationOptions: AnimationOptions;
		Lights: Light[];
	}) => {
		try {
			await saveAnimationOptions(AnimationOptions);
			await saveLightOptions(Lights);
			setSaveMessage(t('Common:saved-success-message'));
		} catch (error) {
			console.error(error);
		}
	};

	useEffect(() => {
		fetchLedOptions();
	}, []);

	useEffect(() => {
		if (!initialized) return;
		if (Lights.length === 0) {
			setAdvancedMode(true);
		}
	}, [Lights, initialized]);

	if (loading || !initialized) {
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
			initialValues={{ AnimationOptions, Lights }}
			validateOnChange={true}
		>
			{({
				handleSubmit,
				handleChange,
				values,
				errors,
				setFieldValue,
				setValues,
			}) => (
				<Form onSubmit={handleSubmit}>
					<Section title="Led configuration">
						{/* <Row className="mb-3">
							<Col md={6} className="d-flex flex-column justify-content-end">
								<FormSelect
									label={'Preview configured profile'}
									className="form-select-sm"
									groupClassName="mb-3"
									value={previewProfileIndex}
									onChange={(e) => {
										setPreviewProfileIndex(parseInt(e.target.value));
									}}
								>
									{values.AnimationOptions.profiles.map((_, profileIndex) => (
										<option
											key={`profile-select-${profileIndex}`}
											value={profileIndex}
										>
											{t('Leds:profile-number', {
												profileNumber: profileIndex + 1,
											})}
										</option>
									))}
								</FormSelect>
								<Button
									variant="secondary"
									onClick={() => {
										activateLedsProfile(
											values.AnimationOptions.profiles[previewProfileIndex],
										);
									}}
								>
									Profile Test
								</Button>
							</Col>
							<Col md={6} className="d-flex flex-column justify-content-end">
								<p>Turns off all the lights</p>
								<Button
									variant="danger"
									onClick={() => {
										turnOffLeds();
									}}
								>
									Lights Off
								</Button>
							</Col>
						</Row>
						<hr /> */}
						<Row>
							<FormSelect
								label={t('Leds:profile-label')}
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
										{t('Leds:profile-number', {
											profileNumber: profileIndex + 1,
										})}
									</option>
								))}
							</FormSelect>

							<FormControl
								type="number"
								label={'Brightness'}
								name="AnimationOptions.brightness"
								className="form-control-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.AnimationOptions.brightness}
								error={errors.AnimationOptions?.brightness}
								isInvalid={Boolean(errors.AnimationOptions?.brightness)}
								onChange={handleChange}
								min={0}
								max={100}
							/>

							<FormControl
								type="number"
								label={t('Leds:idle-timout-label')}
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
						</Row>
						<FormGroup>
							<Form.Label>{t('Leds:custom-color-label')}</Form.Label>
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
									activeKey={activeTab}
									onSelect={(eventKey) => {
										if (!eventKey) return;
										if ('profile-add' === eventKey) {
											arrayHelpers.push(emptyAnimationProfile);
											setActiveTab(
												`profile-${values.AnimationOptions.profiles.length}`,
											);
										} else {
											setActiveTab(eventKey);
										}
									}}
									className="my-3 pb-0"
								>
									{values.AnimationOptions.profiles.map(
										(profile, profileIndex) => (
											<Tab
												key={`profile-${profileIndex}`}
												eventKey={`profile-${profileIndex}`}
												title={t('Leds:profile-number', {
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
																	{t('Leds:switch-enabled-description')}
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
														label={t('Leds:case-animation-label')}
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
																	{t(`Leds:animations.${key}`)}
																</option>
															),
														)}
													</FormSelect>
													<FormSelect
														label={t('Leds:pressed-animation-label')}
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
																	{t(`Leds:animations.${key}`)}
																</option>
															),
														)}
													</FormSelect>

													<FormSelect
														label={t('Leds:idle-animation-label')}
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
																	{t(`Leds:animations.${key}`)}
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
																	{t(`Leds:switch-case-light-pressed-label`)}
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
														label={t(`Leds:pressed-fade-out-time-label`)}
														name={`AnimationOptions.profiles.${profileIndex}.buttonPressFadeOutTimeInMs`}
														className="form-control-sm"
														groupClassName="col-sm-4 mb-3"
														value={profile.buttonPressFadeOutTimeInMs}
														onChange={handleChange}
													/>
													<FormControl
														type="number"
														label={t('Leds:pressed-hold-time-label')}
														name={`AnimationOptions.profiles.${profileIndex}.buttonPressHoldTimeInMs`}
														className="form-control-sm"
														groupClassName="col-sm-4 mb-3"
														value={profile.buttonPressHoldTimeInMs}
														onChange={handleChange}
													/>
												</Row>
												<Row>
													<FormControl
														type="color"
														label={t(`Leds:pressed-special-color-label`)}
														name={`AnimationOptions.profiles.${profileIndex}.pressedSpecialColor`}
														groupClassName="col-sm-4 mb-3"
														className="form-control-sm p-0 border-0 mb-3"
														defaultValue={rgbIntToHex(
															profile.pressedSpecialColor,
														)}
														onBlur={(e) =>
															setFieldValue(
																`AnimationOptions.profiles.${profileIndex}.pressedSpecialColor`,
																hexToInt((e.target as HTMLInputElement).value),
															)
														}
													/>
													<FormControl
														type="color"
														label={t(`Leds:idle-special-color-label`)}
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
																] as any
															)?.nonPressedSpecialColor
														}
														isInvalid={Boolean(
															(
																errors.AnimationOptions?.profiles?.[
																	profileIndex
																] as any
															)?.nonPressedSpecialColor,
														)}
														onBlur={(e) =>
															setFieldValue(
																`AnimationOptions.profiles.${profileIndex}.nonPressedSpecialColor`,
																hexToInt((e.target as HTMLInputElement).value),
															)
														}
													/>
												</Row>
												<hr />

												<Form.Check
													type="switch"
													label="Advanced mode"
													className="mb-3"
													checked={advancedMode}
													disabled={!values.Lights.length}
													onChange={(e) => setAdvancedMode(e.target.checked)}
												/>
												{advancedMode ? (
													<LightCoordsSection
														errors={errors}
														values={values}
														pressedStaticColors={profile.pressedStaticColors}
														notPressedStaticColors={
															profile.notPressedStaticColors
														}
														caseStaticColors={profile.caseStaticColors}
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
					<PreviewLedChanges advancedMode={advancedMode} />
				</Form>
			)}
		</Formik>
	);
}
