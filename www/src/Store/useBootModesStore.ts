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
	// validation fields
	saveAttempted: boolean;
	modesWithDuplicates: string[];
	errorMessage?: string;
};

type Actions = {
	addBootMode: () => void;
	removeBootMode: (key: string) => void;
	fetchBootModeOptions: () => void;
	saveBootModeOptions: () => void;
	addPin: (key: string, gpioPin: number) => void;
	removePin: (key: string, gpioPinIndex: number) => void;
	setInputMode: (key: string, inputMode?: InputMode) => void;
	setProfileIndex: (key: string, defaultProfileIndex?: number) => void;
	setEnabled: (value: boolean) => void;
	clearErrors: () => void;
	validatePins: () => boolean;
	validateRequired: () => boolean;
};

type APIResponseData = {
	webConfigPinMask: number;
	usbModePinMask: number;
	inputModeMappings: {
		pinMask: number;
		inputMode: number;
		profileIndex: number;
	}[];
};

function maskToSet(mask: number) {
	let s = new Set<number>();
	if (mask === -1) {
		return s;
	}
	for (let i = 0; i < NUM_PINS; i++) {
		if ((1 << i) & mask) {
			s.add(i);
		}
	}
	return s;
}

function setToMask(pins: Set<number>) {
	if (pins.size == 0) {
		return -1;
	}
	return [...pins].reduce((mask, v) => mask | (1 << v), 0);
}

function findDuplicates(bootModes: { [key: string]: BootModeMapping }) {
	let seen: { [key: number]: string[] } = {};
	for (const [key, mapping] of Object.entries(bootModes)) {
		let mask = setToMask(mapping.pins);
		if (mask == -1) {
			continue;
		}
		if (!(mask in seen)) {
			seen[mask] = [key];
		} else {
			seen[mask].push(key);
		}
	}
	return Object.values(seen)
		.filter((a) => a.length > 1)
		.flat();
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
	saveAttempted: false,
	modesWithDuplicates: [],
	errorMessage: undefined,
};

export const useBootModeStore = create<State & { actions: Actions }>()((set, get) => ({
	...INITIAL_STATE,
	actions: {
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
			let response: APIResponseData;
			try {
				let { data } = await WebApi.getBootModeOptions();
				response = data;
			} catch (error) {
				console.error(error);
				set({
					errorMessage: 'Failed to load boot mode options',
					loadingBootModes: false,
				});
				return;
			}

			let { webConfigPinMask, usbModePinMask, inputModeMappings } = response;

			let inputModes: { [key: string]: BootModeMapping } = {};
			for (const m of inputModeMappings) {
				if (m.inputMode == -1) {
					continue;
				}
				inputModes[`inputMode-${nanoid()}`] = {
					pins: maskToSet(m.pinMask),
					inputMode: m.inputMode as InputMode,
					profileIndex: m.profileIndex == -1 ? undefined : m.profileIndex,
				};
			}

			set((state) => ({
				...state,
				loadingBootModes: false,
				bootModes: {
					webConfig: {
						pins: maskToSet(webConfigPinMask),
						inputMode: undefined,
						profileIndex: undefined,
					},
					usbMode: {
						pins: maskToSet(usbModePinMask),
						inputMode: undefined,
						profileIndex: undefined,
					},
					...inputModes,
				},
			}));
		},

		saveBootModeOptions: async () => {
			const { validatePins, validateRequired } = get().actions;
			const valid = validatePins() && validateRequired();
			if (valid) {
				try {
					await WebApi.setBootModeOptions();
				} catch (error) {
					set({ saveAttempted: true, errorMessage: 'Save Failed' });
				}
			}
			set({ saveAttempted: true });
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

		clearErrors: () => {
			set({ saveAttempted: false, errorMessage: undefined });
		},

		validatePins: () => {
			const { bootModes } = get();
			const duplicates = findDuplicates(bootModes);
			if (duplicates.length > 0) {
				set({
					errorMessage: 'Mapped GPIO pins cannot contain duplicates',
					modesWithDuplicates: duplicates,
				});
				return false;
			} else {
				set({
					errorMessage: undefined,
					modesWithDuplicates: [],
				});
				return true;
			}
		},

		validateRequired: () => {
			const { bootModes } = get();
			for (const [key, mode] of Object.entries(bootModes)) {
				let valid = true;
				if (key.startsWith('inputMode-')) {
					valid = !(mode.inputMode === undefined || mode.pins.size == 0);
				} else {
					valid = mode.pins.size > 0;
				}
				if (!valid) {
					set({ errorMessage: 'Required fields are missing' });
					return false;
				}
			}
			set({ errorMessage: undefined });
			return true;
		},
	},
}));

export const useBootModeStoreActions = () => useBootModeStore((state) => state.actions);
