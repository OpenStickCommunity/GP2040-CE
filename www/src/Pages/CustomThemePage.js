import React, { useContext, useEffect, useState } from 'react';
import Button from 'react-bootstrap/Button';
import Col from 'react-bootstrap/Col';
import Container from 'react-bootstrap/Container';
import Fade from 'react-bootstrap/Fade';
import Form from 'react-bootstrap/Form';
import FormCheck from 'react-bootstrap/FormCheck';
import Modal from 'react-bootstrap/Modal';
import Overlay from 'react-bootstrap/Overlay';
import Popover from 'react-bootstrap/Popover';
import Row from 'react-bootstrap/Row';
import Stack from 'react-bootstrap/Stack';
import { SketchPicker } from '@hello-pangea/color-picker';
import Gradient from "javascript-color-gradient";

import { AppContext } from '../Contexts/AppContext';
import FormSelect from '../Components/FormSelect';
import Section from '../Components/Section';
import WebApi from '../Services/WebApi';
import { BUTTONS, MAIN_BUTTONS, AUX_BUTTONS, KEYBOARD_LAYOUT, STICK_LAYOUT, STICKLESS_LAYOUT } from '../Data/Buttons';
import LEDColors from '../Data/LEDColors';

import './CustomThemePage.scss';

const BUTTON_LAYOUTS = [
	{
		label: 'Stick',
		value: 0,
		stickLayout: 'standard',
		matrix: STICK_LAYOUT,
	},
	{
		label: 'Stickless',
		value: 1,
		stickLayout: 'stickless',
		matrix: STICKLESS_LAYOUT,
	},
	{
		label: 'WASD',
		value: 2,
		stickLayout: 'keyboard',
		matrix: KEYBOARD_LAYOUT,
	},
];

const defaultCustomTheme = Object.keys(BUTTONS.gp2040)
	?.filter(p => p !== 'label' && p !== 'value')
	.reduce((a, p) => {
		a[p] = { normal: '#000000', pressed: '#000000' };
		return a;
	}, {});

defaultCustomTheme['ALL'] = { normal: '#000000', pressed: '#000000' };
defaultCustomTheme['GRADIENT NORMAL'] = { normal: '#00ffff', pressed: '#ff00ff' };
defaultCustomTheme['GRADIENT PRESSED'] = { normal: '#ff00ff', pressed: '#00ffff' };

const specialButtons = ['ALL', 'GRADIENT NORMAL', 'GRADIENT PRESSED'];

const LEDButton = ({ id, name, buttonType, buttonColor, buttonPressedColor, className, labelUnder, onClick, ...props }) => {
	const [pressed, setPressed] = useState(false);

	const handlePressedShow = (e) => {
		// Show pressed state on right-click
		if (e.button === 2)
			setPressed(true);
	};

	const handlePressedHide = (e) => {
		// Revert to normal state
		setPressed(false);
	};

	return (
		<div
			className={`led-button ${className}`}
			style={{ backgroundColor: pressed ? buttonPressedColor : buttonColor }}
			onClick={onClick}
			onMouseDown={(e) => handlePressedShow(e)}
			onMouseUp={(e) => handlePressedHide(e)}
			onMouseLeave={(e) => handlePressedHide(e)}
			onContextMenu={(e) => e.preventDefault()}
		>
			<span className={`button-label ${labelUnder ? 'under' : ''}`}>{name}</span>
		</div>
	);
};

const ledColors = LEDColors.map(c => ({ title: c.name, color: c.value}));
const customColors = (colors) => colors.map(c => ({ title: c, color: c }));

