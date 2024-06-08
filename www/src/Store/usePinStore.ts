import { create } from 'zustand';

import WebApi from '../Services/WebApi';
import { BUTTON_ACTIONS, PinActionValues } from '../Data/Pins';

type CustomMasks = {
	customButtonMask: number;
	customDpadMask: number;
};

export type MaskPayload = {
	action: PinActionValues;
} & CustomMasks;

type State = {
	pins: { [key: string]: MaskPayload };
	loadingPins: boolean;
};
export type setPinType = (
	pin: string,
	{ action, customButtonMask, customDpadMask }: MaskPayload,
) => void;

type Actions = {
	fetchPins: () => void;
	setPin: setPinType;
	savePins: () => Promise<object>;
};

const DEFAULT_PIN_STATE = {
	action: BUTTON_ACTIONS.NONE,
	customButtonMask: 0,
	customDpadMask: 0,
};

const INITIAL_STATE: State = {
	pins: {
		pin00: DEFAULT_PIN_STATE,
		pin01: DEFAULT_PIN_STATE,
		pin02: DEFAULT_PIN_STATE,
		pin03: DEFAULT_PIN_STATE,
		pin04: DEFAULT_PIN_STATE,
		pin05: DEFAULT_PIN_STATE,
		pin06: DEFAULT_PIN_STATE,
		pin07: DEFAULT_PIN_STATE,
		pin08: DEFAULT_PIN_STATE,
		pin09: DEFAULT_PIN_STATE,
		pin10: DEFAULT_PIN_STATE,
		pin11: DEFAULT_PIN_STATE,
		pin12: DEFAULT_PIN_STATE,
		pin13: DEFAULT_PIN_STATE,
		pin14: DEFAULT_PIN_STATE,
		pin15: DEFAULT_PIN_STATE,
		pin16: DEFAULT_PIN_STATE,
		pin17: DEFAULT_PIN_STATE,
		pin18: DEFAULT_PIN_STATE,
		pin19: DEFAULT_PIN_STATE,
		pin20: DEFAULT_PIN_STATE,
		pin21: DEFAULT_PIN_STATE,
		pin22: DEFAULT_PIN_STATE,
		pin23: DEFAULT_PIN_STATE,
		pin24: DEFAULT_PIN_STATE,
		pin25: DEFAULT_PIN_STATE,
		pin26: DEFAULT_PIN_STATE,
		pin27: DEFAULT_PIN_STATE,
		pin28: DEFAULT_PIN_STATE,
		pin29: DEFAULT_PIN_STATE,
	},
	loadingPins: false,
};

const usePinStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	fetchPins: async () => {
		set({ loadingPins: true });
		const pins = await WebApi.getPinMappingsV2();
		set((state) => ({
			...state,
			pins,
			loadingPins: false,
		}));
	},

	setPin: (pin, { action, customButtonMask = 0, customDpadMask = 0 }) =>
		set((state) => ({
			...state,
			pins: {
				...state.pins,
				[pin]: {
					action,
					customButtonMask,
					customDpadMask,
				},
			},
		})),
	savePins: async () => WebApi.setPinMappingsV2(get().pins),
}));

export default usePinStore;
