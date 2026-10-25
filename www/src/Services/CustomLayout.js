const MAX_CUSTOM_LAYOUT_ELEMENTS = 24;

const ELEMENT_VALUES = {
	GP_ELEMENT_BTN_BUTTON: 2,
	GP_ELEMENT_DIR_BUTTON: 3,
	GP_ELEMENT_PIN_BUTTON: 4,
	GP_ELEMENT_LEVER: 5,
	GP_ELEMENT_SHAPE: 8,
	GP_ELEMENT_HE_BUTTON: 9,
};

const DEFAULT_MAX_PIN = 47;

const MAX_VALUE = {
	[ELEMENT_VALUES.GP_ELEMENT_HE_BUTTON]: 31,
};

const SHAPE_VALUES = {
	GP_SHAPE_ELLIPSE: 0,
	GP_SHAPE_SQUARE: 1,
	GP_SHAPE_LINE: 2,
	GP_SHAPE_POLYGON: 3,
	GP_SHAPE_ARC: 4,
	GP_SHAPE_PILL: 5,
};

const PARAMETER_VALUES = {
	...SHAPE_VALUES,
	GP_LEVER_MODE_NONE: 0,
	GP_LEVER_MODE_DIGITAL: 1,
	GP_LEVER_MODE_LEFT_ANALOG: 2,
	GP_LEVER_MODE_RIGHT_ANALOG: 4,
	GP_LEVER_MODE_INVERT_X: 8,
	GP_LEVER_MODE_INVERT_Y: 16,
	GP_LEVER_MODE_DPAD: 32,
	GAMEPAD_MASK_UP: 1 << 0,
	GAMEPAD_MASK_DOWN: 1 << 1,
	GAMEPAD_MASK_LEFT: 1 << 2,
	GAMEPAD_MASK_RIGHT: 1 << 3,
	GAMEPAD_MASK_B1: 1 << 0,
	GAMEPAD_MASK_B2: 1 << 1,
	GAMEPAD_MASK_B3: 1 << 2,
	GAMEPAD_MASK_B4: 1 << 3,
	GAMEPAD_MASK_L1: 1 << 4,
	GAMEPAD_MASK_R1: 1 << 5,
	GAMEPAD_MASK_L2: 1 << 6,
	GAMEPAD_MASK_R2: 1 << 7,
	GAMEPAD_MASK_S1: 1 << 8,
	GAMEPAD_MASK_S2: 1 << 9,
	GAMEPAD_MASK_L3: 1 << 10,
	GAMEPAD_MASK_R3: 1 << 11,
	GAMEPAD_MASK_A1: 1 << 12,
	GAMEPAD_MASK_A2: 1 << 13,
	GAMEPAD_MASK_A3: 1 << 14,
	GAMEPAD_MASK_A4: 1 << 15,
};

const ELEMENT_NAMES = Object.fromEntries(
	Object.entries(ELEMENT_VALUES).map(([name, value]) => [value, name]),
);

const SHAPE_NAMES = Object.fromEntries(
	Object.entries(SHAPE_VALUES).map(([name, value]) => [value, name]),
);

const LEVER_MODE_NAMES = {
	1: 'GP_LEVER_MODE_DIGITAL',
	2: 'GP_LEVER_MODE_LEFT_ANALOG',
	4: 'GP_LEVER_MODE_RIGHT_ANALOG',
	32: 'GP_LEVER_MODE_DPAD',
};

const PARAM_COUNT = 11;
const BYTES_PER_ELEMENT = 12;
const PARAM_NAMES = [
	'x1',
	'y1',
	'x2',
	'y2',
	'stroke',
	'fill',
	'value',
	'shape',
	'angleStart',
	'angleEnd',
	'closed',
];
const MAX_ANGLE = 720;
const PARAM_MAX = [
	255,
	255,
	255,
	255,
	1,
	3,
	0xffff,
	5,
	MAX_ANGLE,
	MAX_ANGLE,
	1,
];
const VALUE_PARAM_INDEX = 6;
const SHAPE_PARAM_INDEX = 7;
const MIN_PARAMS = 7;
const X1_PARAM_INDEX = 0;
const Y1_PARAM_INDEX = 1;
const X2_PARAM_INDEX = 2;
const Y2_PARAM_INDEX = 3;
const MAX_POLYGON_SIDES = 32;

function resolveToken(token) {
	const parts = token
		.replace(/[()]/g, '')
		.split('|')
		.map((part) => part.trim());
	if (parts.some((part) => part === '')) return null;
	let result = 0;
	for (const part of parts) {
		let value;
		if (/^0[xX][0-9a-fA-F]+$/.test(part)) {
			value = parseInt(part, 16);
		} else if (/^[0-9]+$/.test(part)) {
			value = parseInt(part, 10);
		} else if (part in PARAMETER_VALUES) {
			value = PARAMETER_VALUES[part];
		} else {
			return null;
		}
		result |= value;
	}
	return result;
}

