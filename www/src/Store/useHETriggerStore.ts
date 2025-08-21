import { create } from 'zustand';

import WebApi from '../Services/WebApi';
import { PinActionValues, PinDirectionValues } from '../Data/Pins';

type State = {
	triggers: {
		[key: string]: {
			action: PinActionValues;
			idle: number;
			active: number;
			max: number;
			polarity: number;
		};
	};
	loadingTriggers: boolean;
};

type Actions = {
	fetchHETriggers: () => void;
	setHETrigger: (
		key: string,
		action: PinActionValues,
		idle: number,
		active: number,
		max: number,
		polarity: number
	) => void;
	saveHETriggers: () => Promise<object>;
};

const INITIAL_STATE: State = {
    triggers: {
		he0: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he1: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he2: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he3: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he4: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he5: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he6: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he7: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he8: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he9: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he10: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he11: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he12: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he13: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he14: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he15: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he16: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he17: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he18: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he19: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he20: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he21: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he22: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he23: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he24: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he25: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he26: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he27: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he28: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he29: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he30: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
		he31: { action:-10 ,idle:100,active:2000,max:3500,polarity:0},
	},
	loadingTriggers: false,
};

const useHETriggerStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	fetchHETriggers: async () => {
		set({ loadingTriggers: true });
		const options = await WebApi.getHETriggerOptions();
		set((state) => ({
			...state,
			...options,
			loadingOptions: false,
		}));
	},
	setHETrigger: (he, action, idle, active, max, polarity) => {
		set((state) => {
			var newTriggers = { ...state.triggers };
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
