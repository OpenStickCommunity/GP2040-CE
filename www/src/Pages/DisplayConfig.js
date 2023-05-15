import React, { useContext, useEffect, useState, useRef } from 'react';
import { Button, Form, Row, Col, FormLabel } from 'react-bootstrap';
import { Formik, useFormikContext, Field } from 'formik';
import chunk from 'lodash/chunk';
import * as yup from 'yup';

import { AppContext } from '../Contexts/AppContext';
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

const BUTTON_LAYOUTS = [
	{ label: 'Stick', value: 0 },			// BUTTON_LAYOUT_STICK
	{ label: 'Stickless', value: 1 },		// BUTTON_LAYOUT_STICKLESS
	{ label: 'Buttons Angled', value: 2 },  // BUTTON_LAYOUT_BUTTONS_ANGLED
	{ label: 'Buttons Basic', value: 3 },   // BUTTON_LAYOUT_BUTTONS_BASIC
	{ label: 'Keyboard Angled', value: 4 }, // BUTTON_LAYOUT_KEYBOARD_ANGLED
	{ label: 'Keyboard', value: 5 },        // BUTTON_LAYOUT_KEYBOARDA
	{ label: 'Dancepad', value: 6 },        // BUTTON_LAYOUT_DANCEPADA
	{ label: 'Twinstick', value: 7 },       // BUTTON_LAYOUT_TWINSTICKA
	{ label: 'Blank', value: 8 },           // BUTTON_LAYOUT_BLANKA
	{ label: 'VLX', value: 9 },              // BUTTON_LAYOUT_VLXA
	{ label: 'Fightboard', value: 10 },              // BUTTON_LAYOUT_FIGHTBOARD_STICK
	{ label: 'Fightboard Mirrored', value: 11 },     // BUTTON_LAYOUT_FIGHTBOARD_MIRRORED
	{ label: 'Custom', value: 12 }              // BUTTON_LAYOUT_CUSTOM
];

const BUTTON_LAYOUTS_RIGHT = [
	{ label: 'Arcade', value: 0 },			 // BUTTON_LAYOUT_ARCADE
	{ label: 'Stickless', value: 1 },        // BUTTON_LAYOUT_STICKLESSB
	{ label: 'Buttons Angled', value: 2 },   // BUTTON_LAYOUT_BUTTONS_ANGLEDB
	{ label: 'Viewlix', value: 3 },			 // BUTTON_LAYOUT_VEWLIX
	{ label: 'Viewlix 7', value: 4 },        // BUTTON_LAYOUT_VEWLIX7
	{ label: 'Capcom', value: 5 },           // BUTTON_LAYOUT_CAPCOM
	{ label: 'Capcom 6', value: 6 },		 // BUTTON_LAYOUT_CAPCOM6
	{ label: 'Sega 2P', value: 7 },			 // BUTTON_LAYOUT_SEGA2P
	{ label: 'Noir 8', value: 8 },			 // BUTTON_LAYOUT_NOIR8
	{ label: 'Keyboard', value: 9 },		 // BUTTON_LAYOUT_KEYBOARDB
	{ label: 'Dancepad', value: 10 },		 // BUTTON_LAYOUT_DANCEPADB
	{ label: 'Twinstick', value: 11 },	     // BUTTON_LAYOUT_TWINSTICKB
	{ label: 'Blank', value: 12 },		     // BUTTON_LAYOUT_BLANKB
	{ label: 'VLX', value: 13 },		         // BUTTON_LAYOUT_VLXB
	{ label: 'Fightboard', value: 14 },				// BUTTON_LAYOUT_FIGHTBOARD
	{ label: 'Fightboard Mirrored', value: 15 },	// BUTTON_LAYOUT_FIGHTBOARD_STICK_MIRRORED
	{ label: 'Custom', value: 16 }		         // BUTTON_LAYOUT_CUSTOM
];

