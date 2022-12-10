import React, { useEffect, useState, useRef } from 'react';
import { Button, Form, Row, Col } from 'react-bootstrap';
import { Formik, useFormikContext, ErrorMessage, Field } from 'formik';
import * as yup from 'yup';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import Section from '../Components/Section';
import WebApi from '../Services/WebApi';

const ON_OFF_OPTIONS = [
	{ label: 'Disabled', value: 0 },
	{ label: 'Enabled', value: 1 },
];

const defaultValues = {
	enabled: false,
	buzzerPin: -1,
	buzzerVolume: 100
};

let usedPins = [];

const schema = yup.object().shape({
	enabled: yup.number().required().min(0).max(1).label('Enabled?'),
	// eslint-disable-next-line no-template-curly-in-string
	buzzerPin: yup.number().required().min(-1).max(29).test('', '${originalValue} is already assigned!', (value) => usedPins.indexOf(value) === -1).label('Buzzer Pin'),
	buzzerVolume: yup.number().required().min(0).max(100).label('Buzzer Volume'),
});

const FormContext = () => {
	const { values, setValues } = useFormikContext();

	useEffect(() => {
		async function fetchData() {
			const data = await WebApi.getBuzzerOptions();
			usedPins = data.usedPins;
			setValues(data);
		}
		fetchData();
	}, [setValues]);

	useEffect(() => {
		if (!!values.enabled)
			values.enabled = parseInt(values.enabled);
		if (!!values.buzzerPin)
			values.buzzerPin = parseInt(values.buzzerPin);
		if (!!values.buzzerVolume)
			values.buzzerVolume = parseInt(values.buzzerVolume);
	}, [values, setValues]);

	return null;
};

export default function BuzzerConfigPage() {
	const [saveMessage, setSaveMessage] = useState('');

	const onSuccess = async (values) => {
		const success = await WebApi.setBuzzerOptions(values);
		setSaveMessage(success ? 'Saved! Please Restart Your Device' : 'Unable to Save');
	};
	
	return (
		<Formik validationSchema={schema} onSubmit={onSuccess} initialValues={defaultValues}>
			{({
				handleSubmit,
				handleChange,
				handleBlur,
				values,
				touched,
				errors,
			}) => console.log('errors', errors) || (console.log('values', values) ||
				<Section title="Buzzer Configuration">
					<p>
						A passive buzzer can be used to alert when the joystick is connected or other events. Ensure your buzzer speaker
						has the following attributes:
					</p>
					<ul>
						<li>Passive type</li>
						<li>Supports 3v or 5v</li>
					</ul>
					<Form noValidate onSubmit={handleSubmit}>
						<Row>
							<FormSelect
								label="Use Buzzer"
								name="enabled"
								className="form-select-sm"
								groupClassName="col-sm-4 mb-4"
								value={values.enabled}
								error={errors.enabled}
								isInvalid={errors.enabled}
								onChange={handleChange}
							>
								{ON_OFF_OPTIONS.map((o, i) => <option key={`enabled-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
							<FormControl type="number"
								label="Buzzer Pin"
								name="buzzerPin"
								className="form-control-sm"
								groupClassName="col-sm-4 mb-4"
								value={values.buzzerPin}
								error={errors.buzzerPin}
								isInvalid={errors.buzzerPin}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormControl type="number"
								label="Buzzer Volume"
								name="buzzerVolume"
								className="form-control-sm"
								groupClassName="col-sm-4 mb-4"
								value={values.buzzerVolume}
								error={errors.buzzerVolume}
								isInvalid={errors.buzzerVolume}
								onChange={handleChange}
								min={0}
								max={100}
							/>
						</Row>
						<div className="mt-3">
							<Button type="submit">Save</Button>
							{saveMessage ? <span className="alert">{saveMessage}</span> : null}
						</div>
						<FormContext />
					</Form>
				</Section>
			)}
		</Formik>
	);
}

