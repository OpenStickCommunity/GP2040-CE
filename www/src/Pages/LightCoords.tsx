import { useState, useCallback, useEffect, useRef } from 'react';
import {
	useSensor,
	MouseSensor,
	TouchSensor,
	KeyboardSensor,
	useSensors,
	DragEndEvent,
	DndContext,
} from '@dnd-kit/core';
import {
	snapCenterToCursor,
	restrictToParentElement,
} from '@dnd-kit/modifiers';
import { Coordinates } from '@dnd-kit/core/dist/types';
import { LightIndicator } from '../Components/LightIndicator';
import { Row, Col, Button } from 'react-bootstrap';
import WebApi from '../Services/WebApi';
import { set } from 'lodash';

const useGetDivDimensions = () => {
	const [dimensions, setDimensions] = useState({ height: 0, width: 0 });

	const containerRef = useRef<HTMLDivElement | null>(null);

	useEffect(() => {
		const resizeObserver = new ResizeObserver((event) => {
			setDimensions({
				height: event[0].contentBoxSize[0].blockSize,
				width: event[0].contentBoxSize[0].inlineSize,
			});
		});

		if (containerRef?.current) resizeObserver.observe(containerRef.current);
	}, []);

	return { dimensions, containerRef };
};

type light = {
	id: string;
	coords: Coordinates;
};

const GRID_SIZE = 64;
const MIN_GRID_SIZE = 10;
const MAX_GRID_SIZE = 30;

