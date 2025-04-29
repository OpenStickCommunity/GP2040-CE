export default {
	rgb: {
		'header-text': 'RGB LED Configuration',
		'data-pin-label': 'Data GPIO Pin (-1 for disabled)',
		'led-format-label': 'LED Format',
		'led-layout-label': 'LED Layout',
		'leds-per-button-label': 'LEDs Per Button',
		'led-brightness-maximum-label': 'Max Brightness',
		'led-brightness-steps-label': 'Brightness Steps',
	},
	player: {
		'header-text': 'Player LEDs',
		'pwm-sub-header-text':
			'For PWM LEDs, set each LED to a dedicated GPIO pin.',
		'rgb-sub-header-text':
			'For RGB LEDs, the indexes must be after the last LED button defined in <1>RGB LED Button Order</1> section and likely <3>starts at index {{rgbLedStartIndex}}</3>.',
		'pled-type-label': 'Player LED Type',
		'pled-type-off': 'Off',
		'pled-type-pwm': 'PWM',
		'pled-type-rgb': 'RGB',
		'pled-color-label': 'RGB PLED Color',
	},
	case: {
		'header-text': 'Case RGB LEDs',
		'sub-header-text':
			'For Case RGB LEDs, set a starting index and the case RGB count. The index must be after the last LED button defined but can be before or after the player LEDs and turbo LED.',
		'case-index-label': 'RGB LED Index',
		'case-count-label': 'RGB LED Count',
		'case-type-label': 'Color Type',
		'case-type-off': 'Off',
		'case-type-ambient': 'Ambient',
		'case-type-linked': 'Linked',
	},
	'pled-pin-label': 'PLED #{{pin}} GPIO Pin',
	'pled-index-label': 'PLED #{{index}} Index',
	'rgb-order': {
		'header-text': 'RGB LED Button Order',
		'sub-header-text':
			'Here you can define which buttons have RGB LEDs and in what order they run from the control board. This is required for certain LED animations and static theme support.',
		'sub-header1-text':
			'Drag and drop list items to assign and reorder the RGB LEDs.',
		'available-header-text': 'Available Buttons',
		'assigned-header-text': 'Assigned Buttons',
	},
	'turn-off-when-suspended': 'Turn Off When Suspended',
};
