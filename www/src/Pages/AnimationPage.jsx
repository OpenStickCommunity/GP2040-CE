import React, { useEffect, useState } from 'react';
import { Button, Row } from 'react-bootstrap';
import Form from 'react-bootstrap/Form';
import { Formik } from 'formik';
import * as yup from 'yup';

import Section from '../Components/Section';
import useAnimationStore from '../Store/useAnimationStore';
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

export default function AnimationPage() {
	const { animationOptions, fetchAnimationOptions } = useAnimationStore();

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

							<Button type="submit">{t('Common:button-save-label')}</Button>
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
