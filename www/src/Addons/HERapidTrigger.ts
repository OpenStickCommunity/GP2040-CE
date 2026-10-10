// Mirror of headers/addons/he_rapid_trigger.h so the calibration preview matches the firmware.
// Both are checked against tests/he_rapid_trigger/vectors.txt.

export type RapidTriggerConfig = {
	actuation: number;
	pressSensitivity: number;
	releaseSensitivity: number;
	noise: number;
	travel: number;
	rapidTrigger: boolean;
	continuous: boolean;
};

export type RapidTriggerState = {
	active: boolean;
	engaged: boolean;
	extreme: number;
};

const RELEASED_ZONE_DIVISOR = 20;

export const createRapidTriggerState = (): RapidTriggerState => ({
	active: false,
	engaged: false,
	extreme: 0,
});

export const updateRapidTrigger = (
	state: RapidTriggerState,
	config: RapidTriggerConfig,
	depth: number,
): boolean => {
	const noise = Math.max(config.noise, 0);
	const actuation = Math.max(config.actuation, 1);
	const releaseBelow = actuation - Math.min(noise, actuation - 1);

	if (!config.rapidTrigger) {
		state.engaged = false;
		state.extreme = depth;
		state.active = state.active ? depth >= releaseBelow : depth >= actuation;
		return state.active;
	}

	if (!state.engaged) {
		state.engaged = state.active = depth >= actuation;
		state.extreme = depth;
		return state.active;
	}

	let exitBelow = releaseBelow;
	if (config.continuous) {
		const releasedZone = Math.max(
			noise,
			Math.trunc(Math.max(config.travel, 0) / RELEASED_ZONE_DIVISOR),
		);
		exitBelow = Math.min(releasedZone + 1, releaseBelow);
	}
	if (depth < exitBelow) {
		state.engaged = state.active = false;
		state.extreme = depth;
		return state.active;
	}

	const minSensitivity = Math.max(noise, 1);
	if (state.active) {
		if (depth > state.extreme) {
			state.extreme = depth;
		} else if (
			state.extreme - depth >=
			Math.max(config.releaseSensitivity, minSensitivity)
		) {
			state.active = false;
			state.extreme = depth;
		}
	} else if (depth < state.extreme) {
		state.extreme = depth;
	} else if (
		depth - state.extreme >=
		Math.max(config.pressSensitivity, minSensitivity)
	) {
		state.active = true;
		state.extreme = depth;
	}
	return state.active;
};

export type TravelCalibration = {
	idle: number;
	pressed: number;
	is_polarized: boolean;
};

const direction = (cal: TravelCalibration) => (cal.is_polarized ? -1 : 1);

// Conversions are linear between the calibrated idle (0mm) and pressed (full travel) readings.
export const travelSpan = (cal: TravelCalibration) =>
	Math.abs(cal.pressed - cal.idle);

export const canConvert = (travelMm: number, cal: TravelCalibration) =>
	travelMm > 0 && travelSpan(cal) > 0;

export const toDepth = (reading: number, cal: TravelCalibration) =>
	direction(cal) * (reading - cal.idle);

export const fromDepth = (depth: number, cal: TravelCalibration) =>
	cal.idle + direction(cal) * depth;

export const countsToMm = (
	counts: number,
	travelMm: number,
	cal: TravelCalibration,
) => (counts / travelSpan(cal)) * travelMm;

export const mmToCounts = (
	mm: number,
	travelMm: number,
	cal: TravelCalibration,
) => Math.round((mm / travelMm) * travelSpan(cal));

export const readingToMm = (
	reading: number,
	travelMm: number,
	cal: TravelCalibration,
) => countsToMm(toDepth(reading, cal), travelMm, cal);

export const mmToReading = (
	mm: number,
	travelMm: number,
	cal: TravelCalibration,
) => fromDepth(mmToCounts(mm, travelMm, cal), cal);

export const formatMm = (mm: number) => `${mm.toFixed(2)} mm`;

export const formatReading = (
	reading: number,
	travelMm: number,
	cal: TravelCalibration,
) =>
	canConvert(travelMm, cal)
		? formatMm(readingToMm(reading, travelMm, cal))
		: String(reading);

export const formatDistance = (
	counts: number,
	travelMm: number,
	cal: TravelCalibration,
) =>
	canConvert(travelMm, cal)
		? formatMm(countsToMm(counts, travelMm, cal))
		: String(counts);
