import { useContext, useEffect, useState } from 'react';
import Button from 'react-bootstrap/Button';
import Col from 'react-bootstrap/Col';
import Container from 'react-bootstrap/Container';
import Form from 'react-bootstrap/Form';
import Overlay from 'react-bootstrap/Overlay';
import Popover from 'react-bootstrap/Popover';
import Row from 'react-bootstrap/Row';
import { SketchPicker } from '@hello-pangea/color-picker';
import { useTranslation } from 'react-i18next';

import { AppContext } from '../Contexts/AppContext';
import LEDColors from '../Data/LEDColors';

import './ColorPicker.scss';

const ledColors = LEDColors.map((c) => ({ title: c.name, color: c.value }));
const customColors = (colors) => colors.map((c) => ({ title: c, color: c }));

const ColorPicker = ({
	types,
	onChange,
	onDismiss,
	pickerOnly,
	placement,
	show,
	target,
	title,
	...props
}) => {
	const { savedColors, setSavedColors } = useContext(AppContext);
	const [colorPalette, setColorPalette] = useState([
		...ledColors,
		...customColors(savedColors),
	]);
	const [colorTypes, setColorTypes] = useState(types);
	const [selectedColor, setSelectedColor] = useState('#000000');
	const [selectedColorType, setSelectedColorType] = useState(types[0]);

	const { t } = useTranslation('');

	const deleteCurrentColor = () => {
		const colorIndex = savedColors.indexOf(selectedColor);
		if (colorIndex < 0) return;

		const newColors = [...savedColors];
		newColors.splice(colorIndex, 1);
		setSavedColors(newColors);
		setColorPalette([...ledColors, ...customColors(newColors)]);
	};

	const saveCurrentColor = () => {
		if (
			!selectedColor ||
			colorPalette.filter((c) => c.color === selectedColor).length > 0
		)
			return;

		const newColors = [...savedColors];
		newColors.push(selectedColor);
		setSavedColors(newColors);
		setColorPalette([...ledColors, ...customColors(newColors)]);
	};

	const selectColor = (c, e) => {
		if (onChange) onChange(c.hex, e);

		selectedColorType.value = c.hex;

		setSelectedColor(c.hex);
		setColorTypes(colorTypes);
	};

	useEffect(() => {
		// Hide color picker when anywhere but picker is clicked
		window.addEventListener('click', (e) => onDismiss(e));
		setSelectedColorType(colorTypes[0]);
		setSelectedColor(colorTypes[0].value);
	}, []);

	return (
		<Overlay
			show={show}
			target={target}
			placement={placement || 'bottom'}
			container={this}
			containerPadding={20}
		>
			<Popover onClick={(e) => e.stopPropagation()}>
				<Container className="color-picker">
					<h6 className="text-center">{title}</h6>
					<Row>
						{colorTypes.map((o, i) => (
							<Form.Group
								as={Col}
								key={`colorType${i}`}
								className={`${o === selectedColorType ? 'selected' : ''}`}
								onClick={() => setSelectedColorType(o)}
							>
								{o.title && <Form.Label>{o.title}</Form.Label>}
								<div
									className={`color color-normal`}
									style={{ backgroundColor: o.value }}
								></div>
							</Form.Group>
						))}
					</Row>
					<Row>
						<Col>
							<SketchPicker
								color={selectedColorType.value}
								onChange={(c, e) => selectColor(c, e)}
								disableAlpha={true}
								presetColors={colorPalette}
								width={180}
							/>
						</Col>
					</Row>
					<div className="button-group d-flex justify-content-between mt-2">
						<Button size="sm" onClick={() => saveCurrentColor()}>
							{t('Common:button-save-color-label')}
						</Button>
						<Button size="sm" onClick={() => deleteCurrentColor()}>
							{t('Common:button-delete-color-label')}
						</Button>
					</div>
				</Container>
			</Popover>
		</Overlay>
	);
};

export default ColorPicker;