export default function LightCoords() {
	const { dimensions, containerRef } = useGetDivDimensions();
	const [gridSize, setGridSize] = useState(GRID_SIZE);
	const [cellWidth, setCellWidth] = useState(dimensions.width / gridSize);
	const [selectedLight, setSelectedLight] = useState<string | null>(null);
	const [lights, setLights] = useState<light[]>([]);

	const mouseSensor = useSensor(MouseSensor, {
		activationConstraint: undefined,
	});
	const touchSensor = useSensor(TouchSensor, {
		activationConstraint: undefined,
	});
	// Make keyboard sensor handle dynamic cell width with getNextCoordinates
	const keyboardSensor = useSensor(KeyboardSensor, {});
	const sensors = useSensors(
		// useSensor(PointerSensor, {
		// 	activationConstraint: { delay: 150, tolerance: 0 },
		// }),
		mouseSensor,
		touchSensor,
		keyboardSensor,
	);

	const gridPxToCoords = useCallback(
		(pixels: number) =>
			Math.max(0, Math.min(Math.round(pixels / cellWidth), gridSize - 1)) || 0,
		[cellWidth, gridSize],
	);

	useEffect(() => {
		if (dimensions.width === 0) return;
		const newCellWidth = dimensions.width / gridSize - 1 / gridSize;

		setCellWidth(newCellWidth);
	}, [gridSize, dimensions.width]);

	useEffect(() => {
		setLights((prev) =>
			prev.map((light) => ({
				...light,
				coords: {
					x: gridPxToCoords(light.coords.x * cellWidth),
					y: gridPxToCoords(light.coords.y * cellWidth),
				},
			})),
		);
	}, [cellWidth]);

	const handleDragEnd = useCallback(
		function handleDragEnd(event: DragEndEvent) {
			setLights((prev) =>
				prev.map((light) =>
					light.id === event.active.id
						? {
								...light,
								coords: {
									x: gridPxToCoords(light.coords.x * cellWidth + event.delta.x),
									y: gridPxToCoords(light.coords.y * cellWidth + event.delta.y),
								},
							}
						: light,
				),
			);
		},
		[cellWidth],
	);

	const handleDragStart = useCallback(
		function handleDragStart(event: DragEndEvent) {
			setSelectedLight(String(event.active.id));
		},
		[lights],
	);

	const fetchLayout = useCallback(async () => {
		const { displayLayouts } = await WebApi.getButtonLayouts();

		const buttonLayout = Object.values(displayLayouts.buttonLayout).map(
			(item: any, index: number) => ({
				id: `left-${index}`,
				coords: {
					x: Math.round(item.parameters.x1 / 2),
					y: Math.round(item.parameters.y1 / 2),
				},
			}),
		);
		const buttonLayoutRight = Object.values(
			displayLayouts.buttonLayoutRight,
		).map((item: any, index: number) => ({
			id: `right-${index}`,
			coords: {
				x: Math.round(item.parameters.x1 / 2),
				y: Math.round(item.parameters.y1 / 2),
			},
		}));

		setLights([...buttonLayout, ...buttonLayoutRight]);
	}, []);

	const selectedLightData = lights.find((light) => light.id === selectedLight);
	return (
		<div className={`card`}>
			<div className={`card-header`}>
				<strong>Light Coordinates</strong>
			</div>
			<div className="card-body">
				<Row className="mb-3">
					<Col md={8}>
						<div
							ref={containerRef}
							style={{
								position: 'relative',
								minHeight: dimensions.width,
							}}
						>
							<svg
								width={dimensions.width}
								height={dimensions.width}
								xmlns="http://www.w3.org/2000/svg"
								style={{ position: 'absolute' }}
							>
								<defs>
									<pattern
										id="smallGrid"
										width={cellWidth}
										height={cellWidth}
										patternUnits="userSpaceOnUse"
									>
										<path
											d={`M ${cellWidth} 0 L 0 0 0 ${cellWidth}`}
											fill="none"
											stroke="gray"
											strokeWidth="0.5"
										/>
									</pattern>

									<pattern
										id="grid"
										width={cellWidth * 10}
										height={cellWidth * 10}
										patternUnits="userSpaceOnUse"
									>
										<rect
											width={cellWidth * 10}
											height={cellWidth * 10}
											fill="url(#smallGrid)"
										/>
										<path
											d={`M ${cellWidth * 10} 0 L 0 0 0 ${cellWidth * 10}`}
											fill="none"
											stroke="silver"
											strokeWidth="2"
										/>
									</pattern>
								</defs>
								<rect width="100%" height="100%" fill="url(#grid)" />
							</svg>
							<DndContext
								sensors={sensors}
								modifiers={[snapCenterToCursor, restrictToParentElement]}
								onDragStart={handleDragStart}
								onDragEnd={handleDragEnd}
							>
								{lights.map((light) => (
									<LightIndicator
										key={light.id}
										id={light.id}
										x={light.coords.x}
										y={light.coords.y}
										cellWidth={cellWidth}
									/>
								))}
							</DndContext>
						</div>
					</Col>
					<Col md={4}>
						<div className="d-flex flex-column h-100">
							<div className="d-flex flex-grow-1 justify-content-center align-items-center text-center">
								{selectedLightData ? (
									<div>
										<h5>Light {selectedLightData.id}</h5>
										<div className="mt-3">
											<div className="mb-2">
												<label className="form-label">First LED Index</label>
												<input type="number" className="form-control" min={0} />
											</div>
											<div className="mb-2">
												<label className="form-label">Num LEDs On Light</label>
												<input type="number" className="form-control" min={1} />
											</div>
											<div className="mb-2">
												<label className="form-label">X Coord</label>
												<input
													type="number"
													className="form-control"
													value={selectedLightData.coords.x}
													onChange={(e) => {
														setLights((prev) =>
															prev.map((light) =>
																light.id === selectedLightData.id
																	? {
																			...light,
																			coords: {
																				...light.coords,
																				x: Number(e.target.value),
																			},
																		}
																	: light,
															),
														);
													}}
													min={0}
													max={gridSize - 1}
												/>
											</div>
											<div className="mb-2">
												<label className="form-label">Y Coord</label>
												<input
													type="number"
													className="form-control"
													value={selectedLightData.coords.y}
													onChange={(e) => {
														setLights((prev) =>
															prev.map((light) =>
																light.id === selectedLightData.id
																	? {
																			...light,
																			coords: {
																				...light.coords,
																				y: Number(e.target.value),
																			},
																		}
																	: light,
															),
														);
													}}
													min={0}
													max={gridSize - 1}
												/>
											</div>
											<div className="mb-2">
												<label className="form-label">
													GPIO Pin or Case Chain Index
												</label>
												<input type="number" className="form-control" min={0} />
											</div>
											<div className="mb-2">
												<label className="form-label">Light Type</label>
												<select className="form-select" value={0}>
													<option value={0}>ActionButton</option>
													<option value={1}>Case</option>
													<option value={2}>Turbo</option>
													<option value={3}>PlayerLight</option>
												</select>
											</div>
										</div>
									</div>
								) : (
									<p>Select a light to configure</p>
								)}
							</div>
							<hr className="mt-3" />
							<div className="d-flex flex-column justify-content-between align-items-center">
								<Button
									onClick={() => {
										fetchLayout();
									}}
								>
									Fetch Button Layout
								</Button>
								<hr />
								<Button
									onClick={() => {
										setLights((prev) => [
											...prev,
											{
												id: `${lights.length + 1}`,
												coords: { x: 0, y: 0 },
											},
										]);
									}}
								>
									Add light
								</Button>
							</div>
						</div>
					</Col>
				</Row>
			</div>
		</div>
	);
}
