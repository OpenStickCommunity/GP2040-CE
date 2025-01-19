import { create } from 'zustand';
import WebApi from '../Services/WebApi';

export const MAX_CUSTOM_COLORS = 8;
export const MAX_ANIMATION_PROFILES = 4;

type Profile = {
	bEnabled: boolean;
	baseCaseEffect: number;
	baseNonPressedEffect: number;
	basePressedEffect: number;
	buttonPressFadeOutTimeInMs: number;
	buttonPressHoldTimeInMs: number;
	caseStaticColors: number[];
	nonPressedSpecialColour: number;
	notPressedStaticColors: number[];
	pressedSpecialColour: number;
	pressedStaticColors: number[];
};

type AnimationOptions = {
	brightness: number;
	baseProfileIndex: number;
	customColors: number[];
	profiles: Profile[];
};

type State = {
	animationOptions: AnimationOptions;
	loadingAnimationOptions: boolean;
};

type Actions = {
	fetchAnimationOptions: () => void;
};

const INITIAL_STATE: State = {
	animationOptions: {
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
		set(() => ({
			animationOptions: AnimationOptions,
			loadingAnimationOptions: false,
		}));
	},
}));

export default useAnimationStore;