const SPLASH_MODES = [
	{ label: 'Enabled', value: 0 },			 // STATICSPLASH
	//	{ label: 'Close In', value: 1 },		 // CLOSEIN
	{ label: 'Disabled', value: 3 },         // NOSPLASH
];

const SPLASH_DURATION_CHOICES = [
	{ label: 'Default', value: 0 },
	{ label: '5 seconds', value: 5000 },
	{ label: '10 seconds', value: 10000 },
	{ label: '30 seconds', value: 30000 },
	{ label: 'Always ON', value: -1 }
];

const DISPLAY_SAVER_TIMEOUT_CHOICES = [
	{ label: 'Off', value: 0 },
	{ label: '1 minute', value: 1 },
	{ label: '2 minutes', value: 2 },
	{ label: '5 minutes', value: 5 },
	{ label: '10 minutes', value: 10 },
	{ label: '20 minutes', value: 20 },
	{ label: '30 minutes', value: 30 },
];

const DISPLAY_FLIP_MODES = [
 { label: "None", value: 0 },
 { label: "Flip", value: 1 },
 { label: "Mirror", value: 2 },
 { label: "Flip and Mirror", value: 3 },
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
	buttonLayoutRight: 3,
	splashDuration: 0,
	splashMode: 3,
	splashImage: Array(16*64).fill(0), // 128 columns represented by bytes so 16 and 64 rows
	invertSplash: false,
	buttonLayoutCustomOptions: {
		params: {
			layout: 0,
			startX: 8,
			startY: 28,
			buttonRadius: 8,
			buttonPadding: 2
		},
		paramsRight: {
			layout: 3,
			startX: 8,
			startY: 28,
			buttonRadius: 8,
			buttonPadding: 2
		}
	},
	displaySaverTimeout: 0,
};

const buttonLayoutSchema = yup.number().required().oneOf(BUTTON_LAYOUTS.map(o => o.value)).label('Button Layout Left')
const buttonLayoutRightSchema = yup.number().required().oneOf(BUTTON_LAYOUTS_RIGHT.map(o => o.value)).label('Button Layout Right')

const schema = yup.object().shape({
	enabled: yup.number().label('Enabled?'),
	i2cAddress: yup.string().required().label('I2C Address'),
	sdaPin: yup.number().label('SDA Pin').validatePinWhenValue('sdaPin'),
	sclPin: yup.number().label('SCL Pin').validatePinWhenValue('sclPin'),
	i2cBlock: yup.number().required().oneOf(I2C_BLOCKS.map(o => o.value)).label('I2C Block'),
	i2cSpeed: yup.number().required().label('I2C Speed'),
	flipDisplay: yup.number().oneOf(DISPLAY_FLIP_MODES.map(o => o.value)).label('Flip Display'),
	invertDisplay: yup.number().label('Invert Display'),
	buttonLayout: buttonLayoutSchema,
	buttonLayoutRight: buttonLayoutRightSchema,
	splashMode: yup.number().required().oneOf(SPLASH_MODES.map(o => o.value)).label('Splash Screen'),
	buttonLayoutCustomOptions: yup.object().shape({
		params: yup.object().shape({
			layout: buttonLayoutSchema,
			startX: yup.number().required().min(0).max(128).label('Start X'),
			startY: yup.number().required().min(0).max(64).label('Start Y'),
			buttonRadius: yup.number().required().min(0).max(20).label('Button Radius'),
			buttonPadding: yup.number().required().min(0).max(20).label('Button Padding')
		}),
		paramsRight: yup.object().shape({
			layout: buttonLayoutRightSchema,
			startX: yup.number().required().min(0).max(128).label('Start X'),
			startY: yup.number().required().min(0).max(64).label('Start Y'),
			buttonRadius: yup.number().required().min(0).max(20).label('Button Radius'),
			buttonPadding: yup.number().required().min(0).max(20).label('Button Padding')
		})
	}),
	splashDuration: yup.number().required().min(0).label('Splash Duration'),
	displaySaverTimeout: yup.number().required().min(0).label('Display Saver'),
});

