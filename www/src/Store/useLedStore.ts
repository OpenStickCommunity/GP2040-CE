import { create } from 'zustand';
import WebApi from '../Services/WebApi';
import {
	ANIMATION_NON_PRESSED_EFFECTS,
	ANIMATION_PRESSED_EFFECTS,
} from '../Data/Animations';

export const MAX_CUSTOM_COLORS = 8;
export const MAX_CASE_LIGHTS = 40;
export const MAX_LIGHTS = 100;
export const MAX_ANIMATION_PROFILES = 4;

export type AnimationProfile = {
	bEnabled: 0 | 1;
	bUseCaseLightsInPressedAnimations: 0 | 1;
	baseCaseEffect: typeof ANIMATION_NON_PRESSED_EFFECTS;
	baseNonPressedEffect: typeof ANIMATION_NON_PRESSED_EFFECTS;
	basePressedEffect: typeof ANIMATION_PRESSED_EFFECTS;
	buttonPressFadeOutTimeInMs: number;
	buttonPressHoldTimeInMs: number;
	caseStaticColors: number[];
	nonPressedSpecialColor: number;
	notPressedStaticColors: number[];
	pressedSpecialColor: number;
	pressedStaticColors: number[];
};

export type Light = {
	GPIOPinorCaseChainIndex: number;
	firstLedIndex: number;
	lightType: number;
	numLedsOnLight: number;
	xCoord: number;
	yCoord: number;
};

export type AnimationOptions = {
	brightness: number;
	baseProfileIndex: number;
	customColors: number[];
	profiles: AnimationProfile[];
	idletimeout: number;
};

export type LedOptions = {
	dataPin: number;
	ledFormat: number;
	turnOffWhenSuspended: number;
	brightnessMaximum: number;
};

type State = {
	ledOptions: LedOptions;
	AnimationOptions: AnimationOptions;
	Lights: Light[];
	loading: boolean;
	initialized: boolean;
};

type Actions = {
	fetchLedOptions: () => void;
	saveAnimationOptions: (AnimationOptions: AnimationOptions) => Promise<object>;
	saveLightOptions: (Lights: Light[]) => Promise<object>;
	saveLedOptions: (ledOptions: LedOptions) => Promise<boolean>;
};

const INITIAL_STATE: State = {
	ledOptions: {
		dataPin: -1,
		ledFormat: 0,
		turnOffWhenSuspended: 0,
		brightnessMaximum: 0,
	},
	AnimationOptions: {
		brightness: 0,
		idletimeout: 0,
		baseProfileIndex: 0,
		customColors: [],
		profiles: [],
	},
	Lights: [],
	loading: false,
	initialized: false,
};

const useLedStore = create<State & Actions>()((set) => ({
	...INITIAL_STATE,
	fetchLedOptions: async () => {
		set({ loading: true });

		const { AnimationOptions } = await WebApi.getAnimationOptions();
		const { LightData } = await WebApi.getLightsDataOptions();
		const ledOptions = await WebApi.getLedOptions();

		set((state) => ({
			...state,
			ledOptions,
			AnimationOptions: {
				...AnimationOptions,
				customColors: [
					...AnimationOptions.customColors,
					...Array(Math.max(0, MAX_CUSTOM_COLORS - AnimationOptions.customColors.length)).fill(0)
				]
			},
			Lights: LightData?.Lights || [],
			loading: false,
			initialized: true,
		}));
	},
	saveAnimationOptions: async (AnimationOptions: AnimationOptions) => {
		return WebApi.setAnimationOptions({ AnimationOptions });
	},
	saveLightOptions: async (Lights: Light[]) => {
		return WebApi.setLightsDataOptions({ LightData: { Lights } });
	},
	saveLedOptions: async (ledOptions: LedOptions) => {
		return WebApi.setLedOptions(ledOptions);
	},
}));

export default useLedStore;
