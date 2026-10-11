// Sensitivity levels, expressed as percentages of the idle -> pressed travel.
//
// The firmware stores only the resolved numbers, never the level, so it has a
// single code path and per-button hand tuning stays first class. The UI infers
// which level is active by matching stored values back against this table, and
// reports "Custom" when nothing matches.

export type HEPreset = {
	// 1 = most sensitive, 10 = deepest. A plain number scale rather than names,
	// so "more sensitive" is unambiguous and comparable at a glance.
	level: number;
	actuationPoint: number;
	rtPressSensitivity: number;
	rtReleaseSensitivity: number;
	continuousRapidTrigger: boolean;
};

// Level 1 sits at 3% travel with 1% rapid trigger deltas -- roughly a hair
// trigger, and meaningfully quicker than the old most-sensitive setting (8%/3%).
// The firmware clamps the rapid trigger deltas up to the measured noise floor of
// each switch, so a level this fine degrades safely on a noisy channel rather
// than chattering.
export const HE_PRESETS: HEPreset[] = [
	{
		level: 1,
		actuationPoint: 3,
		rtPressSensitivity: 1,
		rtReleaseSensitivity: 1,
		continuousRapidTrigger: true,
	},
	{
		level: 2,
		actuationPoint: 6,
		rtPressSensitivity: 2,
		rtReleaseSensitivity: 2,
		continuousRapidTrigger: true,
	},
	{
		level: 3,
		actuationPoint: 10,
		rtPressSensitivity: 3,
		rtReleaseSensitivity: 3,
		continuousRapidTrigger: true,
	},
	{
		level: 4,
		actuationPoint: 15,
		rtPressSensitivity: 5,
		rtReleaseSensitivity: 5,
		continuousRapidTrigger: true,
	},
	{
		level: 5,
		actuationPoint: 25,
		rtPressSensitivity: 7,
		rtReleaseSensitivity: 7,
		continuousRapidTrigger: false,
	},
	{
		level: 6,
		actuationPoint: 35,
		rtPressSensitivity: 10,
		rtReleaseSensitivity: 10,
		continuousRapidTrigger: false,
	},
	{
		level: 7,
		actuationPoint: 45,
		rtPressSensitivity: 12,
		rtReleaseSensitivity: 12,
		continuousRapidTrigger: false,
	},
	{
		level: 8,
		actuationPoint: 55,
		rtPressSensitivity: 15,
		rtReleaseSensitivity: 15,
		continuousRapidTrigger: false,
	},
	{
		level: 9,
		actuationPoint: 65,
		rtPressSensitivity: 18,
		rtReleaseSensitivity: 18,
		continuousRapidTrigger: false,
	},
	{
		level: 10,
		actuationPoint: 75,
		rtPressSensitivity: 20,
		rtReleaseSensitivity: 20,
		continuousRapidTrigger: false,
	},
];

export const DEFAULT_PRESET_LEVEL = 6;

export const getPreset = (level: number): HEPreset =>
	HE_PRESETS.find((preset) => preset.level === level) ||
	(HE_PRESETS.find(
		(preset) => preset.level === DEFAULT_PRESET_LEVEL,
	) as HEPreset);

// Returns the level whose values match, or null when the values were hand tuned.
export const matchPreset = (values: {
	actuationPoint: number;
	rtPressSensitivity: number;
	rtReleaseSensitivity: number;
	continuousRapidTrigger: boolean;
}): HEPreset | null =>
	HE_PRESETS.find(
		(preset) =>
			preset.actuationPoint === values.actuationPoint &&
			preset.rtPressSensitivity === values.rtPressSensitivity &&
			preset.rtReleaseSensitivity === values.rtReleaseSensitivity &&
			preset.continuousRapidTrigger === values.continuousRapidTrigger,
	) || null;