const FormContext = () => {
	const { values, setValues } = useFormikContext();

	useEffect(() => {
		async function fetchData() {
			const data = await WebApi.getDisplayOptions();
			const splashImageResponse = await WebApi.getSplashImage();
			data.splashImage = splashImageResponse.splashImage;
			setValues(data);
		}
		fetchData();
	}, [setValues]);

	useEffect(() => {
		async function setDisplayOptions() {
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
			if (!!values.splashMode)
				values.splashMode = parseInt(values.splashMode);
			if (!!values.splashChoice)
				values.splashChoice = parseInt(values.splashChoice);
			if (!!values.splashDuration)
				values.splashDuration = parseInt(values.splashDuration);

			await WebApi.setDisplayOptions(values, true);
		}

		setDisplayOptions();
	}, [values, setValues]);

	useEffect(() => {
		async function setSplashImage() {
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
			if (!!values.splashMode)
				values.splashMode = parseInt(values.splashMode);
			if (!!values.splashChoice)
				values.splashChoice = parseInt(values.splashChoice);

			await WebApi.setDisplayOptions(values, true);
		}

		setSplashImage();
	}, [values.splashImage]);

	return null;
};

const isButtonLayoutCustom = (values) => values.buttonLayout === 12 || values.buttonLayoutRight === 16

