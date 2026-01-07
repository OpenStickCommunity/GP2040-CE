import { ReactElement, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Button, Col, OverlayTrigger, Popover, Row } from 'react-bootstrap';

import { LED_COLORS, LIGHT_TYPES } from '../../Data/Leds';
import { Light, MAX_CASE_LIGHTS } from '../../Store/useLedStore';
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
			Math.abs(other.xCoord - light.xCoord) <= range &&
			Math.abs(other.yCoord - light.yCoord) <= range,
	);

const BASE_LIGHT_SIZE = 0.9; // Base size
const SMALL_LIGHT_SIZE = BASE_LIGHT_SIZE / 2; // Size when has neighbor within 1 cell

const calculateLightSize = (light: Light, lights: Light[]) =>
	hasNeighbor(light, lights, 1) ? SMALL_LIGHT_SIZE : BASE_LIGHT_SIZE;

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
	caseStaticColors,
	profileIndex,
	setFieldValue,
	customColors = [],
	Lights,
}: {
	pressedStaticColors: number[];
	notPressedStaticColors: number[];
	caseStaticColors: number[];
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
		value: LED_COLORS.length + index,
		label: `Custom ${index + 1}`,
		color: rgbIntToHex(color),
	}));
	const colorOptions = [...LED_COLORS, ...customColorOptions];

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
			<hr />
			<div className="mb-3">
				<svg
					width="20"
					height="20"
					className="d-inline-block mx-2 align-middle"
					aria-label='circle'
				>
					<circle
						cx="10"
						cy="10"
						r="8"
						fill="currentColor"
						stroke="black"
						strokeWidth="1"
					/>
				</svg>
				Action Button
				<svg
					width="20"
					height="20"
					className="d-inline-block mx-2 align-middle"
					aria-label='square'
				>
					<rect
						x="2"
						y="2"
						width="16"
						height="16"
						fill="currentColor"
						stroke="black"
						strokeWidth="1"
					/>
				</svg>
				Case Light
				<svg
					width="20"
					height="20"
					className="d-inline-block mx-2 align-middle"
					aria-label='hexagon'
				>
					<polygon
						points={[0, 1, 2, 3, 4, 5]
							.map((i) => {
								const angle = (Math.PI / 3) * i - Math.PI / 2;
								const x = 10 + 8 * Math.cos(angle);
								const y = 10 + 8 * Math.sin(angle);
								return `${x},${y}`;
							})
							.join(' ')}
						fill="currentColor"
						stroke="black"
						strokeWidth="1"
					/>
				</svg>
				Turbo / Player Light
			</div>

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
						{Lights.map((light, index) => {
							switch (light.lightType) {
								case LIGHT_TYPES.ActionButton: {
									return (
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
																pressedStaticColors[
																	light.GPIOPinorCaseChainIndex
																]
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
													key={`button-el-light-${index}`}
													cx={light.xCoord}
													cy={light.yCoord}
													r={calculateLightSize(light, Lights)}
													fill={
														colorOptions[
															pressed
																? pressedStaticColors[
																		light.GPIOPinorCaseChainIndex
																	]
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
									);
								}
								case LIGHT_TYPES.Case: {
									return (
										<ColorSelectOverlay
											key={`case-light-${index}`}
											title={`Case ${light.GPIOPinorCaseChainIndex}`}
											content={
												<div style={{ minWidth: 200 }}>
													<p>Case color</p>
													<ColorSelector
														options={colorOptions}
														value={
															colorOptions[
																caseStaticColors[light.GPIOPinorCaseChainIndex]
															] || null
														}
														onChange={(selected) => {
															setFieldValue(
																`AnimationOptions.profiles.${profileIndex}.caseStaticColors.${light.GPIOPinorCaseChainIndex}`,
																selected?.value || 0,
															);
														}}
													/>
												</div>
											}
										>
											<g style={{ cursor: 'pointer' }}>
												<rect
													key={`case-el-light-${index}`}
													x={light.xCoord - SMALL_LIGHT_SIZE}
													y={light.yCoord - SMALL_LIGHT_SIZE}
													width={SMALL_LIGHT_SIZE * 2}
													height={SMALL_LIGHT_SIZE * 2}
													fill={
														colorOptions[
															caseStaticColors[light.GPIOPinorCaseChainIndex]
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
													{`C${light.GPIOPinorCaseChainIndex}`}
												</text>
											</g>
										</ColorSelectOverlay>
									);
								}
								case LIGHT_TYPES.Turbo:
								case LIGHT_TYPES.PlayerLight:
									return (
										<ColorSelectOverlay
											key={`misc-light-${index}`}
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

													<p className="mt-3">Active color</p>
													<ColorSelector
														options={colorOptions}
														value={
															colorOptions[
																pressedStaticColors[
																	light.GPIOPinorCaseChainIndex
																]
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
											<g key={`${light.lightType}-light-${index}`}>
												<polygon
													points={[0, 1, 2, 3, 4, 5]
														.map((i) => {
															const angle = (Math.PI / 3) * i - Math.PI / 2;
															const size = SMALL_LIGHT_SIZE;
															const x = light.xCoord + size * Math.cos(angle);
															const y = light.yCoord + size * Math.sin(angle);
															return `${x},${y}`;
														})
														.join(' ')}
													fill={
														colorOptions[
															pressed
																? pressedStaticColors[
																		light.GPIOPinorCaseChainIndex
																	]
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
									);
								default:
									return null;
							}
						})}
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
						<p className="mt-3">Case color</p>
						<ColorSelector
							options={colorOptions}
							onChange={(selected) => {
								setFieldValue(
									`AnimationOptions.profiles.${profileIndex}.caseStaticColors`,
									Array(MAX_CASE_LIGHTS).fill(selected?.value || 0),
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
