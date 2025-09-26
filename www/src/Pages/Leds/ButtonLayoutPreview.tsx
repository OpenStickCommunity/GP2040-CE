import { useTranslation } from 'react-i18next';
import { components, ControlProps, Props, StylesConfig } from 'react-select';
import {
	Col,
	ContainerProps,
	OverlayTrigger,
	Popover,
	Row,
	Tooltip,
} from 'react-bootstrap';

import LEDColors from '../../Data/LEDColors';
import CustomSelect from '../../Components/CustomSelect';
import { Light } from '../../Store/useLedStore';
import { rgbIntToHex } from '../../Services/Utilities';

const colorDot = (color = 'transparent') => ({
	alignItems: 'center',
	display: 'flex',

	':before': {
		backgroundColor: color,
		borderRadius: 15,
		content: '" "',
		display: 'block',
		flexShrink: 0,
		marginRight: 8,
		height: 15,
		width: 15,
	},
});

import type { GroupBase } from 'react-select';


const colorStyles: StylesConfig<(typeof LEDColors)[number]> = {
	control: (styles) => ({ ...styles, backgroundColor: 'white' }),
	option: (styles, { data }) => ({ ...styles, ...colorDot(data.color) }),
	input: (styles) => ({ ...styles }),
	placeholder: (styles) => ({ ...styles, ...colorDot('#ccc') }),
	singleValue: (styles, { data }) => ({
		...styles,
		...colorDot(data.color),
	}),
};

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

const hasNeighbors = (light: Light, lights: Light[]) =>
	lights.some(
		(other) =>
			other !== light &&
			Math.abs(other.xCoord - light.xCoord) <= 1 &&
			Math.abs(other.yCoord - light.yCoord) <= 1,
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

	const lightSize = 0.8;
	const strokeWidth = 0.05;
	const padding = 1;
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

	return (
		<>
			<Row className="justify-content-center">
				<Col lg={8}>
					<div className="p-3">
						<svg
							width="100%"
							viewBox={`${viewBoxX} ${viewBoxY} ${viewBoxWidth} ${viewBoxHeight}`}
							xmlns="http://www.w3.org/2000/svg"
						>
							{Lights.filter((light) => light.lightType !== 1).map(
								(light, index) => (
									<OverlayTrigger
										key={`light-${index}`}
										trigger="click"
										placement="auto"
										rootClose
										overlay={
											<Popover>
												<Popover.Header as="h3">{`GPIO ${light.GPIOPinorCaseChainIndex}`}</Popover.Header>
												<Popover.Body>
													<div style={{ minWidth: 200 }}>
														<p>Idle color</p>
														<CustomSelect
															options={colorOptions}
															styles={colorStyles}
															isMulti={false}
															onChange={(selected) => {
																setFieldValue(
																	`AnimationOptions.profiles.${profileIndex}.notPressedStaticColors.${light.GPIOPinorCaseChainIndex}`,
																	selected?.value || 0,
																);
															}}
															value={
																colorOptions[
																	notPressedStaticColors[
																		light.GPIOPinorCaseChainIndex
																	]
																] || null
															}
														/>
														<p className="mt-3">Pressed color</p>
														<CustomSelect
															options={colorOptions}
															styles={colorStyles}
															isMulti={false}
															onChange={(selected) => {
																setFieldValue(
																	`AnimationOptions.profiles.${profileIndex}.pressedStaticColors.${light.GPIOPinorCaseChainIndex}`,
																	selected?.value || 0,
																);
															}}
															value={
																colorOptions[
																	pressedStaticColors[
																		light.GPIOPinorCaseChainIndex
																	]
																] || null
															}
														/>
													</div>
												</Popover.Body>
											</Popover>
										}
									>
										<g style={{ cursor: 'pointer' }}>
											<circle
												key={`light-${index}`}
												cx={light.xCoord}
												cy={light.yCoord}
												r={
													hasNeighbors(light, Lights)
														? lightSize / 1.8
														: lightSize
												}
												fill={
													colorOptions[
														notPressedStaticColors[
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
									</OverlayTrigger>
								),
							)}
						</svg>
					</div>
				</Col>
			</Row>

		</>
	);
}

export default ButtonLayoutPreview;
