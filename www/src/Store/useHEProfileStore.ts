import { create } from 'zustand';

import WebApi from '../Services/WebApi';

// Total binding profiles: the base set plus three alternates. Must match
// HE_PROFILE_COUNT in headers/addons/he_trigger.h.
export const HE_PROFILE_COUNT = 4;
export const HE_TRIGGER_COUNT = 32;

// Bindings, plus optional per-channel tuning overrides. Calibration itself
// (idle/pressed/polarity/noise) lives on the triggers and is always shared,
// because it describes the switch rather than how it should behave.
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

// Prefix makes a mis-pasted string fail with a clear message rather than
// importing garbage; the version guards against a future format change.
export const HE_PROFILE_CODE_PREFIX = 'HE1:';
export const HE_PROFILE_CODE_VERSION = 1;

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
	// Copies one profile's bindings and tuning over another. The destination's
	// enabled flag is preserved -- copying content should not silently switch a
	// profile into the cycle rotation.
	copyProfile: (fromIndex: number, toIndex: number) => void;
	// Serialises one profile to a shareable string, and restores from one.
	exportProfile: (profileIndex: number) => string;
	importProfile: (profileIndex: number, code: string) => void;
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

	copyProfile: (fromIndex, toIndex) => {
		set((state) => {
			const source = state.profiles[fromIndex];
			if (!source || fromIndex === toIndex) return state;
			return {
				...state,
				profiles: state.profiles.map((profile, index) =>
					index === toIndex
						? {
								...profile,
								actions: [...source.actions],
								rapidTrigger: [...source.rapidTrigger],
								actuationPoint: [...source.actuationPoint],
								rtPressSensitivity: [...source.rtPressSensitivity],
								rtReleaseSensitivity: [...source.rtReleaseSensitivity],
							}
						: profile,
				),
			};
		});
	},

	exportProfile: (profileIndex) => {
		// Run-length encode each column. Most channels share a value -- unbound
		// runs, whole blocks on one sensitivity -- so this roughly halves the code
		// and keeps it short enough to paste without wrapping.
		const rle = (values: number[]) => {
			const out: string[] = [];
			let i = 0;
			while (i < values.length) {
				let j = i;
				while (j < values.length && values[j] === values[i]) j++;
				out.push(j - i > 1 ? `${values[i]}x${j - i}` : `${values[i]}`);
				i = j;
			}
			return out.join(',');
		};
		const profile = get().profiles[profileIndex];
		if (!profile) return '';
		// Compact positional form rather than JSON: the result is pasted by hand,
		// so it needs to stay short enough to copy without wrapping.
		const payload = [
			HE_PROFILE_CODE_VERSION,
			rle(profile.actions),
			rle(profile.rapidTrigger),
			rle(profile.actuationPoint),
			rle(profile.rtPressSensitivity),
			rle(profile.rtReleaseSensitivity),
		].join('|');
		return `${HE_PROFILE_CODE_PREFIX}${btoa(payload)}`;
	},

	importProfile: (profileIndex, code) => {
		const trimmed = code.trim();
		if (!trimmed.startsWith(HE_PROFILE_CODE_PREFIX)) {
			throw new Error('Not a hall effect profile code');
		}
		let payload: string;
		try {
			payload = atob(trimmed.slice(HE_PROFILE_CODE_PREFIX.length));
		} catch {
			throw new Error('Profile code is corrupt');
		}
		const parts = payload.split('|');
		if (parts.length !== 6 || parts[0] !== String(HE_PROFILE_CODE_VERSION)) {
			throw new Error('Profile code is from an incompatible version');
		}
		// Pad or truncate to the channel count so a code from a board with a
		// different channel count still imports what it can.
		const column = (raw: string, fallback: number) => {
			const values: number[] = [];
			for (const token of raw.split(',')) {
				if (!token) continue;
				const [value, count] = token.split('x');
				const repeat = count ? Number(count) : 1;
				for (let k = 0; k < repeat; k++) values.push(Number(value));
			}
			return Array.from({ length: HE_TRIGGER_COUNT }, (_, i) =>
				Number.isFinite(values[i]) ? values[i] : fallback,
			);
		};
		const actions = column(parts[1], -10);
		const rapidTrigger = column(parts[2], 0);
		const actuationPoint = column(parts[3], 0);
		const rtPressSensitivity = column(parts[4], 0);
		const rtReleaseSensitivity = column(parts[5], 0);

		set((state) => ({
			...state,
			profiles: state.profiles.map((profile, index) =>
				index === profileIndex
					? {
							...profile,
							actions,
							rapidTrigger,
							actuationPoint,
							rtPressSensitivity,
							rtReleaseSensitivity,
						}
					: profile,
			),
		}));
	},

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
