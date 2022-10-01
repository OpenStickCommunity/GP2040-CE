import React, { useEffect, useState } from 'react';
import { Button, Form, Row, Col } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import * as yup from 'yup';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import Section from '../Components/Section';
import WebApi from '../Services/WebApi';

const schema = yup.object().shape({
	turboPin: yup.number().required().min(-1).max(29).test('', '${originalValue} is already assigned!', (value) => usedPins.indexOf(value) === -1).label('Turbo Pin'),
	turboPinLED: yup.number().required().min(-1).max(29).test('', '${originalValue} is already assigned!', (value) => usedPins.indexOf(value) === -1).label('Turbo Pin LED'),
	sliderLSPin: yup.number().required().min(-1).max(29).test('', '${originalValue} is already assigned!', (value) => usedPins.indexOf(value) === -1).label('Slider LS Pin'),
	sliderRSPin: yup.number().required().min(-1).max(29).test('', '${originalValue} is already assigned!', (value) => usedPins.indexOf(value) === -1).label('Slider RS Pin'),
	turboShotCount: yup.number().required().min(5).max(30).label('Turbo Shot Count'),
	reversePin: yup.number().required().min(-1).max(29).test('', '${originalValue} is already assigned!', (value) => usedPins.indexOf(value) === -1).label('Reverse Pin'),
	reversePinLED: yup.number().required().min(-1).max(29).test('', '${originalValue} is already assigned!', (value) => usedPins.indexOf(value) === -1).label('Reverse Pin LED'),
});

const defaultValues = {
	turboPin: -1,
	turboPinLED: -1,
	sliderLSPin: -1,
	sliderRSPin: -1,
	turboShotCount: 5,
	reversePin: -1,
	reversePinLED: -1
};

const REVERSE_ACTION = [
	{ label: 'Disable', value: 0 },
	{ label: 'Enable', value: 1 },
	{ label: 'Neutral', value: 2 },
];

let usedPins = [];

const FormContext = () => {
	const { values, setValues } = useFormikContext();

	useEffect(() => {
		async function fetchData() {
			const data = await WebApi.getAddonsOptions();
			usedPins = data.usedPins;
			setValues(data);
		}
		fetchData();
	}, [setValues]);

	useEffect(() => {
		if (!!values.turboPin)
			values.turboPin = parseInt(values.turboPin);
		if (!!values.turboPinLED)
			values.turboPinLED = parseInt(values.turboPinLED);
		if (!!values.sliderLSPin)
			values.sliderLSPin = parseInt(values.sliderLSPin);
		if (!!values.sliderRSPin)
			values.sliderRSPin = parseInt(values.sliderRSPin);
		if (!!values.turboShotCount)
			values.turboShotCount = parseInt(values.turboShotCount);
		if (!!values.reversePin)
			values.reversePin = parseInt(values.reversePin);
		if (!!values.reversePinLED)
			values.reversePinLED = parseInt(values.reversePinLED);
		if (!!values.reverseActionUp)
			values.reverseActionUp = parseInt(values.reverseActionUp);
		if (!!values.reverseActionDown)
			values.reverseActionDown = parseInt(values.reverseActionDown);
		if (!!values.reverseActionLeft)
			values.reverseActionLeft = parseInt(values.reverseActionLeft);
		if (!!values.reverseActionRight)
			values.reverseActionRight = parseInt(values.reverseActionRight);
	}, [values, setValues]);

	return null;
};

export default function AddonsConfigPage() {
	const [saveMessage, setSaveMessage] = useState('');

	const onSuccess = async (values) => {
		const success = WebApi.setAddonsOptions(values);
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
			}) => (
				<Form noValidate onSubmit={handleSubmit}>
					<Section title="Add-Ons Configuration">
						<p>Use the form below to reconfigure experimental options in GP2040-CE.</p>
						<p>Please note: these options are experimental for the time being.</p>
					</Section>
					<Section title="Turbo">
						<Col>
							<FormControl type="number"
								label="Turbo Pin"
								name="turboPin"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.turboPin}
								error={errors.turboPin}
								isInvalid={errors.turboPin}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormControl type="number"
								label="Turbo Pin LED"
								name="turboPinLED"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.turboPinLED}
								error={errors.turboPinLED}
								isInvalid={errors.turboPinLED}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormControl type="number"
								label="Turbo Shot Count"
								name="turboShotCount"
								className="form-control-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.turboShotCount}
								error={errors.turboShotCount}
								isInvalid={errors.turboShotCount}
								onChange={handleChange}
								min={2}
								max={30}
							/>
						</Col>
					</Section>
					<Section title="Slider">
						<Col>
							<FormControl type="number"
								label="Slider LS Pin"
								name="sliderLSPin"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.sliderLSPin}
								error={errors.sliderLSPin}
								isInvalid={errors.sliderLSPin}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormControl type="number"
								label="Slider RS Pin"
								name="sliderRSPin"
								className="form-control-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.sliderRSPin}
								error={errors.sliderRSPin}
								isInvalid={errors.sliderRSPin}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
						</Col>
					</Section>
					<Section title="Input Reverse">
						<Col>
							<FormControl type="number"
								label="Reverse Input Pin"
								name="reversePin"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.reversePin}
								error={errors.reversePin}
								isInvalid={errors.reversePin}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormControl type="number"
								label="Reverse Input Pin LED"
								name="reversePinLED"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.reversePinLED}
								error={errors.reversePinLED}
								isInvalid={errors.reversePinLED}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormSelect
								label="Reverse Up"
								name="reverseActionUp"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.reverseActionUp}
								error={errors.reverseActionUp}
								isInvalid={errors.reverseActionUp}
								onChange={handleChange}
							>
								{REVERSE_ACTION.map((o, i) => <option key={`reverseActionUp-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
							<FormSelect
								label="Reverse Down"
								name="reverseActionDown"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.reverseActionDown}
								error={errors.reverseActionDown}
								isInvalid={errors.reverseActionDown}
								onChange={handleChange}
							>
								{REVERSE_ACTION.map((o, i) => <option key={`reverseActionDown-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
							<FormSelect
								label="Reverse Left"
								name="reverseActionLeft"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.reverseActionLeft}
								error={errors.reverseActionLeft}
								isInvalid={errors.reverseActionLeft}
								onChange={handleChange}
							>
								{REVERSE_ACTION.map((o, i) => <option key={`reverseActionLeft-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
							<FormSelect
								label="Reverse Right"
								name="reverseActionRight"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.reverseActionRight}
								error={errors.reverseActionRight}
								isInvalid={errors.reverseActionRight}
								onChange={handleChange}
							>
								{REVERSE_ACTION.map((o, i) => <option key={`reverseActionRight-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
						</Col>
					</Section>
					<div className="mt-3">
						<Button type="submit">Save</Button>
						{saveMessage ? <span className="alert">{saveMessage}</span> : null}
					</div>
					<FormContext />
				</Form>
			)}
		</Formik>
	);
}
