import { useCallback, useRef } from 'react';
import { Col, FormCheck, Row } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

import './AbsoluteAnalog.scss';

// Stored as -255 to 255; the firmware scales these to the gamepad axis range
export const ABSOLUTE_ANALOG_AXIS_MAX = 255;

const PAD_SIZE = 180;
const PAD_CENTER = PAD_SIZE / 2;
const PAD_RADIUS = 70;

const ENTRIES = [
	{
		number: 1,
		enabledKey: 'absoluteAnalog1Enabled',
		stickKey: 'absoluteAnalog1Stick',
		xKey: 'absoluteAnalog1X',
		yKey: 'absoluteAnalog1Y',
	},
	{
		number: 2,
		enabledKey: 'absoluteAnalog2Enabled',
		stickKey: 'absoluteAnalog2Stick',
		xKey: 'absoluteAnalog2X',
		yKey: 'absoluteAnalog2Y',
	},
	{
		number: 3,
		enabledKey: 'absoluteAnalog3Enabled',
		stickKey: 'absoluteAnalog3Stick',
		xKey: 'absoluteAnalog3X',
		yKey: 'absoluteAnalog3Y',
	},
	{
		number: 4,
		enabledKey: 'absoluteAnalog4Enabled',
		stickKey: 'absoluteAnalog4Stick',
		xKey: 'absoluteAnalog4X',
		yKey: 'absoluteAnalog4Y',
	},
	{
		number: 5,
		enabledKey: 'absoluteAnalog5Enabled',
		stickKey: 'absoluteAnalog5Stick',
		xKey: 'absoluteAnalog5X',
		yKey: 'absoluteAnalog5Y',
	},
	{
		number: 6,
		enabledKey: 'absoluteAnalog6Enabled',
		stickKey: 'absoluteAnalog6Stick',
		xKey: 'absoluteAnalog6X',
		yKey: 'absoluteAnalog6Y',
	},
	{
		number: 7,
		enabledKey: 'absoluteAnalog7Enabled',
		stickKey: 'absoluteAnalog7Stick',
		xKey: 'absoluteAnalog7X',
		yKey: 'absoluteAnalog7Y',
	},
	{
		number: 8,
		enabledKey: 'absoluteAnalog8Enabled',
		stickKey: 'absoluteAnalog8Stick',
		xKey: 'absoluteAnalog8X',
		yKey: 'absoluteAnalog8Y',
	},
	{
		number: 9,
		enabledKey: 'absoluteAnalog9Enabled',
		stickKey: 'absoluteAnalog9Stick',
		xKey: 'absoluteAnalog9X',
		yKey: 'absoluteAnalog9Y',
	},
] as const;

const clampAxis = (value: number) => {
	const rounded = Math.round(Number(value));
	if (Number.isNaN(rounded)) return 0;
	return Math.min(
		Math.max(rounded, -ABSOLUTE_ANALOG_AXIS_MAX),
		ABSOLUTE_ANALOG_AXIS_MAX,
	);
};

const axisToPad = (value: number) =>
	PAD_CENTER + (clampAxis(value) / ABSOLUTE_ANALOG_AXIS_MAX) * PAD_RADIUS;

type AnalogPadProps = {
	x: number;
	y: number;
	disabled: boolean;
	labels: { up: string; down: string; left: string; right: string };
	onChange: (x: number, y: number) => void;
};

