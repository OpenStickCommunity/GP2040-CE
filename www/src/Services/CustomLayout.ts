import invert from 'lodash/invert';
import * as yup from 'yup';
import { GPElement, GPLever_Mode, GPShape_Type } from '@proto/enums';
import { BUTTON_MASKS, DPAD_MASKS } from '../Data/Buttons';
import { createEnumRecord } from './Utilities';

const MAX_CUSTOM_LAYOUT_ELEMENTS = 24;
const DEFAULT_MAX_PIN = 47;

const ELEMENT_VALUES: Record<string, number> = Object.fromEntries(
	[
		GPElement.GP_ELEMENT_BTN_BUTTON,
		GPElement.GP_ELEMENT_DIR_BUTTON,
		GPElement.GP_ELEMENT_PIN_BUTTON,
		GPElement.GP_ELEMENT_LEVER,
		GPElement.GP_ELEMENT_SHAPE,
		GPElement.GP_ELEMENT_HE_BUTTON,
	].map((value) => [GPElement[value], value]),
);

const MAX_VALUE: Record<number, number> = {
	[GPElement.GP_ELEMENT_HE_BUTTON]: 31,
};

const SHAPE_VALUES: Record<string, number> = createEnumRecord(GPShape_Type);
const LEVER_MODE_VALUES: Record<string, number> =
	createEnumRecord(GPLever_Mode);

const VALUE_NAMES: Record<string, number> = {
	...LEVER_MODE_VALUES,
	...Object.fromEntries(
		[...DPAD_MASKS, ...BUTTON_MASKS].map(({ label, value }) => [
			`GAMEPAD_MASK_${label.toUpperCase()}`,
			value,
		]),
	),
};

const ELEMENT_NAMES = invert(ELEMENT_VALUES);
const SHAPE_NAMES = invert(SHAPE_VALUES);
const LEVER_MODE_NAMES = invert(LEVER_MODE_VALUES);

const BYTES_PER_ELEMENT = 12;
const MAX_ANGLE = 720;
const MAX_POLYGON_SIDES = 32;

const resolveNames =
	(names: Record<string, number>) => (value: unknown, original: unknown) => {
		if (typeof original !== 'string') return value;
		let result = 0;
		for (const part of original.split('|').map((name) => name.trim())) {
			if (!(part in names)) return NaN;
			result |= names[part];
		}
		return result;
	};

const param = (name: string, max: number) =>
	yup
		.number()
		.label(name)
		.typeError('${path} must be a number')
		.integer()
		.min(0)
		.max(max);

const layoutSchema = (maxPin: number) => {
	const fields = {
		type: yup
			.number()
			.label('type')
			.transform(resolveNames(ELEMENT_VALUES))
			.typeError(
				`\${path} must be one of ${Object.keys(ELEMENT_VALUES).join(', ')}`,
			)
			.oneOf(
				Object.values(ELEMENT_VALUES),
				`\${path} must be one of ${Object.keys(ELEMENT_VALUES).join(', ')}`,
			)
			.required(),
		x1: param('x1', 255).required(),
		y1: param('y1', 255).required(),
		x2: param('x2', 255).required(),
		y2: param('y2', 255).required(),
		stroke: param('stroke', 1).required(),
		fill: param('fill', 3).required(),
		value: param('value', 0xffff)
			.transform(resolveNames(VALUE_NAMES))
			.typeError('${path} must be a number or a known constant name')
			.when('type', ([type], schema) => {
				const max =
					type === GPElement.GP_ELEMENT_PIN_BUTTON ? maxPin : MAX_VALUE[type];
				return max === undefined ? schema : schema.max(max);
			})
			.required(),
		shape: param('shape', 0xff)
			.transform(resolveNames(SHAPE_VALUES))
			.typeError(
				`\${path} must be one of ${Object.keys(SHAPE_VALUES).join(', ')}`,
			)
			.oneOf(
				Object.values(SHAPE_VALUES),
				`\${path} must be one of ${Object.keys(SHAPE_VALUES).join(', ')}`,
			)
			.default(0),
		angleStart: param('angleStart', MAX_ANGLE).default(0),
		angleEnd: param('angleEnd', MAX_ANGLE).default(0),
		closed: param('closed', 1).default(0),
	};
	const element = yup
		.object(fields)
		.test('no-unknown', (_, context) => {
			const unknown = Object.keys(context.originalValue ?? {}).filter(
				(key) => !(key in fields),
			);
			return (
				unknown.length === 0 ||
				context.createError({
					message: `unknown field(s) ${unknown.join(', ')}`,
				})
			);
		})
		.test('shape-bounds', (element, context) => {
			const { type, shape, x1, y1, x2, y2 } = element;
			if (type === GPElement.GP_ELEMENT_LEVER) return true;
			if (
				shape === GPShape_Type.GP_SHAPE_POLYGON &&
				(y2 < 1 || y2 > MAX_POLYGON_SIDES)
			)
				return context.createError({
					message: `polygon needs 1 to ${MAX_POLYGON_SIDES} sides in y2 (got ${y2})`,
				});
			if (
				(shape === GPShape_Type.GP_SHAPE_ARC ||
					shape === GPShape_Type.GP_SHAPE_POLYGON) &&
				(x1 < x2 || y1 < x2)
			)
				return context.createError({
					message: `${shape === GPShape_Type.GP_SHAPE_ARC ? 'arc' : 'polygon'} runs off the left or top of the screen (x1 and y1 must be at least the radius ${x2})`,
				});
			return true;
		});

	return yup
		.array(element)
		.typeError('Layout must be a JSON array of elements')
		.required()
		.min(
			1,
			'No layout elements found, expected entries like {"type": "GP_ELEMENT_PIN_BUTTON", "x1": 47, "y1": 19, "x2": 4, "y2": 4, "stroke": 1, "fill": 1, "value": 27, "shape": "GP_SHAPE_ELLIPSE"}',
		)
		.max(
			MAX_CUSTOM_LAYOUT_ELEMENTS,
			`Too many elements (maximum is ${MAX_CUSTOM_LAYOUT_ELEMENTS})`,
		);
};

