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
import { Form, Formik } from 'formik';
import * as yup from 'yup';

import { LightIndicator } from '../Components/LightIndicator';
import { Row, Col, Button, Alert } from 'react-bootstrap';
import useLedStore, { Light } from '../Store/useLedStore';
import { useTranslation } from 'react-i18next';

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

		const node = containerRef.current;
		if (node) resizeObserver.observe(node);

		return () => {
			if (node) resizeObserver.unobserve(node);
			resizeObserver.disconnect();
		};
	}, [containerRef.current]); // Re-run if ref changes

	return { dimensions, containerRef };
};

const GRID_SIZE = 30;

const schema = yup.object({
	Lights: yup.array().of(
		yup.object({
			GPIOPinorCaseChainIndex: yup.number().required(),
			firstLedIndex: yup.number().required(),
			lightType: yup.number().required(),
			numLedsOnLight: yup.number().required(),
			xCoord: yup.number().required(),
			yCoord: yup.number().required(),
		}),
	),
});

export default function LightCoordsSection() {
	const { dimensions, containerRef } = useGetDivDimensions();
	const { Lights, saveLightOptions } = useLedStore();
	const { t } = useTranslation('');

	const [gridSize, setGridSize] = useState(GRID_SIZE);
	const [cellWidth, setCellWidth] = useState(dimensions.width / gridSize);
	const [selectedLight, setSelectedLight] = useState<number | null>(null);
	const [saveMessage, setSaveMessage] = useState('');

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

	const onSuccess = async ({ Lights }: { Lights: Light[] }) => {
		try {
			await saveLightOptions(Lights);
			setSaveMessage(t('Common:saved-success-message'));
		} catch (error) {
			console.error(error);
		}
	};

	useEffect(() => {
		if (dimensions.width === 0) return;
		const newCellWidth = dimensions.width / gridSize - 1 / gridSize;

		setCellWidth(newCellWidth);
	}, [gridSize, dimensions.width]);

	const handleDragStart = useCallback(function handleDragStart(
		event: DragEndEvent,
	) {
		setSelectedLight(Number(event.active.id));
	}, []);

	return (
		<div className={`card`}>
			<div className={`card-header`}>
				<strong>Light Coordinates</strong>
			</div>
			<div className="card-body">
				<Row>
					<Col md={12} className="mb-3">
						<p>
							This section allows you to visually arrange and configure the
							position of each light. This is useful for mapping LEDs or light
							indicators to their physical locations on your device or case.
							Drag lights on the grid or adjust their coordinates manually to
							match your setup.
						</p>
					</Col>
				</Row>
				<Formik
					validationSchema={schema}
					onSubmit={onSuccess}
					initialValues={{ Lights }}
					validateOnChange={false}
				>
					{({
						handleSubmit,
						handleChange,
						values,
						errors,
						setFieldValue,
						setValues,
					}) => (
						<Form noValidate onSubmit={handleSubmit}>
							<Row className="mb-3">
								<Col md={3}>
									<div className="d-flex flex-grow-1">
										{selectedLight !== null ? (
											<div className="w-100 mb-3">
												<h3>Light {selectedLight + 1}</h3>
												<div className="mb-2 ">
													<label className="form-label">First LED Index</label>
													<input
														type="number"
														className="form-control "
														name={`Lights[${selectedLight}].firstLedIndex`}
														min={0}
														value={
															values.Lights[Number(selectedLight)]
																?.firstLedIndex
														}
														onChange={handleChange}
													/>
												</div>
												<div className="mb-2">
													<label className="form-label">
														Num LEDs On Light
													</label>
													<input
														type="number"
														className="form-control"
														name={`Lights[${selectedLight}].numLedsOnLight`}
														min={1}
														value={
															values.Lights[Number(selectedLight)]
																?.numLedsOnLight
														}
														onChange={handleChange}
													/>
												</div>
												<div className="mb-2">
													<label className="form-label">X Coord</label>
													<input
														type="number"
														className="form-control"
														name={`Lights[${selectedLight}].xCoord`}
														value={values.Lights[Number(selectedLight)]?.xCoord}
														onChange={handleChange}
														min={0}
														max={gridSize - 1}
													/>
												</div>
												<div className="mb-2">
													<label className="form-label">Y Coord</label>
													<input
														type="number"
														className="form-control"
														name={`Lights[${selectedLight}].yCoord`}
														value={values.Lights[Number(selectedLight)]?.yCoord}
														onChange={handleChange}
														min={0}
														max={gridSize - 1}
													/>
												</div>
												<div className="mb-2">
													<label className="form-label">
														GPIO Pin or Case Chain Index
													</label>
													<input
														type="number"
														className="form-control"
														name={`Lights[${selectedLight}].GPIOPinorCaseChainIndex`}
														min={0}
														value={
															values.Lights[Number(selectedLight)]
																?.GPIOPinorCaseChainIndex
														}
														onChange={handleChange}
													/>
												</div>
												<div className="mb-2">
													<label className="form-label">Light Type</label>
													<select
														className="form-select"
														name={`Lights[${selectedLight}].lightType`}
														value={
															values.Lights[Number(selectedLight)]?.lightType
														}
														onChange={(e) => {
															setFieldValue(
																`Lights[${selectedLight}].lightType`,
																parseInt(e.target.value),
															);
														}}
													>
														<option value={0}>ActionButton</option>
														<option value={1}>Case</option>
														<option value={2}>Turbo</option>
														<option value={3}>PlayerLight</option>
													</select>
												</div>
												<Button
													variant="danger"
													className="w-100 mt-3"
													onClick={() => {
														setValues({
															Lights: values.Lights.filter(
																(_, index) => index !== selectedLight,
															),
														});
														setSelectedLight(null);
													}}
												>
													Delete Light
												</Button>
											</div>
										) : (
											<div className="w-100 d-flex flex-column justify-content-center">
												<h3>Select a Light</h3>
												<p>
													Click on a light in the grid to view and edit its
													properties.
												</p>
											</div>
										)}
									</div>
									<div className="d-flex flex-column justify-content-between align-items-center">
										<Button
											className="w-100"
											variant="secondary"
											onClick={() => {
												setValues({
													Lights: [
														...values.Lights,
														{
															GPIOPinorCaseChainIndex: 0,
															firstLedIndex: values.Lights.length,
															lightType: 0,
															numLedsOnLight: 1,
															xCoord: 0,
															yCoord: 0,
														},
													],
												});
											}}
										>
											Add light
										</Button>
									</div>
									<hr className="mt-3" />
									<Button className="w-100 mb-3" type="submit">
										{t('Common:button-save-label')}
									</Button>
									{saveMessage && <Alert variant="info">{saveMessage}</Alert>}
								</Col>
								<Col md={9} className="mt-3 mt-md-0">
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
											onDragEnd={(event: DragEndEvent) => {
												const activeId = Number(event.active.id);
												setFieldValue(`Lights[${event.active.id}]`, {
													...values.Lights[activeId],
													xCoord: gridPxToCoords(
														values.Lights[activeId].xCoord * cellWidth +
															event.delta.x,
													),
													yCoord: gridPxToCoords(
														values.Lights[activeId].yCoord * cellWidth +
															event.delta.y,
													),
												});
											}}
										>
											{values.Lights.map((light, index) => (
												<LightIndicator
													key={index}
													id={index}
													cellWidth={cellWidth}
													active={selectedLight === index}
													{...light}
												/>
											))}
										</DndContext>
									</div>
								</Col>
							</Row>
						</Form>
					)}
				</Formik>
			</div>
		</div>
	);
}
