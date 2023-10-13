export default {
	'header-text': 'Display Configuration',
	'sub-header-text':
		'A monochrome display can be used to show controller status and button activity. Ensure your display module has the following attributes:',
	'list-text':
		'<1>Monochrome display with 128x64 resolution</1> <1>Uses I2C with a SSD1306, SH1106, SH1107 or other compatible display IC</1> <1>Supports 3.3v operation</1>',
	table: {
		header:
			'Use these tables to determine which I2C block to select based on the configured SDA and SCL pins:',
		'sda-scl-pins-header': 'SDA/SCL Pins',
		'i2c-block-header': 'I2C Block',
	},
	form: {
		'i2c-block-label': 'I2C Block',
		'sda-pin-label': 'SDA Pin',
		'scl-pin-label': 'SCL Pin',
		'i2c-address-label': 'I2C Address',
		'i2c-speed-label': 'I2C Speed',
		'flip-display-label': 'Flip Display',
		'invert-display-label': 'Invert Display',
		'button-layout-label': 'Button Layout (Left)',
		'button-layout-right-label': 'Button Layout (Right)',
		'button-layout-custom-header': 'Custom Button Layout Params',
		'button-layout-custom-left-label': 'Layout Left',
		'button-layout-custom-right-label': 'Layout Right',
		'button-layout-custom-start-x-label': 'Start X',
		'button-layout-custom-start-y-label': 'Start Y',
		'button-layout-custom-button-radius-label': 'Button Radius',
		'button-layout-custom-button-padding-label': 'Button Padding',
		'splash-mode-label': 'Splash Mode',
		'splash-duration-label': 'Splash Duration (seconds, 0 for Always On)',
		'display-saver-timeout-label': 'Display Saver Timeout (minutes)',
		'inverted-label': 'Inverted',
		'turn-off-when-suspended': 'Turn Off When Suspended'
	},
};