function defineBody(text) {
	const open = text.indexOf('{');
	let depth = 0;
	for (let i = open; i < text.length; i++) {
		if (text[i] === '{') depth++;
		else if (text[i] === '}' && --depth === 0) return text.slice(open + 1, i);
	}
	return null;
}

const ELEMENT_PATTERN =
	/\{\s*([A-Za-z_][A-Za-z0-9_]*|\d+)\s*,\s*\{([^{}]*)\}\s*\}/g;

export function parseLayoutCode(code, defineName, maxPin = DEFAULT_MAX_PIN) {
	if (!code || code.trim() === '') return { elements: [], error: null };

	let cleaned = code
		.replace(/\/\*[\s\S]*?\*\/|\/\/.*$/gm, '')
		.replace(/\\\s*$/gm, '');

	const define = [
		...cleaned.matchAll(/#define\s+([A-Za-z_][A-Za-z0-9_]*)\s*\{/g),
	].find((match) => match[1] === defineName);
	if (!define)
		return {
			elements: null,
			error: `Missing #define ${defineName} {`,
		};
	cleaned = defineBody(cleaned.slice(define.index));
	if (cleaned === null)
		return {
			elements: null,
			error: `Layout ${defineName} is missing its closing }`,
		};

	const rest = cleaned.replace(ELEMENT_PATTERN, (match) =>
		' '.repeat(match.length),
	);
	const leftoverAt = rest.search(/[^\s,]/);
	if (leftoverAt !== -1)
		return {
			elements: null,
			error: `Could not read "${rest.slice(leftoverAt).trim().split('\n')[0].slice(0, 40)}", check for a missing comma or brace`,
		};

	const elements = [];
	for (const match of cleaned.matchAll(ELEMENT_PATTERN)) {
		const elementNumber = elements.length + 1;
		if (elements.length >= MAX_CUSTOM_LAYOUT_ELEMENTS)
			return {
				elements: null,
				error: `Too many elements (maximum is ${MAX_CUSTOM_LAYOUT_ELEMENTS})`,
			};

		const [, typeToken, paramsText] = match;
		let elementType;
		if (/^\d+$/.test(typeToken) && ELEMENT_NAMES[parseInt(typeToken, 10)]) {
			elementType = parseInt(typeToken, 10);
		} else if (typeToken in ELEMENT_VALUES) {
			elementType = ELEMENT_VALUES[typeToken];
		} else {
			return {
				elements: null,
				error: `Unsupported element type "${typeToken}" in element ${elementNumber}`,
			};
		}

		const tokens = paramsText.split(',').map((token) => token.trim());
		if (tokens.length > 1 && tokens[tokens.length - 1] === '') tokens.pop();
		if (tokens.includes(''))
			return {
				elements: null,
				error: `Element ${elementNumber} has an empty parameter`,
			};
		if (tokens.length < MIN_PARAMS || tokens.length > PARAM_COUNT)
			return {
				elements: null,
				error: `Element ${elementNumber} has ${tokens.length} parameters (expected ${MIN_PARAMS} to ${PARAM_COUNT})`,
			};

		const params = [];
		for (const token of tokens) {
			const value = resolveToken(token);
			if (value === null || isNaN(value) || value < 0 || value > 0xffff)
				return {
					elements: null,
					error: `Invalid parameter "${token}" in element ${elementNumber}`,
				};
			params.push(value);
		}
		while (params.length < PARAM_COUNT) params.push(0);

		const tooHigh = params.findIndex(
			(value, index) => value > PARAM_MAX[index],
		);
		if (tooHigh !== -1)
			return {
				elements: null,
				error: `${PARAM_NAMES[tooHigh]} ${params[tooHigh]} in element ${elementNumber} is too high (maximum is ${PARAM_MAX[tooHigh]})`,
			};

		const maxValue =
			elementType === ELEMENT_VALUES.GP_ELEMENT_PIN_BUTTON
				? maxPin
				: MAX_VALUE[elementType];
		if (maxValue !== undefined && params[VALUE_PARAM_INDEX] > maxValue)
			return {
				elements: null,
				error: `Value ${params[VALUE_PARAM_INDEX]} in element ${elementNumber} is too high (maximum is ${maxValue})`,
			};

		if (elementType !== ELEMENT_VALUES.GP_ELEMENT_LEVER) {
			const shape = params[SHAPE_PARAM_INDEX];
			if (
				shape === SHAPE_VALUES.GP_SHAPE_POLYGON &&
				(params[Y2_PARAM_INDEX] < 1 ||
					params[Y2_PARAM_INDEX] > MAX_POLYGON_SIDES)
			)
				return {
					elements: null,
					error: `Polygon in element ${elementNumber} needs 1 to ${MAX_POLYGON_SIDES} sides (got ${params[Y2_PARAM_INDEX]})`,
				};
			if (
				(shape === SHAPE_VALUES.GP_SHAPE_ARC ||
					shape === SHAPE_VALUES.GP_SHAPE_POLYGON) &&
				(params[X1_PARAM_INDEX] < params[X2_PARAM_INDEX] ||
					params[Y1_PARAM_INDEX] < params[X2_PARAM_INDEX])
			)
				return {
					elements: null,
					error: `${shape === SHAPE_VALUES.GP_SHAPE_ARC ? 'Arc' : 'Polygon'} in element ${elementNumber} runs off the left or top of the screen (x1 and y1 must be at least the radius ${params[X2_PARAM_INDEX]})`,
				};
		}

		elements.push([elementType, ...params]);
	}

	if (elements.length === 0)
		return {
			elements: null,
			error:
				'No layout elements found, expected entries like {GP_ELEMENT_PIN_BUTTON, {47, 19, 4, 4, 1, 1, 27, GP_SHAPE_ELLIPSE}},',
		};

	return { elements, error: null };
}

function generateLayoutCode(elements, defineName) {
	if (!elements || elements.length === 0) return '';
	const lines = elements.map(([elementType, ...params]) => {
		const typeName = ELEMENT_NAMES[elementType] ?? String(elementType);
		const keep =
			elementType === ELEMENT_VALUES.GP_ELEMENT_LEVER
				? MIN_PARAMS
				: SHAPE_PARAM_INDEX + 1;
		let count = params.length;
		while (count > keep && params[count - 1] === 0) count--;
		const paramText = params
			.slice(0, count)
			.map((value, index) => {
				if (index === SHAPE_PARAM_INDEX && value in SHAPE_NAMES)
					return SHAPE_NAMES[value];
				if (
					index === VALUE_PARAM_INDEX &&
					elementType === ELEMENT_VALUES.GP_ELEMENT_LEVER &&
					value in LEVER_MODE_NAMES
				)
					return LEVER_MODE_NAMES[value];
				return String(value);
			})
			.join(', ');
		return `    {${typeName}, {${paramText}}},\\`;
	});
	return `#define ${defineName} {\\\n${lines.join('\n')}\n}`;
}

export function layoutCodeToBase64(code, defineName, maxPin) {
	const { elements, error } = parseLayoutCode(code, defineName, maxPin);
	return error === null ? elementsToBase64(elements) : null;
}

export function base64ToLayoutCode(base64, defineName) {
	return generateLayoutCode(base64ToElements(base64), defineName);
}

function elementsToBase64(elements) {
	const bytes = new Uint8Array(elements.length * BYTES_PER_ELEMENT);
	elements.forEach((values, elementIndex) => {
		const [
			elementType,
			x1,
			y1,
			x2,
			y2,
			stroke,
			fill,
			value,
			shape,
			angleStart,
			angleEnd,
			closed,
		] = values;
		const offset = elementIndex * BYTES_PER_ELEMENT;
		bytes.set(
			[
				elementType,
				x1,
				y1,
				x2,
				y2,
				stroke | (fill << 1) | (shape << 3) | (closed << 6),
				value & 0xff,
				value >> 8,
				angleStart & 0xff,
				angleStart >> 8,
				angleEnd & 0xff,
				angleEnd >> 8,
			],
			offset,
		);
	});
	return btoa(String.fromCharCode(...bytes));
}

function base64ToElements(base64) {
	if (!base64) return [];
	let binary;
	try {
		binary = atob(base64);
	} catch {
		return [];
	}
	const elements = [];
	const count = Math.floor(binary.length / BYTES_PER_ELEMENT);
	for (let elementIndex = 0; elementIndex < count; elementIndex++) {
		const offset = elementIndex * BYTES_PER_ELEMENT;
		const byte = (index) => binary.charCodeAt(offset + index);
		const word = (index) => byte(index) | (byte(index + 1) << 8);
		const flags = byte(5);
		elements.push([
			byte(0),
			byte(1),
			byte(2),
			byte(3),
			byte(4),
			flags & 0x01,
			(flags >> 1) & 0x03,
			word(6),
			(flags >> 3) & 0x07,
			word(8),
			word(10),
			(flags >> 6) & 0x01,
		]);
	}
	return elements;
}
