export default {
	'header-text': 'Configuração do Display',
	'sub-header-text':
		'Um display monocromático pode ser usado para mostrar o status do controle e a atividade dos botões. Verifique se o módulo de display possui as seguintes características:',
	'list-text':
		'<1>Display monocromático com resolução de 128x64</1> <1>Usa I2C com um CI de display compatível com SSD1306, SH1106, SH1107 ou outro compatível</1> <1>Suporta operação de 3,3V</1>',
	table: {
		header:
			'Use essas tabelas para determinar qual bloco I2C selecionar com base nos pinos SDA e SCL configurados:',
		'sda-scl-pins-header': 'Pinos SDA/SCL',
		'i2c-block-header': 'Bloco I2C',
	},
	form: {
		'i2c-block-label': 'Bloco I2C',
		'sda-pin-label': 'Pino SDA',
		'scl-pin-label': 'Pino SCL',
		'i2c-address-label': 'Endereço I2C',
		'i2c-speed-label': 'Velocidade I2C',
		'flip-display-label': 'Virar Display',
		'invert-display-label': 'Inverter Display',
		'button-layout-label': 'Layout dos Botões (Esquerda)',
		'button-layout-right-label': 'Layout dos Botões (Direita)',
		'button-layout-custom-header':
			'Parâmetros de Layout Personalizado dos Botões',
		'button-layout-custom-left-label': 'Layout Esquerdo',
		'button-layout-custom-right-label': 'Layout Direito',
		'button-layout-custom-start-x-label': 'Início X',
		'button-layout-custom-start-y-label': 'Início Y',
		'button-layout-custom-button-radius-label': 'Raio dos Botões',
		'button-layout-custom-button-padding-label': 'Preenchimento dos Botões',
		'splash-mode-label': 'Modo de Splash',
		'splash-duration-label':
			'Duração do Splash (segundos, 0 para sempre ligado)',
		'display-saver-timeout-label': 'Tempo Limite do Protetor de Tela (minutos)',
		'inverted-label': 'Invertido',
	},
};
