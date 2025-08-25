import { create } from 'zustand';

import WebApi from '../Services/WebApi';
import { PinActionValues } from '../Data/Pins';

export type Trigger = {
	action: PinActionValues;
	idle: number;
	active: number;
	max: number;
	polarity: number;
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
    triggers: [
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		{ action:-10 ,idle:100,active:2000,max:3500,polarity:0},
	],
	loadingTriggers: false,
};

const useHETriggerStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	fetchHETriggers: async () => {
		set({ loadingTriggers: true });
		const triggers = await WebApi.getHETriggerOptions();
		set((state) => ({
			...state,
			...triggers,
			loadingTriggers: false,
		}));
	},
	setHETrigger: ({ id, action, idle, active, max, polarity }) => {
		set((state) => {
			const newTriggers = [...state.triggers];
			if (newTriggers[id]) {
				newTriggers[id] = { action, idle, active, max, polarity };
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

	saveHETriggers: async () => WebApi.setHETriggerOptions(get()),
}));

export default useHETriggerStore;
