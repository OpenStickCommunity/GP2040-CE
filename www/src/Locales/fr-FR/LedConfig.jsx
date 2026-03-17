export default {
	rgb: {
		'header-text': 'Configuration des LED RGB',
		'data-pin-label': 'Broche GPIO des données (-1 pour désactivé)',
		'led-format-label': 'Format LED',
		'led-layout-label': 'Disposition des LEDs',
		'leds-per-button-label': 'LED par bouton',
		'led-brightness-maximum-label': 'Luminosité maximale',
		'led-brightness-steps-label': 'Étapes de luminosité',
	},
	player: {
		'header-text': 'LED du joueur',
		'pwm-sub-header-text':
			'Pour les LEDs PWM, définissez chaque LED sur une broche GPIO dédiée.',
		'rgb-sub-header-text':
			'Pour les LEDs RGB, les index doivent être placés après la dernière LED de bouton définie dans la section <1>Ordre des LED RGB des boutons</1> et probablement <3>commence à l\'index {{rgbLedStartIndex}}</3>.',
		'pled-type-label': 'Type de LED du joueur',
		'pled-type-off': 'Éteint',
		'pled-type-pwm': 'PWM',
		'pled-type-rgb': 'RGB',
		'pled-color-label': 'Couleur RGB de la LED du joueur',
	},
	case: {
		'header-text': 'LEDs RGB du Boitier',
		'sub-header-text':
			'Pour les LEDs RGB du boîtier, définissez un index de départ ainsi que le nombre de LED du boîtier. L\'index doit être placé après la dernière LED de bouton définie, mais peut se situer avant ou après les LEDs des joueurs et la LED turbo.',
		'case-index-label': 'Index des LED RGB',
        'case-count-label': 'Nombre de LED RGB',
        'case-type-label': 'Type de couleur',
        'case-type-off': 'Éteint',
		'case-type-ambient': 'Ambiant',
		'case-type-linked': 'Synchronisé',
    },
	'pled-pin-label': 'Broche GPIO de la LED du joueur #{{pin}}',
	'pled-index-label': 'Index de la LED du joueur #{{index}}',
	'rgb-order': {
		'header-text': 'Ordre des LED boutons RGB',
		'sub-header-text':
			'Ici, vous pouvez définir quels boutons possèdent des LEDs RGB et dans quel ordre elles sont disposées depuis la carte contrôle. Cela est nécessaire pour certaines animations LED et la prise en charge des thèmes statiques.',
		'sub-header1-text':
			'Faites glisser et déposez les éléments de la liste pour attribuer et réorganiser les LED RGB.',
		'available-header-text': 'Boutons disponibles',
		'assigned-header-text': 'Boutons assignés',
	},
	'turn-off-when-suspended': 'Éteindre lors de la mise en veille',
};
