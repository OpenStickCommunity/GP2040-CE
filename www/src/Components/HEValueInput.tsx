import { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';

import FormControl from './FormControl';
import {
	TravelCalibration,
	canConvert,
	countsToMm,
	mmToCounts,
	mmToReading,
	readingToMm,
} from '../Addons/HERapidTrigger';

type HEValueInputProps = {
	label: string;
	name: string;
	/** ADC counts: an absolute reading or a distance, depending on `kind`. */
	value: number;
	kind: 'reading' | 'distance';
	calibration: TravelCalibration;
	travelMm: number;
	min: number;
	max: number;
	onChange: (counts: number) => void;
};

const clamp = (v: number, lo: number, hi: number) =>
	Math.min(Math.max(v, lo), hi);

/** Stores ADC counts but shows and accepts mm when a switch travel is set. */
const HEValueInput = ({
	label,
	name,
	value,
	kind,
	calibration,
	travelMm,
	min,
	max,
	onChange,
}: HEValueInputProps) => {
	const { t } = useTranslation('');
	const useMm = canConvert(travelMm, calibration);
	const lo = Math.min(min, max);
	const hi = Math.max(min, max);

	const toDisplay = (counts: number) => {
		if (!useMm) return String(counts);
		const mm =
			kind === 'reading'
				? readingToMm(counts, travelMm, calibration)
				: countsToMm(counts, travelMm, calibration);
		return mm.toFixed(2);
	};

	const toCounts = (displayValue: number) => {
		if (!useMm) return Math.round(displayValue);
		return kind === 'reading'
			? mmToReading(displayValue, travelMm, calibration)
			: mmToCounts(displayValue, travelMm, calibration);
	};

	const [text, setText] = useState(toDisplay(value));
	const [focused, setFocused] = useState(false);

	useEffect(() => {
		if (!focused) setText(toDisplay(value));
	}, [
		value,
		focused,
		useMm,
		travelMm,
		calibration.idle,
		calibration.pressed,
		calibration.is_polarized,
	]);

	const displayBounds = [Number(toDisplay(lo)), Number(toDisplay(hi))];

	return (
		<FormControl
			type="number"
			label={useMm ? `${label} (${t('HETrigger:unit-mm')})` : label}
			name={name}
			className="form-select-sm"
			value={text}
			step={useMm ? 0.01 : 1}
			min={Math.min(...displayBounds)}
			max={Math.max(...displayBounds)}
			onFocus={() => setFocused(true)}
			onBlur={() => {
				setFocused(false);
				const parsed = parseFloat(text);
				const counts = clamp(
					Number.isNaN(parsed) ? value : toCounts(parsed),
					lo,
					hi,
				);
				if (counts !== value) onChange(counts);
				setText(toDisplay(counts));
			}}
			onChange={(e) => {
				const raw = (e.target as HTMLInputElement).value;
				setText(raw);
				const parsed = parseFloat(raw);
				if (Number.isNaN(parsed)) return;
				// Only commit in-range values while typing; blur clamps the rest.
				const counts = toCounts(parsed);
				if (counts >= lo && counts <= hi) onChange(counts);
			}}
		/>
	);
};

export default HEValueInput;
