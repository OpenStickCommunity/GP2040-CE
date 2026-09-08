import { create } from 'zustand';

import WebApi from '../Services/WebApi';
import { PinActionValues } from '../Data/Pins';

export type Trigger = {
	action: PinActionValues;
	idle: number;
	pressed: number;
	is_polarized: boolean;
	noise: number;
	rapidTrigger: boolean;
	// Rapid trigger v2. These are percentages of the idle->pressed travel, so they
	// stay meaningful across different switches and survive recalibration.
	actuationPoint: number;
	rtPressSensitivity: number;
	rtReleaseSensitivity: number;
	continuousRapidTrigger: boolean;
	travelDeadzone: number;
};

export const DEFAULT_TRIGGER: Trigger = {
	action: -10 as PinActionValues,
	idle: 150,
	pressed: 3500,
	is_polarized: false,
	noise: 30,
	rapidTrigger: false,
	actuationPoint: 35,
	rtPressSensitivity: 10,
	rtReleaseSensitivity: 10,
	continuousRapidTrigger: false,
	travelDeadzone: 3,
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
	// Array(32) creates holes, and .map() skips holes -- the previous form produced
	// 32 empty slots rather than 32 defaults, so any render before the fetch
	// resolved would read undefined. Array.from actually populates.
	triggers: Array.from({ length: 32 }, () => ({ ...DEFAULT_TRIGGER })),
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
	setHETrigger: ({ id, ...trigger}) => {
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
