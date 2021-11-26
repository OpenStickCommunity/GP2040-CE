import React, { useContext, useEffect, useState } from 'react';
import { Button, Form, Row } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import { orderBy } from 'lodash';
import * as yup from 'yup';
import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import DraggableListGroup from '../Components/DraggableListGroup';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import BUTTONS from '../Data/Buttons.json';
import WebApi from '../Services/WebApi';

const LED_FORMATS = [
	{ label: 'GRB', value: 0 },
	{ label: 'RGB', value: 1 },
	{ label: 'GRBW', value: 2 },
	{ label: 'RGBW', value: 3 },
];

const BUTTON_LAYOUTS = [
	{ label: '8-Button Layout', value: 0 },
	{ label: 'Hit Box Layout', value: 1 },
	{ label: 'WASD Layout', value: 2 },
];

const defaultValue = {
	brightnessMaximum: 255,
	brightnessSteps: 5,
	dataPin: -1,
	ledFormat: 0,
	ledLayout: 0,
	ledsPerButton: 2,
};

let usedPins = [];

const schema = yup.object().shape({
	brightnessMaximum : yup.number().required().positive().integer().min(0).max(255).label('Max Brightness'),
	brightnessSteps   : yup.number().required().positive().integer().min(1).max(10).label('Brightness Steps'),
	// eslint-disable-next-line no-template-curly-in-string
	dataPin           : yup.number().required().min(-1).max(29).test('', '${originalValue} is already assigned!', (value) => usedPins.indexOf(value) === -1).label('Data Pin'),
	ledFormat         : yup.number().required().positive().integer().min(0).max(3).label('LED Format'),
	ledLayout         : yup.number().required().positive().integer().min(0).max(2).label('LED Layout'),
	ledsPerButton      : yup.number().required().positive().integer().min(1).label('LEDs Per Pixel'),
});

const getLedButtons = (buttonLabels, map, excludeNulls) => {
	return orderBy(
		Object
			.keys(BUTTONS[buttonLabels])
			.filter(p => p !== 'label' && p !== 'value')
			.filter(p => excludeNulls ? map[p] > -1 : true)
			.map(p => ({ id: p, label: BUTTONS[buttonLabels][p], value: map[p] })),
		"value"
	);
}

const getLedMap = (buttonLabels, ledButtons, excludeNulls) => {
	if (!ledButtons)
		return;

	const map = Object
		.keys(BUTTONS[buttonLabels])
		.filter(p => p !== 'label' && p !== 'value')
		.filter(p => excludeNulls ? ledButtons[p].value > -1 : true)
		.reduce((p, n) => { p[n] = null; return p }, {});

	for (let i = 0; i < ledButtons.length; i++)
		map[ledButtons[i].id] = i;

	return map;
}

const FormContext = ({ buttonLabels, ledButtonMap, ledFormat, setDataSources }) => {
	const { setFieldValue, setValues } = useFormikContext();

	useEffect(() => {
		async function fetchData() {
			const data = await WebApi.getLedOptions();

			let available = {};
			let assigned = {};

			Object.keys(data.ledButtonMap).forEach(p => {
				if (data.ledButtonMap[p] === null)
					available[p] = data.ledButtonMap[p];
				else
					assigned[p] = data.ledButtonMap[p];
			});

			const dataSources = [
				getLedButtons(buttonLabels, available, true),
				getLedButtons(buttonLabels, assigned, true),
			];
			usedPins = data.usedPins;
			setDataSources(dataSources);
			setValues(data);
		}
		fetchData();
	}, [buttonLabels]);

	useEffect(() => {
		if (!!ledFormat)
			setFieldValue('ledFormat', parseInt(ledFormat));
	}, [ledFormat, setFieldValue]);

	useEffect(() => {
		setFieldValue('ledButtonMap', ledButtonMap);
	}, [ledButtonMap, setFieldValue]);

	return null;
};

export default function LEDConfigPage() {
	const { buttonLabels } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');
	const [ledButtonMap, setLedButtonMap] = useState([]);
	const [dataSources, setDataSources] = useState([[], []]);

	const ledOrderChanged = (ledOrderArrays) => {
		if (ledOrderArrays.length === 2)
			setLedButtonMap(getLedMap(buttonLabels, ledOrderArrays[1]));
	};

	const onSuccess = async (values) => {
		const success = WebApi.setLedOptions(values);
		setSaveMessage(success ? 'Saved!' : 'Unable to Save');
	};

	return (
		<Formik validationSchema={schema} onSubmit={onSuccess} initialValues={defaultValue}>
			{({
				handleSubmit,
				handleChange,
				handleBlur,
				values,
				touched,
				errors,
			}) => (
				<Form noValidate onSubmit={handleSubmit}>
					<Section title="LED Configuration">
						<Row>
							<FormControl type="number"
								label="Data Pin (-1 for disabled)"
								name="dataPin"
								className="form-control-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.dataPin}
								error={errors.dataPin}
								isInvalid={errors.dataPin}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormSelect
								label="LED Format"
								name="ledFormat"
								className="form-select-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.ledFormat}
								error={errors.ledFormat}
								isInvalid={errors.ledFormat}
								onChange={handleChange}
								>
								{LED_FORMATS.map((o, i) => <option key={`ledFormat-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
							<FormSelect
								label="LED Layout"
								name="ledLayout"
								className="form-select-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.ledLayout}
								error={errors.ledLayout}
								isInvalid={errors.ledLayout}
								onChange={handleChange}
							>
								{BUTTON_LAYOUTS.map((o, i) => <option key={`ledLayout-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
						</Row>
						<Row>
							<FormControl type="number"
								label="LEDs Per Button"
								name="ledsPerButton"
								className="form-control-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.ledsPerButton}
								error={errors.ledsPerButton}
								isInvalid={errors.ledsPerButton}
								onChange={handleChange}
								min={1}
							/>
							<FormControl type="number"
								label="Max Brightness"
								name="brightnessMaximum"
								className="form-control-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.brightnessMaximum}
								error={errors.brightnessMaximum}
								isInvalid={errors.brightnessMaximum}
								onChange={handleChange}
								min={0}
								max={255}
							/>
							<FormControl type="number"
								label="Brightness Steps"
								name="brightnessSteps"
								className="form-control-sm"
								groupClassName="col-sm-4 mb-3"
								value={values.brightnessSteps}
								error={errors.brightnessSteps}
								isInvalid={errors.brightnessSteps}
								onChange={handleChange}
								min={1}
								max={10}
							/>
						</Row>
					</Section>
					<Section title="LED Button Order">
						<p className="card-text">
							Here you can define which buttons have RGB LEDs and in what order they run from the control board.
							This is required for certain LED animations and static theme support.
						</p>
						<p className="card-text">
							Drag and drop list items to assign and reorder the RGB LEDs.
						</p>
						<DraggableListGroup
							groupName="test"
							titles={['Available Buttons', 'Assigned Buttons']}
							dataSources={dataSources}
							onChange={ledOrderChanged}
						/>
					</Section>
					<Button type="submit">Save</Button>
					{saveMessage ? <span className="alert">{saveMessage}</span> : null}
					<FormContext {...{
						buttonLabels,
						ledButtonMap,
						setDataSources,
						ledFormat: values.ledFormat
					}} />
				</Form>
			)}
		</Formik>
	);
}