const AnalogPad = ({ x, y, disabled, labels, onChange }: AnalogPadProps) => {
	const svgRef = useRef<SVGSVGElement>(null);
	const dragging = useRef(false);

	const updateFromEvent = useCallback(
		(event: React.PointerEvent<SVGSVGElement>) => {
			const svg = svgRef.current;
			if (!svg) return;

			const rect = svg.getBoundingClientRect();
			const normalizedX =
				(((event.clientX - rect.left) / rect.width) * PAD_SIZE - PAD_CENTER) /
				PAD_RADIUS;
			const normalizedY =
				(((event.clientY - rect.top) / rect.height) * PAD_SIZE - PAD_CENTER) /
				PAD_RADIUS;

			// Dragging is clamped to the circle; typed values can still reach the corners
			const magnitude = Math.hypot(normalizedX, normalizedY);
			const scale = magnitude > 1 ? 1 / magnitude : 1;

			onChange(
				clampAxis(normalizedX * scale * ABSOLUTE_ANALOG_AXIS_MAX),
				clampAxis(normalizedY * scale * ABSOLUTE_ANALOG_AXIS_MAX),
			);
		},
		[onChange],
	);

	return (
		<svg
			ref={svgRef}
			className={`absolute-analog-pad${disabled ? ' disabled' : ''}`}
			width={PAD_SIZE}
			height={PAD_SIZE}
			viewBox={`0 0 ${PAD_SIZE} ${PAD_SIZE}`}
			onPointerDown={(event) => {
				if (disabled) return;
				dragging.current = true;
				event.currentTarget.setPointerCapture(event.pointerId);
				updateFromEvent(event);
			}}
			onPointerMove={(event) => {
				if (disabled || !dragging.current) return;
				updateFromEvent(event);
			}}
			onPointerUp={(event) => {
				dragging.current = false;
				if (event.currentTarget.hasPointerCapture(event.pointerId)) {
					event.currentTarget.releasePointerCapture(event.pointerId);
				}
			}}
			onPointerCancel={() => {
				dragging.current = false;
			}}
		>
			<circle
				className="absolute-analog-pad-guide"
				cx={PAD_CENTER}
				cy={PAD_CENTER}
				r={PAD_RADIUS}
			/>
			<line
				className="absolute-analog-pad-line"
				x1={PAD_CENTER - PAD_RADIUS}
				y1={PAD_CENTER}
				x2={PAD_CENTER + PAD_RADIUS}
				y2={PAD_CENTER}
			/>
			<line
				className="absolute-analog-pad-line"
				x1={PAD_CENTER}
				y1={PAD_CENTER - PAD_RADIUS}
				x2={PAD_CENTER}
				y2={PAD_CENTER + PAD_RADIUS}
			/>
			<text
				className="absolute-analog-pad-label"
				x={PAD_CENTER}
				y={12}
				textAnchor="middle"
			>
				{labels.up}
			</text>
			<text
				className="absolute-analog-pad-label"
				x={PAD_CENTER}
				y={PAD_SIZE - 4}
				textAnchor="middle"
			>
				{labels.down}
			</text>
			<text
				className="absolute-analog-pad-label"
				x={4}
				y={PAD_CENTER + 4}
				textAnchor="start"
			>
				{labels.left}
			</text>
			<text
				className="absolute-analog-pad-label"
				x={PAD_SIZE - 4}
				y={PAD_CENTER + 4}
				textAnchor="end"
			>
				{labels.right}
			</text>
			<circle
				className="absolute-analog-pad-dot"
				cx={axisToPad(x)}
				cy={axisToPad(y)}
				r={7}
			/>
		</svg>
	);
};

