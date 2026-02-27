import { create } from "zustand";
import { INPUT_BOOT_MODE, InputBootModeValues } from "../Data/InputBootModes";
import WebApi from "src/Services/WebApi";

type BootModeMapping = {
    gpioPin: number,
    inputMode?: InputBootModeValues;
    defaultProfileIndex?: number;
};

type State = {
    bootModes: BootModeMapping[];
    loadingBootModes: boolean;
};

type Actions = {
    addMapping: () => void;
    removeMapping: (bootModeIndex: number) => void;
    fetchMappings: () => void;
    saveMappings: () => Promise<object>;
    setPin: (bootModeIndex: number, gpioPin: number) => void;
    setInputMode: (bootModeIndex: number, inputMode: InputBootModeValues) => void;
    setDefaultProfileIndex: (bootModeIndex: number, defaultProfileIndex?: number) => void;

};

const INITIAL_STATE = {
    bootModes: [],
    loadingBootModes: false,
};

const useBootModesStore = create<State & Actions>()((set, get) => ({
    ...INITIAL_STATE,

    addMapping: () => {
        set((state) => ({
            ...state,
            bootModes: [
                ...state.bootModes,
                {
                    gpioPin: state.bootModes.reduce((max, m) => Math.max(max, m.gpioPin) + 1, 0),
                    inputMode: undefined,
                    defaultProfileIndex: undefined
                }
            ]
        }))
    },

    removeMapping: (bootModeIndex: number) => {
        // TODO how do you validate stuff like this?
        set((state) => ({
            ...state,
            bootModes: state.bootModes.splice(bootModeIndex, 1)
        }));
    },

    fetchMappings: async () => {
        set({ loadingBootModes: true });
        const bootModeMappings = await WebApi.getBootModeMappings();

        set((state) => ({
            ...state,
            loadingBootModes: false,
            bootModes: bootModeMappings.map((m: any) => ({
                ...m,
                inputMode: m.inputMode == -1
                    ? undefined : m.inputMode,
                defaultProfileIndex: m.defaultProfileIndex == -1
                    ? undefined : m.defaultProfileIndex,
            })),
        }));
    },

    saveMappings: async () => {
        const bootModes = get().bootModes;
        return WebApi.setBootModeMappings(bootModes.map((m) => ({
            ...m,
            inputMode: m.inputMode ?? -1,
            defaultProfileIndex: m.defaultProfileIndex ?? -1,
        })));
    },

    setPin: (bootModeIndex: number, gpioPin: number) => {
        set((state) => {
            let bootModes = state.bootModes;
            bootModes[bootModeIndex].gpioPin = gpioPin;
            return { ...state, bootModes: bootModes };
        });
    },

    setInputMode: (bootModeIndex: number, inputMode: InputBootModeValues) => { 
        set((state) => {
            let bootModes = state.bootModes;
            bootModes[bootModeIndex].inputMode = inputMode;
            return { ...state, bootModes: bootModes };
        });
    },

    setDefaultProfileIndex: (bootModeIndex: number, defaultProfileIndex?: number) => { 
        set((state) => {
            let bootModes = state.bootModes;
            bootModes[bootModeIndex].defaultProfileIndex = defaultProfileIndex;
            return { ...state, bootModes: bootModes };
        });
    },
}));