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

type State = {
	triggers: Trigger[];
	loadingTriggers: boolean;
};

type Actions = {
	fetchHETriggers: () => void;
	setHETrigger: (trigger: Trigger & { id: number }) => void;
	setAllHETriggers: (trigger: Partial<Trigger>) => void;
	saveHETriggers: (switchTravel: number) => Promise<object>;
};

const INITIAL_STATE: State = {
	triggers: Array.from({ length: 32 }, () => ({
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

	saveHETriggers: async (switchTravel) =>
		WebApi.setHETriggerCalibrations({
			triggers: get().triggers,
			heTriggerSwitchTravel: switchTravel,
		}),
}));

export default useHETriggerStore;
