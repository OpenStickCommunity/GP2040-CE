import { ReactElement, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Button, Col, OverlayTrigger, Popover, Row } from 'react-bootstrap';

import LEDColors from '../../Data/LEDColors';
import { Light } from '../../Store/useLedStore';
import { rgbIntToHex } from '../../Services/Utilities';
import ColorSelector from './ColorSlector';
import boards from '../../Data/Boards.json';

const GPIO_PIN_LENGTH =
	boards[import.meta.env.VITE_GP2040_BOARD as keyof typeof boards].maxPin + 1;

const getViewBox = (lights: { xCoord: number; yCoord: number }[]) =>
	lights.reduce(
		(acc, light) => ({
			minX: Math.min(acc.minX, light.xCoord),
			minY: Math.min(acc.minY, light.yCoord),
			maxX: Math.max(acc.maxX, light.xCoord),
			maxY: Math.max(acc.maxY, light.yCoord),
		}),
		{ minX: Infinity, minY: Infinity, maxX: -Infinity, maxY: -Infinity },
	);

const hasNeighbor = (light: Light, lights: Light[], range: number) =>
	lights.some(
		(other) =>
			other !== light &&
			other.lightType === light.lightType &&
			Math.abs(other.xCoord - light.xCoord) <= range &&
			Math.abs(other.yCoord - light.yCoord) <= range,
	);

// Calculate the size of the light based on its neighbors,
// Three sizes: big(no neighbors within 2 cells), normal(has neighbors within 2 cells), small(has neighbors within 1 cell)
const calculateLightSize = (light: Light, lights: Light[]) => {
	const lightSize = 1.3; // Base size
	if (hasNeighbor(light, lights, 2)) {
		return hasNeighbor(light, lights, 1) ? lightSize / 2.8 : lightSize / 1.5;
	}
	return lightSize;
};

const ColorSelectOverlay = ({
	title,
	content,
	children,
}: {
	title: string;
	content: React.ReactNode;
	children: ReactElement;
}) => (
	<OverlayTrigger
		trigger="click"
		placement="auto"
		rootClose
		overlay={
			<Popover>
				<Popover.Header as="h3">{title}</Popover.Header>
				<Popover.Body>{content}</Popover.Body>
			</Popover>
		}
	>
		{children}
	</OverlayTrigger>
);

function ButtonLayoutPreview({
	pressedStaticColors,
	notPressedStaticColors,
	profileIndex,
	setFieldValue,
	customColors = [],
	Lights,
}: {
	pressedStaticColors: number[];
	notPressedStaticColors: number[];
	profileIndex: number;
	customColors?: number[];
	setFieldValue: (field: string, value: any, shouldValidate?: boolean) => void;
	Lights: Light[];
}) {
	const { t } = useTranslation('');
	const { minX, minY, maxX, maxY } = getViewBox(Lights);
	const [pressed, setPressed] = useState(false);

	const strokeWidth = 0.03;
	const padding = 1.4;
	const viewBoxX = minX - padding;
	const viewBoxY = minY - padding;
	const viewBoxWidth = maxX - minX + padding * 2;
	const viewBoxHeight = maxY - minY + padding * 2;

	const customColorOptions = customColors.map((color, index) => ({
		value: LEDColors.length + index,
		label: `Custom ${index + 1}`,
		color: rgbIntToHex(color),
	}));
	const colorOptions = [...LEDColors, ...customColorOptions];

	const handlePressedShow = (e: React.MouseEvent<HTMLElement, MouseEvent>) => {
		if (e.button === 2) setPressed(true);
	};

	const handlePressedHide = () => {
		setPressed(false);
	};

	return (
		<div>
			<p>
				Here you can see a preview of your button layout and the colors assigned
				to each GPIO pin.
				<br />
				If you have buttons assigned to the same GPIO they will share color.
			</p>
			<ul>
				<li>Click a button to change its idle and pressed color.</li>
				<li>Right-click on the layout to preview the pressed colors.</li>
			</ul>
			<Row
				className="justify-content-center py-3"
				onMouseDown={(e) => handlePressedShow(e)}
				onMouseUp={() => handlePressedHide()}
				onMouseLeave={() => handlePressedHide()}
				onContextMenu={(e) => e.preventDefault()}
			>
				<Col lg={10}>
					<svg
						width="100%"
						viewBox={`${viewBoxX} ${viewBoxY} ${viewBoxWidth} ${viewBoxHeight}`}
						xmlns="http://www.w3.org/2000/svg"
					>
						{Lights.filter((light) => light.lightType !== 1).map(
							(light, index) => (
								<ColorSelectOverlay
									key={`button-light-${index}`}
									title={`GPIO ${light.GPIOPinorCaseChainIndex}`}
									content={
										<div style={{ minWidth: 200 }}>
											<p>Idle color</p>
											<ColorSelector
												options={colorOptions}
												value={
													colorOptions[
														notPressedStaticColors[
															light.GPIOPinorCaseChainIndex
														]
													] || null
												}
												onChange={(selected) => {
													setFieldValue(
														`AnimationOptions.profiles.${profileIndex}.notPressedStaticColors.${light.GPIOPinorCaseChainIndex}`,
														selected?.value || 0,
													);
												}}
											/>

											<p className="mt-3">Pressed color</p>
											<ColorSelector
												options={colorOptions}
												value={
													colorOptions[
														pressedStaticColors[light.GPIOPinorCaseChainIndex]
													] || null
												}
												onChange={(selected) => {
													setFieldValue(
														`AnimationOptions.profiles.${profileIndex}.pressedStaticColors.${light.GPIOPinorCaseChainIndex}`,
														selected?.value || 0,
													);
												}}
											/>
										</div>
									}
								>
									<g style={{ cursor: 'pointer' }}>
										<circle
											key={`light-${index}`}
											cx={light.xCoord}
											cy={light.yCoord}
											r={calculateLightSize(light, Lights)}
											fill={
												colorOptions[
													pressed
														? pressedStaticColors[light.GPIOPinorCaseChainIndex]
														: notPressedStaticColors[
																light.GPIOPinorCaseChainIndex
															]
												]?.color || 'black'
											}
											stroke="currentColor"
											strokeWidth={strokeWidth}
										/>
										<text
											x={light.xCoord}
											y={light.yCoord}
											textAnchor="middle"
											fill="white"
											dominantBaseline="central"
											style={{
												fontSize: 0.3,
												fontWeight: 'bold',
												textShadow: '0 0 3px black',
											}}
										>
											{`GP${light.GPIOPinorCaseChainIndex}`}
										</text>
									</g>
								</ColorSelectOverlay>
							),
						)}
					</svg>
				</Col>
			</Row>

			<ColorSelectOverlay
				title={'Set all colors'}
				content={
					<div style={{ minWidth: 200 }}>
						<p>Idle color</p>
						<ColorSelector
							options={colorOptions}
							onChange={(selected) => {
								setFieldValue(
									`AnimationOptions.profiles.${profileIndex}.notPressedStaticColors`,
									Array(GPIO_PIN_LENGTH).fill(selected?.value || 0),
								);
							}}
						/>
						<p className="mt-3">Pressed color</p>
						<ColorSelector
							options={colorOptions}
							onChange={(selected) => {
								setFieldValue(
									`AnimationOptions.profiles.${profileIndex}.pressedStaticColors`,
									Array(GPIO_PIN_LENGTH).fill(selected?.value || 0),
								);
							}}
						/>
					</div>
				}
			>
				<Button variant="secondary">Set all colors</Button>
			</ColorSelectOverlay>
		</div>
	);
}

export default ButtonLayoutPreview;
