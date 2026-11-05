import { useEffect, useState } from 'react';
import { Button, Form, Table } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import isEqual from 'lodash/isEqual';

import HEValueInput from './HEValueInput';
import useHETriggerStore, {
	getMuxCount,
	Trigger,
} from '../Store/useHETriggerStore';
import {
	formatDistance,
	formatReading,
	fromDepth,
} from '../Addons/HERapidTrigger';
import './HEVoltageTable.scss';

import {
	ADC_MAX,
	getTriggerLimits,
	normalizeTrigger,
	recalibrateTrigger,
} from '../Addons/HETriggerRules';

type HEVoltageTableProps = {
	triggers: Trigger[];
	values: { [key: string]: unknown };
	muxChannels: number;
	travelMm: number;
};

type RawReadingInputProps = {
	label: string;
	value: number;
	onChange: (value: number) => void;
};

// Raw ADC reading, committed on blur so partial input is never applied.
const RawReadingInput = ({ label, value, onChange }: RawReadingInputProps) => {
	const [text, setText] = useState(String(value));
	useEffect(() => setText(String(value)), [value]);
	return (
		<Form.Control
			type="number"
			size="sm"
			aria-label={label}
			min={0}
			max={ADC_MAX}
			step={1}
			value={text}
			onChange={(e) => setText(e.target.value)}
			onBlur={() => {
				const parsed = parseInt(text);
				const next = Number.isNaN(parsed)
					? value
					: Math.min(Math.max(parsed, 0), ADC_MAX);
				setText(String(next));
				if (next !== value) onChange(next);
			}}
		/>
	);
};

