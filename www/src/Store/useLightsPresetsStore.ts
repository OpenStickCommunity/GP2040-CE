import { create } from 'zustand';
import WebApi from '../Services/WebApi';
import { Light } from './useLedStore';

type State = {
	loading: boolean;
	presets: { name: string; lightData: { Lights: Light[] } }[]
};

type Actions = {
	fetchPresets: () => void;
};

const INITIAL_STATE: State = {
	loading: false,
	presets: [],
};

async function* fetchPresetsGenerator() {
	let index = 0;
	while (true) {
		try {
			const preset = await WebApi.getLightsPresets(index);
			if (preset) {
				yield preset;
				index++;
			} else {
				break;
			}
		} catch (error) {
			break;
		}
	}
}

const useLightsPresetsStore = create<State & Actions>()((set) => ({
	fetchPresets: async () => {
		set({ loading: true });
		const fetchedPresets = [];
		for await (const preset of fetchPresetsGenerator()) {
			if (preset.lightData && preset.name) {
				fetchedPresets.push(preset);
			}
		}
		set({ presets: fetchedPresets, loading: false });
	},
	...INITIAL_STATE,
}));


export default useLightsPresetsStore;
