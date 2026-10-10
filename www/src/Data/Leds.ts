// Definitions should match those in /AnimationStation/animation.h
export const LED_COLORS = [
	{ value: 0, label: 'Black', color: '#000000' },
	{ value: 1, label: 'White', color: '#ffffff' },
	{ value: 2, label: 'Red', color: '#ff0000' },
	{ value: 3, label: 'Orange', color: '#ff8000' },
	{ value: 4, label: 'Yellow', color: '#ffff00' },
	{ value: 5, label: 'Lime Green', color: '#80ff00' },
	{ value: 6, label: 'Green', color: '#00ff00' },
	{ value: 7, label: 'Seafoam', color: '#00ff80' },
	{ value: 8, label: 'Aqua', color: '#00ffff' },
	{ value: 9, label: 'Sky Blue', color: '#0080ff' },
	{ value: 10, label: 'Blue', color: '#0000ff' },
	{ value: 11, label: 'Purple', color: '#8000ff' },
	{ value: 12, label: 'Pink', color: '#ff00ff' },
	{ value: 13, label: 'Magenta', color: '#ff0080' },
];

export const LED_FORMATS = [
	{ label: 'GRB', value: 0 },
	{ label: 'RGB', value: 1 },
	{ label: 'GRBW', value: 2 },
	{ label: 'RGBW', value: 3 },
];

export const LIGHT_TYPES = {
	ActionButton: 0,
	Case: 1,
	Turbo: 2,
	PlayerLight: 3,
};

// Must match LightInputSource in proto/enums.proto
export const LIGHT_INPUT_SOURCES = {
	GPIO: 0,
	HallEffect: 1,
};

// Must match MAX_EXT_INPUT_LIGHT_COLOR_INDEXES in pixel.h
export const MAX_EXT_INPUT_LIGHT_COLOR_INDEXES = 32;

type LightInputInfo = {
	lightType: number;
	inputSource?: number;
	GPIOPinOrNonButtonIndex: number;
};

export const isExtInputLight = (light: LightInputInfo) =>
	light.lightType === LIGHT_TYPES.ActionButton &&
	light.inputSource === LIGHT_INPUT_SOURCES.HallEffect;

export const getButtonColorKeys = (light: LightInputInfo) =>
	isExtInputLight(light)
		? ({
				notPressed: 'extNotPressedStaticColors',
				pressed: 'extPressedStaticColors',
			} as const)
		: ({
				notPressed: 'notPressedStaticColors',
				pressed: 'pressedStaticColors',
			} as const);

export const getLightInputLabel = (light: LightInputInfo) =>
	isExtInputLight(light)
		? `HE${light.GPIOPinOrNonButtonIndex + 1}`
		: `GP${light.GPIOPinOrNonButtonIndex}`;
