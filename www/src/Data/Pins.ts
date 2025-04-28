import { GpioAction } from '@proto/enums';
import { createEnumRecord } from '../Services/Utilities';

export const BUTTON_ACTIONS = createEnumRecord(GpioAction)

export const PIN_DIRECTIONS = {
	DIRECTION_INPUT: 0,
	DIRECTION_OUTPUT: 1,
} as const;

export type PinActionKeys = keyof typeof BUTTON_ACTIONS;
export type PinActionValues = (typeof BUTTON_ACTIONS)[PinActionKeys];

type PinDirectionKeys = keyof typeof PIN_DIRECTIONS;
export type PinDirectionValues = (typeof PIN_DIRECTIONS)[PinDirectionKeys];
