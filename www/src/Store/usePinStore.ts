import { create } from 'zustand';

export const BUTTON_ACTIONS = {
	DISABLED: -1,
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
} as const;

type PinActionKeys = keyof typeof BUTTON_ACTIONS;
type PinActionValues = (typeof BUTTON_ACTIONS)[PinActionKeys];

type PinAction = { action: PinActionValues };
type PinState = {
	pins: { [key: number]: PinAction };
	fetchPins: () => void;
	validate: () => void;
	setPinAction: (pin: number, action: PinActionValues) => void;
};

const usePinStore = create<PinState>()((set) => ({
	pins: {
		0: { action: -1 },
		1: { action: -1 },
		2: { action: -1 },
		3: { action: -1 },
		4: { action: -1 },
		5: { action: -1 },
		6: { action: -1 },
		7: { action: -1 },
		8: { action: -1 },
		9: { action: -1 },
		10: { action: -1 },
		11: { action: -1 },
		12: { action: -1 },
		13: { action: -1 },
		14: { action: -1 },
		15: { action: -1 },
		16: { action: -1 },
		17: { action: -1 },
		18: { action: -1 },
		19: { action: -1 },
		20: { action: -1 },
		21: { action: -1 },
		22: { action: -1 },
		23: { action: -1 },
		24: { action: -1 },
		25: { action: -1 },
		26: { action: -1 },
		27: { action: -1 },
		28: { action: -1 },
		29: { action: -1 },
	},
	fetchPins: async () => {
		//const pins = await fetchPinActions()
		set((state) => ({
			...state,
			//pins, //Set pin actions
		}));
	},
	validate: () => {}, // Run yup on pins
	setPinAction: (pin, action) =>
		set((state) => ({
			...state,
			pins: { ...state.pins, [pin]: { action } },
		})),
}));

export default usePinStore;
