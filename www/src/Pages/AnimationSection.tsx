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
import './AnimationSection.scss';

const GPIO_PIN_LENGTH = boards[import.meta.env.VITE_GP2040_BOARD].maxPin + 1;
const GPIO_PIN_ARRAY = Array.from({ length: GPIO_PIN_LENGTH });

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

// Values are indexes of color array in Animation.hpp
const colorOptions = [
	{ value: 0, label: 'Black', color: 'black' },
	{ value: 1, label: 'White', color: 'white' },
	{ value: 2, label: 'Red', color: 'red' },
	{ value: 3, label: 'Orange', color: 'orange' },
	{ value: 4, label: 'Yellow', color: 'yellow' },
	{ value: 5, label: 'Lime Green', color: 'limegreen' },
	{ value: 6, label: 'Green', color: 'green' },
	{ value: 7, label: 'Seafoam', color: 'seagreen' },
	{ value: 8, label: 'Aqua', color: 'aqua' },
	{ value: 9, label: 'Sky Blue', color: 'skyblue' },
	{ value: 10, label: 'Blue', color: 'blue' },
	{ value: 11, label: 'Purple', color: 'purple' },
	{ value: 12, label: 'Pink', color: 'pink' },
	{ value: 13, label: 'Magenta', color: 'magenta' },
];

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
		marginRight: 8,
		height: 15,
		width: 15,
	},
});

const colorStyles: StylesConfig<(typeof colorOptions)[number]> = {
	control: (styles) => ({ ...styles, backgroundColor: 'white' }),
	option: (styles, { data }) => ({ ...styles, ...colorDot(data.color) }),
	input: (styles) => ({ ...styles }),
	placeholder: (styles) => ({ ...styles, ...colorDot('#ccc') }),
	singleValue: (styles, { data }) => ({
		...styles,
		...colorDot(data.color),
	}),
};

const GpioColorSelectorList = memo(function GpioColorSelectorList({
	colors,
	replace,
}: {
	colors: number[];
	replace: FieldArrayRenderProps['replace'];
}) {
	return (
		<div className="pin-grid gap-3 mt-3">
			{GPIO_PIN_ARRAY.map((_, gpioPinIndex) => (
				<div
					key={`select-${gpioPinIndex}`}
					className="d-flex col align-items-center"
				>
					<div className="d-flex flex-shrink-0" style={{ width: '3.5rem' }}>
						<label>GP{gpioPinIndex}</label>
					</div>
					<CustomSelect
						isClearable
						options={colorOptions}
						styles={colorStyles}
						isMulti={false}
						onChange={(selected) => {
							replace(gpioPinIndex, selected?.value || 0);
						}}
						value={
							colors[gpioPinIndex]
								? colorOptions.find(
										({ value }) => value === colors[gpioPinIndex],
									)
								: colorOptions[0]
						}
					/>
				</div>
			))}
		</div>
	);
});
const CaseColorSelectorList = memo(function GpioColorSelectorList({
	colors,
	replace,
}: {
	colors: number[];
	replace: FieldArrayRenderProps['replace'];
}) {
	return (
		<div className="case-grid gap-2">
			{Array.from({ length: MAX_CASE_LIGHTS }).map((_, caseIndex) => (
				<div
					key={`select-${caseIndex}`}
					className="d-flex col align-items-center"
				>
					<div className="d-flex flex-shrink-0" style={{ width: '2rem' }}>
						<label>{caseIndex + 1}</label>
					</div>
					<CustomSelect
						options={colorOptions}
						styles={colorStyles}
						isMulti={false}
						onChange={(selected) => {
							replace(caseIndex, selected?.value || 0);
						}}
						value={
							colors[caseIndex]
								? colorOptions.find(({ value }) => value === colors[caseIndex])
								: colorOptions[0]
						}
					/>
				</div>
			))}
		</div>
	);
});

