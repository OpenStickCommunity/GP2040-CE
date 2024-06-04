import { create } from 'zustand';
import { GpioAction } from '../proto/enums_pb';

import WebApi from '../Services/WebApi';
import { PinActionValues } from '../Data/Pins';

type State = {
	pins: { [key: string]: PinActionValues };
	loadingPins: boolean;
};

type Actions = {
	fetchPins: () => void;
	setPinAction: (pin: string, action: PinActionValues) => void;
	savePins: () => Promise<object>;
};

const INITIAL_STATE: State = {
	pins: {
		pin00: GpioAction.NONE,
		pin01: GpioAction.NONE,
		pin02: GpioAction.NONE,
		pin03: GpioAction.NONE,
		pin04: GpioAction.NONE,
		pin05: GpioAction.NONE,
		pin06: GpioAction.NONE,
		pin07: GpioAction.NONE,
		pin08: GpioAction.NONE,
		pin09: GpioAction.NONE,
		pin10: GpioAction.NONE,
		pin11: GpioAction.NONE,
		pin12: GpioAction.NONE,
		pin13: GpioAction.NONE,
		pin14: GpioAction.NONE,
		pin15: GpioAction.NONE,
		pin16: GpioAction.NONE,
		pin17: GpioAction.NONE,
		pin18: GpioAction.NONE,
		pin19: GpioAction.NONE,
		pin20: GpioAction.NONE,
		pin21: GpioAction.NONE,
		pin22: GpioAction.NONE,
		pin23: GpioAction.NONE,
		pin24: GpioAction.NONE,
		pin25: GpioAction.NONE,
		pin26: GpioAction.NONE,
		pin27: GpioAction.NONE,
		pin28: GpioAction.NONE,
		pin29: GpioAction.NONE,
	},
	loadingPins: false,
};

const usePinStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	fetchPins: async () => {
		set({ loadingPins: true });
		const pins = await WebApi.getPinMappings();
		set((state) => ({
			...state,
			pins: { ...state.pins, ...pins }, // Merge saved pins with defaults
			loadingPins: false,
		}));
	},
	setPinAction: (pin, action) =>
		set((state) => ({
			...state,
			pins: { ...state.pins, [pin]: action },
		})),
	savePins: async () => WebApi.setPinMappings(get().pins),
}));

export default usePinStore;
