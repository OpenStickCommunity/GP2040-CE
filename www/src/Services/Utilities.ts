// Convert a hex string to a number
const hexToInt = (hex: string): number => {
	return parseInt(hex.replace('#', ''), 16);
};

const rgbIntToHex = (color: number) =>
	`#${color.toString(16).padStart(6, '0')}`;

const createEnumRecord = <T extends Record<string, number | string>>(
	enumObj: T,
): Record<keyof T, T[keyof T]> =>
Object.fromEntries(
		Object.entries(enumObj).filter(([, value]) => typeof value === 'number'),
	) as Record<keyof T, T[keyof T]>;


export { hexToInt, rgbIntToHex, createEnumRecord };