export const absoluteAnalogScheme = {
	AbsoluteAnalogAddonEnabled: yup
		.number()
		.required()
		.label('Absolute Analog Add-On Enabled'),
	absoluteAnalog1Enabled: yup
		.number()
		.label('Absolute Analog 1 Enabled')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog1Stick: yup
		.number()
		.label('Absolute Analog 1 Stick')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog1X: yup
		.number()
		.label('Absolute Analog 1 X')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog1Y: yup
		.number()
		.label('Absolute Analog 1 Y')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog2Enabled: yup
		.number()
		.label('Absolute Analog 2 Enabled')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog2Stick: yup
		.number()
		.label('Absolute Analog 2 Stick')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog2X: yup
		.number()
		.label('Absolute Analog 2 X')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog2Y: yup
		.number()
		.label('Absolute Analog 2 Y')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog3Enabled: yup
		.number()
		.label('Absolute Analog 3 Enabled')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog3Stick: yup
		.number()
		.label('Absolute Analog 3 Stick')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog3X: yup
		.number()
		.label('Absolute Analog 3 X')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog3Y: yup
		.number()
		.label('Absolute Analog 3 Y')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog4Enabled: yup
		.number()
		.label('Absolute Analog 4 Enabled')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog4Stick: yup
		.number()
		.label('Absolute Analog 4 Stick')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog4X: yup
		.number()
		.label('Absolute Analog 4 X')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog4Y: yup
		.number()
		.label('Absolute Analog 4 Y')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog5Enabled: yup
		.number()
		.label('Absolute Analog 5 Enabled')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog5Stick: yup
		.number()
		.label('Absolute Analog 5 Stick')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog5X: yup
		.number()
		.label('Absolute Analog 5 X')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog5Y: yup
		.number()
		.label('Absolute Analog 5 Y')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog6Enabled: yup
		.number()
		.label('Absolute Analog 6 Enabled')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog6Stick: yup
		.number()
		.label('Absolute Analog 6 Stick')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog6X: yup
		.number()
		.label('Absolute Analog 6 X')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog6Y: yup
		.number()
		.label('Absolute Analog 6 Y')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog7Enabled: yup
		.number()
		.label('Absolute Analog 7 Enabled')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog7Stick: yup
		.number()
		.label('Absolute Analog 7 Stick')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog7X: yup
		.number()
		.label('Absolute Analog 7 X')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog7Y: yup
		.number()
		.label('Absolute Analog 7 Y')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog8Enabled: yup
		.number()
		.label('Absolute Analog 8 Enabled')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog8Stick: yup
		.number()
		.label('Absolute Analog 8 Stick')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog8X: yup
		.number()
		.label('Absolute Analog 8 X')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog8Y: yup
		.number()
		.label('Absolute Analog 8 Y')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog9Enabled: yup
		.number()
		.label('Absolute Analog 9 Enabled')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog9Stick: yup
		.number()
		.label('Absolute Analog 9 Stick')
		.validateRangeWhenValue('AbsoluteAnalogAddonEnabled', 0, 1),
	absoluteAnalog9X: yup
		.number()
		.label('Absolute Analog 9 X')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
	absoluteAnalog9Y: yup
		.number()
		.label('Absolute Analog 9 Y')
		.validateRangeWhenValue(
			'AbsoluteAnalogAddonEnabled',
			-ABSOLUTE_ANALOG_AXIS_MAX,
			ABSOLUTE_ANALOG_AXIS_MAX,
		),
};

export const absoluteAnalogState = {
	AbsoluteAnalogAddonEnabled: 0,
	absoluteAnalog1Enabled: 0,
	absoluteAnalog1Stick: 0,
	absoluteAnalog1X: 0,
	absoluteAnalog1Y: 0,
	absoluteAnalog2Enabled: 0,
	absoluteAnalog2Stick: 0,
	absoluteAnalog2X: 0,
	absoluteAnalog2Y: 0,
	absoluteAnalog3Enabled: 0,
	absoluteAnalog3Stick: 0,
	absoluteAnalog3X: 0,
	absoluteAnalog3Y: 0,
	absoluteAnalog4Enabled: 0,
	absoluteAnalog4Stick: 0,
	absoluteAnalog4X: 0,
	absoluteAnalog4Y: 0,
	absoluteAnalog5Enabled: 0,
	absoluteAnalog5Stick: 0,
	absoluteAnalog5X: 0,
	absoluteAnalog5Y: 0,
	absoluteAnalog6Enabled: 0,
	absoluteAnalog6Stick: 0,
	absoluteAnalog6X: 0,
	absoluteAnalog6Y: 0,
	absoluteAnalog7Enabled: 0,
	absoluteAnalog7Stick: 0,
	absoluteAnalog7X: 0,
	absoluteAnalog7Y: 0,
	absoluteAnalog8Enabled: 0,
	absoluteAnalog8Stick: 0,
	absoluteAnalog8X: 0,
	absoluteAnalog8Y: 0,
	absoluteAnalog9Enabled: 0,
	absoluteAnalog9Stick: 0,
	absoluteAnalog9X: 0,
	absoluteAnalog9Y: 0,
};

