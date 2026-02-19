import { create } from 'zustand';

import WebApi from '../Services/WebApi';
import { PinActionValues } from '../Data/Pins';

export type Trigger = {
	action: PinActionValues;
	idle: number;
	active: number;
	pressed: number;
	polarity: boolean;
	release: number;
	sensitivity: number;
	rapidTrigger: boolean;
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
    triggers: Array(32).map(()=>({ 
		action:-10,
		idle:100,
		active:2000,
		pressed:3500,
		polarity:false,
		release:2000,
		sensitivity:50,
		rapidTrigger:false
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
