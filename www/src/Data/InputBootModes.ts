import { InputMode, InputModeDeviceType } from '@proto/enums';

export type InputModeGroup = 'primary' | 'mini';

export type InputModeOptions = {
	labelKey: string;
	value: InputMode;
	group: InputModeGroup;
	required: string[];
	optional: string[];
	authentication: string[];
	deviceTypes: InputModeDeviceType[];
};

export const INPUT_MODE_OPTIONS: InputModeOptions[] = [
	{
		labelKey: 'input-mode-options.xinput',
		value: InputMode.INPUT_MODE_XINPUT,
		group: 'primary',
		optional: ['usb'],
		authentication: ['none', 'usb'],
		deviceTypes: [
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_GAMEPAD,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_WHEEL,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_GUITAR,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_DRUM,
		],
		required: [],
	},
	{
		labelKey: 'input-mode-options.nintendo-switch',
		value: InputMode.INPUT_MODE_SWITCH,
		group: 'primary',
		required: [],
		optional: [],
		authentication: [],
		deviceTypes: [],
	},
	{
		labelKey: 'input-mode-options.ps3',
		value: InputMode.INPUT_MODE_PS3,
		group: 'primary',
		deviceTypes: [
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_GAMEPAD,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_GAMEPAD_ALT,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_WHEEL,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_GUITAR,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_DRUM,
		],
		required: [],
		optional: [],
		authentication: [],
	},
	{
		labelKey: 'input-mode-options.keyboard',
		value: InputMode.INPUT_MODE_KEYBOARD,
		group: 'primary',
		required: [],
		optional: [],
		authentication: [],
		deviceTypes: [],
	},
	{
		labelKey: 'input-mode-options.ps4',
		value: InputMode.INPUT_MODE_PS4,
		group: 'primary',
		optional: ['usb'],
		authentication: ['none', 'key', 'usb'],
		deviceTypes: [
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_GAMEPAD,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_WHEEL,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_HOTAS,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_GUITAR,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_DRUM,
		],
		required: [],
	},
	{
		labelKey: 'input-mode-options.xbone',
		value: InputMode.INPUT_MODE_XBONE,
		group: 'primary',
		required: ['usb'],
		optional: [],
		authentication: [],
		deviceTypes: [],
	},
	{
		labelKey: 'input-mode-options.mdmini',
		value: InputMode.INPUT_MODE_MDMINI,
		group: 'mini',
		required: [],
		optional: [],
		authentication: [],
		deviceTypes: [],
	},
	{
		labelKey: 'input-mode-options.neogeo',
		value: InputMode.INPUT_MODE_NEOGEO,
		group: 'mini',
		required: [],
		optional: [],
		authentication: [],
		deviceTypes: [],
	},
	{
		labelKey: 'input-mode-options.pcemini',
		value: InputMode.INPUT_MODE_PCEMINI,
		group: 'mini',
		required: [],
		optional: [],
		authentication: [],
		deviceTypes: [],
	},
	{
		labelKey: 'input-mode-options.egret',
		value: InputMode.INPUT_MODE_EGRET,
		group: 'mini',
		required: [],
		optional: [],
		authentication: [],
		deviceTypes: [],
	},
	{
		labelKey: 'input-mode-options.astro',
		value: InputMode.INPUT_MODE_ASTRO,
		group: 'mini',
		required: [],
		optional: [],
		authentication: [],
		deviceTypes: [],
	},
	{
		labelKey: 'input-mode-options.psclassic',
		value: InputMode.INPUT_MODE_PSCLASSIC,
		group: 'mini',
		required: [],
		optional: [],
		authentication: [],
		deviceTypes: [],
	},
	{
		labelKey: 'input-mode-options.xboxoriginal',
		value: InputMode.INPUT_MODE_XBOXORIGINAL,
		group: 'primary',
		required: [],
		optional: [],
		authentication: [],
		deviceTypes: [],
	},
	{
		labelKey: 'input-mode-options.ps5',
		value: InputMode.INPUT_MODE_PS5,
		group: 'primary',
		optional: ['usb'],
		authentication: ['none', 'usb'],
		deviceTypes: [
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_GAMEPAD,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_ARCADE_STICK,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_WHEEL,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_HOTAS,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_GUITAR,
			InputModeDeviceType.INPUT_MODE_DEVICE_TYPE_DRUM,
		],
		required: [],
	},
	{
		labelKey: 'input-mode-options.generic',
		value: InputMode.INPUT_MODE_GENERIC,
		group: 'primary',
		required: [],
		optional: [],
		authentication: [],
		deviceTypes: [],
	},
	{
		labelKey: 'input-mode-options.nintendo-switch-pro',
		value: InputMode.INPUT_MODE_SWITCH_PRO,
		group: 'primary',
		required: [],
		optional: [],
		authentication: [],
		deviceTypes: [],
	},
	{
		labelKey: 'input-mode-options.p5general',
		value: InputMode.INPUT_MODE_P5GENERAL,
		group: 'primary',
		required: [],
		optional: ['usb'],
		authentication: ['usb'],
		deviceTypes: []
	},
	{
		labelKey: 'input-mode-options.mayflashs5',
		value: InputMode.INPUT_MODE_MAYFLASHS5,
		group: 'primary',
		required: [],
		optional: ['usb'],
		authentication: ['usb'],
		deviceTypes: []
	},
];
