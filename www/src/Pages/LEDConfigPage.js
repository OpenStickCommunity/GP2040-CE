import React, { useContext, useEffect, useState } from 'react';
import Button from 'react-bootstrap/Button';
import Col from 'react-bootstrap/Col';
import Container from 'react-bootstrap/Container';
import Form from 'react-bootstrap/Form';
import Row from 'react-bootstrap/Row';
import { Formik, useFormikContext } from 'formik';
import * as yup from 'yup';
import orderBy from 'lodash/orderBy';
import { SketchPicker } from '@hello-pangea/color-picker';

import { AppContext } from '../Contexts/AppContext';
import ColorPicker from '../Components/ColorPicker';
import Section from '../Components/Section';
import DraggableListGroup from '../Components/DraggableListGroup';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import { BUTTONS } from '../Data/Buttons';
import LEDColors from '../Data/LEDColors';
import { hexToInt, rgbIntToHex } from '../Services/Utilities';
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

const PLED_LABELS = [
	{ 0: 'PLED #1 Pin', 1: 'PLED #1 Index' },
	{ 0: 'PLED #2 Pin', 1: 'PLED #2 Index' },
	{ 0: 'PLED #3 Pin', 1: 'PLED #3 Index' },
	{ 0: 'PLED #4 Pin', 1: 'PLED #4 Index' },
];

const defaultValue = {
	brightnessMaximum: 255,
	brightnessSteps: 5,
	dataPin: -1,
	ledFormat: 0,
	ledLayout: 0,
	ledsPerButton: 2,
	pledType: -1,
	pledPin1: -1,
	pledPin2: -1,
	pledPin3: -1,
	pledPin4: -1,
	pledIndex1: -1,
	pledIndex2: -1,
	pledIndex3: -1,
	pledIndex4: -1,
	pledColor: '#00ff00',
};

const schema = yup.object().shape({
	brightnessMaximum : yup.number().required().positive().integer().min(0).max(255).label('Max Brightness'),
	brightnessSteps   : yup.number().required().positive().integer().min(1).max(10).label('Brightness Steps'),
	dataPin           : yup.number().required().validatePinWhenValue('dataPin'),
	ledFormat         : yup.number().required().positive().integer().min(0).max(3).label('LED Format'),
	ledLayout         : yup.number().required().positive().integer().min(0).max(2).label('LED Layout'),
	ledsPerButton     : yup.number().required().positive().integer().min(1).label('LEDs Per Pixel'),
	pledType          : yup.number().required().label('Player LED Type'),
	pledColor         : yup.string().label('RGB Player LEDs').validateColor(),
	pledPin1          : yup.number().label('PLED 1').validatePinWhenEqualTo('pledPins1', 'pledType', 0),
	pledPin2          : yup.number().label('PLED 2').validatePinWhenEqualTo('pledPins2', 'pledType', 0),
	pledPin3          : yup.number().label('PLED 3').validatePinWhenEqualTo('pledPins3', 'pledType', 0),
	pledPin4          : yup.number().label('PLED 4').validatePinWhenEqualTo('pledPins4', 'pledType', 0),
	pledIndex1        : yup.number().label('PLED Index 1').validateMinWhenEqualTo('pledType', 1, 0),
	pledIndex2        : yup.number().label('PLED Index 2').validateMinWhenEqualTo('pledType', 1, 0),
	pledIndex3        : yup.number().label('PLED Index 3').validateMinWhenEqualTo('pledType', 1, 0),
	pledIndex4        : yup.number().label('PLED Index 4').validateMinWhenEqualTo('pledType', 1, 0),
});

