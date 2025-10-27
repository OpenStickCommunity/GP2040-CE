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
			'Pour les LEDs RGB, les index doivent être après le dernier bouton LED défini dans la section <1>Ordre des boutons LED RGB</1> et probablement <3>commence à l\'index {{rgbLedStartIndex}}</3>.',
		'pled-type-label': 'Type de LED du joueur',
		'pled-type-off': 'Éteint',
		'pled-type-pwm': 'PWM',
		'pled-type-rgb': 'RGB',
		'pled-color-label': 'Couleur RGB de la LED du joueur',
	},
	case: {
		'header-text': 'LEDs RGB du Boitier',
		'sub-header-text':
			'Pour les LEDs RGB d\'ambiance du boîtier, définissez un numéro d\'index de départ et le nombre de LED RGB du boîtier. L\'index doit être après le dernier bouton LED défini mais peut être avant ou après les LEDs des joueurs et la LED turbo.',
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
		'header-text': 'Ordre des boutons LED RGB',
		'sub-header-text':
			'Ici, vous pouvez définir quels boutons ont des LEDs RGB et dans quel ordre ils fonctionnent à partir de la carte de contrôle. Cela est nécessaire pour certaines animations LED et la prise en charge des thèmes statiques.',
		'sub-header1-text':
			'Faites glisser et déposez les éléments de la liste pour attribuer et réorganiser les LED RGB.',
		'available-header-text': 'Boutons disponibles',
		'assigned-header-text': 'Boutons attribués',
	},
	'turn-off-when-suspended': 'Éteindre lors de la mise en veille',
};
