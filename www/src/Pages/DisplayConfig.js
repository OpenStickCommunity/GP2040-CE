import React, { useEffect, useState, useRef } from 'react';
import { Button, Form, Row, Col } from 'react-bootstrap';
import { Formik, useFormikContext, ErrorMessage, Field } from 'formik';
import * as yup from 'yup';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import Section from '../Components/Section';
import WebApi from '../Services/WebApi';
import { chunk } from 'lodash';

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
	{ label: 'VLX', value: 9 }              // BUTTON_LAYOUT_VLXA
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
	{ label: 'VLX', value: 13 }		         // BUTTON_LAYOUT_VLXB
];

const SPLASH_MODES = [
	{ label: 'Enabled', value: 0 },			 // STATICSPLASH
	//	{ label: 'Close In', value: 1 },		 // CLOSEIN
	{ label: 'Disabled', value: 3 },         // NOSPLASH
];

const SPLASH_CHOICES = [
	{ label: 'Main', value: 0 },			 // MAIN
	{ label: 'X', value: 1 },		         // X
	{ label: 'Y', value: 2 },                // Y
	{ label: 'Z', value: 3 },                // Z
	{ label: 'Custom', value: 4 },           // CUSTOM
	{ label: 'Legacy', value: 5 },           // LEGACY
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
	splashMode: 3,
	splashChoice: 0,
	splashImage: Array(16*64).fill(0), // 128 columns represented by bytes so 16 and 64 rows
	invertSplash: false
};

let usedPins = [];

const schema = yup.object().shape({
	enabled: yup.number().label('Enabled?'),
	i2cAddress: yup.string().required().label('I2C Address'),
	// eslint-disable-next-line no-template-curly-in-string
	sdaPin: yup.number().required().min(-1).max(29).test('', '${originalValue} is already assigned!', (value) => true).label('SDA Pin'),
	// eslint-disable-next-line no-template-curly-in-string
	sclPin: yup.number().required().min(-1).max(29).test('', '${originalValue} is already assigned!', (value) => true).label('SCL Pin'),
	i2cBlock: yup.number().required().oneOf(I2C_BLOCKS.map(o => o.value)).label('I2C Block'),
	i2cSpeed: yup.number().required().label('I2C Speed'),
	flipDisplay: yup.number().label('Flip Display'),
	invertDisplay: yup.number().label('Invert Display'),
	buttonLayout: yup.number().required().oneOf(BUTTON_LAYOUTS.map(o => o.value)).label('Button Layout Left'),
	buttonLayoutRight: yup.number().required().oneOf(BUTTON_LAYOUTS_RIGHT.map(o => o.value)).label('Button Layout Right'),
	splashMode: yup.number().required().oneOf(SPLASH_MODES.map(o => o.value)).label('Splash Screen'),
	splashChoice: yup.number().required().oneOf(SPLASH_CHOICES.map(o => o.value)).label('Splash Screen Choice')
});

const FormContext = () => {
	const { values, setValues } = useFormikContext();

	useEffect(() => {
		async function fetchData() {
			const data = await WebApi.getDisplayOptions();
			usedPins = data.usedPins;
			const splashImageResponse = await WebApi.getSplashImage();
			data.splashImage = splashImageResponse.splashImage;
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
		if (!!values.splashMode)
			values.splashMode = parseInt(values.splashMode);
		if (!!values.splashChoice)
			values.splashChoice = parseInt(values.splashChoice);
	}, [values, setValues]);

	return null;
};

export default function DisplayConfigPage() {
	const [saveMessage, setSaveMessage] = useState('');

	const onSuccess = async (values) => {
		const success = await WebApi.setDisplayOptions(values).then(() => WebApi.setSplashImage(values));
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
							<FormSelect
								label="Splash Choice"
								name="splashChoice"
								className="form-select-sm"
								groupClassName="col-sm-3 mb-3"
								value={values.splashChoice}
								error={errors.splashChoice}
								isInvalid={errors.splashChoice}
								onChange={handleChange}
							>
								{SPLASH_CHOICES.map((o, i) => <option key={`splashChoice-option-${i}`} value={o.value}>{o.label}</option>)}
							</FormSelect>
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