const HEVoltageTable = ({
	triggers,
	values,
	muxChannels,
	travelMm,
}: HEVoltageTableProps) => {
	const { t } = useTranslation('');
	const setHETrigger = useHETriggerStore((state) => state.setHETrigger);
	const savedTriggers = useHETriggerStore((state) => state.savedTriggers);
	const resetHETriggers = useHETriggerStore((state) => state.resetHETriggers);
	const [editing, setEditing] = useState(false);

	const unsavedCount = triggers.filter(
		(trigger, i) => !isEqual(trigger, savedTriggers[i]),
	).length;

	const update = (id: number, trigger: Trigger) =>
		setHETrigger({ id, ...trigger });

	// Actuation stays put (clamped to the new travel); polarity follows the higher reading.
	const setReading = (
		id: number,
		trigger: Trigger,
		field: 'idle' | 'pressed',
		value: number,
	) => {
		const next = { ...trigger, [field]: value };
		if (next.idle !== next.pressed)
			next.is_polarized = next.pressed < next.idle;
		update(id, normalizeTrigger(next));
	};

	// Flipping polarity swaps idle and pressed, as in the calibration modal.
	const setPolarity = (id: number, trigger: Trigger, polarized: boolean) => {
		const high = Math.max(trigger.idle, trigger.pressed);
		const low = Math.min(trigger.idle, trigger.pressed);
		update(
			id,
			polarized
				? recalibrateTrigger(trigger, high, low, true)
				: recalibrateTrigger(trigger, low, high, false),
		);
	};

	// Idle and pressed are edited as raw values even when a switch travel is set.
	const rawUnit = t('HETrigger:unit-raw');
	const distanceUnit = travelMm > 0 ? t('HETrigger:unit-mm') : rawUnit;
	const readingUnit = editing ? rawUnit : distanceUnit;
	const header = (labelKey: string, unit?: string) => (
		<th>
			{t(labelKey)}
			{unit && <span className="he-unit">{unit}</span>}
		</th>
	);

	const naLabel = t('HETrigger:voltage-table-na-label');
	const onOff = (on: boolean) =>
		on ? t('Common:status-enabled') : t('Common:status-disabled');

	const readRow = (trigger: Trigger) => (
		<>
			<td>{formatReading(trigger.idle, travelMm, trigger)}</td>
			<td>{formatReading(trigger.active, travelMm, trigger)}</td>
			<td>{formatReading(trigger.pressed, travelMm, trigger)}</td>
			<td>{trigger.is_polarized ? 'S' : 'N'}</td>
			<td>{onOff(trigger.rapidTrigger)}</td>
			<td>
				{trigger.rapidTrigger ? onOff(trigger.rtSeparateSensitivity) : naLabel}
			</td>
			<td>
				{trigger.rapidTrigger
					? formatDistance(trigger.rtPressSensitivity, travelMm, trigger)
					: naLabel}
			</td>
			<td>
				{trigger.rapidTrigger
					? formatDistance(
							trigger.rtSeparateSensitivity
								? trigger.rtReleaseSensitivity
								: trigger.rtPressSensitivity,
							travelMm,
							trigger,
						)
					: naLabel}
			</td>
			<td>{trigger.rapidTrigger ? onOff(trigger.rtContinuous) : naLabel}</td>
			<td>{formatDistance(trigger.noise, travelMm, trigger)}</td>
		</>
	);

	const editRow = (id: number, trigger: Trigger) => {
		const { span, minActuation, minSensitivity } = getTriggerLimits(trigger);
		const distanceInput = (
			label: string,
			name: string,
			value: number,
			field: keyof Trigger,
			min: number,
			disabled = false,
		) => (
			<fieldset disabled={disabled}>
				<HEValueInput
					hideLabel
					label={label}
					name={`${name}-${id}`}
					kind="distance"
					value={value}
					calibration={trigger}
					travelMm={travelMm}
					min={min}
					max={span}
					onChange={(v) =>
						update(id, normalizeTrigger({ ...trigger, [field]: v }))
					}
				/>
			</fieldset>
		);
		const check = (
			label: string,
			field: keyof Trigger,
			disabled = false,
			onChange?: (checked: boolean) => void,
		) => (
			<Form.Check
				type="switch"
				id={`he-table-${field}-${id}`}
				aria-label={label}
				disabled={disabled}
				checked={Boolean(trigger[field])}
				onChange={(e) =>
					onChange
						? onChange(e.target.checked)
						: update(id, { ...trigger, [field]: e.target.checked })
				}
			/>
		);
		return (
			<>
				<td>
					<RawReadingInput
						label={t('HETrigger:voltage-table-idle-text')}
						value={trigger.idle}
						onChange={(v) => setReading(id, trigger, 'idle', v)}
					/>
				</td>
				<td>
					<HEValueInput
						hideLabel
						label={t('HETrigger:voltage-table-trigger-text')}
						name={`active-${id}`}
						kind="reading"
						value={trigger.active}
						calibration={trigger}
						travelMm={travelMm}
						min={fromDepth(minActuation, trigger)}
						max={trigger.pressed}
						onChange={(v) => update(id, { ...trigger, active: v })}
					/>
				</td>
				<td>
					<RawReadingInput
						label={t('HETrigger:voltage-table-pressed-text')}
						value={trigger.pressed}
						onChange={(v) => setReading(id, trigger, 'pressed', v)}
					/>
				</td>
				<td>
					<Form.Select
						size="sm"
						aria-label={t('HETrigger:voltage-table-polarity-text')}
						value={trigger.is_polarized ? 'S' : 'N'}
						onChange={(e) => setPolarity(id, trigger, e.target.value === 'S')}
					>
						<option value="N">N</option>
						<option value="S">S</option>
					</Form.Select>
				</td>
				<td>
					{check(
						t('HETrigger:voltage-table-rapid-trigger-text'),
						'rapidTrigger',
					)}
				</td>
				<td>
					{check(
						t('HETrigger:voltage-table-separate-text'),
						'rtSeparateSensitivity',
						!trigger.rapidTrigger,
						(checked) =>
							update(id, {
								...trigger,
								rtSeparateSensitivity: checked,
								// Start the upstroke where the shared sensitivity was, like the modal
								rtReleaseSensitivity: checked
									? trigger.rtPressSensitivity
									: trigger.rtReleaseSensitivity,
							}),
					)}
				</td>
				<td>
					{distanceInput(
						t('HETrigger:voltage-table-press-sensitivity-text'),
						'rtPressSensitivity',
						trigger.rtPressSensitivity,
						'rtPressSensitivity',
						minSensitivity,
						!trigger.rapidTrigger,
					)}
				</td>
				<td>
					{distanceInput(
						t('HETrigger:voltage-table-release-sensitivity-text'),
						'rtReleaseSensitivity',
						trigger.rtReleaseSensitivity,
						'rtReleaseSensitivity',
						minSensitivity,
						!trigger.rapidTrigger || !trigger.rtSeparateSensitivity,
					)}
				</td>
				<td>
					{check(
						t('HETrigger:voltage-table-continuous-text'),
						'rtContinuous',
						!trigger.rapidTrigger,
					)}
				</td>
				<td>
					{distanceInput(
						t('HETrigger:voltage-table-noise-text'),
						'noise',
						trigger.noise,
						'noise',
						0,
					)}
				</td>
			</>
		);
	};

	return (
		<>
			<div>
				<h1>{t('HETrigger:voltage-table-header-text')}</h1>
			</div>
			<div className="d-flex flex-wrap align-items-center gap-3">
				<Form.Check
					type="switch"
					id="he-voltage-table-edit"
					label={t('HETrigger:voltage-table-edit-label')}
					checked={editing}
					onChange={(e) => setEditing(e.target.checked)}
				/>
				{unsavedCount > 0 && (
					<>
						<span className="text-primary-emphasis">
							{t('HETrigger:voltage-table-unsaved-text', {
								count: unsavedCount,
							})}
						</span>
						<Button
							size="sm"
							variant="outline-danger"
							onClick={() => {
								if (
									window.confirm(t('HETrigger:voltage-table-discard-confirm'))
								)
									resetHETriggers();
							}}
						>
							{t('HETrigger:voltage-table-discard-button')}
						</Button>
					</>
				)}
			</div>
			{Array.from({ length: getMuxCount(muxChannels) }, (_, i) => (
				<div
					key={`voltage-table-header-${i}`}
					className="mt-3 mb-3"
					hidden={Number(values[`muxADCPin${i}`]) === -1}
				>
					<div className="d-flex flex-shrink-0">
						<label>
							{muxChannels > 1
								? `${t('HETrigger:multiplexer-label')} ${i}`
								: t('HETrigger:direct-label')}{' '}
							{`(ADC ${String(values[`muxADCPin${i}`])})`}
						</label>
					</div>
					{values[`muxADCPin${i}`] !== -1 && (
						<div
							className={
								editing
									? 'mt-0 mb-0'
									: `action-grid-HE-trigger-${muxChannels} gap-0 mt-0 mb-0 d-inline-flex`
							}
						>
							<Table
								bordered
								responsive={editing}
								size={editing ? 'sm' : undefined}
								className="he-voltage-table mb-0 mt-0"
							>
								<thead>
									<tr>
										{header('HETrigger:channel-label')}
										{header('HETrigger:voltage-table-idle-text', readingUnit)}
										{header(
											'HETrigger:voltage-table-trigger-text',
											distanceUnit,
										)}
										{header(
											'HETrigger:voltage-table-pressed-text',
											readingUnit,
										)}
										{header('HETrigger:voltage-table-polarity-text', 'N / S')}
										{header('HETrigger:voltage-table-rapid-trigger-text')}
										{header('HETrigger:voltage-table-separate-text')}
										{header(
											'HETrigger:voltage-table-press-sensitivity-text',
											distanceUnit,
										)}
										{header(
											'HETrigger:voltage-table-release-sensitivity-text',
											distanceUnit,
										)}
										{header('HETrigger:voltage-table-continuous-text')}
										{header('HETrigger:voltage-table-noise-text', distanceUnit)}
									</tr>
								</thead>
								<tbody>
									{triggers
										.map((_, triggerIndex) => triggerIndex)
										.splice(i * muxChannels, muxChannels)
										.map((key, index) => (
											<tr
												key={`table-tr-triggers-${key}`}
												className={
													isEqual(triggers[key], savedTriggers[key])
														? undefined
														: 'he-row-unsaved'
												}
											>
												<td>
													{index}{' '}
													{triggers[key].action === -10
														? t('HETrigger:voltage-table-disabled-label')
														: ''}
												</td>
												{editing
													? editRow(key, triggers[key])
													: readRow(triggers[key])}
											</tr>
										))}
								</tbody>
							</Table>
						</div>
					)}
				</div>
			))}
		</>
	);
};

export default HEVoltageTable;
