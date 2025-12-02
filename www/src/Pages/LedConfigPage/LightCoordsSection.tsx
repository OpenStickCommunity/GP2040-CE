import { useState, useCallback, useEffect } from 'react';
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
import { FormikErrors, FormikHandlers } from 'formik';
import { Row, Col, Button, Alert } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';

import {
	AnimationOptions,
	LedOptions,
	Light,
	MAX_CASE_LIGHTS,
} from '../../Store/useLedStore';
import useLedsPreview from '../../Hooks/useLedsPreview';
import { useGetContainerDimensions } from '../../Hooks/useGetContainerDimensions';

import FormControl from '../../Components/FormControl';
import FormSelect from '../../Components/FormSelect';

import { LED_COLORS, LIGHT_TYPES } from '../../Data/Leds';
import boards from '../../Data/Boards.json';

import { rgbIntToHex } from '../../Services/Utilities';
import ColorSelector from './ColorSlector';
import { LightIndicator } from './LightIndicator';

const GRID_SIZE = 30;
const GPIO_PIN_LENGTH =
	boards[import.meta.env.VITE_GP2040_BOARD as keyof typeof boards].maxPin + 1;

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

export default function LightCoordsSection({
	pressedStaticColors,
	notPressedStaticColors,
	caseStaticColors,
	profileIndex,
	values,
	errors,
	setFieldValue,
	setValues,
	handleChange,
}: {
	pressedStaticColors: number[];
	notPressedStaticColors: number[];
	caseStaticColors: number[];
	profileIndex: number;
	values: {
		ledOptions: LedOptions;
		Lights: Light[];
		AnimationOptions: AnimationOptions;
	};
	errors: FormikErrors<{
		AnimationOptions: AnimationOptions;
		Lights: Light[];
	}>;
	setFieldValue: (field: string, value: any, shouldValidate?: boolean) => void;
	setValues: (
		values: {
			ledOptions: LedOptions;
			Lights: Light[];
			AnimationOptions: AnimationOptions;
		},
		shouldValidate?: boolean,
	) => void;
	handleChange: FormikHandlers['handleChange'];
}) {
	const { dimensions, containerRef } = useGetContainerDimensions();
	const { t } = useTranslation('');
	const { activateLedsOnId, activateLedsChase, turnOffLeds } = useLedsPreview();
	const [previewGpioPin, setPreviewGpioPin] = useState(0);
	const [previewCaseId, setPreviewCaseId] = useState(0);

	const [gridSize, setGridSize] = useState(GRID_SIZE);
	const [cellWidth, setCellWidth] = useState(dimensions.width / gridSize);
	const [selectedLight, setSelectedLight] = useState<number | null>(null);

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

	const handleDeleteLight = useCallback(
		(index: number) => {
			setValues({
				...values,
				Lights: values.Lights.filter((_, i) => i !== index),
			});
			if (selectedLight === index) {
				setSelectedLight(null);
			}
		},
		[setValues, values, selectedLight],
	);

	const customColorOptions = values.AnimationOptions.customColors.map(
		(color, index) => ({
			value: LED_COLORS.length + index,
			label: `Custom ${index + 1}`,
			color: rgbIntToHex(color),
		}),
	);
	const colorOptions = [...LED_COLORS, ...customColorOptions];

	return (
		<div>
			<p>
				This section allows you to visually arrange and configure the position
				of each light. This is useful for mapping LEDs or light indicators to
				their physical locations on your device or case. Drag lights on the grid
				or adjust their coordinates manually to match your setup.
			</p>
			<Alert variant="warning">
				Changing layout configuration options can break your LED setup. Proceed
				with caution.
			</Alert>
			<hr />
			<Row className="mb-3">
				<Col md={3} className="d-flex flex-column justify-content-end mb-2">
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
				<Col md={3} className="d-flex flex-column justify-content-end mb-2">
					<FormSelect
						label={'Active light tied to case ID'}
						className="form-select-sm"
						groupClassName="mb-3"
						value={previewCaseId}
						onChange={(e) => {
							setPreviewCaseId(parseInt((e.target as HTMLSelectElement).value));
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
				<Col md={3} className="d-flex flex-column justify-content-end mb-2">
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
				<Col md={3} className="d-flex flex-column justify-content-end mb-2">
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
			<Row className="mb-3">
				<Col md={3}>
					<div className="d-flex flex-grow-1">
						{selectedLight !== null ? (
							<div className="w-100 mb-3">
								<h3>Light {selectedLight + 1}</h3>
								<FormControl
									type="number"
									label={'First LED Index'}
									name={`Lights[${selectedLight}].firstLedIndex`}
									className="form-control"
									groupClassName="mb-3"
									value={values.Lights[selectedLight]?.firstLedIndex}
									// TODO: Fix error typing
									error={(errors.Lights?.[selectedLight] as any)?.firstLedIndex}
									isInvalid={Boolean(
										(errors.Lights?.[selectedLight] as any)?.firstLedIndex,
									)}
									onChange={handleChange}
									min={0}
								/>
								<FormControl
									type="number"
									label={'Number of LEDs on Light'}
									name={`Lights[${selectedLight}].numLedsOnLight`}
									className="form-control"
									groupClassName="mb-3"
									value={values.Lights[selectedLight]?.numLedsOnLight}
									// TODO: Fix error typing
									error={
										(errors.Lights?.[selectedLight] as any)?.numLedsOnLight
									}
									isInvalid={Boolean(
										(errors.Lights?.[selectedLight] as any)?.numLedsOnLight,
									)}
									onChange={handleChange}
									min={1}
								/>
								<FormSelect
									label={
										values.Lights[selectedLight]?.lightType == LIGHT_TYPES.Case
											? ' Case Id'
											: ' GPIO Pin'
									}
									className="form-select"
									groupClassName="mb-3"
									value={values.Lights[selectedLight]?.GPIOPinorCaseChainIndex}
									onChange={handleChange}
									name={`Lights[${selectedLight}].GPIOPinorCaseChainIndex`}
								>
									{values.Lights[selectedLight]?.lightType == LIGHT_TYPES.Case ? (
										<>
											{Array.from({ length: MAX_CASE_LIGHTS }).map(
												(_, caseIndex) => (
													<option key={caseIndex} value={caseIndex}>
														Case ID {caseIndex + 1}
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
								</FormSelect>
								{values.Lights[selectedLight]?.lightType == LIGHT_TYPES.Case ? (
									<div className="mb-3">
										<label className="form-label">Case Color</label>
										<ColorSelector
											options={colorOptions}
											value={
												colorOptions[
													caseStaticColors[
														values.Lights[selectedLight].GPIOPinorCaseChainIndex
													]
												]
											}
											onChange={(selected) => {
												setFieldValue(
													`AnimationOptions.profiles.${profileIndex}.caseStaticColors.${values.Lights[selectedLight].GPIOPinorCaseChainIndex}`,
													selected?.value || 0,
												);
											}}
										/>
									</div>
								) : (
									<>
										<div className="mb-3">
											<label className="form-label">GPIO Idle Color</label>
											<ColorSelector
												options={colorOptions}
												value={
													colorOptions[
														notPressedStaticColors[
															values.Lights[selectedLight]
																.GPIOPinorCaseChainIndex
														]
													]
												}
												onChange={(selected) => {
													setFieldValue(
														`AnimationOptions.profiles.${profileIndex}.notPressedStaticColors.${values.Lights[selectedLight].GPIOPinorCaseChainIndex}`,
														selected?.value || 0,
													);
												}}
											/>
										</div>
										<div className="mb-3">
											<label className="form-label">Pressed Color</label>
											<ColorSelector
												options={colorOptions}
												value={
													colorOptions[
														pressedStaticColors[
															values.Lights[selectedLight]
																.GPIOPinorCaseChainIndex
														]
													]
												}
												onChange={(selected) => {
													setFieldValue(
														`AnimationOptions.profiles.${profileIndex}.pressedStaticColors.${values.Lights[selectedLight].GPIOPinorCaseChainIndex}`,
														selected?.value || 0,
													);
												}}
											/>
										</div>
									</>
								)}
								<FormSelect
									label="Light Type"
									className="form-select"
									groupClassName="mb-3"
									value={values.Lights[selectedLight]?.lightType}
									onChange={(e) => {
										setFieldValue(
											`Lights[${selectedLight}].lightType`,
											parseInt(e.target.value),
										);
									}}
									name={`Lights[${selectedLight}].lightType`}
								>
									<option value={LIGHT_TYPES.ActionButton}>ActionButton</option>
									<option value={LIGHT_TYPES.Case}>Case</option>
									<option value={LIGHT_TYPES.Turbo}>Turbo</option>
									<option value={LIGHT_TYPES.PlayerLight}>PlayerLight</option>
								</FormSelect>

								<div className="d-flex flex-row gap-2 mb-3">
									<div className="flex-grow-1">
										<FormControl
											type="number"
											label={'X Coord'}
											name={`Lights[${selectedLight}].xCoord`}
											className="form-control"
											value={values.Lights[selectedLight]?.xCoord}
											onChange={handleChange}
											error={(errors.Lights?.[selectedLight] as any)?.xCoord}
											isInvalid={Boolean(
												(errors.Lights?.[selectedLight] as any)?.xCoord,
											)}
											min={0}
											max={gridSize - 1}
										/>
									</div>
									<div className="flex-grow-1">
										<FormControl
											type="number"
											label={'Y Coord'}
											name={`Lights[${selectedLight}].yCoord`}
											className="form-control"
											value={values.Lights[selectedLight]?.yCoord}
											onChange={handleChange}
											error={(errors.Lights?.[selectedLight] as any)?.yCoord}
											isInvalid={Boolean(
												(errors.Lights?.[selectedLight] as any)?.yCoord,
											)}
											min={0}
											max={gridSize - 1}
										/>
									</div>
								</div>
								<Button
									variant="danger"
									className="w-100 mt-3"
									onClick={() => {
										handleDeleteLight(selectedLight);
									}}
								>
									Delete Light
								</Button>
							</div>
						) : (
							<div className="w-100 d-flex flex-column justify-content-center">
								<h3>Select a Light</h3>
								<p>
									Click on a light in the grid to view and edit its properties.
								</p>
							</div>
						)}
					</div>
					{/* <div className="d-flex flex-column justify-content-between align-items-center">
						<Button
							className="w-100"
							variant="secondary"
							disabled={values.Lights.length >= MAX_LIGHTS}
							onClick={() => {
								const emptyCoord = getFirstEmptyLightCoord(values.Lights);
								if (!emptyCoord) return;

								setValues({
									AnimationOptions: values.AnimationOptions,
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
					</div> */}
				</Col>
				<Col md={9} className="pt-2 mt-md-0">
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
							onClick={(e) => {
								const rect = e.currentTarget.getBoundingClientRect();

								const gridX = Math.floor((e.clientX - rect.left) / cellWidth);
								const gridY = Math.floor((e.clientY - rect.top) / cellWidth);
								const clampedX = Math.max(0, Math.min(gridX, gridSize - 1));
								const clampedY = Math.max(0, Math.min(gridY, gridSize - 1));

								setValues({
									...values,
									Lights: [
										...values.Lights,
										{
											GPIOPinorCaseChainIndex: 0,
											firstLedIndex: values.Lights.length,
											lightType: LIGHT_TYPES.ActionButton,
											numLedsOnLight: 1,
											xCoord: clampedX,
											yCoord: clampedY,
										},
									],
								});
								setSelectedLight(values.Lights.length);
							}}
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
										values.Lights[activeId].xCoord * cellWidth + event.delta.x,
									),
									yCoord: gridPxToCoords(
										values.Lights[activeId].yCoord * cellWidth + event.delta.y,
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
		</div>
	);
}