const getLedButtons = (buttonLabels, map, excludeNulls, swapTpShareLabels) => {
	return orderBy(
		Object
			.keys(BUTTONS[buttonLabels])
			.filter(p => p !== 'label' && p !== 'value')
			.filter(p => excludeNulls ? map[p] > -1 : true)
			.map(p => {
				let label = BUTTONS[buttonLabels][p];
				if (p === "S1" && swapTpShareLabels) {
					label = BUTTONS[buttonLabels]["A2"];
				}
				if (p === "A2" && swapTpShareLabels) {
					label = BUTTONS[buttonLabels]["S1"];
				}
				return ({ id: p, label: BUTTONS[buttonLabels][p], value: map[p] });
			}),
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

const FormContext = ({
	buttonLabels, ledButtonMap, ledFormat, pledColor, pledType, swapTpShareLabels,
	pledPin1, pledPin2, pledPin3, pledPin4,
	pledIndex1, pledIndex2, pledIndex3, pledIndex4,
	setDataSources
}) => {
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
				getLedButtons(buttonLabels, available, true, swapTpShareLabels),
				getLedButtons(buttonLabels, assigned, true, swapTpShareLabels),
			];

			data.pledIndex1 = data.pledType === 1 ? data.pledPin1 : -1;
			data.pledIndex2 = data.pledType === 1 ? data.pledPin2 : -1;
			data.pledIndex3 = data.pledType === 1 ? data.pledPin3 : -1;
			data.pledIndex4 = data.pledType === 1 ? data.pledPin4 : -1;

			setDataSources(dataSources);
			setValues(data);
		}
		fetchData();
		console.log('update');
	}, [buttonLabels, swapTpShareLabels]);

	useEffect(() => {
		if (!!ledFormat)
			setFieldValue('ledFormat', parseInt(ledFormat));
	}, [ledFormat, setFieldValue]);
	
	useEffect(() => {
		setFieldValue('ledButtonMap', ledButtonMap);
	}, [ledButtonMap, setFieldValue]);
	
	useEffect(() => {
		if (!!pledPin1)
			setFieldValue('pledPin1', parseInt(pledPin1));
	}, [pledPin1, setFieldValue]);
	useEffect(() => {
		if (!!pledPin2)
			setFieldValue('pledPin2', parseInt(pledPin2));
	}, [pledPin2, setFieldValue]);
	useEffect(() => {
		if (!!pledPin3)
			setFieldValue('pledPin3', parseInt(pledPin3));
	}, [pledPin3, setFieldValue]);
	useEffect(() => {
		if (!!pledPin4)
			setFieldValue('pledPin4', parseInt(pledPin4));
	}, [pledPin4, setFieldValue]);
	useEffect(() => {
		if (!!pledIndex1)
			setFieldValue('pledIndex1', parseInt(pledIndex1));
	}, [pledIndex1, setFieldValue]);
	useEffect(() => {
		if (!!pledIndex2)
			setFieldValue('pledIndex2', parseInt(pledIndex2));
	}, [pledIndex2, setFieldValue]);
	useEffect(() => {
		if (!!pledIndex3)
			setFieldValue('pledIndex3', parseInt(pledIndex3));
	}, [pledIndex3, setFieldValue]);
	useEffect(() => {
		if (!!pledIndex4)
			setFieldValue('pledIndex4', parseInt(pledIndex4));
	}, [pledIndex4, setFieldValue]);
	useEffect(() => {
		if (!!pledColor)
			setFieldValue('pledColor', pledColor);
	}, [pledColor, setFieldValue]);

	return null;
};

