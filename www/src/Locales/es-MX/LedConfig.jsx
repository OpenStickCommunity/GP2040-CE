export default {
	rgb: {
		'header-text': 'Configuración de LED RGB',
		'data-pin-label': 'Pin GPIO de Datos (-1 para desactivado)',
		'led-format-label': 'Formato LED',
		'led-layout-label': 'Diseño LED',
		'leds-per-button-label': 'LEDs Por Botón',
		'led-brightness-maximum-label': 'Brillo Máximo',
		'led-brightness-steps-label': 'Pasos de Brillo',
	},
	player: {
		'header-text': 'LEDs de Jugador',
		'pwm-sub-header-text':
			'Para LEDs PWM, asigne cada LED a un pin GPIO dedicado.',
		'rgb-sub-header-text':
			'Para LEDs RGB, los índices deben estar después del último botón LED definido en la sección <1>Orden de Botones LED RGB</1> y probablemente <3>comienza en el índice {{rgbLedStartIndex}}</3>.',
		'pled-type-label': 'Tipo de LED de Jugador',
		'pled-type-off': 'Apagado',
		'pled-type-pwm': 'PWM',
		'pled-type-rgb': 'RGB',
		'pled-color-label': 'Color RGB PLED',
	},
	case: {
		'header-text': 'LEDs RGB de Carcasa',
		'sub-header-text':
			'Para LEDs RGB de Carcasa, establezca un índice inicial y la cantidad de RGB de carcasa. El índice debe estar después del último botón LED definido pero puede estar antes o después de los LEDs de jugador y LED turbo.',
		'case-index-label': 'Índice LED RGB',
		'case-count-label': 'Cantidad de LEDs RGB',
		'case-type-label': 'Tipo de Color',
		'case-type-off': 'Apagado',
		'case-type-ambient': 'Ambiental',
		'case-type-linked': 'Vinculado',
	},
	'pled-pin-label': 'Pin GPIO PLED #{{pin}}',
	'pled-index-label': 'Índice PLED #{{index}}',
	'rgb-order': {
		'header-text': 'Orden de Botones LED RGB',
		'sub-header-text':
			'Aquí puede definir qué botones tienen LEDs RGB y en qué orden se ejecutan desde la placa de control. Esto es necesario para ciertas animaciones LED y soporte de temas estáticos.',
		'sub-header1-text':
			'Arrastre y suelte elementos de la lista para asignar y reordenar los LEDs RGB.',
		'available-header-text': 'Botones Disponibles',
		'assigned-header-text': 'Botones Asignados',
	},
	'turn-off-when-suspended': 'Apagar Cuando Suspendido',
};