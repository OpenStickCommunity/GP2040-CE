import { create } from 'zustand';

import WebApi from '../Services/WebApi';
import { PinActionValues, PinDirectionValues } from '../Data/Pins';

type State = {
	pins: {
		[key: string]: [
			{
				[key: string]: {
					option: PinActionValues;
					direction: PinDirectionValues;
				};
			},
		];
	};
	loadingPins: boolean;
};

type Actions = {
	fetchPins: () => void;
	setPinAction: (
		expansion: string,
		index: number,
		pin: string,
		action: PinActionValues,
	) => void;
	setPinDirection: (
		expansion: string,
		index: number,
		pin: string,
		dir: PinDirectionValues,
	) => void;
	savePins: () => Promise<object>;
};

const INITIAL_STATE: State = {
	pins: {
		pcf8575: [
			{
				pin00: { option: -10, direction: 0 },
				pin01: { option: -10, direction: 0 },
				pin02: { option: -10, direction: 0 },
				pin03: { option: -10, direction: 0 },
				pin04: { option: -10, direction: 0 },
				pin05: { option: -10, direction: 0 },
				pin06: { option: -10, direction: 0 },
				pin07: { option: -10, direction: 0 },
				pin08: { option: -10, direction: 0 },
				pin09: { option: -10, direction: 0 },
				pin10: { option: -10, direction: 0 },
				pin11: { option: -10, direction: 0 },
				pin12: { option: -10, direction: 0 },
				pin13: { option: -10, direction: 0 },
				pin14: { option: -10, direction: 0 },
				pin15: { option: -10, direction: 0 },
			},
		],
	},
	loadingPins: false,
};

const useExpansionPinStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	fetchPins: async () => {
		set({ loadingPins: true });
		const pins = await WebApi.getExpansionPins();
		set((state) => ({
			...state,
			...pins,
			loadingPins: false,
		}));
	},
	setPinAction: (expansion, index, pin, action) => {
		set((state) => {
			const newPins = { ...state.pins };

			if (
				newPins[expansion] &&
				newPins[expansion][index] &&
				newPins[expansion][index][pin]
			) {
				newPins[expansion][index] = {
					...newPins[expansion][index],
					[pin]: {
						...newPins[expansion][index][pin],
						option: action,
					},
				};
			}

			return {
				...state,
				pins: newPins,
			};
		});
	},
	setPinDirection: (expansion, index, pin, dir) => {
		set((state) => {
			const newPins = { ...state.pins };

			if (
				newPins[expansion] &&
				newPins[expansion][index] &&
				newPins[expansion][index][pin]
			) {
				newPins[expansion][index] = {
					...newPins[expansion][index],
					[pin]: {
						...newPins[expansion][index][pin],
						direction: dir,
					},
				};
			}

			return {
				...state,
				pins: newPins,
			};
		});
	},
	savePins: async () => WebApi.setExpansionPins(get()),
}));

export default useExpansionPinStore;