export default function LEDConfigPage() {
	const { buttonLabels, updateUsedPins } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');
	const [ledButtonMap, setLedButtonMap] = useState([]);
	const [dataSources, setDataSources] = useState([[], []]);
	const [colorPickerTarget, setColorPickerTarget] = useState(null);
	const [showPicker, setShowPicker] = useState(false);
	const [rgbLedStartIndex, setRgbLedStartIndex] = useState(0);

	const { buttonLabelType, swapTpShareLabels } = buttonLabels;

	const ledOrderChanged = (ledOrderArrays, ledsPerButton) => {
		if (ledOrderArrays.length === 2) {
			setLedButtonMap(getLedMap(buttonLabelType, ledOrderArrays[1]));
			setRgbLedStartIndex(ledOrderArrays[1].length * (ledsPerButton || 0));
			console.log('new start index: ', ledOrderArrays[1].length * (ledsPerButton || 0), ledOrderArrays);
		}
	};

	const ledsPerButtonChanged = (e, handleChange) => {
		const ledsPerButton = parseInt(e.target.value);
		setRgbLedStartIndex(dataSources[1].length * (ledsPerButton || 0));
		handleChange(e);
	};

	const setPledColor = (values, hexColor) => {
		values.pledColor = hexColor;
	};

	const showRgbPledPicker = (e) => {
		setColorPickerTarget(e.target);
		setShowPicker(true);
	};
	
	const toggleRgbPledPicker = (e) => {
		e.stopPropagation();
		setColorPickerTarget(e.target);
		setShowPicker(!showPicker);
	};

	const onSuccess = async (values) => {
		const data = { ...values };
		data.pledType = parseInt(values.pledType);
		if (data.pledColor)
			data.pledColor = hexToInt(values.pledColor);

		const success = await WebApi.setLedOptions(data);
		if (success)
			updateUsedPins();

		setSaveMessage(success ? 'Saved! Please Restart Your Device' : 'Unable to Save');
	};

	const onSubmit = (e, handleSubmit, setValues, values) => {
		setSaveMessage('');
		e.preventDefault();

		values.pledType = parseInt(values.pledType);

		// Consolidate PLED fields based on selected type
		switch (values.pledType) {
			case 0:
				// PLED pin already set
				break;
				
			case 1:
				values.pledPin1 = values.pledIndex1;
				values.pledPin2 = values.pledIndex2;
				values.pledPin3 = values.pledIndex3;
				values.pledPin4 = values.pledIndex4;
				break;

			default:
				values.pledPin1 = -1;
				values.pledPin2 = -1;
				values.pledPin3 = -1;
				values.pledPin4 = -1;
				break;
		}

		setValues(values);
		handleSubmit();
	};

	return (
		<Formik validationSchema={schema} onSubmit={onSuccess} initialValues={defaultValue}>
			{({
				handleSubmit,
				handleChange,
				handleBlur,
				setValues,
				values,
				touched,
				errors,
			}) => (
				<Form noValidate onSubmit={(e) => onSubmit(e, handleSubmit, setValues, values)}>
					<Section title="RGB LED Configuration">
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
								onChange={(e) => ledsPerButtonChanged(e, handleChange)}
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
					<Section title="Player LEDs (XInput)">
						<Form.Group as={Col}>
							<Row>
								<FormSelect
									label="Player LED Type"
									name="pledType"
									className="form-select-sm"
									groupClassName="col-sm-2 mb-3"
									value={values.pledType}
									error={errors.pledType}
									isInvalid={errors.pledType}
									onChange={handleChange}
								>
									<option value="-1" defaultValue={true}>Off</option>
									<option value="0">PWM</option>
									<option value="1">RGB</option>
								</FormSelect>
								<FormControl type="number"
									name="pledPin1"
									hidden={parseInt(values.pledType) !== 0}
									label={PLED_LABELS[0][values.pledType]}
									className="form-control-sm"
									groupClassName="col-sm-2 mb-3"
									value={values.pledPin1}
									error={errors.pledPin1}
									isInvalid={errors.pledPin1}
									onChange={handleChange}
									min={0}
								/>
								<FormControl type="number"
									name="pledPin2"
									hidden={parseInt(values.pledType) !== 0}
									label={PLED_LABELS[1][values.pledType]}
									className="form-control-sm"
									groupClassName="col-sm-2 mb-3"
									value={values.pledPin2}
									error={errors.pledPin2}
									isInvalid={errors.pledPin2}
									onChange={handleChange}
									min={0}
								/>
								<FormControl type="number"
									name="pledPin3"
									hidden={parseInt(values.pledType) !== 0}
									label={PLED_LABELS[2][values.pledType]}
									className="form-control-sm"
									groupClassName="col-sm-2 mb-3"
									value={values.pledPin3}
									error={errors.pledPin3}
									isInvalid={errors.pledPin3}
									onChange={handleChange}
									min={0}
								/>
								<FormControl type="number"
									name="pledPin4"
									hidden={parseInt(values.pledType) !== 0}
									label={PLED_LABELS[3][values.pledType]}
									className="form-control-sm"
									groupClassName="col-sm-2 mb-3"
									value={values.pledPin4}
									error={errors.pledPin4}
									isInvalid={errors.pledPin4}
									onChange={handleChange}
									min={0}
								/>
								<FormControl type="number"
									name="pledIndex1"
									hidden={parseInt(values.pledType) !== 1}
									label={PLED_LABELS[0][values.pledType]}
									className="form-control-sm"
									groupClassName="col-sm-2 mb-3"
									value={values.pledIndex1}
									error={errors.pledIndex1}
									isInvalid={errors.pledIndex1}
									onChange={handleChange}
									min={0}
								/>
								<FormControl type="number"
									name="pledIndex2"
									hidden={parseInt(values.pledType) !== 1}
									label={PLED_LABELS[1][values.pledType]}
									className="form-control-sm"
									groupClassName="col-sm-2 mb-3"
									value={values.pledIndex2}
									error={errors.pledIndex2}
									isInvalid={errors.pledIndex2}
									onChange={handleChange}
									min={0}
								/>
								<FormControl type="number"
									name="pledIndex3"
									hidden={parseInt(values.pledType) !== 1}
									label={PLED_LABELS[2][values.pledType]}
									className="form-control-sm"
									groupClassName="col-sm-2 mb-3"
									value={values.pledIndex3}
									error={errors.pledIndex3}
									isInvalid={errors.pledIndex3}
									onChange={handleChange}
									min={0}
								/>
								<FormControl type="number"
									name="pledIndex4"
									hidden={parseInt(values.pledType) !== 1}
									label={PLED_LABELS[3][values.pledType]}
									className="form-control-sm"
									groupClassName="col-sm-2 mb-3"
									value={values.pledIndex4}
									error={errors.pledIndex4}
									isInvalid={errors.pledIndex4}
									onChange={handleChange}
									min={0}
								/>
								<FormControl
									label="RGB PLED Color"
									hidden={parseInt(values.pledType) !== 1}
									name="pledColor"
									className="form-control-sm"
									groupClassName="col-sm-2 mb-3"
									value={values.pledColor}
									error={errors.pledColor}
									isInvalid={errors.pledColor}
									onBlur={handleBlur}
									onClick={toggleRgbPledPicker}
									onChange={(e) => {
										handleChange(e);
										setShowPicker(false);
									}}
								/>
								<ColorPicker
									name="pledColor"
									types={[{ value: values.pledColor }]}
									onChange={(c, e) => setPledColor(values, c)}
									onDismiss={(e) => setShowPicker(false)}
									placement="bottom"
									presetColors={LEDColors.map(c => ({ title: c.name, color: c.value}))}
									show={showPicker}
									target={colorPickerTarget}
								></ColorPicker>
							</Row>
							<p hidden={parseInt(values.pledType) !== 0}>For PWM LEDs, set each LED to a dedicated GPIO pin.</p>
							<p hidden={parseInt(values.pledType) !== 1}>For RGB LEDs, the indexes must be after the last LED button defined in <em>RGB LED Button Order</em> section and likely <strong>starts at index {rgbLedStartIndex}</strong>.</p>
						</Form.Group>
					</Section>
					<Section title="RGB LED Button Order">
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
							onChange={(a) => ledOrderChanged(a, values.ledsPerButton)}
						/>
					</Section>
					<Button type="submit">Save</Button>
					{saveMessage ? <span className="alert">{saveMessage}</span> : null}
					<FormContext {...{
						buttonLabels: buttonLabelType,
						swapTpShareLabels,
						ledButtonMap,
						setDataSources,
						ledFormat: values.ledFormat
					}} />
				</Form>
			)}
		</Formik>
	);
}
