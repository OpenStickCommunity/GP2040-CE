import { create } from 'zustand';
import WebApi from '../Services/WebApi';
import { PinActionValues } from '../Data/Pins';

// Max number of profiles that can be created, including the base profile
export const MAX_PROFILES = 4;

type CustomMasks = {
	customButtonMask: number;
	customDpadMask: number;
};

export type MaskPayload = {
	action: PinActionValues;
} & CustomMasks;

export type PinsType = {
	pin00: MaskPayload;
	pin01: MaskPayload;
	pin02: MaskPayload;
	pin03: MaskPayload;
	pin04: MaskPayload;
	pin05: MaskPayload;
	pin06: MaskPayload;
	pin07: MaskPayload;
	pin08: MaskPayload;
	pin09: MaskPayload;
	pin10: MaskPayload;
	pin11: MaskPayload;
	pin12: MaskPayload;
	pin13: MaskPayload;
	pin14: MaskPayload;
	pin15: MaskPayload;
	pin16: MaskPayload;
	pin17: MaskPayload;
	pin18: MaskPayload;
	pin19: MaskPayload;
	pin20: MaskPayload;
	pin21: MaskPayload;
	pin22: MaskPayload;
	pin23: MaskPayload;
	pin24: MaskPayload;
	pin25: MaskPayload;
	pin26: MaskPayload;
	pin27: MaskPayload;
	pin28: MaskPayload;
	pin29: MaskPayload;
	profileLabel: string;
	enabled: boolean;
};

type State = {
	profiles: PinsType[];
	loadingProfiles: boolean;
};

export type SetProfilePinType = (
	profileIndex: number,
	pin: string,
	{ action, customButtonMask, customDpadMask }: MaskPayload,
) => void;

type Actions = {
	addProfile: () => void;
	copyBaseProfile: (profileIndex: number) => void;
	fetchProfiles: () => void;
	saveProfiles: () => Promise<object>;
	setProfileLabel: (profileIndex: number, profileLabel: string) => void;
	setProfilePin: SetProfilePinType;
	toggleProfileEnabled: (profileIndex: number) => void;
};

const INITIAL_STATE: State = {
	profiles: [
		// Profiles will be populated dynamically
	],
	loadingProfiles: false,
};

const useProfilesStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	addProfile: () => {
		if (get().profiles.length < MAX_PROFILES) {
			set((state) => ({
				profiles: [
					...state.profiles,
					{
						...state.profiles[0],
						profileLabel: `Profile ${state.profiles.length + 1}`,
					},
				],
			}));
		}
	},
	fetchProfiles: async () => {
		set({ loadingProfiles: true });

		// TODO, unify baseProfile with other profiles when done in web api
		const baseProfile = await WebApi.getPinMappings();
		const profiles = await WebApi.getProfileOptions();

		set((state) => ({
			...state,
			profiles: [baseProfile, ...profiles],
			loadingProfiles: false,
		}));
	},
	copyBaseProfile: (profileIndex) =>
		set((state) => ({
			...state,
			profiles: state.profiles.map((profile, index) =>
				index === profileIndex
					? {
							...profile,
							...state.profiles[0],
							profileLabel: profile.profileLabel,
						}
					: profile,
			),
		})),
	setProfilePin: (
		profileIndex,
		pin,
		{ action, customButtonMask = 0, customDpadMask = 0 },
	) =>
		set((state) => {
			const profiles = [...state.profiles];
			profiles[profileIndex] = {
				...profiles[profileIndex],
				[pin]: {
					action,
					customButtonMask,
					customDpadMask,
				},
			};
			return { profiles };
		}),
	setProfileLabel: (profileIndex, profileLabel) =>
		set((state) => {
			const profiles = [...state.profiles];
			profiles[profileIndex] = { ...profiles[profileIndex], profileLabel };
			return { profiles };
		}),
	saveProfiles: async () => {
		const [baseProfile, ...profiles] = get().profiles;
		return Promise.all([
			WebApi.setPinMappings(baseProfile),
			WebApi.setProfileOptions(profiles),
		]);
	},
	toggleProfileEnabled: (profileIndex) =>
		set((state) => {
			const profiles = [...state.profiles];
			profiles[profileIndex] = {
				...profiles[profileIndex],
				enabled: !profiles[profileIndex].enabled,
			};
			return { ...state, profiles };
		}),
}));

export default useProfilesStore;
