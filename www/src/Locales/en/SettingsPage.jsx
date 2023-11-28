export default {
	'settings-header-text': 'Settings',
	'input-mode-label': 'Input Mode',
	'input-mode-extra-label': 'Switch Touchpad and Share',
	'input-mode-options': {
        none: 'No Mode Selected',
		xinput: 'XInput',
		'nintendo-switch': 'Nintendo Switch',
		ps3: 'PS3/DirectInput',
		keyboard: 'Keyboard',
		ps4: 'PS4',
		neogeo: 'NEOGEO mini',
		mdmini: 'Sega Genesis/MegaDrive Mini',
	},
    'boot-input-mode-label': 'Boot Input Modes',
	'ps4-mode-options': {
		controller: 'Controller',
		arcadestick: 'Arcade Stick',
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
	'ps4-compatibility-label':
		'For <strong>PS5 compatibility</strong>, use "Arcade Stick" and enable PS Passthrough add-on<br/>For <strong>PS4 support</strong>, use "Controller" and enable PS4 Mode add-on if you have the necessary files',
	'hotkey-settings-label': 'Hotkey Settings',
	'hotkey-settings-sub-header':
		"The <1>Fn</1> slider provides a mappable Function button in the <3 exact='true' to='/pin-mapping'>Pin Mapping</3> page. By selecting the <1>Fn</1> slider option, the Function button must be held along with the selected hotkey settings.<5 />Additionally, select <1>None</1> from the dropdown to unassign any button.",
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
		'touchpad-button': 'Touchpad Button',
		'load-profile-1': 'Load Profile #1',
		'load-profile-2': 'Load Profile #2',
		'load-profile-3': 'Load Profile #3',
		'load-profile-4': 'Load Profile #4',
		'reboot-default': 'Reboot GP2040-CE',
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
};
