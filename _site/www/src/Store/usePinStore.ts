import axios from 'axios';
import { create } from 'zustand';

import { baseUrl } from '../Services/WebApi';
import { PinActionValues } from '../Data/Pins';

type State = {
	pins: { [key: string]: PinActionValues };
	loadingPins: boolean;
};

type Actions = {
	fetchPins: () => void;
	setPinAction: (pin: string, action: PinActionValues) => void;
	savePins: () => Promise<{}>;
};

const INITIAL_STATE: State = {
	pins: {
		pin00: -10,
		pin01: -10,
		pin02: -10,
		pin03: -10,
		pin04: -10,
		pin05: -10,
		pin06: -10,
		pin07: -10,
		pin08: -10,
		pin09: -10,
		pin10: -10,
		pin11: -10,
		pin12: -10,
		pin13: -10,
		pin14: -10,
		pin15: -10,
		pin16: -10,
		pin17: -10,
		pin18: -10,
		pin19: -10,
		pin20: -10,
		pin21: -10,
		pin22: -10,
		pin23: -10,
		pin24: -10,
		pin25: -10,
		pin26: -10,
		pin27: -10,
		pin28: -10,
		pin29: -10,
	},
	loadingPins: false,
};

const usePinStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	fetchPins: async () => {
		set({ loadingPins: true });
		const { data } = await axios.get(`${baseUrl}/api/getPinMappings`);
		set((state) => ({
			...state,
			pins: { ...state.pins, ...data }, // Merge saved pins with defaults
			loadingPins: false,
		}));
	},
	setPinAction: (pin, action) =>
		set((state) => ({
			...state,
			pins: { ...state.pins, [pin]: action },
		})),
	savePins: async () => axios.post(`${baseUrl}/api/setPinMappings`, get().pins),
}));

export default usePinStore;
