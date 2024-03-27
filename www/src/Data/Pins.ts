// Hide from select options / Disable select if returned from board
export const NON_SELECTABLE_BUTTON_ACTIONS = [-5, 0];

// These could theoretically be created from enums.proto
export const BUTTON_ACTIONS = {
	NONE: -10,
	RESERVED: -5,
	ASSIGNED_TO_ADDON: 0,
	BUTTON_PRESS_UP: 1,
	BUTTON_PRESS_DOWN: 2,
	BUTTON_PRESS_LEFT: 3,
	BUTTON_PRESS_RIGHT: 4,
	BUTTON_PRESS_B1: 5,
	BUTTON_PRESS_B2: 6,
	BUTTON_PRESS_B3: 7,
	BUTTON_PRESS_B4: 8,
	BUTTON_PRESS_L1: 9,
	BUTTON_PRESS_R1: 10,
	BUTTON_PRESS_L2: 11,
	BUTTON_PRESS_R2: 12,
	BUTTON_PRESS_S1: 13,
	BUTTON_PRESS_S2: 14,
	BUTTON_PRESS_A1: 15,
	BUTTON_PRESS_A2: 16,
	BUTTON_PRESS_L3: 17,
	BUTTON_PRESS_R3: 18,
	BUTTON_PRESS_FN: 19,
	BUTTON_PRESS_DDI_UP: 20,
	BUTTON_PRESS_DDI_DOWN: 21,
	BUTTON_PRESS_DDI_LEFT: 22,
	BUTTON_PRESS_DDI_RIGHT: 23,
	SUSTAIN_DP_MODE_DP: 24,
	SUSTAIN_DP_MODE_LS: 25,
	SUSTAIN_DP_MODE_RS: 26,
	SUSTAIN_SOCD_MODE_UP_PRIO: 27,
	SUSTAIN_SOCD_MODE_NEUTRAL: 28,
	SUSTAIN_SOCD_MODE_SECOND_WIN: 29,
	SUSTAIN_SOCD_MODE_FIRST_WIN: 30,
	SUSTAIN_SOCD_MODE_BYPASS: 31,
	BUTTON_PRESS_TURBO: 32,
	BUTTON_PRESS_MACRO: 33,
	BUTTON_PRESS_MACRO_01: 34,
	BUTTON_PRESS_MACRO_02: 35,
	BUTTON_PRESS_MACRO_03: 36,
	BUTTON_PRESS_MACRO_04: 37,
	BUTTON_PRESS_MACRO_05: 38,
	BUTTON_PRESS_MACRO_06: 39,
} as const;

type PinActionKeys = keyof typeof BUTTON_ACTIONS;
export type PinActionValues = (typeof BUTTON_ACTIONS)[PinActionKeys];
