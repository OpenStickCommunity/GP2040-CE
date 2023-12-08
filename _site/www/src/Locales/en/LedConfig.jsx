export default {
	rgb: {
		'header-text': 'RGB LED Configuration',
		'data-pin-label': 'Data Pin (-1 for disabled)',
		'led-format-label': 'LED Format',
		'led-layout-label': 'LED Layout',
		'leds-per-button-label': 'LEDs Per Button',
		'led-brightness-maximum-label': 'Max Brightness',
		'led-brightness-steps-label': 'Brightness Steps',
	},
	player: {
		'header-text': 'Player LEDs (XInput)',
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
	'pled-pin-label': 'PLED #{{pin}} Pin',
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
	'turn-off-when-suspended': 'Turn Off When Suspended'
};
