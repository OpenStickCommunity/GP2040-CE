export default {
	rgb: {
		'header-text': 'Configuração do LED RGB',
		'data-pin-label': 'Pino de Dados (-1 para desabilitado)',
		'led-format-label': 'Formato do LED',
		'led-layout-label': 'Layout do LED',
		'leds-per-button-label': 'LEDs por Botão',
		'led-brightness-maximum-label': 'Brilho Máximo',
		'led-brightness-steps-label': 'Passos de Brilho',
	},
	player: {
		'header-text': 'LEDs do Jogador (XInput)',
		'pwm-sub-header-text':
			'Para LEDs PWM, configure cada LED para um pino GPIO dedicado.',
		'rgb-sub-header-text':
			'Para LEDs RGB, os índices devem ser após o último botão LED definido na seção <1>Ordem dos Botões de LED RGB</1> e provavelmente <3>começam no índice {{rgbLedStartIndex}}</3>.',
		'pled-type-label': 'Tipo de LED do Jogador',
		'pled-type-off': 'Desligado',
		'pled-type-pwm': 'PWM',
		'pled-type-rgb': 'RGB',
		'pled-color-label': 'Cor do LED RGB do Jogador',
	},
	'pled-pin-label': 'PLED Pino #{{pin}}',
	'pled-index-label': 'Índice do PLED #{{index}}',
	'rgb-order': {
		'header-text': 'Ordem dos Botões de LED RGB',
		'sub-header-text':
			'Aqui você pode definir quais botões possuem LEDs RGB e em que ordem eles são controlados pela placa. Isso é necessário para determinadas animações de LED e suporte a temas estáticos.',
		'sub-header1-text':
			'Arraste e solte os itens da lista para atribuir e reordenar os LEDs RGB.',
		'available-header-text': 'Botões Disponíveis',
		'assigned-header-text': 'Botões Atribuídos',
	},
};
