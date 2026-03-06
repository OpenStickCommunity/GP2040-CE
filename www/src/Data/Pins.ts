import { GpioAction } from '@proto/enums';
import { createEnumRecord } from '../Services/Utilities';

//export const BUTTON_ACTIONS = createEnumRecord(GpioAction)
// www/src/Data/Pins.ts の BUTTON_ACTIONS を以下のように書き換える
export const BUTTON_ACTIONS = {
	...createEnumRecord(GpioAction),
	BUTTON_PRESS_A3: 41,
	BUTTON_PRESS_A4: 42,
	BUTTON_PRESS_E1: 43,
	BUTTON_PRESS_E2: 44,
  BUTTON_PRESS_E3: 45,
  BUTTON_PRESS_E4: 46,
  BUTTON_PRESS_E5: 47,
  BUTTON_PRESS_E6: 48,
  BUTTON_PRESS_E7: 49,
  BUTTON_PRESS_E8: 50,
  BUTTON_PRESS_E9: 51,
  BUTTON_PRESS_E10: 52,
  BUTTON_PRESS_E11: 53,
  BUTTON_PRESS_E12: 54,
		// ... 必要な分だけ手動で追加
} as const;

export const PIN_DIRECTIONS = {
	DIRECTION_INPUT: 0,
	DIRECTION_OUTPUT: 1,
} as const;

export type PinActionKeys = keyof typeof BUTTON_ACTIONS;
export type PinActionValues = (typeof BUTTON_ACTIONS)[PinActionKeys];

type PinDirectionKeys = keyof typeof PIN_DIRECTIONS;
export type PinDirectionValues = (typeof PIN_DIRECTIONS)[PinDirectionKeys];
