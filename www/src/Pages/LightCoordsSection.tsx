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
import { Formik } from 'formik';
import * as yup from 'yup';
import { Row, Col, Button, Alert, Form } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';

import FormSelect from '../Components/FormSelect';
import { LightIndicator } from '../Components/LightIndicator';
import useLedStore, {
	Light,
	MAX_CASE_LIGHTS,
	MAX_LIGHTS,
} from '../Store/useLedStore';
import boards from '../Data/Boards.json';
import useLedsPreview from '../Hooks/useLedsPreview';

const GRID_SIZE = 30;
const GPIO_PIN_LENGTH = boards[import.meta.env.VITE_GP2040_BOARD].maxPin + 1;

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

const schema = yup.object({
	Lights: yup
		.array()
		.of(
			yup.object({
				GPIOPinorCaseChainIndex: yup.number().required(),
				firstLedIndex: yup.number().required(),
				lightType: yup.number().required(),
				numLedsOnLight: yup.number().required(),
				xCoord: yup.number().required(),
				yCoord: yup.number().required(),
			}),
		)
		.test('no-duplicate-coords', 'Overlapping light', function (lights) {
			if (!lights) return true;
			const overlappingCoords = new Map();
			for (let i = 0; i < lights.length; i++) {
				const key = `${lights[i].xCoord},${lights[i].yCoord}`;
				if (overlappingCoords.has(key)) {
					return this.createError({
						path: `Lights[${i}]`,
						message: `Overlapping light`,
					});
				}
				overlappingCoords.set(key, i);
			}
			return true;
		}),
});

const getFirstEmptyLightCoord = (lights: Light[]) => {
	const existingCoords = lights.map(
		(light) => `${light.xCoord},${light.yCoord}`,
	);
	const total = GRID_SIZE * GRID_SIZE;

	for (let i = 0; i < total; i++) {
		const x = i % GRID_SIZE;
		const y = Math.floor(i / GRID_SIZE);
		if (!existingCoords.includes(`${x},${y}`)) {
			return { xCoord: x, yCoord: y };
		}
	}
	return null;
};

