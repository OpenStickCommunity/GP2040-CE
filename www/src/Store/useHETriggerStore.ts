import { create } from 'zustand';

import WebApi from '../Services/WebApi';
import { PinActionValues, PinDirectionValues } from '../Data/Pins';

type State = {
	triggers: Array<object>;
	loadingTriggers: boolean;
};

type Actions = {
	fetchHETriggers: () => void;
	setHETrigger: (
		id: number,
		action: PinActionValues,
		idle: number,
		active: number,
		max: number,
		polarity: number
	) => void;
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
	setHETrigger: (he, action, idle, active, max, polarity) => {
		set((state) => {
			var newTriggers = state.triggers;
			if ( newTriggers &&
				newTriggers[he]) {
				newTriggers[he].action = action;
				newTriggers[he].idle = idle;
				newTriggers[he].active = active;
				newTriggers[he].max = max;
				newTriggers[he].polarity = polarity;
			}

			return {
				...state,
				triggers: newTriggers,
			};
		});
	},

	saveHETriggers: async () => WebApi.setHETriggerOptions(get()),
}));

export default useHETriggerStore;
