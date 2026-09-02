import { GpioAction, GpioDirection } from '@proto/enums';
import { createEnumRecord } from '../Services/Utilities';

export const BUTTON_ACTIONS = createEnumRecord(GpioAction)

export const PIN_DIRECTIONS = createEnumRecord(GpioDirection);

export type PinActionKeys = keyof typeof BUTTON_ACTIONS;
export type PinActionValues = (typeof BUTTON_ACTIONS)[PinActionKeys];

type PinDirectionKeys = keyof typeof PIN_DIRECTIONS;
export type PinDirectionValues = (typeof PIN_DIRECTIONS)[PinDirectionKeys];
