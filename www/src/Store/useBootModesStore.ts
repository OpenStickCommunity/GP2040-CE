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

type Actions = {
	addBootMode: () => void;
	removeBootMode: (bootModeIndex: number) => void;
	fetchBootModeOptions: () => void;
	saveBootModeOptions: () => Promise<object>;
	addWebConfigPin: (gpioPin: number) => void;
	removeWebConfigPin: (gpioPin: number) => void;
	addUsbModePin: (gpioPin: number) => void;
	removeUsbModePin: (gpioPin: number) => void;
	addPin: (bootModeIndex: number, gpioPin: number) => void;
	removePin: (bootModeIndex: number, gpioPinIndex: number) => void;
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

	addWebConfigPin: (gpioPin: number) => {
		set((state) => {
			let pins = state.webConfigPins;
			pins.add(gpioPin);
			return { ...state, webConfigPins: pins };
		});
	},

	removeWebConfigPin: (gpioPin: number) => {
		set((state) => {
			let pins = state.webConfigPins;
			pins.delete(gpioPin);
			return { ...state, webConfigPins: pins };
		});
	},

	addUsbModePin: (gpioPin: number) => {
		set((state) => {
			let pins = state.usbModePins;
			pins.add(gpioPin);
			return { ...state, usbModePins: pins };
		});
	},

	removeUsbModePin: (gpioPin: number) => {
		set((state) => {
			let pins = state.usbModePins;
			pins.delete(gpioPin);
			return { ...state, usbModePins: pins };
		});
	},

	addPin: (bootModeIndex: number, gpioPin: number) => {
		set((state) => {
			let bootModes = state.bootModes;
			bootModes[bootModeIndex].pins.add(gpioPin);
			return { ...state, bootModes: bootModes };
		});
	},

	removePin: (bootModeIndex: number, gpioPin: number) => {
		set((state) => {
			let bootModes = state.bootModes;
			bootModes[bootModeIndex].pins.delete(gpioPin);
			return { ...state, bootModes: bootModes };
		});
	},

	setInputMode: (bootModeIndex: number, inputMode?: InputMode) => {
		set((state) => {
			let bootModes = state.bootModes;
			bootModes[bootModeIndex].inputMode = inputMode;
			return { ...state, bootModes: bootModes };
		});
	},

	setProfileIndex: (bootModeIndex: number, defaultProfileIndex?: number) => {
		set((state) => {
			let bootModes = state.bootModes;
			bootModes[bootModeIndex].profileIndex = defaultProfileIndex;
			return { ...state, bootModes: bootModes };
		});
	},

	setEnabled: (value: boolean) => {
		set((state) => ({
			...state,
			enabled: value,
		}));
	},
}));