export default function LightCoordsSection() {
	const { dimensions, containerRef } = useGetDivDimensions();
	const { Lights, saveLightOptions } = useLedStore();
	const { t } = useTranslation('');
	const { activateLedsOnId, activateLedsChase, turnOffLeds } = useLedsPreview();
	const [previewGpioPin, setPreviewGpioPin] = useState(0);
	const [previewCaseId, setPreviewCaseId] = useState(0);

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
		setSaveMessage('');
	}, []);

	return (
		<div className={`card`}>
			<div className={`card-header`}>
				<strong>Light Coordinates</strong>
			</div>
			<div className="card-body">
				<Row>
					<Col md={12}>
						<p>
							This section allows you to visually arrange and configure the
							position of each light. This is useful for mapping LEDs or light
							indicators to their physical locations on your device or case.
							Drag lights on the grid or adjust their coordinates manually to
							match your setup.
						</p>
					</Col>
				</Row>
				<hr />
				<Row className="mb-3">
					<Col md={6} className="d-flex flex-column justify-content-end">
						<FormSelect
							label={'Active light tied to GPIO pin'}
							className="form-select-sm"
							groupClassName="mb-3"
							value={previewGpioPin}
							onChange={(e) => {
								setPreviewGpioPin(
									parseInt((e.target as HTMLSelectElement).value),
								);
							}}
						>
							{Array.from({ length: GPIO_PIN_LENGTH }).map((_, pinIndex) => (
								<option key={pinIndex} value={pinIndex}>
									GPIO Pin {pinIndex}
								</option>
							))}
						</FormSelect>

						<Button
							variant="secondary"
							onClick={() => {
								activateLedsOnId(previewGpioPin);
							}}
						>
							GPIO Pin Test
						</Button>
					</Col>
					<Col md={6} className="d-flex flex-column justify-content-end">
						<FormSelect
							label={'Active light tied to case ID'}
							className="form-select-sm"
							groupClassName="mb-3"
							value={previewCaseId}
							onChange={(e) => {
								setPreviewCaseId(
									parseInt((e.target as HTMLSelectElement).value),
								);
							}}
						>
							{Array.from({ length: MAX_CASE_LIGHTS }).map((_, caseIndex) => (
								<option key={caseIndex} value={caseIndex}>
									Case ID {caseIndex + 1}
								</option>
							))}
						</FormSelect>

						<Button
							variant="secondary"
							onClick={() => {
								activateLedsOnId(previewCaseId, true);
							}}
						>
							Case ID Test
						</Button>
					</Col>
				</Row>
				<Row className="mb-3">
					<Col md={6} className="d-flex flex-column justify-content-end">
						<p>
							Run a chase animation from left to right and then top to bottom to
							help verify correct grid positioning of the lights
						</p>
						<Button
							variant="secondary"
							onClick={() => {
								activateLedsChase();
							}}
						>
							Layout Test
						</Button>
					</Col>

					<Col md={6} className="d-flex flex-column justify-content-end">
						<p>Turns off all the lights</p>
						<Button
							variant="danger"
							onClick={() => {
								turnOffLeds();
							}}
						>
							Lights Off
						</Button>
					</Col>
				</Row>
				<hr />
				<Formik
					validationSchema={schema}
					onSubmit={onSuccess}
					initialValues={{ Lights }}
					validateOnChange={true}
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
													<label className="form-label">
														{values.Lights[Number(selectedLight)]?.lightType ===
														1
															? ' Case Color'
															: ' GPIO Pin'}
													</label>
													<select
														className="form-select"
														name={`Lights[${selectedLight}].GPIOPinorCaseChainIndex`}
														value={
															values.Lights[Number(selectedLight)]
																?.GPIOPinorCaseChainIndex
														}
														onChange={handleChange}
													>
														{values.Lights[Number(selectedLight)]?.lightType ===
														1 ? (
															<>
																{Array.from({ length: MAX_CASE_LIGHTS }).map(
																	(_, caseIndex) => (
																		<option key={caseIndex} value={caseIndex}>
																			Case color {caseIndex + 1}
																		</option>
																	),
																)}
															</>
														) : (
															<>
																{Array.from({ length: GPIO_PIN_LENGTH }).map(
																	(_, pinIndex) => (
																		<option key={pinIndex} value={pinIndex}>
																			GPIO Pin {pinIndex}
																		</option>
																	),
																)}
															</>
														)}
													</select>
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
												<div className="d-flex flex-row gap-2 mb-2">
													<div className="flex-grow-1">
														<label className="form-label">X Coord</label>
														<input
															type="number"
															className="form-control"
															name={`Lights[${selectedLight}].xCoord`}
															value={
																values.Lights[Number(selectedLight)]?.xCoord
															}
															onChange={handleChange}
															min={0}
															max={gridSize - 1}
														/>
													</div>
													<div className="flex-grow-1">
														<label className="form-label">Y Coord</label>
														<input
															type="number"
															className="form-control"
															name={`Lights[${selectedLight}].yCoord`}
															value={
																values.Lights[Number(selectedLight)]?.yCoord
															}
															onChange={handleChange}
															min={0}
															max={gridSize - 1}
														/>
													</div>
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
											disabled={values.Lights.length >= MAX_LIGHTS}
											onClick={() => {
												const emptyCoord = getFirstEmptyLightCoord(
													values.Lights,
												);
												if (!emptyCoord) return;

												setValues({
													Lights: [
														...values.Lights,
														{
															GPIOPinorCaseChainIndex: 0,
															firstLedIndex: values.Lights.length,
															lightType: 0,
															numLedsOnLight: 1,
															xCoord: emptyCoord.xCoord,
															yCoord: emptyCoord.yCoord,
														},
													],
												});
												setSelectedLight(values.Lights.length);
											}}
										>
											Add light
										</Button>
									</div>
									<hr className="mt-3" />
									<Button
										className="w-100 mb-3"
										type="submit"
										disabled={!!errors.Lights}
									>
										{t('Common:button-save-label')}
									</Button>
									{saveMessage && <Alert variant="info">{saveMessage}</Alert>}

									{Array.isArray(errors.Lights) &&
										errors.Lights.some(Boolean) && (
											<Alert variant="danger">Verify light settings</Alert>
										)}
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
													error={errors?.Lights?.[index]?.toString()}
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