export type LayoutElement = yup.InferType<
	ReturnType<typeof layoutSchema>
>[number];

type ParseResult =
	| { elements: LayoutElement[]; error: null }
	| { elements: null; error: string };

const formatError = (error: yup.ValidationError) => {
	const index = error.path?.match(/^\[(\d+)\]/)?.[1];
	return index === undefined
		? error.message
		: `Element ${Number(index) + 1}: ${error.message}`;
};

export function parseLayoutCode(
	code: string,
	maxPin = DEFAULT_MAX_PIN,
): ParseResult {
	if (!code || code.trim() === '') return { elements: [], error: null };

	let json: unknown;
	try {
		json = JSON.parse(code);
	} catch (error) {
		return {
			elements: null,
			error: `Invalid JSON: ${(error as Error).message}`,
		};
	}

	try {
		return {
			elements: layoutSchema(maxPin).validateSync(json),
			error: null,
		};
	} catch (error) {
		if (error instanceof yup.ValidationError)
			return { elements: null, error: formatError(error) };
		throw error;
	}
}

function generateLayoutCode(elements: LayoutElement[]) {
	if (elements.length === 0) return '';
	const layout = elements.map(({ type, value, shape, ...rest }) => {
		const isLever = type === GPElement.GP_ELEMENT_LEVER;
		const fields: Record<string, number | string> = {
			type: ELEMENT_NAMES[type] ?? type,
			x1: rest.x1,
			y1: rest.y1,
			x2: rest.x2,
			y2: rest.y2,
			stroke: rest.stroke,
			fill: rest.fill,
			value: (isLever && LEVER_MODE_NAMES[value]) || value,
		};
		if (!isLever || shape !== 0) fields.shape = SHAPE_NAMES[shape] ?? shape;
		if (rest.angleStart) fields.angleStart = rest.angleStart;
		if (rest.angleEnd) fields.angleEnd = rest.angleEnd;
		if (rest.closed) fields.closed = rest.closed;
		return fields;
	});
	return JSON.stringify(layout, null, 2);
}

export function layoutCodeToBase64(code: string, maxPin?: number) {
	const { elements, error } = parseLayoutCode(code, maxPin);
	return error === null ? elementsToBase64(elements) : null;
}

export function base64ToLayoutCode(base64: string) {
	return generateLayoutCode(base64ToElements(base64));
}

function elementsToBase64(elements: LayoutElement[]) {
	const bytes = new Uint8Array(elements.length * BYTES_PER_ELEMENT);
	elements.forEach((element, elementIndex) => {
		const { type, x1, y1, x2, y2, stroke, fill, value, shape } = element;
		const { angleStart, angleEnd, closed } = element;
		bytes.set(
			[
				type,
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
			elementIndex * BYTES_PER_ELEMENT,
		);
	});
	return btoa(String.fromCharCode(...bytes));
}

function base64ToElements(base64: string): LayoutElement[] {
	if (!base64) return [];
	let binary: string;
	try {
		binary = atob(base64);
	} catch {
		return [];
	}
	const elements: LayoutElement[] = [];
	const count = Math.floor(binary.length / BYTES_PER_ELEMENT);
	for (let elementIndex = 0; elementIndex < count; elementIndex++) {
		const offset = elementIndex * BYTES_PER_ELEMENT;
		const byte = (index: number) => binary.charCodeAt(offset + index);
		const word = (index: number) => byte(index) | (byte(index + 1) << 8);
		const flags = byte(5);
		elements.push({
			type: byte(0),
			x1: byte(1),
			y1: byte(2),
			x2: byte(3),
			y2: byte(4),
			stroke: flags & 0x01,
			fill: (flags >> 1) & 0x03,
			value: word(6),
			shape: (flags >> 3) & 0x07,
			angleStart: word(8),
			angleEnd: word(10),
			closed: (flags >> 6) & 0x01,
		});
	}
	return elements;
}
