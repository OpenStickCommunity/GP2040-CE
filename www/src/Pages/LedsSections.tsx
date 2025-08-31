import { memo, useState } from 'react';
import { useTranslation } from 'react-i18next';
import {
	Form,
	Alert,
	Button,
	FormCheck,
	FormGroup,
	OverlayTrigger,
	Row,
	Tab,
	Tabs,
	Tooltip,
	Col,
} from 'react-bootstrap';
import { FieldArray, FieldArrayRenderProps, Formik } from 'formik';
import * as yup from 'yup';
import { StylesConfig } from 'react-select';
import { parseInt } from 'lodash';

import Section from '../Components/Section';
import CustomSelect from '../Components/CustomSelect';

import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import InfoCircle from '../Icons/InfoCircle';
import {
	ANIMATION_NON_PRESSED_EFFECTS,
	ANIMATION_PRESSED_EFFECTS,
} from '../Data/Animations';
import useLedStore, {
	AnimationOptions,
	MAX_ANIMATION_PROFILES,
	MAX_CASE_LIGHTS,
	MAX_CUSTOM_COLORS,
} from '../Store/useLedStore';
import boards from '../Data/Boards.json';
import useLedsPreview from '../Hooks/useLedsPreview';
import './LedsSections.scss';
import LEDColors from '../Data/LEDColors';

const GPIO_PIN_LENGTH = boards[import.meta.env.VITE_GP2040_BOARD].maxPin + 1;

const schema = yup.object().shape({
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
});

const emptyAnimationProfile = {
	bEnabled: 1,
	baseCaseEffect: 0,
	baseNonPressedEffect: 0,
	basePressedEffect: 0,
	buttonPressFadeOutTimeInMs: 0,
	buttonPressHoldTimeInMs: 0,
	caseStaticColors: Array.from({ length: MAX_CASE_LIGHTS }, () => 1),
	nonPressedSpecialColor: 0,
	pressedSpecialColor: 0,
	notPressedStaticColors: Array.from({ length: GPIO_PIN_LENGTH }, () => 0),
	pressedStaticColors: Array.from({ length: GPIO_PIN_LENGTH }, () => 1),
};

const convertToHex = (color: number) =>
	`#${color.toString(16).padStart(6, '0')}`;

const convertToDecimal = (hex: string) => parseInt(hex.replace('#', ''), 16);

const colorDot = (color = 'transparent') => ({
	alignItems: 'center',
	display: 'flex',

	':before': {
		backgroundColor: color,
		borderRadius: 15,
		content: '" "',
		display: 'block',
		flexShrink: 0,
		marginRight: 8,
		height: 15,
		width: 15,
	},
});

const colorStyles: StylesConfig<(typeof LEDColors)[number]> = {
	control: (styles) => ({ ...styles, backgroundColor: 'white' }),
	option: (styles, { data }) => ({ ...styles, ...colorDot(data.color) }),
	input: (styles) => ({ ...styles }),
	placeholder: (styles) => ({ ...styles, ...colorDot('#ccc') }),
	singleValue: (styles, { data }) => ({
		...styles,
		...colorDot(data.color),
	}),
};

const ColorSelectorList = memo(function ColorSelectorList({
	length,
	colors,
	customColors,
	LabelComponent,
	containerClassName = '',
	isClearable = false,
	replace,
}: {
	length: number;
	colors: number[];
	customColors: number[];
	LabelComponent: ({ index }: { index: number }) => JSX.Element;
	containerClassName?: string;
	isClearable?: boolean;
	replace: FieldArrayRenderProps['replace'];
}) {
	const customColorOptions = customColors.map((color, index) => ({
		value: LEDColors.length + index,
		label: `Custom ${index + 1}`,
		color: convertToHex(color),
	}));
	const colorOptions = [...LEDColors, ...customColorOptions];

	return (
		<>
			<div className={containerClassName}>
				{Array.from({ length }).map((_, index) => (
					<div
						key={`select-${index}-${colorOptions.find(({ value }) => value === colors[index])?.label}`}
						className="d-flex col align-items-center"
					>
						{<LabelComponent index={index} />}
						<CustomSelect
							isClearable={isClearable}
							options={colorOptions}
							styles={colorStyles}
							isMulti={false}
							onChange={(selected) => {
								replace(index, selected?.value || 0);
							}}
							value={
								colors[index]
									? colorOptions.find(({ value }) => value === colors[index]) ||
										colorOptions[0]
									: colorOptions[0]
							}
						/>
					</div>
				))}
			</div>
			<div className="col-md-4 mt-3">
				<CustomSelect
					placeholder="Set all colors"
					options={colorOptions}
					styles={colorStyles}
					value={null}
					isMulti={false}
					isClearable
					onChange={(selected) => {
						if (selected?.value !== undefined) {
							Array.from({ length }).forEach((_, i) => {
								replace(i, selected.value);
							});
						}
					}}
				/>
			</div>
		</>
	);
});

