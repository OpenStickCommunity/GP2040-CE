import { Trigger } from '../Store/useHETriggerStore';
import { fromDepth, toDepth, travelSpan } from './HERapidTrigger';

export const ADC_MAX = 4095;

const clamp = (v: number, lo: number, hi: number) =>
	Math.min(Math.max(v, lo), hi);

export const getTriggerLimits = (trigger: Trigger) => {
	const span = Math.max(1, travelSpan(trigger));
	const noise = clamp(trigger.noise, 0, span);
	return {
		span,
		noise,
		minActuation: Math.min(noise + 1, span),
		minSensitivity: Math.min(Math.max(noise, 1), span),
	};
};

// Clamps actuation, hysteresis and RT sensitivities to the trigger's travel, as the calibration modal does.
export const normalizeTrigger = (trigger: Trigger): Trigger => {
	const { span, noise, minActuation, minSensitivity } =
		getTriggerLimits(trigger);
	const actuation = clamp(toDepth(trigger.active, trigger), minActuation, span);
	return {
		...trigger,
		noise,
		active: fromDepth(actuation, trigger),
		rtPressSensitivity: clamp(trigger.rtPressSensitivity, minSensitivity, span),
		rtReleaseSensitivity: clamp(
			trigger.rtReleaseSensitivity,
			minSensitivity,
			span,
		),
	};
};

// Keeps the actuation point at the same share of travel; polarity follows the readings unless equal.
export const recalibrateTrigger = (
	trigger: Trigger,
	idle: number,
	pressed: number,
	is_polarized = idle === pressed ? trigger.is_polarized : pressed < idle,
): Trigger => {
	const oldSpan = travelSpan(trigger);
	const share =
		oldSpan > 0 ? clamp(toDepth(trigger.active, trigger) / oldSpan, 0, 1) : 0.5;
	const calibration = { idle, pressed, is_polarized };
	const active = fromDepth(
		Math.round(share * travelSpan(calibration)),
		calibration,
	);
	return normalizeTrigger({ ...trigger, ...calibration, active });
};
