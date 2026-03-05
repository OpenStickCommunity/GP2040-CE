import { create } from 'zustand';
import { INPUT_MODE_OPTIONS } from '../Data/InputBootModes';
import WebApi from '../Services/WebApi';
import { InputMode } from '@proto/enums';
import { nanoid } from 'nanoid';

// Should this come from config somewhere?
export const NUM_PINS = 30;

type BootModeMapping = {
	key: string;
	pins: Set<number>;
	inputMode?: InputMode;
	profileIndex?: number;
};

type State = {
	// Required mappings. Profile doesn't apply
	webConfigPins: Set<number>;
	usbModePins: Set<number>;
	// Up to 8 additional mappings
	bootModes: BootModeMapping[];
	loadingBootModes: boolean;
	// If false, use old method of mapping
	enabled: boolean;
};

export type PinsKey = 'webConfigPins' | 'usbModePins' | number;

type Actions = {
	addBootMode: () => void;
	removeBootMode: (bootModeIndex: number) => void;
	fetchBootModeOptions: () => void;
	saveBootModeOptions: () => Promise<object>;
	addPin: (key: PinsKey, gpioPin: number) => void;
	removePin: (key: PinsKey, gpioPinIndex: number) => void;
	setInputMode: (bootModeIndex: number, inputMode?: InputMode) => void;
	setProfileIndex: (bootModeIndex: number, defaultProfileIndex?: number) => void;
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
	webConfigPins: new Set<number>(),
	usbModePins: new Set<number>(),
	bootModes: [],
	loadingBootModes: false,
	enabled: false,
};

export const useBootModesStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,

	addBootMode: () => {
		set((state) => ({
			...state,
			bootModes: [
				...state.bootModes,
				{
					key: nanoid(),
					pins: new Set(),
					inputMode: undefined,
					profileIndex: undefined,
				},
			],
		}));
	},

	removeBootMode: (bootModeIndex: number) => {
		set((state) => ({
			...state,
			bootModes: [
				...state.bootModes.slice(0, bootModeIndex),
				...state.bootModes.slice(bootModeIndex + 1),
			],
		}));
	},

	fetchBootModeOptions: async () => {
		set({ loadingBootModes: true });
		const { webConfigPinMask, usbModePinMask, bootModeMappings } =
			await WebApi.getBootModeMappings();

		await new Promise((resolve) => setTimeout(resolve, 500));

		set((state) => ({
			...state,
			loadingBootModes: false,
			webConfigPins: mask_to_set(webConfigPinMask),
			usbModePins: mask_to_set(usbModePinMask),
			bootModes: bootModeMappings.map((m: any) => ({
				pins: mask_to_set(m.pinMask),
				inputMode: m.inputMode == -1 ? undefined : m.inputMode,
				profileIndex: m.defaultProfileIndex == -1 ? undefined : m.defaultProfileIndex,
			})),
		}));
	},

	saveBootModeOptions: async () => {
		const bootModes = get().bootModes;
		return WebApi.setBootModeMappings(
			bootModes.map((m) => ({
				...m,
				inputMode: m.inputMode ?? -1,
				defaultProfileIndex: m.profileIndex ?? -1,
			})),
		);
	},

	addPin: (key: PinsKey, pin: number) => {
		set((state) => {
			if (key == 'usbModePins' || key == 'webConfigPins') {
				let newPins = new Set([...state[key], pin]);
				return { ...state, [key]: newPins };
			}
			let newModes = [...state.bootModes];
			let newPins = new Set([...newModes[key].pins, pin]);
			newModes[key].pins = newPins;
			return { ...state, bootModes: newModes };
		});
	},

	removePin: (key: PinsKey, pin: number) => {
		set((state) => {
			if (key == 'usbModePins' || key == 'webConfigPins') {
				let newPins = new Set([...state[key], pin]);
				newPins.delete(pin);
				return { ...state, key: newPins };
			}
			let newModes = [...state.bootModes];
			let newPins = new Set([...newModes[key].pins]);
			newPins.delete(pin);
			newModes[key].pins = newPins;
			return { ...state, bootModes: newModes };
		});
	},

	setInputMode: (bootModeIndex: number, inputMode?: InputMode) => {
		set((state) => {
			let newModes = [...state.bootModes];
			newModes[bootModeIndex].inputMode = inputMode;
			return { ...state, bootModes: newModes };
		});
	},

	setProfileIndex: (bootModeIndex: number, defaultProfileIndex?: number) => {
		set((state) => {
			let newModes = [...state.bootModes];
			newModes[bootModeIndex].profileIndex = defaultProfileIndex;
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
