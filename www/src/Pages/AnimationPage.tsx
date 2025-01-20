import React, { useEffect, useState } from 'react';
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
} from 'react-bootstrap';
import { ErrorMessage, FieldArray, Formik } from 'formik';
import * as yup from 'yup';

import Section from '../Components/Section';
import useAnimationStore, {
	MAX_ANIMATION_PROFILES,
	MAX_CASE_LIGHTS,
	MAX_CUSTOM_COLORS,
	MAX_PRESSED_COLORS,
} from '../Store/useAnimationStore';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import { useTranslation } from 'react-i18next';
import InfoCircle from '../Icons/InfoCircle';
import { parseInt } from 'lodash';
import {
	ANIMATION_NON_PRESSED_EFFECTS,
	ANIMATION_PRESSED_EFFECTS,
} from '../Data/Animations';

const schema = yup.object().shape({
	baseProfileIndex: yup.number().required('Selecting a profile is required'),
	brightness: yup
		.number()
		.min(0, 'Brightness must be at least 0')
		.max(100, 'Brightness cannot be more than 100')
		.required('Brightness is required'),
	profiles: yup.array().of(
		yup.object().shape({
			bEnabled: yup.number().required(),
			baseCaseEffect: yup.number().required(),
			baseNonPressedEffect: yup.number().required(),
			basePressedEffect: yup.number().required(),
			buttonPressFadeOutTimeInMs: yup.number().required(),
			buttonPressHoldTimeInMs: yup.number().required(),
			caseStaticColors: yup.array().of(yup.number()).required(),
			nonPressedSpecialColour: yup.number().required(),
			notPressedStaticColors: yup
				.array()
				.of(yup.number())
				.required()
				.test(
					'same-length',
					'Not Pressed Static Colors and Pressed Static Colors must be the same length',
					function (value) {
						return value.length === this.parent.pressedStaticColors.length;
					},
				),
			pressedSpecialColour: yup.number().required(),
			pressedStaticColors: yup
				.array()
				.of(yup.number())
				.required()
				.test(
					'same-length',
					'Pressed Static Colors and Not Pressed Static Colors must be the same length',
					function (value) {
						return value.length === this.parent.notPressedStaticColors.length;
					},
				),
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
	caseStaticColors: [],
	nonPressedSpecialColour: 0,
	notPressedStaticColors: [],
	pressedSpecialColour: 0,
	pressedStaticColors: [],
};

const convertToHex = (color: number) =>
	`#${color.toString(16).padStart(6, '0')}`;

const convertToDecimal = (hex: string) => parseInt(hex.replace('#', ''), 16);

export default function AnimationPage() {
	const {
		animationOptions,
		loadingAnimationOptions,
		fetchAnimationOptions,
		saveAnimationOptions,
	} = useAnimationStore();
	const [key, setKey] = useState(
		`profile-${animationOptions.baseProfileIndex}`,
	);
	const { t } = useTranslation('');

	const [saveMessage, setSaveMessage] = useState('');

	useEffect(() => {
		fetchAnimationOptions();
	}, []);

	const onSuccess = async () => {
		try {
			await saveAnimationOptions();
			setSaveMessage(t('Common:saved-success-message'));
		} catch (error) {
			console.error(error);
		}
	};

	if (loadingAnimationOptions) {
		return (
			<div className="d-flex justify-content-center">
				<span className="spinner-border"></span>
			</div>
		);
	}

	return (
		<Section title="Animation">
			<Formik
				validationSchema={schema}
				onSubmit={onSuccess}
				initialValues={animationOptions}
			>
				{({ handleSubmit, handleChange, values, errors, setFieldValue }) => (
					<Form noValidate onSubmit={handleSubmit}>
						<Row>
							<FormSelect
								label={'Active profile'}
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
								{values.profiles.map((_, i) => (
									<option key={`profile-select-${i}`} value={i}>
										Profile {i + 1}
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
						</Row>
						<FormGroup>
							<Form.Label>Custom colors</Form.Label>
							<FieldArray
								name="customColors"
								render={(arrayHelpers) => (
									<div className="d-flex col gap-2">
										{values.customColors.map((color, index) => (
											<div
												key={`color-${index}`}
												className="d-flex gap-1 flex-wrap"
											>
												<FormControl
													type="color"
													name={`customColors.${index}`}
													className="form-control-sm p-0 border-0"
													value={convertToHex(color)}
													onChange={(e) =>
														arrayHelpers.replace(
															index,
															convertToDecimal(e.target.value),
														)
													}
												/>
												<Button
													size="sm"
													onClick={() => {
														arrayHelpers.remove(index);
													}}
												>
													{'✕'}
												</Button>
											</div>
										))}
										{values.customColors.length !== MAX_CUSTOM_COLORS && (
											<>
												<div className="vr"></div>
												<Button size="sm" onClick={() => arrayHelpers.push(0)}>
													{'+ Add color'}
												</Button>
											</>
										)}
									</div>
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
									className="my-3"
								>
									{values.profiles.map((profile, index) => (
										<Tab
											key={`profile-${index}`}
											eventKey={`profile-${index}`}
											title={`Profile ${index + 1}`}
										>
											<FormCheck
												size={3}
												name={`profiles.${index}.bEnabled`}
												label={
													<OverlayTrigger
														overlay={<Tooltip>Enabled does something</Tooltip>}
													>
														<div className="d-flex gap-1">
															<label>Enabled</label>
															<InfoCircle />
														</div>
													</OverlayTrigger>
												}
												type="switch"
												reverse
												checked={Boolean(profile.bEnabled)}
												onChange={() =>
													setFieldValue(
														`profiles.${index}.bEnabled`,
														Number(!profile.bEnabled),
													)
												}
											/>
											<Row>
												<FormSelect
													label={'Base Case Effect'}
													name={`profiles.${index}.baseCaseEffect`}
													className="form-select-sm"
													groupClassName="col-sm-4 mb-3"
													value={Number(profile.baseCaseEffect)}
													onChange={(e) =>
														setFieldValue(
															`profiles.${index}.baseCaseEffect`,
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
																{key}
															</option>
														),
													)}
												</FormSelect>
												<FormSelect
													label={'Base Non Pressed Effect'}
													name={`profiles.${index}.baseNonPressedEffect`}
													className="form-select-sm"
													groupClassName="col-sm-4 mb-3"
													value={Number(profile.baseNonPressedEffect)}
													onChange={(e) =>
														setFieldValue(
															`profiles.${index}.baseNonPressedEffect`,
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
																{key}
															</option>
														),
													)}
												</FormSelect>
												<FormSelect
													label={'Base Pressed Effect'}
													name={`profiles.${index}.basePressedEffect`}
													className="form-select-sm"
													groupClassName="col-sm-4 mb-3"
													value={Number(profile.basePressedEffect)}
													onChange={(e) =>
														setFieldValue(
															`profiles.${index}.basePressedEffect`,
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
																{key}
															</option>
														),
													)}
												</FormSelect>
											</Row>
											<Row>
												<FormControl
													type="number"
													label={'Button Press Fade Out Time (ms)'}
													name={`profiles.${index}.buttonPressFadeOutTimeInMs`}
													className="form-control-sm"
													groupClassName="col-sm-4 mb-3"
													value={profile.buttonPressFadeOutTimeInMs}
													onChange={handleChange}
												/>
												<FormControl
													type="number"
													label={'Button Press Hold Time (ms)'}
													name={`profiles.${index}.buttonPressHoldTimeInMs`}
													className="form-control-sm"
													groupClassName="col-sm-4 mb-3"
													value={profile.buttonPressHoldTimeInMs}
													onChange={handleChange}
												/>
											</Row>
											<FormControl
												type="color"
												label={'Pressed Special Colour'}
												name={`profiles.${index}.pressedSpecialColour`}
												className="form-control-sm p-0 border-0 mb-3"
												value={convertToHex(profile.pressedSpecialColour)}
												onChange={(e) =>
													setFieldValue(
														`profiles.${index}.pressedSpecialColour`,
														convertToDecimal(e.target.value),
													)
												}
											/>
											<FormControl
												type="color"
												label={'Non Pressed Special Colour'}
												name={`profiles.${index}.nonPressedSpecialColour`}
												className="form-control-sm p-0 border-0 mb-3"
												value={convertToHex(profile.nonPressedSpecialColour)}
												onChange={(e) =>
													setFieldValue(
														`profiles.${index}.nonPressedSpecialColour`,
														convertToDecimal(e.target.value),
													)
												}
											/>
											<FormGroup className="mb-3">
												<Form.Label>Case Static Colors</Form.Label>
												<FieldArray
													name={`profiles.${index}.caseStaticColors`}
													render={(arrayHelpers) => (
														<div className="d-flex col gap-2 flex-wrap">
															{profile.caseStaticColors.map((color, i) => (
																<div
																	key={`caseStaticColors-${i}`}
																	className="d-flex gap-1"
																>
																	<FormControl
																		type="color"
																		name={`profiles.${index}.caseStaticColors.${i}`}
																		className="form-control-sm p-0 border-0"
																		value={convertToHex(color)}
																		onChange={(e) =>
																			arrayHelpers.replace(
																				i,
																				convertToDecimal(e.target.value),
																			)
																		}
																	/>
																	<Button
																		size="sm"
																		onClick={() => {
																			arrayHelpers.remove(i);
																		}}
																	>
																		{'✕'}
																	</Button>
																</div>
															))}
															{profile.caseStaticColors.length !==
																MAX_CASE_LIGHTS && (
																<>
																	<div className="vr"></div>
																	<Button
																		size="sm"
																		onClick={() => arrayHelpers.push(0)}
																	>
																		{'+ Add color'}
																	</Button>
																</>
															)}
														</div>
													)}
												/>
											</FormGroup>
											<FormGroup className="mb-3">
												<Form.Label>Pressed Static Colors</Form.Label>
												<FieldArray
													name={`profiles.${index}.pressedStaticColors`}
													render={(arrayHelpers) => (
														<>
															<div className="d-flex col gap-2 flex-wrap">
																{profile.pressedStaticColors.map((color, i) => (
																	<div
																		key={`pressedStaticColors-${i}`}
																		className="d-flex gap-1"
																	>
																		<FormControl
																			type="color"
																			name={`profiles.${index}.pressedStaticColors.${i}`}
																			className="form-control-sm p-0 border-0"
																			value={convertToHex(color)}
																			onChange={(e) =>
																				arrayHelpers.replace(
																					i,
																					convertToDecimal(e.target.value),
																				)
																			}
																		/>
																		<Button
																			size="sm"
																			onClick={() => {
																				arrayHelpers.remove(i);
																			}}
																		>
																			{'✕'}
																		</Button>
																	</div>
																))}
																{profile.pressedStaticColors.length !==
																	MAX_PRESSED_COLORS && (
																	<>
																		<div className="vr"></div>
																		<Button
																			size="sm"
																			onClick={() => arrayHelpers.push(0)}
																		>
																			{'+ Add color'}
																		</Button>
																	</>
																)}
															</div>
															<ErrorMessage
																name={`profiles.${index}.pressedStaticColors`}
																render={(msg) => (
																	<p className="text-danger">{msg}</p>
																)}
															/>
														</>
													)}
												/>
											</FormGroup>
											<FormGroup className="mb-3">
												<Form.Label>Non Pressed Static Colors</Form.Label>
												<FieldArray
													name={`profiles.${index}.notPressedStaticColors`}
													render={(arrayHelpers) => (
														<>
															<div className="d-flex col gap-2 flex-wrap">
																{profile.notPressedStaticColors.map(
																	(color, i) => (
																		<div
																			key={`notPressedStaticColors-${i}`}
																			className="d-flex gap-1"
																		>
																			<FormControl
																				type="color"
																				name={`profiles.${index}.notPressedStaticColors.${i}`}
																				className="form-control-sm p-0 border-0"
																				value={convertToHex(color)}
																				onChange={(e) =>
																					arrayHelpers.replace(
																						i,
																						convertToDecimal(e.target.value),
																					)
																				}
																			/>
																			<Button
																				size="sm"
																				onClick={() => {
																					arrayHelpers.remove(i);
																				}}
																			>
																				{'✕'}
																			</Button>
																		</div>
																	),
																)}
																{profile.notPressedStaticColors.length !==
																	MAX_PRESSED_COLORS && (
																	<>
																		<div className="vr"></div>
																		<Button
																			size="sm"
																			onClick={() => arrayHelpers.push(0)}
																		>
																			{'+ Add color'}
																		</Button>
																	</>
																)}
															</div>

															<ErrorMessage
																name={`profiles.${index}.notPressedStaticColors`}
																render={(msg) => (
																	<p className="text-danger">{msg}</p>
																)}
															/>
														</>
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
				)}
			</Formik>
		</Section>
	);
}
