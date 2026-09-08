import { create } from 'zustand';

import WebApi from '../Services/WebApi';

// Total binding profiles: the base set plus three alternates. Must match
// HE_PROFILE_COUNT in headers/addons/he_trigger.h.
export const HE_PROFILE_COUNT = 4;
export const HE_TRIGGER_COUNT = 32;

// Bindings only. Calibration and tuning live on the triggers themselves and are
// deliberately shared across profiles, to keep the config small.
export type HEProfile = {
	enabled: boolean;
	actions: number[];
	// Per-channel tuning overrides. 0 means "not set for this profile", so the
	// base switch value is used. rapidTrigger stores 1 = off, 2 = on, leaving 0
	// free as the not-set sentinel.
	rapidTrigger: number[];
	actuationPoint: number[];
	rtPressSensitivity: number[];
	rtReleaseSensitivity: number[];
};

export const RT_UNSET = 0;
export const RT_OFF = 1;
export const RT_ON = 2;

type State = {
	profiles: HEProfile[];
	activeProfile: number;
	loadingProfiles: boolean;
	// False until fetchHEProfiles has returned. Saving before that would write
	// INITIAL_STATE -- every binding NONE -- over whatever is on the device.
	profilesLoaded: boolean;
};

type Actions = {
	fetchHEProfiles: () => Promise<void>;
	setProfileAction: (
		profileIndex: number,
		channel: number,
		action: number,
	) => void;
	toggleProfileEnabled: (profileIndex: number) => void;
	setProfileTuning: (
		profileIndex: number,
		channel: number,
		patch: Partial<
			Record<
				| 'rapidTrigger'
				| 'actuationPoint'
				| 'rtPressSensitivity'
				| 'rtReleaseSensitivity',
				number
			>
		>,
	) => void;
	setActiveProfile: (profileIndex: number) => void;
	saveHEProfiles: () => Promise<object>;
};

const zeros = () => Array.from({ length: HE_TRIGGER_COUNT }, () => 0);

const emptyProfile = (): HEProfile => ({
	enabled: false,
	actions: Array.from({ length: HE_TRIGGER_COUNT }, () => -10),
	rapidTrigger: zeros(),
	actuationPoint: zeros(),
	rtPressSensitivity: zeros(),
	rtReleaseSensitivity: zeros(),
});

const INITIAL_STATE: State = {
	profiles: Array.from({ length: HE_PROFILE_COUNT }, (_, index) => ({
		...emptyProfile(),
		// The base profile is always active; it cannot be disabled.
		enabled: index === 0,
	})),
	activeProfile: 0,
	loadingProfiles: false,
	profilesLoaded: false,
};

const useHEProfileStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,

	fetchHEProfiles: async () => {
		set({ loadingProfiles: true });
		const data = await WebApi.getHETriggerProfiles();
		set((state) => ({
			...state,
			// Tolerate a short or missing response rather than rendering undefined.
			profiles: Array.from({ length: HE_PROFILE_COUNT }, (_, index) => {
				const incoming = data?.profiles?.[index];
				if (!incoming) return { ...emptyProfile(), enabled: index === 0 };
				const column = (key: keyof HEProfile) =>
					Array.from(
						{ length: HE_TRIGGER_COUNT },
						(__, channel) =>
							(incoming[key] as number[] | undefined)?.[channel] ?? 0,
					);
				return {
					enabled: index === 0 ? true : Boolean(incoming.enabled),
					actions: Array.from(
						{ length: HE_TRIGGER_COUNT },
						(__, channel) => incoming.actions?.[channel] ?? -10,
					),
					rapidTrigger: column('rapidTrigger'),
					actuationPoint: column('actuationPoint'),
					rtPressSensitivity: column('rtPressSensitivity'),
					rtReleaseSensitivity: column('rtReleaseSensitivity'),
				};
			}),
			activeProfile: data?.activeProfile ?? 0,
			loadingProfiles: false,
			// Only mark loaded when the device actually answered; a failed fetch
			// must not licence a save that would overwrite the board.
			profilesLoaded: Boolean(data?.profiles),
		}));
	},

	setProfileAction: (profileIndex, channel, action) => {
		set((state) => {
			const profiles = state.profiles.map((profile, index) => {
				if (index !== profileIndex) return profile;
				const actions = [...profile.actions];
				actions[channel] = action;
				// Editing a profile enables it. Cycling skips disabled profiles, so a
				// profile that was configured but never explicitly switched on is
				// invisible at runtime -- which reads as "profile switching is
				// broken" rather than "this profile is off".
				return { ...profile, actions, enabled: true };
			});
			return { ...state, profiles };
		});
	},

	setProfileTuning: (profileIndex, channel, patch) => {
		set((state) => ({
			...state,
			profiles: state.profiles.map((profile, index) => {
				if (index !== profileIndex) return profile;
				const next = { ...profile };
				for (const [key, value] of Object.entries(patch)) {
					const column = [...(next[key as keyof HEProfile] as number[])];
					column[channel] = value as number;
					(next as Record<string, unknown>)[key] = column;
				}
				return next;
			}),
		}));
	},

	toggleProfileEnabled: (profileIndex) => {
		// The base profile is the fallback for every other profile, so it must
		// always remain enabled.
		if (profileIndex === 0) return;
		set((state) => ({
			...state,
			profiles: state.profiles.map((profile, index) =>
				index === profileIndex
					? { ...profile, enabled: !profile.enabled }
					: profile,
			),
		}));
	},

	setActiveProfile: (profileIndex) =>
		set((state) => ({ ...state, activeProfile: profileIndex })),

	saveHEProfiles: async () => {
		const { profiles, activeProfile, profilesLoaded } = get();
		// Refuse to save a store that was never populated. setHETriggerProfiles
		// overwrites every binding on the device, so posting INITIAL_STATE would
		// wipe the board's profiles -- which is exactly what happened when the
		// calibration wizard saved before anything had fetched.
		if (!profilesLoaded) {
			throw new Error(
				'Refusing to save hall effect profiles before they have been loaded',
			);
		}
		return WebApi.setHETriggerProfiles({ profiles, activeProfile });
	},
}));

export default useHEProfileStore;