const CustomThemePage = () => {
	const {
		buttonLabels,
		gradientNormalColor1,
		gradientNormalColor2,
		gradientPressedColor1,
		gradientPressedColor2,
		savedColors,
		setGradientNormalColor1,
		setGradientNormalColor2,
		setGradientPressedColor1,
		setGradientPressedColor2,
		setSavedColors
	} = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');
	const [ledLayout, setLedLayout] = useState(0);
	const [pickerType, setPickerType] = useState(null);
	const [selectedButton, setSelectedButton] = useState('');
	const [selectedColor, setSelectedColor] = useState('#000000');
	const [hasCustomTheme, setHasCustomTheme] = useState(false);
	const [customTheme, setCustomTheme] = useState({ ...defaultCustomTheme });
	const [ledOverlayTarget, setLedOverlayTarget] = useState(document.body);
	const [pickerVisible, setPickerVisible] = useState(false);
	const [modalVisible, setModalVisible] = useState(false);
	const [presetColors, setPresetColors] = useState([...ledColors, ...customColors(savedColors)]);

	const confirmClearAll = () => {
		setSelectedColor(null);

		// Reset all custom LEDs
		Object.keys(customTheme).forEach((b, i) => {
			Object.keys(customTheme[b]).forEach((s, i) => {
				customTheme[b][s] = '#000000';
			});
		});

		setCustomTheme(customTheme);
		setModalVisible(false);
	};

	const deleteCurrentColor = () => {
		const colorIndex = savedColors.indexOf(selectedColor.hex);
		if (colorIndex < 0)
			return;

		const newColors = [...savedColors];
		newColors.splice(colorIndex, 1);
		setSavedColors(newColors);
		setPresetColors([...ledColors, ...customColors(newColors)]);
	};

	const handleLedColorClick = (pickerType) => {
		setSelectedColor(customTheme[selectedButton][pickerType]);
		setPickerType({ type: pickerType, button: selectedButton });
	};

	const handleLedColorChange = (c) => {
		if (selectedButton) {
			if (selectedButton === 'ALL') {
				Object.keys(customTheme).filter(b => b === 'ALL' || specialButtons.indexOf(b) === -1).forEach(p => customTheme[p][pickerType.type] = c.hex);
			}
			else if (selectedButton === 'GRADIENT NORMAL' || selectedButton === 'GRADIENT PRESSED') {
				customTheme[selectedButton][pickerType.type] = c.hex;

				// Apply the gradient across action buttons only, 7-8 columns
				const matrix = BUTTON_LAYOUTS[ledLayout].matrix;
				const count = matrix.length;

				let steps = [customTheme[selectedButton].normal];
				steps.push(...new Gradient()
					.setColorGradient(customTheme[selectedButton].normal, customTheme[selectedButton].pressed)
					.setMidpoint(count - 2)
					.getColors()
				);
				steps.push(customTheme[selectedButton].pressed);

				if (selectedButton === 'GRADIENT NORMAL') {
					matrix.forEach((r, i) => r.filter(b => !!b).forEach(b => customTheme[b] = { normal: steps[i], pressed: customTheme[b].pressed }));
					if (pickerType.type === 'pressed') {
						setGradientNormalColor1(customTheme[selectedButton].normal);
						setGradientNormalColor2(c.hex);
					}
					else {
						setGradientNormalColor1(c.hex);
						setGradientNormalColor2(customTheme[selectedButton].pressed);
					}
				}
				else if (selectedButton === 'GRADIENT PRESSED') {
					matrix.forEach((r, i) => r.filter(b => !!b).forEach(b => customTheme[b] = { normal: customTheme[b].normal, pressed: steps[i] }));
					if (pickerType.type === 'pressed') {
						setGradientPressedColor1(customTheme[selectedButton].normal);
						setGradientPressedColor2(c.hex);
					}
					else {
						setGradientPressedColor1(c.hex);
						setGradientPressedColor2(customTheme[selectedButton].pressed);
					}
				}
			}
			else {
				customTheme[selectedButton][pickerType.type] = c.hex;
			}
		}

		setCustomTheme(customTheme);
		setSelectedColor(c);
	};

	const saveCurrentColor = () => {
		const color = selectedColor.hex;
		if (!color || presetColors.filter(c => c.color === color).length > 0)
			return;

		const newColors = [...savedColors];
		newColors.push(selectedColor.hex);
		setSavedColors(newColors);
		setPresetColors([...ledColors, ...customColors(newColors)]);
	};

	const toggleCustomTheme = (e) => {
		setHasCustomTheme(e.target.checked);
	};

	const toggleSelectedButton = (e, buttonName) => {
		e.stopPropagation();
		if (selectedButton === buttonName) {
			setPickerVisible(false);
		}
		else {
			setLedOverlayTarget(e.target);
			setSelectedButton(buttonName);
			setSelectedColor(buttonName === 'ALL' ? '#000000' : customTheme[buttonName].normal);
			setPickerType({ type: 'normal', button: buttonName });
			setPickerVisible(true);
		}
	};

	const submit = async () => {
		const leds = { ...customTheme };
		specialButtons.forEach(b => delete leds[b]);
		const success = await WebApi.setCustomTheme({ hasCustomTheme, customTheme: leds });
		setSaveMessage(success ? 'Saved! Please Restart Your Device' : 'Unable to Save');
	};

	useEffect(() => {
		async function fetchData() {
			const data = await WebApi.getCustomTheme();

			setHasCustomTheme(data.hasCustomTheme);
			if (!data.customTheme['ALL'])
				data.customTheme['ALL'] = { normal: '#000000', pressed: '#000000' };
			if (!data.customTheme['GRADIENT NORMAL'])
				data.customTheme['GRADIENT NORMAL'] = { normal: '#00ffff', pressed: '#ff00ff' };
			if (!data.customTheme['GRADIENT PRESSED'])
				data.customTheme['GRADIENT PRESSED'] = { normal: '#00ffff', pressed: '#ff00ff' };

			setCustomTheme(data.customTheme);
		}

		fetchData();

		// Hide color picker when anywhere but picker is clicked
		window.addEventListener('click', (e) => toggleSelectedButton(e, selectedButton));
	}, []);

	useEffect(() => {
		if (!pickerVisible)
			setTimeout(() => setSelectedButton(null), 250); // Delay enough to allow fade animation to finish
	}, [pickerVisible]);

	return <>
		<Section title="Custom LED Theme">
			<div>
				<p>
					Here you can enable and configure a custom LED theme.
					The custom theme will be selectable using the Next and Previous Animation shortcuts on your controller.
				</p>
				{hasCustomTheme &&
					<>
						<Stack>
							<div className="d-flex justify-content-between">
								<div className="d-flex d-none d-md-block">
									<ul>
										<li>Click a button to bring up the normal and pressed color selection.</li>
										<li>Click on the controller background to dismiss the color selection.</li>
										<li>Right-click a button to preview the button's pressed color.</li>
									</ul>
								</div>
								<FormSelect
									label="Preview Layout"
									name="ledLayout"
									value={ledLayout}
									onChange={(e) => setLedLayout(e.target.value)}
									style={{ width: 150 }}
								>
									{BUTTON_LAYOUTS.map((o, i) => <option key={`ledLayout-option-${i}`} value={o.value}>{o.label}</option>)}
								</FormSelect>
							</div>
							<div className="d-flex led-preview-container">
								<div
									className={`led-preview led-preview-${BUTTON_LAYOUTS[ledLayout]?.stickLayout}`}
									onContextMenu={(e) => e.preventDefault()}
								>
									<div className="container-aux">
										{AUX_BUTTONS.map(buttonName => (
											<LEDButton
												key={`led-button-${buttonName}`}
												className={`${buttonName} ${selectedButton === buttonName ? 'selected' : ''}`}
												name={BUTTONS[buttonLabels][buttonName]}
												buttonColor={customTheme[buttonName]?.normal}
												buttonPressedColor={customTheme[buttonName]?.pressed}
												labelUnder={true}
												onClick={(e) => toggleSelectedButton(e, buttonName)}
											/>
										))}
									</div>
									<div className="container-main">
										{MAIN_BUTTONS.map(buttonName => (
											<LEDButton
												key={`led-button-${buttonName}`}
												className={`${buttonName} ${selectedButton === buttonName ? 'selected' : ''}`}
												name={BUTTONS[buttonLabels][buttonName]}
												buttonColor={customTheme[buttonName]?.normal}
												buttonPressedColor={customTheme[buttonName]?.pressed}
												labelUnder={false}
												onClick={(e) => toggleSelectedButton(e, buttonName)}
											/>
										))}
									</div>
								</div>
							</div>
						</Stack>
						<div className="button-group">
							<Button onClick={(e) => setModalVisible(true)}>Clear All</Button>
							<Button onClick={(e) => toggleSelectedButton(e, 'ALL')}>Set All To Color</Button>
							<Button onClick={(e) => toggleSelectedButton(e, 'GRADIENT NORMAL')}>Set Gradient</Button>
							<Button onClick={(e) => toggleSelectedButton(e, 'GRADIENT PRESSED')}>Set Pressed Gradient</Button>
						</div>
					</>
				}
				<Overlay
					show={pickerVisible}
					target={ledOverlayTarget}
					placement={specialButtons.indexOf(selectedButton) > -1 ? 'top' : 'bottom'}
					container={this}
					containerPadding={20}
				>
					<Popover onClick={(e) => e.stopPropagation()}>
						<Container className="led-color-picker">
							<h6 className="text-center">{specialButtons.indexOf(selectedButton) > -1 ? selectedButton : BUTTONS[buttonLabels][selectedButton]}</h6>
							<Row>
								<Form.Group as={Col}
									className={`led-color-option ${pickerType?.type === 'normal' ? 'selected' : ''}`}
									onClick={() => handleLedColorClick('normal')}
								>
									<Form.Label>{selectedButton?.startsWith('GRADIENT') ? 'Color 1' : 'Normal'}</Form.Label>
									<div
										className={`led-color led-color-normal`}
										style={{ backgroundColor: customTheme[selectedButton]?.normal }}
									>
									</div>
								</Form.Group>
								<Form.Group as={Col}
									className={`led-color-option ${pickerType?.type === 'pressed' ? 'selected' : ''}`}
									onClick={() => handleLedColorClick('pressed')}
								>
									<Form.Label>{selectedButton?.startsWith('GRADIENT') ? 'Color 2' : 'Pressed'}</Form.Label>
									<div
										className={`led-color led-color-pressed`}
										style={{ backgroundColor: customTheme[selectedButton]?.pressed }}
									></div>
								</Form.Group>
							</Row>
							<Row className="mb-2">
								<Col>
									<SketchPicker
										color={selectedColor}
										onChange={(c) => handleLedColorChange(c)}
										disableAlpha={true}
										presetColors={presetColors}
										width={180}
									/>
								</Col>
							</Row>
							<div className="button-group d-flex justify-content-between">
								<Button size="sm" onClick={() => saveCurrentColor()}>Save Color</Button>
								<Button size="sm" onClick={() => deleteCurrentColor()}>Delete Color</Button>
							</div>
						</Container>
					</Popover>
				</Overlay>
			</div>
			<FormCheck
				label="Enable"
				type="switch"
				id="hasCustomTheme"
				reverse="true"
				error={undefined}
				isInvalid={false}
				checked={hasCustomTheme}
				onChange={(e) => toggleCustomTheme(e)}
			/>
		</Section>
		<div>
			<Button onClick={submit}>Save</Button>
			{saveMessage ? <span className="alert">{saveMessage}</span> : null}
		</div>
		<Modal show={modalVisible} onHide={() => setModalVisible(false)}>
			<Modal.Header closeButton>
				<Modal.Title>Confirm Clear Custom Theme</Modal.Title>
			</Modal.Header>
			<Modal.Body>Are you sure you would like to clear your current custom LED theme?</Modal.Body>
			<Modal.Footer>
				<Button variant="danger" onClick={() => setModalVisible(false)}>No</Button>
				<Button variant="success" onClick={() => confirmClearAll()}>Yes</Button>
			</Modal.Footer>
		</Modal>
	</>;
};

export default CustomThemePage;
