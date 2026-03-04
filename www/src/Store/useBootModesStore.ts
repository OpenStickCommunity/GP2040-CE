import { create } from 'zustand';
import { INPUT_MODE_OPTIONS } from '../Data/InputBootModes';
import WebApi from '../Services/WebApi';
import { InputMode } from '@proto/enums';

// Should this come from config somewhere?
export const NUM_PINS = 30;

type BootModeMapping = {
	pins: Set<number>;
	inputMode?: InputMode;
	profileIndex?: number;
};

type State = {
	webConfigPins: Set<number>;
	usbModePins: Set<number>;
	bootModes: BootModeMapping[];
	loadingBootModes: boolean;
};

type Actions = {
	addBootMode: () => void;
	removeBootMode: (bootModeIndex: number) => void;
	fetchBootModes: () => void;
	saveBootModes: () => Promise<object>;
	addWebConfigPin: (gpioPin: number) => void;
	removeWebConfigPin: (gpioPin: number) => void;
	addUsbModePin: (gpioPin: number) => void;
	removeUsbModePin: (gpioPin: number) => void;
	setPin: (bootModeIndex: number, gpioPin: number) => void;
	clearPin: (bootModeIndex: number, gpioPinIndex: number) => void;
	setInputMode: (bootModeIndex: number, inputMode?: InputMode) => void;
	setProfileIndex: (bootModeIndex: number, defaultProfileIndex?: number) => void;
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
};

export const useBootModesStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,

	addBootMode: () => {
		set((state) => ({
			...state,
			bootModes: [
				...state.bootModes,
				{
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

	fetchBootModes: async () => {
		set({ loadingBootModes: true });
		const { webConfigPinMask, usbModePinMask, bootModeMappings } =
			await WebApi.getBootModeMappings();

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

	saveBootModes: async () => {
		const bootModes = get().bootModes;
		return WebApi.setBootModeMappings(
			bootModes.map((m) => ({
				...m,
				inputMode: m.inputMode ?? -1,
				defaultProfileIndex: m.profileIndex ?? -1,
			})),
		);
	},

	addWebConfigPin: (gpioPin: number) => {},

	removeWebConfigPin: (gpioPin: number) => {},

	addUsbModePin: (gpioPin: number) => {},

	removeUsbModePin: (gpioPin: number) => {},

	setPin: (bootModeIndex: number, gpioPin: number) => {
		set((state) => {
			let bootModes = state.bootModes;
			bootModes[bootModeIndex].pins.add(gpioPin);
			return { ...state, bootModes: bootModes };
		});
	},

	clearPin: (bootModeIndex: number, gpioPin: number) => {
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
}));
