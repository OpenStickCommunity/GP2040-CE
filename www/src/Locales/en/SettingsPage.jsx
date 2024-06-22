export default {
	'auth-settings-label': 'Authentication Settings',
	'no-mode-settings-text': 'There are no input mode settings for {{mode}}.',
	'settings-header-text': 'Input Mode Settings',
	'gamepad-settings-header-text': 'Gamepad Settings',
	'input-mode-label': 'Input Mode',
	'current-input-mode-label': 'Current Input Mode',
	'input-mode-extra-label': 'Switch Touchpad and Share',
	'input-mode-options': {
		none: 'No Mode Selected',
		xinput: 'XInput',
		'nintendo-switch': 'Nintendo Switch',
		ps3: 'PS3',
		generic: 'Generic HID',
		keyboard: 'Keyboard',
		ps4: 'PS4',
		ps5: 'PS5',
		neogeo: 'NEOGEO mini',
		mdmini: 'Sega Genesis/MegaDrive Mini',
		pcemini: 'PC Engine/Turbografx 16 Mini',
		egret: 'EGRET II mini',
		astro: 'ASTROCITY Mini',
		psclassic: 'Playstation Classic',
		xboxoriginal: 'Original Xbox',
		xbone: 'Xbox One',
	},
	'input-mode-group': {
		primary: 'Primary Input Modes',
		mini: 'Mini/Classic Console Modes',
	},
	'boot-input-mode-label': 'Boot Input Modes',
	'ps4-mode-options': {
		controller: 'Controller',
		arcadestick: 'Arcade Stick',
	},
	'input-mode-authentication': {
		none: 'None',
		key: 'Uploaded Key Files',
		usb: 'Host USB',
		i2c: 'Attached I2C',
	},
	'd-pad-mode-label': 'D-Pad Mode',
	'd-pad-mode-options': {
		'd-pad': 'D-Pad',
		'left-analog': 'Left Analog',
		'right-analog': 'Right Analog',
	},
	'socd-cleaning-mode-label': 'SOCD Cleaning Mode',
	'socd-cleaning-mode-note':
		'Note: PS4, PS3, Nintendo Switch, and mini series modes do not support setting SOCD Cleaning to Off and will default to Neutral SOCD Cleaning mode.',
	'socd-cleaning-mode-options': {
		'up-priority': 'Up Priority',
		neutral: 'Neutral',
		'last-win': 'Last Win',
		'first-win': 'First Win',
		off: 'Off',
	},
	'profile-number-label': 'Profile Number',
	'debounce-delay-label': 'Debounce Delay in milliseconds',
	'ps4-mode-explanation-text': 'PS4 mode allows GP2040-CE to run as an authenticated PS4 controller.',
	'ps4-mode-warning-text': '<span>⏳ WARNING ⏳:</span> PS4 will timeout after 8 minutes without authentication.',
	'ps4-usb-host-mode-text': '<span>INFO:</span> Please ensure USB Peripheral is enabled and a PS4 compatible USB device is plugged in.',
	'ps5-mode-explanation-text': 'PS5 mode allows GP2040-CE to run as an authenticated PS5 compatible arcade stick.',
	'ps5-mode-warning-text': '<span>⏳ WARNING ⏳:</span> PS5 will timeout after 8 minutes without authentication.',
	'ps5-usb-host-mode-text': '<span>INFO:</span> Please ensure USB Peripheral is enabled and a PS5 compatible USB device is plugged in.',
	'xbone-mode-text': '<span>INFO:</span> Xbox One requires a USB host connection and USB dongle to properly authenticate in Xbox One mode.',
	'hotkey-settings-label': 'Hotkey Settings',
	'hotkey-settings-sub-header':
		'The <strong>Fn</strong> slider provides a mappable Function button in the <link_pinmap>Pin Mapping</link_pinmap> page. By selecting the <strong>Fn</strong> slider option, the Function button must be held along with the selected hotkey settings. <br /> Additionally, select <strong>None</strong> from the dropdown to unassign any button.',
	'hotkey-settings-warning':
		'Function button is not mapped. The Fn slider will be disabled.',
	'hotkey-actions': {
		'no-action': 'No Action',
		'dpad-digital': 'Dpad Digital',
		'dpad-left-analog': 'Dpad Left Analog',
		'dpad-right-analog': 'Dpad Right Analog',
		'home-button': 'Home Button',
		'capture-button': 'Capture Button',
		'socd-up-priority': 'SOCD Up Priority',
		'socd-neutral': 'SOCD Neutral',
		'socd-last-win': 'SOCD Last Win',
		'socd-first-win': 'SOCD First Win',
		'socd-off': 'SOCD Cleaning Off',
		'invert-x': 'Invert X Axis',
		'invert-y': 'Invert Y Axis',
		'toggle-4way-joystick-mode': 'Toggle 4-Way Joystick Mode',
		'toggle-ddi-4way-joystick-mode': 'Toggle DDI 4-Way Joystick Mode',
		'b1-button': 'B1 Button',
		'b2-button': 'B2 Button',
		'b3-button': 'B3 Button',
		'b4-button': 'B4 Button',
		'l1-button': 'L1 Button',
		'r1-button': 'R1 Button',
		'l2-button': 'L2 Button',
		'r2-button': 'R2 Button',
		'l3-button': 'L3 Button',
		'r3-button': 'R3 Button',
		's1-button': 'S1 Button',
		's2-button': 'S2 Button',
		'a1-button': 'A1 Button',
		'a2-button': 'A2 Button',
		'a3-button': 'A3 Button',
		'a4-button': 'A4 Button',
		'touchpad-button': 'Touchpad Button',
		'load-profile-1': 'Load Profile #1',
		'load-profile-2': 'Load Profile #2',
		'load-profile-3': 'Load Profile #3',
		'load-profile-4': 'Load Profile #4',
		'next-profile': 'Next Profile',
		'reboot-default': 'Reboot GP2040-CE',
		'next-profile': 'Next Profile',
	},
	'forced-setup-mode-label': 'Forced Setup Mode',
	'forced-setup-mode-options': {
		off: 'Off',
		'disable-input-mode': 'Disable Input Mode',
		'disable-web-config': 'Disable Web Config',
		'disable-both': 'Disable Both',
	},
	'forced-setup-mode-modal-title': 'Forced Setup Mode Warning',
	'forced-setup-mode-modal-body':
		'If you reboot to Controller mode after saving, you will no longer have access to the web-config. Please type "<strong>{{warningCheckText}}</strong>" below to unlock the Save button if you fully acknowledge this and intend it. Clicking on Dismiss will revert this setting which then is to be saved.',
	'4-way-joystick-mode-label': '4-Way Joystick Mode',
	'lock-hotkeys-label': 'Lock Hotkeys',
	'keyboard-mapping-header-text': 'Keyboard Mapping',
	'keyboard-mapping-sub-header-text':
		'Use the form below to reconfigure your button-to-key mapping.',
};
