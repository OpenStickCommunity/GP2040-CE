export const INPUT_BOOT_MODE = {
    NONE: -1,
    XINPUT: 0,
	NINTENDO_SWITCH: 1,
	PS3: 2,
    KEYBOARD: 3,
	PS4: 4,
    XBONE: 5,
    MD_MINI: 6,
    NEO_GEO: 7,
	PCE_MINI: 8,
    EGRET: 9,
    ASTRO: 10,
    PS_CLASSIC: 11,
    XBOX_ORIGINAL: 12,
    PS5: 13,
	GENERIC: 14
};

export type InputBootModeKeys = keyof typeof INPUT_BOOT_MODE;
export type InputBootModeValues = (typeof INPUT_BOOT_MODE)[InputBootModeKeys];