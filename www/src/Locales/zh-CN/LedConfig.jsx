export default {
	rgb: {
		'header-text': 'RGB LED配置',
		'data-pin-label': '数据引脚(-1为禁用)',
		'led-format-label': 'LED格式',
		'led-layout-label': 'LED布局',
		'leds-per-button-label': 'LEDs Per Button',
		'led-brightness-maximum-label': '最大亮度',
		'led-brightness-steps-label': '亮度步进',
	},
	player: {
		'header-text': 'Player LEDs (XInput)',
		'pwm-sub-header-text':
			'For PWM LEDs, set each LED to a dedicated GPIO pin.',
		'rgb-sub-header-text':
			'For RGB LEDs, the indexes must be after the last LED button defined in <1>RGB LED Button Order</1> section and likely <3>starts at index {{rgbLedStartIndex}}</3>.',
		'pled-type-label': 'Player LED类型',
		'pled-type-off': '关',
		'pled-type-pwm': 'PWM',
		'pled-type-rgb': 'RGB',
		'pled-color-label': 'RGB PLED颜色',
	},
	'pled-pin-label': 'PLED #{{pin}} Pin',
	'pled-index-label': 'PLED #{{index}} Index',
	'rgb-order': {
		'header-text': 'RGB LED Button Order',
		'sub-header-text':
			'Here you can define which buttons have RGB LEDs and in what order they run from the control board. This is required for certain LED animations and static theme support.',
		'sub-header1-text':
			'Drag and drop list items to assign and reorder the RGB LEDs.',
		'available-header-text': '可用按钮',
		'assigned-header-text': '已用按钮',
	},
};