const ColorPickerList = memo(function ColorPickerList({
	colors,
	maxLength,
	replace,
	remove,
	push,
}: {
	colors: number[];
	maxLength: number;
	replace: FieldArrayRenderProps['replace'];
	remove: FieldArrayRenderProps['remove'];
	push: FieldArrayRenderProps['push'];
}) {
	const { t } = useTranslation('');

	return (
		<div className="d-flex col gap-2 flex-wrap">
			{colors.map((color, index) => (
				<div
					key={`customColors-${index}-${color}`}
					className="d-flex gap-1 flex-wrap"
				>
					<FormControl
						type="color"
						name={`customColors.${index}`}
						className="form-control-sm p-0 border-0"
						defaultValue={convertToHex(color)}
						onBlur={(e) =>
							replace(
								index,
								convertToDecimal((e.target as HTMLInputElement).value),
							)
						}
					/>
					<Button
						size="sm"
						onClick={() => {
							remove(index);
						}}
					>
						{'✕'}
					</Button>
				</div>
			))}
			{colors.length !== maxLength && (
				<>
					<div className="vr"></div>
					<Button size="sm" onClick={() => push(0)}>
						{t('Leds:add-color')}
					</Button>
				</>
			)}
		</div>
	);
});

export default function LedsSections() {
	const { t } = useTranslation('');
	const { AnimationOptions, saveAnimationOptions } = useLedStore();
	const { activateLedsProfile, turnOffLeds } = useLedsPreview();

	const [key, setKey] = useState(
		`profile-${AnimationOptions.baseProfileIndex}`,
	);
	const [previewProfileIndex, setPreviewProfileIndex] = useState(
		AnimationOptions.baseProfileIndex,
	);
	const [saveMessage, setSaveMessage] = useState('');

	const onSuccess = async (values: AnimationOptions) => {
		try {
			await saveAnimationOptions(values);
			setSaveMessage(t('Common:saved-success-message'));
		} catch (error) {
			console.error(error);
		}
	};

	return (
		<Formik
			validationSchema={schema}
			onSubmit={onSuccess}
			initialValues={AnimationOptions}
			validateOnChange={false}
		>
			{({ handleSubmit, handleChange, values, errors, setFieldValue }) => (
				<div>
					<Section title="Led configuration">
						<Form noValidate onSubmit={handleSubmit}>
							<Row className="mb-3">
								<Col md={6} className="d-flex flex-column justify-content-end">
									<FormSelect
										label={'Preview configured profile'}
										className="form-select-sm"
										groupClassName="mb-3"
										value={previewProfileIndex}
										onChange={(e) => {
											const profileIndex = parseInt(e.target.value);
											setPreviewProfileIndex(profileIndex);
										}}
									>
										{values.profiles.map((_, profileIndex) => (
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
											activateLedsProfile(values.profiles[previewProfileIndex]);
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
							<hr />
							<Row>
								<FormSelect
									label={t('Leds:profile-label')}
									name="baseProfileIndex"
									className="form-select-sm"
									groupClassName="col-sm-4 mb-3"
									value={values.baseProfileIndex}
									error={errors.baseProfileIndex}
									isInvalid={Boolean(errors.baseProfileIndex)}
									onChange={(e) =>
										setFieldValue('baseProfileIndex', parseInt(e.target.value))
									}
								>
									{values.profiles.map((_, profileIndex) => (
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
									name="brightness"
									className="form-control-sm"
									groupClassName="col-sm-4 mb-3"
									value={values.brightness}
									error={errors.brightness}
									isInvalid={Boolean(errors.brightness)}
									onChange={handleChange}
									min={0}
									max={100}
								/>

								<FormControl
									type="number"
									label={t('Leds:idle-timout-label')}
									name="idletimeout"
									className="form-control-sm"
									groupClassName="col-sm-4 mb-3"
									value={values.idletimeout}
									error={errors.idletimeout}
									isInvalid={Boolean(errors.idletimeout)}
									onChange={handleChange}
									min={0}
									max={300}
								/>
							</Row>
							<FormGroup>
								<Form.Label>{t('Leds:custom-color-label')}</Form.Label>
								<FieldArray
									name="customColors"
									render={(arrayHelpers) => (
										<ColorPickerList
											colors={values.customColors}
											maxLength={MAX_CUSTOM_COLORS}
											replace={arrayHelpers.replace}
											remove={arrayHelpers.remove}
											push={arrayHelpers.push}
										/>
									)}
								/>
							</FormGroup>

							<FieldArray
								name="profiles"
								render={(arrayHelpers) => (
									<Tabs
										activeKey={key}
										onSelect={(eventKey) => {
											if (!eventKey) return;
											if ('profile-add' === eventKey) {
												arrayHelpers.push(emptyAnimationProfile);
												setKey(`profile-${values.profiles.length}`);
											} else {
												setKey(eventKey);
											}
										}}
										className="my-3 pb-0"
									>
										{values.profiles.map((profile, profileIndex) => (
											<Tab
												key={`profile-${profileIndex}`}
												eventKey={`profile-${profileIndex}`}
												title={t('Leds:profile-number', {
													profileNumber: profileIndex + 1,
												})}
											>
												<FormCheck
													size={3}
													name={`profiles.${profileIndex}.bEnabled`}
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
															`profiles.${profileIndex}.bEnabled`,
															Number(!profile.bEnabled),
														)
													}
												/>
												<Row>
													<FormSelect
														label={t('Leds:case-animation-label')}
														name={`profiles.${profileIndex}.baseCaseEffect`}
														className="form-select-sm"
														groupClassName="col-sm-4 mb-3"
														value={Number(profile.baseCaseEffect)}
														onChange={(e) =>
															setFieldValue(
																`profiles.${profileIndex}.baseCaseEffect`,
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
														name={`profiles.${profileIndex}.basePressedEffect`}
														className="form-select-sm"
														groupClassName="col-sm-4 mb-3"
														value={Number(profile.basePressedEffect)}
														onChange={(e) =>
															setFieldValue(
																`profiles.${profileIndex}.basePressedEffect`,
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
														name={`profiles.${profileIndex}.baseNonPressedEffect`}
														className="form-select-sm"
														groupClassName="col-sm-4 mb-3"
														value={Number(profile.baseNonPressedEffect)}
														onChange={(e) =>
															setFieldValue(
																`profiles.${profileIndex}.baseNonPressedEffect`,
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
															name={`profiles.${profileIndex}.bUseCaseLightsInPressedAnimations`}
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
																	`profiles.${profileIndex}.bUseCaseLightsInPressedAnimations`,
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
														name={`profiles.${profileIndex}.buttonPressFadeOutTimeInMs`}
														className="form-control-sm"
														groupClassName="col-sm-4 mb-3"
														value={profile.buttonPressFadeOutTimeInMs}
														onChange={handleChange}
													/>
													<FormControl
														type="number"
														label={t('Leds:pressed-hold-time-label')}
														name={`profiles.${profileIndex}.buttonPressHoldTimeInMs`}
														className="form-control-sm"
														groupClassName="col-sm-4 mb-3"
														value={profile.buttonPressHoldTimeInMs}
														onChange={handleChange}
													/>
												</Row>
												<FormControl
													type="color"
													label={t(`Leds:pressed-special-color-label`)}
													name={`profiles.${profileIndex}.pressedSpecialColor`}
													className="form-control-sm p-0 border-0 mb-3"
													defaultValue={convertToHex(
														profile.pressedSpecialColor,
													)}
													onBlur={(e) =>
														setFieldValue(
															`profiles.${profileIndex}.pressedSpecialColor`,
															convertToDecimal(
																(e.target as HTMLInputElement).value,
															),
														)
													}
												/>
												<FormControl
													type="color"
													label={t(`Leds:idle-special-color-label`)}
													name={`profiles.${profileIndex}.nonPressedSpecialColor`}
													className="form-control-sm p-0 border-0 mb-3"
													defaultValue={convertToHex(
														profile.nonPressedSpecialColor,
													)}
													error={
														(errors.profiles?.[profileIndex] as any)
															?.nonPressedSpecialColor
													}
													isInvalid={Boolean(
														(errors.profiles?.[profileIndex] as any)
															?.nonPressedSpecialColor,
													)}
													onBlur={(e) =>
														setFieldValue(
															`profiles.${profileIndex}.nonPressedSpecialColor`,
															convertToDecimal(
																(e.target as HTMLInputElement).value,
															),
														)
													}
												/>

												<Tabs
													defaultActiveKey="pressed"
													className="mb-3 pb-0"
													fill
												>
													<Tab
														eventKey="pressed"
														title={t(`Leds:pressed-colors-label`)}
													>
														<FormGroup className="mb-4">
															<FieldArray
																name={`profiles.${profileIndex}.pressedStaticColors`}
																render={({ replace }) => (
																	<ColorSelectorList
																		length={GPIO_PIN_LENGTH}
																		LabelComponent={({ index }) => (
																			<div
																				className="d-flex flex-shrink-0"
																				style={{ width: '3.5rem' }}
																			>
																				<label>GP{index}</label>
																			</div>
																		)}
																		containerClassName="pin-grid gap-3 mt-3"
																		colors={profile.pressedStaticColors}
																		customColors={values.customColors}
																		replace={replace}
																	/>
																)}
															/>
														</FormGroup>
													</Tab>
													<Tab
														eventKey="nonpressed"
														title={t(`Leds:idle-colors-label`)}
													>
														<FormGroup className="mb-4">
															<FieldArray
																name={`profiles.${profileIndex}.notPressedStaticColors`}
																render={({ replace }) => (
																	<ColorSelectorList
																		length={GPIO_PIN_LENGTH}
																		LabelComponent={({ index }) => (
																			<div
																				className="d-flex flex-shrink-0"
																				style={{ width: '3.5rem' }}
																			>
																				<label>GP{index}</label>
																			</div>
																		)}
																		containerClassName="pin-grid gap-3 mt-3"
																		colors={profile.notPressedStaticColors}
																		customColors={values.customColors}
																		replace={replace}
																	/>
																)}
															/>
														</FormGroup>
													</Tab>
												</Tabs>
												<hr />
												<FormGroup className="mb-4">
													<Form.Label>{t(`Leds:case-colors-label`)}</Form.Label>

													<FieldArray
														name={`profiles.${profileIndex}.caseStaticColors`}
														render={({ replace }) => (
															<ColorSelectorList
																length={MAX_CASE_LIGHTS}
																LabelComponent={({ index }) => (
																	<div
																		className="d-flex flex-shrink-0"
																		style={{ width: '2rem' }}
																	>
																		<label>{index + 1}</label>
																	</div>
																)}
																containerClassName="case-grid gap-3"
																colors={profile.caseStaticColors}
																customColors={values.customColors}
																replace={replace}
															/>
														)}
													/>
												</FormGroup>
											</Tab>
										))}

										{values.profiles.length !== MAX_ANIMATION_PROFILES && (
											<Tab
												key={`add-profile`}
												eventKey={`profile-add`}
												title={`+ Add Profile`}
											/>
										)}
									</Tabs>
								)}
							/>

							<Button className="mb-3" type="submit">
								{t('Common:button-save-label')}
							</Button>
							{saveMessage && <Alert variant="info">{saveMessage}</Alert>}
						</Form>
					</Section>
				</div>
			)}
		</Formik>
	);
}
