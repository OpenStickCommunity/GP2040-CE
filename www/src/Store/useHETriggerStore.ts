import { create } from 'zustand';

import WebApi from '../Services/WebApi';
import { PinActionValues } from '../Data/Pins';

export type Trigger = {
	action: PinActionValues;
	idle: number;
	active: number;
	pressed: number;
	is_polarized: boolean;
	release: number;
	noise: number;
	rapidTrigger: boolean;
};

// Multiplexers the firmware stores, one per muxADCPin0-3
export const MAX_MUXES = 4;

// Number of multiplexers scanned (32 / muxChannels, at most MAX_MUXES)
const getMuxCount = (muxChannels: number) =>
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
	loadingTriggers: boolean;
};

type Actions = {
	fetchHETriggers: () => void;
	setHETrigger: (trigger: Trigger & { id: number }) => void;
	setAllHETriggers: (trigger: Partial<Trigger>) => void;
	saveHETriggers: () => Promise<object>;
};

const INITIAL_STATE: State = {
	triggers: Array(32).map(() => ({
		action: -10,
		idle: 100,
		active: 2000,
		pressed: 3500,
		is_polarized: false,
		release: 2000,
		noise: 50,
		rapidTrigger: false,
	})),
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

	saveHETriggers: async () => WebApi.setHETriggerCalibrations(get()),
}));

export default useHETriggerStore;
