import { create } from 'zustand';
import WebApi from '../Services/WebApi';
import { BUTTON_ACTIONS, PinActionValues } from '../Data/Pins';

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
	fetchProfiles: () => void;
	setProfilePin: SetProfilePinType;
	copyBaseProfile: (profileIndex: number) => void;
	setProfileLabel: (profileIndex: number, profileLabel: string) => void;
	saveProfiles: () => Promise<object>;
};

const DEFAULT_PIN_STATE = {
	action: BUTTON_ACTIONS.NONE,
	customButtonMask: 0,
	customDpadMask: 0,
};

const defaultProfilePins: PinsType = {
	pin00: DEFAULT_PIN_STATE,
	pin01: DEFAULT_PIN_STATE,
	pin02: DEFAULT_PIN_STATE,
	pin03: DEFAULT_PIN_STATE,
	pin04: DEFAULT_PIN_STATE,
	pin05: DEFAULT_PIN_STATE,
	pin06: DEFAULT_PIN_STATE,
	pin07: DEFAULT_PIN_STATE,
	pin08: DEFAULT_PIN_STATE,
	pin09: DEFAULT_PIN_STATE,
	pin10: DEFAULT_PIN_STATE,
	pin11: DEFAULT_PIN_STATE,
	pin12: DEFAULT_PIN_STATE,
	pin13: DEFAULT_PIN_STATE,
	pin14: DEFAULT_PIN_STATE,
	pin15: DEFAULT_PIN_STATE,
	pin16: DEFAULT_PIN_STATE,
	pin17: DEFAULT_PIN_STATE,
	pin18: DEFAULT_PIN_STATE,
	pin19: DEFAULT_PIN_STATE,
	pin20: DEFAULT_PIN_STATE,
	pin21: DEFAULT_PIN_STATE,
	pin22: DEFAULT_PIN_STATE,
	pin23: DEFAULT_PIN_STATE,
	pin24: DEFAULT_PIN_STATE,
	pin25: DEFAULT_PIN_STATE,
	pin26: DEFAULT_PIN_STATE,
	pin27: DEFAULT_PIN_STATE,
	pin28: DEFAULT_PIN_STATE,
	pin29: DEFAULT_PIN_STATE,
	profileLabel: '',
};

const INITIAL_STATE: State = {
	profiles: [
		defaultProfilePins,
		defaultProfilePins,
		defaultProfilePins,
		defaultProfilePins,
	],
	loadingProfiles: false,
};

const useProfilesStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
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
}));

export default useProfilesStore;
