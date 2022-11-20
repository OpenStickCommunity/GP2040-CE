import React, { useEffect, useState } from 'react';
import { Button, Form, Row, Col } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import * as yup from 'yup';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import Section from '../Components/Section';
import WebApi from '../Services/WebApi';

const ON_OFF_OPTIONS = [
	{ label: 'Disabled', value: 0 },
	{ label: 'Enabled', value: 1 },
];

const I2C_BLOCKS = [
	{ label: 'i2c0', value: 0 },
	{ label: 'i2c1', value: 1 },
];

const BUTTON_LAYOUT = [
	{ label: 'Stick', value: 0 },			// BUTTON_LAYOUT_STICK
	{ label: 'Stickless', value: 1 },		// BUTTON_LAYOUT_STICKLESS
	{ label: 'Buttons Angled', value: 2 },  // BUTTON_LAYOUT_BUTTONS_ANGLED
	{ label: 'Buttons Basic', value: 3 },   // BUTTON_LAYOUT_BUTTONS_BASIC
	{ label: 'Keyboard Angled', value: 4 }, // BUTTON_LAYOUT_KEYBOARD_ANGLED
	{ label: 'Keyboard', value: 5 },        // BUTTON_LAYOUT_KEYBOARDA
	{ label: 'Dancepad', value: 6 },        // BUTTON_LAYOUT_DANCEPADA
	{ label: 'Twinstick', value: 7 },       // BUTTON_LAYOUT_TWINSTICKA
	{ label: 'Blank', value: 8 },           // BUTTON_LAYOUT_BLANKA
	{ label: 'VLX', value: 9 }              // BUTTON_LAYOUT_VLXA
];

const BUTTON_LAYOUT_RIGHT = [
	{ label: 'Arcade', value: 0 },			 // BUTTON_LAYOUT_ARCADE
	{ label: 'Stickless', value: 1 },        // BUTTON_LAYOUT_STICKLESSB
	{ label: 'Buttons Anbled', value: 2 },   // BUTTON_LAYOUT_BUTTONS_ANGLEDB
	{ label: 'Viewlix', value: 3 },			 // BUTTON_LAYOUT_VEWLIX
	{ label: 'Viewlix 7', value: 4 },        // BUTTON_LAYOUT_VEWLIX7
	{ label: 'Capcom', value: 5 },           // BUTTON_LAYOUT_CAPCOM
	{ label: 'Capcom 6', value: 6 },		 // BUTTON_LAYOUT_CAPCOM6
	{ label: 'Sega 2P', value: 7 },			 // BUTTON_LAYOUT_SEGA2P
	{ label: 'Noir 8', value: 8 },			 // BUTTON_LAYOUT_NOIR8
	{ label: 'Keyboard', value: 9 },		 // BUTTON_LAYOUT_KEYBOARDB
	{ label: 'Dancepad', value: 10 },		 // BUTTON_LAYOUT_DANCEPADB
	{ label: 'Thumbstick', value: 11 },	     // BUTTON_LAYOUT_TWINSTICKB
	{ label: 'Blank', value: 12 },		     // BUTTON_LAYOUT_BLANKB
	{ label: 'VLX', value: 13 }		         // BUTTON_LAYOUT_VLXB
];

const defaultValues = {
	enabled: false,
	sdaPin: -1,
	sclPin: -1,
	i2cAddress: '0x3C',
	i2cBlock: 0,
	i2cSpeed: 400000,
	flipDisplay: false,
	invertDisplay: false,
	buttonLayout: 0,
	buttonLayoutRight: 0
};

let usedPins = [];

const schema = yup.object().shape({
	enabled: yup.number().label('Enabled?'),
	i2cAddress: yup.string().required().label('I2C Address'),
	// eslint-disable-next-line no-template-curly-in-string
	sdaPin: yup.number().required().min(-1).max(29).test('', '${originalValue} is already assigned!', (value) => usedPins.indexOf(value) === -1).label('SDA Pin'),
	// eslint-disable-next-line no-template-curly-in-string
	sclPin: yup.number().required().min(-1).max(29).test('', '${originalValue} is already assigned!', (value) => usedPins.indexOf(value) === -1).label('SCL Pin'),
	i2cBlock: yup.number().required().oneOf(I2C_BLOCKS.map(o => o.value)).label('I2C Block'),
	i2cSpeed: yup.number().required().label('I2C Speed'),
	flipDisplay: yup.number().label('Flip Display'),
	invertDisplay: yup.number().label('Invert Display'),
	buttonLayout: yup.number().required().oneOf(BUTTON_LAYOUT.map(o => o.value)).label('Button Layout Left'),
	buttonLayoutRight: yup.number().required().oneOf(BUTTON_LAYOUT_RIGHT.map(o => o.value)).label('Button Layout Right'),
});

const FormContext = () => {
	const { values, setValues } = useFormikContext();

	useEffect(() => {
		async function fetchData() {
			const data = await WebApi.getDisplayOptions();
			usedPins = data.usedPins;
			setValues(data);
		}
		fetchData();
	}, [setValues]);

	useEffect(() => {
		if (!!values.enabled)
			values.enabled = parseInt(values.enabled);
		if (!!values.i2cBlock)
			values.i2cBlock = parseInt(values.i2cBlock);
		if (!!values.flipDisplay)
			values.flipDisplay = parseInt(values.flipDisplay);
		if (!!values.invertDisplay)
			values.invertDisplay = parseInt(values.invertDisplay);
		if (!!values.buttonLayout)
			values.buttonLayout = parseInt(values.buttonLayout);
		if (!!values.buttonLayoutRight)
			values.buttonLayoutRight = parseInt(values.buttonLayoutRight);
	}, [values, setValues]);

	return null;
};