const AbsoluteAnalog = ({
	values,
	errors,
	handleChange,
	handleCheckbox,
	setFieldValue,
}: AddonPropTypes) => {
	const { t } = useTranslation();

	const padLabels = {
		up: t('AbsoluteAnalog:pad-up-label'),
		down: t('AbsoluteAnalog:pad-down-label'),
		left: t('AbsoluteAnalog:pad-left-label'),
		right: t('AbsoluteAnalog:pad-right-label'),
	};

	return (
		<Section title={t('AbsoluteAnalog:header-text')}>
			<div
				id="AbsoluteAnalogAddonOptions"
				hidden={!values.AbsoluteAnalogAddonEnabled}
			>
				<div className="alert alert-info" role="alert">
					{t('AbsoluteAnalog:desc-text')}
				</div>
				<Row className="g-3">
					{ENTRIES.map(({ number, enabledKey, stickKey, xKey, yKey }) => (
						<Col key={enabledKey} md={6} xxl={4}>
							<div className="absolute-analog-entry h-100">
								<div className="d-flex justify-content-between align-items-center mb-2">
									<strong>
										{t('AbsoluteAnalog:entry-label', { index: number })}
									</strong>
									<FormCheck
										label={t('Common:switch-enabled')}
										type="switch"
										id={`${enabledKey}Button`}
										reverse
										isInvalid={false}
										checked={Boolean(values[enabledKey])}
										onChange={() => handleCheckbox(enabledKey)}
									/>
								</div>
								<div className="d-flex flex-wrap gap-3 align-items-center">
									<AnalogPad
										x={values[xKey]}
										y={values[yKey]}
										disabled={!values[enabledKey]}
										labels={padLabels}
										onChange={(x, y) => {
											setFieldValue(xKey, x);
											setFieldValue(yKey, y);
										}}
									/>
									<div className="flex-grow-1">
										<FormSelect
											label={t('AbsoluteAnalog:stick-label')}
											name={stickKey}
											className="form-select-sm"
											groupClassName="mb-2"
											value={values[stickKey]}
											error={errors[stickKey]}
											isInvalid={Boolean(errors[stickKey])}
											onChange={handleChange}
										>
											<option value={0}>
												{t('AbsoluteAnalog:stick-left-label')}
											</option>
											<option value={1}>
												{t('AbsoluteAnalog:stick-right-label')}
											</option>
										</FormSelect>
										<FormControl
											label={t('AbsoluteAnalog:x-label')}
											name={xKey}
											className="form-control-sm"
											groupClassName="mb-2"
											type="number"
											min={-ABSOLUTE_ANALOG_AXIS_MAX}
											max={ABSOLUTE_ANALOG_AXIS_MAX}
											value={values[xKey]}
											error={errors[xKey]}
											isInvalid={Boolean(errors[xKey])}
											onChange={(e: React.ChangeEvent<HTMLInputElement>) =>
												setFieldValue(xKey, clampAxis(Number(e.target.value)))
											}
										/>
										<FormControl
											label={t('AbsoluteAnalog:y-label')}
											name={yKey}
											className="form-control-sm"
											type="number"
											min={-ABSOLUTE_ANALOG_AXIS_MAX}
											max={ABSOLUTE_ANALOG_AXIS_MAX}
											value={values[yKey]}
											error={errors[yKey]}
											isInvalid={Boolean(errors[yKey])}
											onChange={(e: React.ChangeEvent<HTMLInputElement>) =>
												setFieldValue(yKey, clampAxis(Number(e.target.value)))
											}
										/>
									</div>
								</div>
							</div>
						</Col>
					))}
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="AbsoluteAnalogAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.AbsoluteAnalogAddonEnabled)}
				onChange={() => handleCheckbox('AbsoluteAnalogAddonEnabled')}
			/>
		</Section>
	);
};

export default AbsoluteAnalog;
