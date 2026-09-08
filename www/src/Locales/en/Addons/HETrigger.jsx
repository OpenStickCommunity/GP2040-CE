export default {
	'header-text': 'Hall-Effect Trigger',
	'desc-header-text': 'Hall Effect Trigger Supports 4-Channel, 8-Channel, and 16-Channel Multiplexers.',
	'available-pins-text': 'Available ADC pins: {{pins}}',
	'multiplexer-channel-select': 'Channels Per Multiplexer',
	'direct-no-mux': 'Direct (No Mux)',
	'4-channels': '４-Channels',
	'8-channels': '8-Channels',
	'16-channels': '16-Channels',
	'select-pin-0': 'Select Pin 0',
	'select-pin-1': 'Select Pin 1',
	'select-pin-2': 'Select Pin 2',
	'select-pin-3': 'Select Pin 3',
	'adc-pin-0': 'ADC Pin 0',
	'adc-pin-1': 'ADC Pin 1',
	'adc-pin-2': 'ADC Pin 2',
	'adc-pin-3': 'ADC Pin 3',
	'action-assignment-sub-header': 'Hall-Effect Action Assignment',
	'multiplexer-label': 'Multiplexer',
	'channel-label': 'Channel',
	'voltage-table-show-label': 'Show Voltage Table',
	'voltage-table-hide-label': 'Hide Voltage Table',
	'voltage-table-header-text': 'Hall-Effect Voltage Table',
	'voltage-table-idle-text': 'Idle',
	'voltage-table-trigger-text': 'Trigger',
	'voltage-table-pressed-text': 'Pressed',
	'voltage-table-polarity-text': 'Polarity',
	'voltage-table-rapid-trigger-text': 'Rapid Trigger',
	'voltage-table-span-text': 'Travel',
	'voltage-table-actuation-text': 'Actuation',
	'voltage-table-rt-press-text': 'RT Press',
	'voltage-table-rt-release-text': 'RT Release',
	'voltage-table-release-text': 'Rapid Trigger Threshold',
	'voltage-table-noise-text': 'Rapid Trigger Noise Filter',
	'voltage-table-disabled-label': '(Disabled)',
	'overwrite-all-warning': 'Overwrite All Triggers',
	'overwrite-confirm': 'Confirm Overwrite All Triggers',
	'next-calibration-text': 'Next Calibration',
	'finish-calibration-text': 'Finish Calibration',
	'calibrate-idle-button': 'Calibrate Idle',
	'calibrate-pressed-button': 'Calibrate Pressed',
	'save-button': 'Save Trigger Values',
	'manual-text': 'Manual Adjustments',
	'restart-text': 'Restart',
	'pressed-text': 'Pressed!',
	'idle-input-text': 'Idle Voltage',
	'activation-input-text': 'Activation Voltage',
	'rapid-trigger-threshold-input-text': 'Rapid Trigger Threshold',
	'rapid-trigger-noise-input-text': 'Rapid Trigger Noise Filter',
	'pressed-input-text': 'Pressed Voltage',
	'activation-reading-text': 'Activation Point Reading:',
	'calibrate-all-button': 'Calibrate All 🧲',
	'calibration-header-text': 'Hall-Effect Calibration',
	'calibration-idle-text': 'Idle Voltage Reading:',
	'calibration-pressed-text': 'Pressed Voltage Reading:',
	'calibration-trigger-text': 'Trigger Voltage',
	'calibration-flip-polarity': 'Flip Polarity',
	'calibration-flip-rapid-trigger': 'Enable Rapid Trigger',
	'calibration-back-button': 'Back',
	'calibration-first-step': 'We need to calibrate the idle voltage and full press voltage of the hall-effect switch. ' +
								'After calibration, we can adjust the trigger-activation point to our desired depth. ' +
								'First, let\'s calibrate the idle voltage. Leave the hall-effect button untouched and click the "Calibrate Idle" button.',
	'calibration-second-step': 'Next, press the button fully to reach our maximum depth. Activation position can be adjusted after calibration.',
	'calibration-third-step': 'Finally, let\'s adjust our current activation point and set the desired trigger point. '+
								'Once adjusted, press the button and verify it activates at the desired position.',
	'calibration-manual-step': 'Please adjust the following attributes of the hall effect button to the desired amounts. '+
							    'Once the desired values have been found, you can copy these values and set all triggers '+
							    'on the device.',

	// Guided calibration wizard
	'wizard-button': 'Calibrate All Buttons 🧲',
	'wizard-title': 'Hall-Effect Calibration',
	'wizard-intro':
		'This will calibrate every assigned button at once. You will be asked to leave the ' +
		'controller alone for a couple of seconds, and then to press each button all the way ' +
		'down once, in any order.',
	'wizard-assigned-count': '{{count}} assigned button(s) will be calibrated.',
	'wizard-baseline-heading': 'Step 1 of 3 — Measuring resting position',
	'wizard-baseline-body': 'Hands off the controller, please. Do not press anything.',
	'wizard-press-heading': 'Step 2 of 3 — Press every button',
	'wizard-press-body':
		'Press each button all the way down, one at a time, in any order. Each tile turns green ' +
		'once that button has been captured.',
	'wizard-press-progress': '{{moved}} of {{total}} buttons captured',
	'wizard-captured': 'Captured ({{span}})',
	'wizard-waiting': 'Waiting…',
	'wizard-unstable-warning':
		'These channels have unusually noisy readings, which usually means a wiring problem: {{channels}}',
	'wizard-feel-heading': 'Step 3 of 3 — Choose how the buttons should feel',
	'wizard-feel-body':
		'This sets how far you have to press before a button registers. You can fine-tune ' +
		'individual buttons afterwards in the voltage table.',
	'wizard-start': 'Start Calibration',
	'wizard-finish-press': 'Done Pressing',
	'wizard-apply': 'Apply and Save',
	'wizard-cancel': 'Cancel',

	'preset-actuation-label': 'Actuates at {{percent}}% of travel',
	'preset-level': 'Level {{level}}',
	'preset-level-desc': 'Rapid trigger reacts to {{press}}% of travel',

	// Live test view
	'monitor-header': 'Test Your Switches',
	'monitor-intro':
		'Press buttons to see how deep each switch is pressed and whether it is currently ' +
		'sending its action. Gamepad output is paused while this is running.',
	'monitor-start': 'Start Testing',
	'monitor-stop': 'Stop Testing',
	'monitor-legend-active': 'Sending input',
	'monitor-legend-idle': 'Pressed, not actuated',
	'monitor-legend-rt': 'Rapid trigger enabled',
	'monitor-none-assigned': 'No buttons are assigned yet, so there is nothing to test.',

	// Binding profiles
	'profiles-header': 'Hall-Effect Binding Profiles',
	'profiles-shared-tuning-note':
		'Profiles change which action each button sends, and can optionally override its ' +
		'sensitivity and rapid trigger. Anything left on Inherit uses the base profile value. ' +
		'Calibration itself is a property of the switch and is always shared. A button left ' +
		'unassigned in a profile sends nothing while that profile is active.',
	'profile-base-label': 'Base',
	'profile-label': 'Profile {{number}}',
	'profile-enabled-label': 'Enabled (included when cycling profiles)',
	'profile-base-note':
		'The base profile is always enabled. Values set here apply to every profile unless a ' +
		'profile overrides them. Calibration is always shared.',
	'profile-unbound-note':
		'Leaving a button unassigned in this profile means it sends nothing while the profile ' +
		'is active.',
	'profile-disabled-warning':
		'This profile is disabled, so the cycle button will skip over it. Enable it to include ' +
		'it when cycling.',
	'mux-group-label': 'Multiplexer {{index}} (ADC {{pin}})',
	'direct-group-label': 'Direct (ADC {{pin}})',
	'option-group-buttons': 'Buttons',
	'option-group-analog': 'Analog Stick (full tilt)',
	'option-group-analog-proportional': 'Analog Stick (proportional to press depth)',
	'analog-proportional-option': '{{action}} — proportional',
	'option-group-profiles': 'HE Profile Switching',
	'sensitivity-title': 'Sensitivity level (1 = lightest, 10 = deepest)',
	'sensitivity-option': 'L{{level}} · {{percent}}%',
	'sensitivity-custom': 'Custom',
	'tuning-inherit': 'Inherit',
	'rt-inherit': 'RT: inherit',
	'rt-off': 'RT: off',
	'rt-on': 'RT: on',
	'rapid-trigger-switch-label': 'RT',
	'rapid-trigger-needs-calibration':
		'Calibrate this switch before enabling rapid trigger.',
	'calibrate-single-title': 'Calibrate this switch',
	'profile-copy-placeholder': 'Copy bindings from…',
	'profile-copy-button': 'Copy',
	'profile-copied': 'Copied {{from}} onto {{to}}. Save to apply.',
	'profile-code-placeholder': 'Paste a profile code here',
	'profile-code-copy': 'Get Code',
	'profile-code-apply': 'Apply Code',
	'profile-code-copied': 'Profile code copied to the clipboard.',
	'profile-code-ready': 'Profile code ready to copy.',
	'profile-code-applied': 'Profile code applied. Save to keep it.',
	'profiles-save-button': 'Save Binding Profiles',
	'action-he_profile_cycle': 'Cycle HE Profile',
	'action-he_profile_1': 'Select HE Profile 1',
	'action-he_profile_2': 'Select HE Profile 2',
	'action-he_profile_3': 'Select HE Profile 3',
	'action-he_profile_4': 'Select HE Profile 4',
};
