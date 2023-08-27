export default {
	'header-text': 'GPIO Pin Mapping',
	'sub-header-text':
		'Use the form below to reconfigure your button-to-GPIO pin mapping.',
	'alert-text':
		"Mapping buttons to GPIO pins that aren't connected or available can leave the device in non-functional state. To clear the the invalid configuration go to the <1>Reset Settings</1> page.",
	'pin-header-label': 'GPIO',
	errors: {
		conflict: 'GPIO {{pin}} is already assigned to {{conflictedMappings}}',
		required: '{{button}} is required',
		invalid: 'GPIO {{pin}} is invalid for this board',
		used: 'GPIO {{pin}} is already assigned to another feature',
	},
};