const ColorSelectorList = memo(function ColorSelectorList({
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
				<div key={`color-${index}`} className="d-flex gap-1 flex-wrap">
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

export default function AnimationSection() {
	const { AnimationOptions, saveAnimationOptions } = useLedStore();
	const {
		activateLedsOnId,
		activateLedsProfile,
		activateLedsChase,
		turnOffLeds,
	} = useLedsPreview();

	const [key, setKey] = useState(
		`profile-${AnimationOptions.baseProfileIndex}`,
	);

	const [previewGpioPin, setPreviewGpioPin] = useState(0);
	const [previewCaseId, setPreviewCaseId] = useState(0);
	const [previewProfileIndex, setPreviewProfileIndex] = useState(
		AnimationOptions.baseProfileIndex,
	);
	const { t } = useTranslation('');

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
					<Section title="Preview helpers">
						<Row className="mb-3">
							<Col md={6} className="d-flex flex-column justify-content-end">
								<p>Active light tied to GPIO pin</p>
								<FormControl
									type="number"
									className="form-control-sm"
									groupClassName="mb-3"
									min={0}
									value={previewGpioPin}
									onChange={(e) => {
										const pin = parseInt((e.target as HTMLInputElement).value);
										setPreviewGpioPin(pin);
									}}
								/>

								<Button
									variant="secondary"
									onClick={() => {
										activateLedsOnId(previewGpioPin);
									}}
								>
									GPIO Pin Test
								</Button>
							</Col>
							<Col md={6} className="d-flex flex-column justify-content-end">
								<p>Active light tied to case ID</p>
								<FormControl
									type="number"
									className="form-control-sm"
									groupClassName=" mb-3"
									min={0}
									value={previewCaseId}
									onChange={(e) => {
										const caseId = parseInt(
											(e.target as HTMLInputElement).value,
										);
										setPreviewCaseId(caseId);
									}}
								/>

								<Button
									variant="secondary"
									onClick={() => {
										activateLedsOnId(previewCaseId, true);
									}}
								>
									Case ID Test
								</Button>
							</Col>
						</Row>
						<Row className="mb-3">
							<Col md={4} className="d-flex flex-column justify-content-end">
								<p>
									Run a chase animation from left to right and then top to
									bottom to help verify correct grid positioning of the lights
								</p>
								<Button
									variant="secondary"
									onClick={() => {
										activateLedsChase();
									}}
								>
									Layout Test
								</Button>
							</Col>
							<Col md={4} className="d-flex flex-column justify-content-end">
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
							<Col md={4} className="d-flex flex-column justify-content-end">
								<p>Turns off all the lights</p>
								<Button
									variant="secondary"
									onClick={() => {
										turnOffLeds();
									}}
								>
									Lights Off
								</Button>
							</Col>
						</Row>
					</Section>
					<Section title="Led configuration">
						<Form noValidate onSubmit={handleSubmit}>
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
										<ColorSelectorList
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
													value={convertToHex(profile.pressedSpecialColor)}
													onChange={(e) =>
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
													value={convertToHex(profile.nonPressedSpecialColor)}
													error={
														(errors.profiles?.[profileIndex] as any)
															?.nonPressedSpecialColor
													}
													isInvalid={Boolean(
														(errors.profiles?.[profileIndex] as any)
															?.nonPressedSpecialColor,
													)}
													onChange={(e) =>
														setFieldValue(
															`profiles.${profileIndex}.nonPressedSpecialColor`,
															convertToDecimal(
																(e.target as HTMLInputElement).value,
															),
														)
													}
												/>
												<FormGroup className="mb-4">
													<Form.Label>{t(`Leds:case-colors-label`)}</Form.Label>
													<FieldArray
														name={`profiles.${profileIndex}.caseStaticColors`}
														render={({ replace }) => (
															<CaseColorSelectorList
																colors={profile.caseStaticColors}
																replace={replace}
															/>
														)}
													/>
												</FormGroup>
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
																	<GpioColorSelectorList
																		colors={profile.pressedStaticColors}
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
																	<GpioColorSelectorList
																		colors={profile.notPressedStaticColors}
																		replace={replace}
																	/>
																)}
															/>
														</FormGroup>
													</Tab>
												</Tabs>
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
