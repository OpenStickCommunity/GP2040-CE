// Definitions in this file should match those in GP2040-CE/lib/AnimationStation/src/animation.hpp

import { rgbArrayToHex } from '../Services/Utilities';

const Black = rgbArrayToHex([0, 0, 0]);
const White = rgbArrayToHex([255, 255, 255]);
const Red = rgbArrayToHex([255, 0, 0]);
const Orange = rgbArrayToHex([255, 128, 0]);
const Yellow = rgbArrayToHex([255, 255, 0]);
const LimeGreen = rgbArrayToHex([128, 255, 0]);
const Green = rgbArrayToHex([0, 255, 0]);
const Seafoam = rgbArrayToHex([0, 255, 128]);
const Aqua = rgbArrayToHex([0, 255, 255]);
const SkyBlue = rgbArrayToHex([0, 128, 255]);
const Blue = rgbArrayToHex([0, 0, 255]);
const Purple = rgbArrayToHex([128, 0, 255]);
const Pink = rgbArrayToHex([255, 0, 255]);
const Magenta = rgbArrayToHex([255, 0, 128]);

const LEDColors = [
	{ name: Black, value: Black },
	{ name: White, value: White },
	{ name: Red, value: Red },
	{ name: Orange, value: Orange },
	{ name: Yellow, value: Yellow },
	{ name: LimeGreen, value: LimeGreen },
	{ name: Green, value: Green },
	{ name: Seafoam, value: Seafoam },
	{ name: Aqua, value: Aqua },
	{ name: SkyBlue, value: SkyBlue },
	{ name: Blue, value: Blue },
	{ name: Purple, value: Purple },
	{ name: Pink, value: Pink },
	{ name: Magenta, value: Magenta },
];

export default LEDColors;
