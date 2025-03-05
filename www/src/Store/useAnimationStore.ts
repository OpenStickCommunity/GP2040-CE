import { create } from 'zustand';
import WebApi from '../Services/WebApi';
import {
	ANIMATION_NON_PRESSED_EFFECTS,
	ANIMATION_PRESSED_EFFECTS,
} from '../Data/Animations';

export const MAX_CUSTOM_COLORS = 8;
export const MAX_CASE_LIGHTS = 40;
export const MAX_PRESSED_COLORS = 30;
export const MAX_ANIMATION_PROFILES = 4;

type Profile = {
	bEnabled: 0 | 1;
	baseCaseEffect: typeof ANIMATION_NON_PRESSED_EFFECTS;
	baseNonPressedEffect: typeof ANIMATION_NON_PRESSED_EFFECTS;
	basePressedEffect: typeof ANIMATION_PRESSED_EFFECTS;
	buttonPressFadeOutTimeInMs: number;
	buttonPressHoldTimeInMs: number;
	caseStaticColors: number[];
	nonPressedSpecialColour: number;
	notPressedStaticColors: number[];
	pressedSpecialColour: number;
	pressedStaticColors: number[];
};

export type AnimationOptions = {
	brightness: number;
	baseProfileIndex: number;
	customColors: number[];
	profiles: Profile[];
};

type State = {
	AnimationOptions: AnimationOptions;
	loadingAnimationOptions: boolean;
};

type Actions = {
	fetchAnimationOptions: () => void;
	saveAnimationOptions: (AnimationOptions: AnimationOptions) => Promise<object>;
};

const INITIAL_STATE: State = {
	AnimationOptions: {
		brightness: 0,
		baseProfileIndex: 0,
		customColors: [],
		profiles: [],
	},
	loadingAnimationOptions: false,
};

const useAnimationStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	fetchAnimationOptions: async () => {
		set({ loadingAnimationOptions: true });

		const { AnimationOptions } = await WebApi.getAnimationOptions();
		set((state) => ({
			...state,
			AnimationOptions,
			loadingAnimationOptions: false,
		}));
	},
	saveAnimationOptions: async (AnimationOptions: AnimationOptions) => {
		return WebApi.setAnimationOptions({ AnimationOptions });
	},
}));

export default useAnimationStore;
