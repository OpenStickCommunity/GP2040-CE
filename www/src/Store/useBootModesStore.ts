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
	dirty: boolean;
	// validation fields
	saveAttempted: boolean;
	modesWithDuplicates: string[];
	errorMessage?: string;
};

type Actions = {
	addBootMode: () => void;
	removeBootMode: (key: string) => void;
	fetchBootModeOptions: () => void;
	saveBootModeOptions: (errorMessage: string) => void;
	addPin: (key: string, gpioPin: number) => void;
	removePin: (key: string, gpioPinIndex: number) => void;
	setInputMode: (key: string, inputMode?: InputMode) => void;
	setProfileIndex: (key: string, defaultProfileIndex?: number) => void;
	setEnabled: (value: boolean) => void;
	setDirty: () => void;
	clearErrors: () => void;
	validatePins: (errorMessage: string) => boolean;
	validateRequired: (errorMessage: string) => boolean;
};

type APIResponseData = {
	webConfigPinMask: number;
	usbModePinMask: number;
	enabled: boolean;
	inputModeMappings: {
		pinMask: number;
		inputMode: number;
		// Profiles are 1-indexed. 0 = no mapped profile
		profileNumber: number;
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
	dirty: false,
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

			let { enabled, webConfigPinMask, usbModePinMask, inputModeMappings } = response;

			let inputModes: { [key: string]: BootModeMapping } = {};
			for (const m of inputModeMappings) {
				if (m.inputMode == -1) {
					continue;
				}
				inputModes[`inputMode-${nanoid()}`] = {
					pins: maskToSet(m.pinMask),
					inputMode: m.inputMode as InputMode,
					profileIndex: m.profileNumber == 0 ? undefined : m.profileNumber - 1,
				};
			}

			set((state) => ({
				...state,
				loadingBootModes: false,
				enabled: !!enabled,
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

		saveBootModeOptions: async (errorMessage: string) => {
			const { bootModes, enabled } = get();
			set({ saveAttempted: true });
			const postData: APIResponseData = {
				webConfigPinMask: setToMask(bootModes['webConfig'].pins),
				usbModePinMask: setToMask(bootModes['usbMode'].pins),
				enabled: enabled,
				inputModeMappings: Object.entries(bootModes)
					.filter(([k, _v], _i) => k.startsWith('inputMode-'))
					.map(([_, m], _i) => ({
						pinMask: setToMask(m.pins),
						inputMode: m.inputMode === undefined ? 0 : m.inputMode,
						profileNumber: m.profileIndex === undefined ? 0 : m.profileIndex + 1,
					})),
			};
			try {
				await WebApi.setBootModeOptions(postData);
				set({ dirty: false });
			} catch (error) {
				set({ saveAttempted: true, errorMessage: errorMessage });
			}
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

		setProfileIndex: (key: string, profileIndex?: number) => {
			set((state) => {
				let newModes = { ...state.bootModes };
				newModes[key].profileIndex = profileIndex;
				return { ...state, bootModes: newModes };
			});
		},

		setEnabled: (value: boolean) => {
			set({ enabled: value });
		},

		setDirty: () => {
			set({ dirty: true });
		},

		clearErrors: () => {
			set({ saveAttempted: false, errorMessage: undefined });
		},

		validatePins: (errorMessage: string) => {
			const { bootModes } = get();
			const duplicates = findDuplicates(bootModes);
			if (duplicates.length > 0) {
				set({
					errorMessage: errorMessage,
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

		validateRequired: (errorMessage: string) => {
			const { bootModes } = get();
			for (const [key, mode] of Object.entries(bootModes)) {
				let valid = true;
				if (key.startsWith('inputMode-')) {
					valid = !(mode.inputMode === undefined || mode.pins.size == 0);
				} else {
					valid = mode.pins.size > 0;
				}
				if (!valid) {
					set({ errorMessage: errorMessage });
					return false;
				}
			}
			set({ errorMessage: undefined });
			return true;
		},
	},
}));

export const useBootModeStoreActions = () => useBootModeStore((state) => state.actions);
