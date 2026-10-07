const MAX_CUSTOM_LAYOUT_ELEMENTS = 30;

const ELEMENT_VALUES = {
	GP_ELEMENT_BTN_BUTTON: 2,
	GP_ELEMENT_DIR_BUTTON: 3,
	GP_ELEMENT_PIN_BUTTON: 4,
	GP_ELEMENT_LEVER: 5,
	GP_ELEMENT_SHAPE: 8,
	GP_ELEMENT_HE_BUTTON: 9,
};

const MAX_VALUE = {
	[ELEMENT_VALUES.GP_ELEMENT_PIN_BUTTON]: 47,
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
const VALUES_PER_ELEMENT = PARAM_COUNT + 1;
const BYTES_PER_ELEMENT = VALUES_PER_ELEMENT * 2;
const VALUE_PARAM_INDEX = 6;
const SHAPE_PARAM_INDEX = 7;
const MIN_PARAMS = 7;

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

export function parseLayoutCode(code, preferredDefine) {
	if (!code || code.trim() === '') return { elements: [], error: null };

	let cleaned = code
		.replace(/\/\*[\s\S]*?\*\//g, '')
		.replace(/\/\/.*$/gm, '')
		.replace(/\\\s*$/gm, '');

	const defines = [
		...cleaned.matchAll(/#define\s+([A-Za-z_][A-Za-z0-9_]*)\s*\{/g),
	];
	if (defines.length > 1) {
		const start = defines.findIndex((define) => define[1] === preferredDefine);
		if (start === -1)
			return {
				elements: null,
				error: `Multiple layout definitions found; paste a single layout or include one named ${preferredDefine}`,
			};
		cleaned = cleaned.slice(
			defines[start].index,
			start + 1 < defines.length ? defines[start + 1].index : undefined,
		);
	}

	const elementPattern =
		/\{\s*([A-Za-z_][A-Za-z0-9_]*|\d+)\s*,\s*\{([^{}]*)\}\s*\}/g;
	const elements = [];
	let match;
	while ((match = elementPattern.exec(cleaned)) !== null) {
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
				error: `Unsupported element type "${typeToken}" in element ${elements.length + 1}`,
			};
		}

		const tokens = paramsText
			.split(',')
			.map((token) => token.trim())
			.filter((token) => token !== '');
		if (tokens.length < MIN_PARAMS || tokens.length > PARAM_COUNT)
			return {
				elements: null,
				error: `Element ${elements.length + 1} has ${tokens.length} parameters (expected ${MIN_PARAMS} to ${PARAM_COUNT})`,
			};

		const params = [];
		for (const token of tokens) {
			const value = resolveToken(token);
			if (value === null || isNaN(value) || value < 0 || value > 0xffff)
				return {
					elements: null,
					error: `Invalid parameter "${token}" in element ${elements.length + 1}`,
				};
			params.push(value);
		}
		while (params.length < PARAM_COUNT) params.push(0);

		if (params[VALUE_PARAM_INDEX] > (MAX_VALUE[elementType] ?? 0xffff))
			return {
				elements: null,
				error: `Value ${params[VALUE_PARAM_INDEX]} in element ${elements.length + 1} is too high (maximum is ${MAX_VALUE[elementType]})`,
			};

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

export function layoutCodeToBase64(code, defineName) {
	const { elements, error } = parseLayoutCode(code, defineName);
	return error === null ? elementsToBase64(elements) : null;
}

export function base64ToLayoutCode(base64, defineName) {
	return generateLayoutCode(base64ToElements(base64), defineName);
}

function elementsToBase64(elements) {
	const bytes = new Uint8Array(elements.length * BYTES_PER_ELEMENT);
	elements.forEach((values, elementIndex) => {
		values.forEach((value, valueIndex) => {
			const offset = elementIndex * BYTES_PER_ELEMENT + valueIndex * 2;
			bytes[offset] = value & 0xff;
			bytes[offset + 1] = (value >> 8) & 0xff;
		});
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
		const values = [];
		for (let valueIndex = 0; valueIndex < VALUES_PER_ELEMENT; valueIndex++) {
			const offset = elementIndex * BYTES_PER_ELEMENT + valueIndex * 2;
			values.push(
				binary.charCodeAt(offset) | (binary.charCodeAt(offset + 1) << 8),
			);
		}
		elements.push(values);
	}
	return elements;
}