export default function DisplayConfigPage() {
	const [saveMessage, setSaveMessage] = useState('');

	const onSuccess = async (values) => {
		const success = WebApi.setDisplayOptions(values);
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
				<Section title="Display Configuration">
					<p>
						A monochrome display can be used to show controller status and button activity. Ensure your display module
						has the following attributes:
					</p>
					<ul>
						<li>Monochrome display with 128x64 resolution</li>
						<li>Uses I2C with a SSD1306, SH1106, SH1107 or other compatible display IC</li>
						<li>Supports 3.3v operation</li>
					</ul>
					<p>
						Use these tables to determine which I2C block to select based on the configured SDA and SCL pins:
					</p>
					<Row>
						<Col>
							<table className="table table-sm mb-4">
								<thead>
									<tr>
										<th>SDA/SCL Pins</th>
										<th>I2C Block</th>
									</tr>
								</thead>
								<tbody>
									<tr><td>0/1</td><td>i2c0</td></tr>
									<tr><td>2/3</td><td>i2c1</td></tr>
									<tr><td>4/5</td><td>i2c0</td></tr>
									<tr><td>6/7</td><td>i2c1</td></tr>
									<tr><td>8/9</td><td>i2c0</td></tr>
									<tr><td>10/11</td><td>i2c1</td></tr>
								</tbody>
							</table>
						</Col>
						<Col>
							<table className="table table-sm mb-4">
								<thead>
									<tr>
										<th>SDA/SCL Pins</th>
										<th>I2C Block</th>
									</tr>
								</thead>
								<tbody>
									<tr><td>12/13</td><td>i2c0</td></tr>
									<tr><td>14/15</td><td>i2c1</td></tr>
									<tr><td>16/17</td><td>i2c0</td></tr>
									<tr><td>18/19</td><td>i2c1</td></tr>
									<tr><td>20/21</td><td>i2c0</td></tr>
									<tr><td>26/27</td><td>i2c1</td></tr>
								</tbody>
							</table>
						</Col>
					</Row>
					<Form noValidate onSubmit={handleSubmit}>
						<Row>
							<FormSelect
								label="Use Display"
								name="enabled"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.enabled}
								error={errors.enabled}
								isInvalid={errors.enabled}
								onChange={handleChange}
							>
								{ON_OFF_OPTIONS.map((o, i) => <option key={`enabled-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
							<FormSelect
								label="I2C Block"
								name="i2cBlock"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.i2cBlock}
								error={errors.i2cBlock}
								isInvalid={errors.i2cBlock}
								onChange={handleChange}
							>
								{I2C_BLOCKS.map((o, i) => <option key={`i2cBlock-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
							<FormControl type="number"
								label="SDA Pin"
								name="sdaPin"
								className="form-control-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.sdaPin}
								error={errors.sdaPin}
								isInvalid={errors.sdaPin}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
							<FormControl type="number"
								label="SCL Pin"
								name="sclPin"
								className="form-control-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.sclPin}
								error={errors.sclPin}
								isInvalid={errors.sclPin}
								onChange={handleChange}
								min={-1}
								max={29}
							/>
						</Row>
						<Row className="mb-3">
							<FormControl type="text"
								label="I2C Address"
								name="i2cAddress"
								className="form-control-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.i2cAddress}
								error={errors.i2cAddress}
								isInvalid={errors.i2cAddress}
								onChange={handleChange}
								maxLength={4}
							/>
							<FormControl type="number"
								label="I2C Speed"
								name="i2cSpeed"
								className="form-control-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.i2cSpeed}
								error={errors.i2cSpeed}
								isInvalid={errors.i2cSpeed}
								onChange={handleChange}
								min={100000}
							/>
							<FormSelect
								label="Flip Display"
								name="flipDisplay"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.flipDisplay}
								error={errors.flipDisplay}
								isInvalid={errors.flipDisplay}
								onChange={handleChange}
							>
								{ON_OFF_OPTIONS.map((o, i) => <option key={`flipDisplay-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
							<FormSelect
								label="Invert Display"
								name="invertDisplay"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.invertDisplay}
								error={errors.invertDisplay}
								isInvalid={errors.invertDisplay}
								onChange={handleChange}
							>
								{ON_OFF_OPTIONS.map((o, i) => <option key={`invertDisplay-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
						</Row>
						<Row className="mb-3">
						<FormSelect
								label="Button Layout (Left)"
								name="buttonLayout"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.buttonLayout}
								error={errors.buttonLayout}
								isInvalid={errors.buttonLayout}
								onChange={handleChange}
							>
								{BUTTON_LAYOUT.map((o, i) => <option key={`buttonLayout-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
							<FormSelect
								label="Button Layout (Right)"
								name="buttonLayoutRight"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.buttonLayoutRight}
								error={errors.buttonLayoutRight}
								isInvalid={errors.buttonLayoutRight}
								onChange={handleChange}
							>
								{BUTTON_LAYOUT_RIGHT.map((o, i) => <option key={`buttonLayoutRight-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
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
