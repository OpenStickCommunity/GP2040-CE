import { create } from 'zustand';
import { INPUT_MODE_OPTIONS } from '../Data/InputBootModes';
import WebApi from '../Services/WebApi';
import { InputMode } from '@proto/enums';
import { nanoid } from 'nanoid';

// Should this come from config somewhere?
export const NUM_PINS = 30;

type BootModeMapping = {
	pins: Set<number>;
	inputMode?: InputMode;
	profileIndex?: number;
};

type State = {
	bootModes: { [key: string]: BootModeMapping };
	loadingBootModes: boolean;
	// If false, use old method of mapping
	enabled: boolean;
};

type Actions = {
	addBootMode: () => void;
	removeBootMode: (key: string) => void;
	fetchBootModeOptions: () => void;
	saveBootModeOptions: () => Promise<object>;
	addPin: (key: string, gpioPin: number) => void;
	removePin: (key: string, gpioPinIndex: number) => void;
	setInputMode: (key: string, inputMode?: InputMode) => void;
	setProfileIndex: (key: string, defaultProfileIndex?: number) => void;
	setEnabled: (value: boolean) => void;
};

function mask_to_set(pins: number) {
	let s = new Set<number>();
	for (let i = 0; i < NUM_PINS; i++) {
		if ((1 << i) & pins) {
			s.add(i);
		}
	}
	return s;
}

const INITIAL_STATE = {
	bootModes: {
		usbMode: {
			pins: new Set<number>(),
			inputMode: undefined,
			profileIndex: undefined,
		},
		webConfig: {
			pins: new Set<number>(),
			inputMode: undefined,
			profileIndex: undefined,
		},
	},
	loadingBootModes: false,
	enabled: false,
};

export const useBootModesStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,

	addBootMode: () => {
		set((state) => ({
			...state,
			bootModes: {
				...state.bootModes,
				[`inputMode-${nanoid()}`]: {
					pins: new Set(),
					inputMode: undefined,
					profileIndex: undefined,
				},
			},
		}));
	},

	removeBootMode: (key: string) => {
		set((state) => {
			let newModes = { ...state.bootModes };
			delete newModes[key];
			return {
				...state,
				bootModes: newModes,
			};
		});
	},

	fetchBootModeOptions: async () => {
		set({ loadingBootModes: true });
		const { webConfigPinMask, usbModePinMask, bootModeMappings } =
			await WebApi.getBootModeMappings();

		await new Promise((resolve) => setTimeout(resolve, 500));
		set({ loadingBootModes: false });
		//TODO
	},

	saveBootModeOptions: async () => {
		const bootModes = get().bootModes;
		return WebApi.setBootModeMappings();

		// TODO
	},

	addPin: (key: string, pin: number) => {
		set((state) => {
			let newModes = { ...state.bootModes };
			let newPins = new Set([...newModes[key].pins, pin]);
			newModes[key].pins = newPins;
			return { ...state, bootModes: newModes };
		});
	},

	removePin: (key: string, pin: number) => {
		set((state) => {
			let newModes = { ...state.bootModes };
			let newPins = new Set([...newModes[key].pins]);
			newPins.delete(pin);
			newModes[key].pins = newPins;
			return { ...state, bootModes: newModes };
		});
	},

	setInputMode: (key: string, inputMode?: InputMode) => {
		set((state) => {
			let newModes = { ...state.bootModes };
			newModes[key].inputMode = inputMode;
			return { ...state, bootModes: newModes };
		});
	},

	setProfileIndex: (key: string, defaultProfileIndex?: number) => {
		set((state) => {
			let newModes = { ...state.bootModes };
			newModes[key].profileIndex = defaultProfileIndex;
			return { ...state, bootModes: newModes };
		});
	},

	setEnabled: (value: boolean) => {
		set((state) => ({
			...state,
			enabled: value,
		}));
	},
}));
