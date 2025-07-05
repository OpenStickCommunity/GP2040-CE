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

export type AnimationProfile = {
	bEnabled: 0 | 1;
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
};

type State = {
	AnimationOptions: AnimationOptions;
	Lights: Light[];
	loading: boolean;
};

type Actions = {
	fetchLedOptions: () => void;
	saveAnimationOptions: (AnimationOptions: AnimationOptions) => Promise<object>;
	saveLightOptions: (Lights: Light[]) => Promise<object>;
};

const INITIAL_STATE: State = {
	AnimationOptions: {
		brightness: 0,
		baseProfileIndex: 0,
		customColors: [],
		profiles: [],
	},
	Lights: [],
	loading: false,
};

const useLedStore = create<State & Actions>()((set, get) => ({
	...INITIAL_STATE,
	fetchLedOptions: async () => {
		set({ loading: true });

		const { AnimationOptions } = await WebApi.getAnimationOptions();
		const { LightData } = await WebApi.getLightsDataOptions();

		set((state) => ({
			...state,
			AnimationOptions,
			Lights: LightData?.Lights || [],
			loading: false,
		}));
	},
	saveAnimationOptions: async (AnimationOptions: AnimationOptions) => {
		return WebApi.setAnimationOptions({ AnimationOptions });
	},
	saveLightOptions: async (Lights: Light[]) => {
		return WebApi.setLightsDataOptions({ LightData: { Lights } });
	},
}));

export default useLedStore;
