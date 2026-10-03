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
	// False until fetchHETriggers has returned. Saving before that would post
	// DEFAULT_TRIGGER for all 32 channels over whatever the board has calibrated,
	// the same failure mode the profile store guards against.
	triggersLoaded: boolean;
};

type Actions = {
	fetchHETriggers: () => Promise<void>;
	setHETrigger: (trigger: Trigger & { id: number }) => void;
	saveHETriggers: () => Promise<object>;
};

const INITIAL_STATE: State = {
	// Array(32) creates holes, and .map() skips holes -- the previous form produced
	// 32 empty slots rather than 32 defaults, so any render before the fetch
	// resolved would read undefined. Array.from actually populates.
	triggers: Array.from({ length: 32 }, () => ({ ...DEFAULT_TRIGGER })),
	loadingTriggers: false,
	triggersLoaded: false,
};

const useHETriggerStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	fetchHETriggers: async () => {
		set({ loadingTriggers: true });
		const data = await WebApi.getHETriggerCalibrations();
		set((state) => ({
			...state,
			...data,
			loadingTriggers: false,
			// Only a real response licenses a later save; a failed fetch must not.
			triggersLoaded: Boolean(data?.triggers),
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
	saveHETriggers: async () => {
		const { triggers, triggersLoaded } = get();
		if (!triggersLoaded) {
			throw new Error(
				'Refusing to save hall effect calibration before it has been loaded',
			);
		}
		// Send only the payload the firmware reads, not the whole store.
		return WebApi.setHETriggerCalibrations({ triggers });
	},
}));

export default useHETriggerStore;
