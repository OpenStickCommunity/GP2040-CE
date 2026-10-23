import { create } from 'zustand';

import WebApi from '../Services/WebApi';
import { PinActionValues } from '../Data/Pins';

export type Trigger = {
	action: PinActionValues;
	idle: number;
	active: number;
	pressed: number;
	is_polarized: boolean;
	noise: number;
	rapidTrigger: boolean;
	rtPressSensitivity: number;
	rtReleaseSensitivity: number;
	rtSeparateSensitivity: boolean;
	rtContinuous: boolean;
};

// Multiplexers the firmware stores, one per muxADCPin0-3
export const MAX_MUXES = 4;

// Number of multiplexers scanned (32 / muxChannels, at most MAX_MUXES)
export const getMuxCount = (muxChannels: number) =>
	Math.min(MAX_MUXES, Math.floor(32 / muxChannels));

// True if the trigger is on a scanned multiplexer with an ADC pin
export const isTriggerScanned = (
	index: number,
	muxChannels: number,
	values: { [key: string]: unknown },
) => {
	const mux = Math.floor(index / muxChannels);
	return (
		mux < getMuxCount(muxChannels) && Number(values[`muxADCPin${mux}`]) !== -1
	);
};

type State = {
	triggers: Trigger[];
	// Last loaded or saved triggers, for spotting unsaved edits
	savedTriggers: Trigger[];
	loadingTriggers: boolean;
};

type Actions = {
	fetchHETriggers: () => void;
	setHETrigger: (trigger: Trigger & { id: number }) => void;
	setAllHETriggers: (trigger: Partial<Trigger>) => void;
	saveHETriggers: (switchTravel: number) => Promise<object>;
	resetHETriggers: () => void;
};

const DEFAULT_TRIGGERS: Trigger[] = Array.from({ length: 32 }, () => ({
	action: -10,
	idle: 100,
	active: 2000,
	pressed: 3500,
	is_polarized: false,
	noise: 30,
	rapidTrigger: false,
	rtPressSensitivity: 100,
	rtReleaseSensitivity: 100,
	rtSeparateSensitivity: false,
	rtContinuous: false,
}));

const INITIAL_STATE: State = {
	triggers: DEFAULT_TRIGGERS,
	savedTriggers: DEFAULT_TRIGGERS,
	loadingTriggers: false,
};

const useHETriggerStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	fetchHETriggers: async () => {
		set({ loadingTriggers: true });
		const triggers = await WebApi.getHETriggerCalibrations();
		set((state) => ({
			...state,
			...triggers,
			savedTriggers: triggers?.triggers ?? state.savedTriggers,
			loadingTriggers: false,
		}));
	},
	setHETrigger: ({ id, ...trigger }) => {
		set((state) => {
			const newTriggers = [...state.triggers];
			if (newTriggers[id]) {
				newTriggers[id] = trigger;
			}

			return {
				...state,
				triggers: newTriggers,
			};
		});
	},
	setAllHETriggers: (triggerValues) => {
		set((state) => ({
			...state,
			triggers: state.triggers.map((trigger) => ({
				...trigger,
				...triggerValues,
			})),
		}));
	},

	saveHETriggers: async (switchTravel) => {
		const triggers = get().triggers;
		const response = await WebApi.setHETriggerCalibrations({
			triggers,
			heTriggerSwitchTravel: switchTravel,
		});
		set({ savedTriggers: triggers });
		return response;
	},
	resetHETriggers: () => {
		set((state) => ({ ...state, triggers: state.savedTriggers }));
	},
}));

export default useHETriggerStore;
