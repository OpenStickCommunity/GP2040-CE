import { create } from 'zustand';
type State = {
	loading: boolean;
	releaseVersion: string;
};

type Actions = {
	getInfo: () => void;
};

const INITIAL_STATE: State = {
	releaseVersion: null,
	loading: false,
};

export const useGitHubInfo = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	getInfo: () => {
		// Hacky way to make sure we don't call this multiple times from different components
		if (get().loading || get().releaseVersion) return;
		set({ loading: true });
		fetch(
			`https://api.github.com/repos/OpenStickCommunity/GP2040-CE/releases/latest`
		)
			.then((res) => res.json())
			.then(({ name }) => set({ loading: false, releaseVersion: name }));
	},
}));
