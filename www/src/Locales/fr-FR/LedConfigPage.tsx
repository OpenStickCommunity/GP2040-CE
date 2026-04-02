export default {
	rgb: {
		'header-text': 'Configuration des LED RGB',
		'data-pin-label': 'Broche GPIO des données (-1 pour désactivé)',
		'led-format-label': 'Format LED',
		'led-brightness-maximum-label': 'Luminosité maximale',
		'turn-off-when-suspended': 'Éteindre lors de la mise en veille',
	},
	player: {
		'header-text': 'LED du joueur',
		'pwm-sub-header-text':
			'Pour les LEDs PWM, définissez chaque LED sur une broche GPIO dédiée.',
		'pled-type-label': 'Type de LED du joueur',
		'pled-type-off': 'Éteint',
		'pled-type-pwm': 'PWM',
		'pled-type-rgb': 'RGB',
		'pled-color-label': 'Couleur RGB de la LED du joueur',
		'pled-pin-label': 'Broche GPIO de la LED du joueur #{{pin}}',
	},
};