export default function DisplayConfigPage() {
	const { updateUsedPins } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');

	const onSuccess = async (values) => {
		const success = await WebApi.setDisplayOptions(values, false)
			.then(() => WebApi.setSplashImage(values));

		if (success)
			await updateUsedPins();

		setSaveMessage(success ? 'Saved! Please Restart Your Device' : 'Unable to Save');

	};

	const onChangeCanvas = (base64, form, field) => {
		return form.setFieldValue(field.name, base64)
	}

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
								{DISPLAY_FLIP_MODES.map((o, i) => <option key={`flipDisplay-option-${i}`} value={o.value}>{o.label}</option>)}
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
								{BUTTON_LAYOUTS.map((o, i) => <option key={`buttonLayout-option-${i}`} value={o.value}>{o.label}</option>)}
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
								{BUTTON_LAYOUTS_RIGHT.map((o, i) => <option key={`buttonLayoutRight-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
							<FormSelect
								label="Splash Mode"
								name="splashMode"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.splashMode}
								error={errors.splashMode}
								isInvalid={errors.splashMode}
								onChange={handleChange}
							>
								{SPLASH_MODES.map((o, i) => <option key={`splashMode-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
						</Row>
						{isButtonLayoutCustom(values) && <Row className="mb-3">
							<FormLabel>Custom Button Layout Params</FormLabel>
							<Col sm="6">
								<Form.Group as={Row}
									name="buttonLayoutCustomOptions">
									<Form.Label column>Layout Left</Form.Label>
									<FormSelect
										name="buttonLayoutCustomOptions.params.layout"
										className="form-select-sm"
										groupClassName="col-sm-10 mb-1"
										value={values.buttonLayoutCustomOptions.params.layout}
										onChange={handleChange}
									>
										{BUTTON_LAYOUTS.slice(0, -1).map((o, i) => <option key={`buttonLayout-option-${i}`} value={o.value}>{o.label}</option>)}
									</FormSelect>
								</Form.Group>
								<Form.Group as={Row}>
									<Form.Label column>Start X</Form.Label>
									<Col sm="10">
										<Field column className="mb-1" name="buttonLayoutCustomOptions.params.startX"
												type="number" as={Form.Control}/>
									</Col>
								</Form.Group>
								<Form.Group as={Row}>
									<Form.Label column>Start Y</Form.Label>
									<Col sm="10">
										<Field column className="mb-1" name="buttonLayoutCustomOptions.params.startY"
												type="number" as={Form.Control}/>
									</Col>
								</Form.Group>
									<Form.Group as={Row}>
									<Form.Label column>Button Radius</Form.Label>
									<Col sm="10">
										<Field column className="mb-1" name="buttonLayoutCustomOptions.params.buttonRadius"
												type="number" as={Form.Control}/>
									</Col>
								</Form.Group>
								<Form.Group as={Row}>
									<Form.Label column>Button Padding</Form.Label>
									<Col sm="10">
										<Field column className="mb-1" name="buttonLayoutCustomOptions.params.buttonPadding"
												type="number" as={Form.Control}/>
									</Col>
								</Form.Group>
							</Col>
							<Col sm="6">
								<Form.Group as={Row}>
									<Form.Label column>Layout Right</Form.Label>
								<FormSelect
									name="buttonLayoutCustomOptions.paramsRight.layout"
									className="form-select-sm"
									groupClassName="col-sm-10 mb-1"
									value={values.buttonLayoutCustomOptions.paramsRight.layout}
									onChange={handleChange}
								>
									{BUTTON_LAYOUTS_RIGHT.slice(0, -1).map((o, i) => <option key={`buttonLayoutRight-option-${i}`} value={o.value}>{o.label}</option>)}
								</FormSelect>
								</Form.Group>
								<Form.Group as={Row}>
									<Form.Label column>Start X</Form.Label>
									<Col sm="10">
										<Field column className="mb-1" name="buttonLayoutCustomOptions.paramsRight.startX"
												type="number" as={Form.Control}/>
									</Col>
								</Form.Group>
								<Form.Group as={Row}>
									<Form.Label column>Start Y</Form.Label>
									<Col sm="10">
										<Field column className="mb-1" name="buttonLayoutCustomOptions.paramsRight.startY"
												type="number" as={Form.Control}/>
									</Col>
								</Form.Group>
									<Form.Group as={Row}>
									<Form.Label column>Button Radius</Form.Label>
									<Col sm="10">
										<Field column className="mb-1" name="buttonLayoutCustomOptions.paramsRight.buttonRadius"
												type="number" as={Form.Control}/>
									</Col>
								</Form.Group>
								<Form.Group as={Row}>
									<Form.Label column>Button Padding</Form.Label>
									<Col sm="10">
										<Field column className="mb-1" name="buttonLayoutCustomOptions.paramsRight.buttonPadding"
												type="number" as={Form.Control}/>
									</Col>
								</Form.Group>
							</Col>
						</Row>}
						<Row className="mb-3">
							<FormControl type="number"
								label="Splash Duration (seconds, 0 for Always On)"
								name="splashDuration"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.splashDuration}
								error={errors.splashDuration}
								isInvalid={errors.splashDuration}
								onChange={handleChange}
								min={0}
							/>
							<FormControl type="number"
								label="Display Saver Timeout (minutes)"
								name="displaySaverTimeout"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.displaySaverTimeout}
								error={errors.displaySaverTimeout}
								isInvalid={errors.displaySaverTimeout}
								onChange={handleChange}
								min={0}
							/>
						</Row>
						<Row>
							<Field name="splashImage">
								{({
									field, // { name, value, onChange, onBlur }
									form, // also values, setXXXX, handleXXXX, dirty, isValid, status, etc.
									meta,
								}) => (
									<div className="mt-3">
										<Canvas onChange={base64 => onChangeCanvas(base64, form, field)} value={field.value} />
									</div>
								)}
							</Field>
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

const Canvas = ({value: bitsArray, onChange}) => {
	const [image, setImage] = useState(null);
	const [canvasContext, setCanvasContext] = useState(null);
	const [inverted, setInverted] = useState(false);
	const canvasRef = useRef();

	useEffect(() => {
		setCanvasContext(canvasRef.current.getContext('2d'));
	}, []);

	// image to bitsArray (binary)
	useEffect(() => {
		if (canvasContext == null || image == null) return

		const ctxWidth = canvasContext.canvas.width,
			ctxHeight = canvasContext.canvas.height;
		const imgWidth = image.width,
			imgHeight = image.height;
		const ratioWidth = imgWidth / ctxWidth,
			ratioHeight = imgHeight / ctxHeight,
			ratioAspect = ratioWidth > 1 ? ratioWidth : ratioHeight > 1 ? ratioHeight : 1;
		const newWidth = imgWidth / ratioAspect,
			newHeight = imgHeight / ratioAspect;
		const offsetX = (ctxWidth / 2) - (newWidth / 2),
			offsetY = (ctxHeight / 2) - (newHeight / 2);
		canvasContext.clearRect(0, 0, canvasRef.current.width, canvasRef.current.height);
		canvasContext.drawImage(image, offsetX, offsetY, newWidth, newHeight);

		var imgPixels = canvasContext.getImageData(0, 0, canvasContext.canvas.width, canvasContext.canvas.height);

		// Convert to monochrome
		for (var i = 0; i < imgPixels.data.length; i = i + 4) {
			var avg = (imgPixels.data[i] + imgPixels.data[i + 1] + imgPixels.data[i + 2]) / 3;
			if (avg > 123) avg = 255
			else avg = 0;
			imgPixels.data[i] = avg;
			imgPixels.data[i + 1] = avg;
			imgPixels.data[i + 2] = avg;
		}

		// Pick only first channel because all of them are same
		const bitsArray = chunk([...(new Uint8Array(imgPixels.data))]
			.filter((x, y) => (y % 4) === 0), 8)
			.map(chunks => chunks.reduce((acc, curr, i) => {
				return acc + ((curr === 255 ? 1 : 0) << (7 - i))
			}, 0));

		onChange(bitsArray.map(a => inverted ? 255 - a : a));

	}, [image, canvasContext]);

	// binary to RGBA
	useEffect(() => {
		if (canvasContext == null) return;

		const w = canvasContext.canvas.width;
		const h = canvasContext.canvas.height;
		const rgbToRgba = [];

		// expand bytes to individual binary bits and then bits to 255 or 0, because monochrome
		const bitsArrayArray = bitsArray.flatMap((a) => {
			const bits = a.toString(2).split('').map(Number);
			const full = Array(8 - bits.length).fill(0).concat(bits);
			return full.map(a => a === 1 ? 255 : 0)
		})

		// fill up the new array as RGBA
		bitsArrayArray.forEach((x) => {
			rgbToRgba.push(x);
			rgbToRgba.push(x);
			rgbToRgba.push(x);
			rgbToRgba.push(255);
		})
		const imageDataCopy = new ImageData(
			new Uint8ClampedArray(rgbToRgba),
			w,
			h
		)
		canvasContext.putImageData(imageDataCopy, 0, 0, 0, 0, w, h);
	}, [bitsArray, canvasContext])

	const onImageAdd = (ev) => {
		var file = ev.target.files[0];
		var fr = new FileReader();
		fr.onload = () => {
			const img = new Image();
			img.onload = () => {
				setImage(img);
			};
			img.src = fr.result;
		};
		fr.readAsDataURL(file);
	}

	const toggleInverted = () => {
		onChange(bitsArray.map(a => 255 - a))
		setInverted(!inverted);
	}

	return (<div style={{ display: "flex", alignItems: "center" }}>
		<canvas ref={canvasRef} width="128" height="64" style={{ background: 'black' }} />
		<div style={{ marginLeft: "11px" }}>
			<input type="file" id="image-input" accept="image/jpeg, image/png, image/jpg" onChange={onImageAdd} />
			<br/>
			<input type="checkbox" checked={inverted} onChange={toggleInverted}/> Invert
			{/* <ErrorMessage name="splashImage" /> */}
		</div>
	</div>)
}
