import React, { useEffect, useState } from 'react';
import { Button, FormGroup, Row, Tab, Tabs } from 'react-bootstrap';
import Form from 'react-bootstrap/Form';
import { FieldArray, Formik } from 'formik';
import * as yup from 'yup';

import Section from '../Components/Section';
import useAnimationStore, {
	MAX_CUSTOM_COLORS,
} from '../Store/useAnimationStore';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import { useTranslation } from 'react-i18next';

const schema = yup.object().shape({
	baseProfileIndex: yup.number().required('Selecting a profile is required'),
	brightness: yup
		.number()
		.min(0, 'Brightness must be at least 0')
		.max(100, 'Brightness cannot be more than 100')
		.required('Brightness is required'),
	profiles: yup.array().of(
		yup.object().shape({
			// TODO add schema for profile
		}),
	),
});

const convertToHex = (color: number) =>
	`#${color.toString(16).padStart(6, '0')}`;

const convertToDecimal = (hex: string) => parseInt(hex.replace('#', ''), 16);

export default function AnimationPage() {
	const { animationOptions, fetchAnimationOptions } = useAnimationStore();
	const [key, setKey] = useState(
		`profile-${animationOptions.baseProfileIndex}`,
	);
	const { t } = useTranslation('');

	const [saveMessage, setSaveMessage] = useState('');
	useEffect(() => {
		fetchAnimationOptions();
	}, []);

	const onSuccess = async (values) => {
		setSaveMessage('Saved');
	};

	return (
		<Section title="Animation">
			<Formik
				validationSchema={schema}
				onSubmit={onSuccess}
				initialValues={animationOptions}
			>
				{({ handleSubmit, handleChange, values, errors, setFieldValue }) =>
					console.log(values, 'values') || (
						<Form noValidate onSubmit={handleSubmit}>
							<Row>
								<FormSelect
									label={'Active profile'}
									name="baseProfileIndex"
									className="form-control-sm"
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
							<Row>
								<FormGroup>
									<Form.Label>Custom colors</Form.Label>
									<FieldArray
										name="customColors"
										render={(arrayHelpers) => (
											<div className="">
												{values.customColors.map((color, index) => (
													<div
														key={`color-${index}`}
														className="d-flex col gap-2"
													>
														<FormControl
															type="color"
															name={`customColors.${index}`}
															className="w-100 form-control-sm"
															groupClassName="col-2"
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
												{values.customColors.length <= MAX_CUSTOM_COLORS && (
													<Button
														size="sm"
														onClick={() => arrayHelpers.push(0)}
													>
														{'+ Add color'}
													</Button>
												)}
											</div>
										)}
									/>
								</FormGroup>
							</Row>

							<FieldArray
								name="profiles"
								render={(arrayHelpers) => (
									<Tabs
										activeKey={key}
										onSelect={(eventKey) => {
											if (!eventKey) return;
											console.log('eventKey', eventKey);
											setKey(eventKey);
										}}
										className="my-3"
									>
										{values.profiles.map((profile, index) => (
											<Tab
												key={`profile-${index}`}
												eventKey={`profile-${index}`}
												title={`Profile ${index + 1}`}
											>
												Tab content for Profile {index + 1}
											</Tab>
										))}
									</Tabs>
								)}
							/>

							<Button className="mt-3" type="submit">
								{t('Common:button-save-label')}
							</Button>
							{saveMessage ? (
								<span className="alert">{saveMessage}</span>
							) : null}
						</Form>
					)
				}
			</Formik>
		</Section>
	);
}
