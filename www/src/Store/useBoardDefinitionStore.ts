import { create } from 'zustand';
import WebApi, { baseUrl } from '../Services/WebApi';

type State = {
	boardDefinition: {
		minPin: number;
		maxPin: number;
		analogPins: Array<number>;
		availablePins: {};
		usedPins: {};
	},
	loading: boolean;
	error: boolean;
};

type Actions = {
	getBoardDefinition: () => void;
};

const INITIAL_STATE: State = {
	boardDefinition: {
		minPin: -1,
		maxPin: -1,
		analogPins: [],
		availablePins: [],
		usedPins: {},
	},
	loading: false,
	error: false,
};

const useBoardDefinition = create<State & Actions>()((set) => ({
	...INITIAL_STATE,
	getBoardDefinition: async () => {
		set({ loading: true });

		try {
			const [definition] = await Promise.all([
				fetch(`${baseUrl}/api/getBoardDefinition`).then((res) => res.json()),
			]);

			set({boardDefinition: definition.pico});
		} catch (error) {
			set({ error: true, loading: false });
		}
	},
}));

export default useBoardDefinition;