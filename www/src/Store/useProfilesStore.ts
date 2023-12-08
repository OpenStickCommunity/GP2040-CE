import axios from 'axios';
import { create } from 'zustand';
import WebApi, { baseUrl } from '../Services/WebApi';
import { PinActionValues } from '../Data/Pins';

type ProfileType = { [key: string]: PinActionValues };
type State = {
	profiles: ProfileType[];
	loadingProfiles: boolean;
};

type Actions = {
	fetchProfiles: () => void;
	setProfileAction: (
		profileIndex: number,
		pin: string,
		action: PinActionValues,
	) => void;
	saveProfiles: () => Promise<object>;
};

const defaultProfilePins: ProfileType = {
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
};

const INITIAL_STATE: State = {
	profiles: [defaultProfilePins, defaultProfilePins, defaultProfilePins],
	loadingProfiles: false,
};

const useProfilesStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	fetchProfiles: async () => {
		set({ loadingProfiles: true });
		const profiles = await WebApi.getProfileOptions();
		set((state) => ({
			...state,
			profiles,
			loadingProfiles: false,
		}));
	},
	setProfileAction: (profileIndex, pin, action) =>
		set((state) => ({
			...state,
			// Update selected profile state
			profiles: state.profiles.map((profile, index) =>
				index === profileIndex ? { ...profile, [pin]: action } : profile,
			),
		})),
	saveProfiles: async () => WebApi.setProfileOptions(get().profiles),
}));

export default useProfilesStore;
