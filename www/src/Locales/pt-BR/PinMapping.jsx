export default {
	'header-text': 'Mapeamento de Pinos',
	'sub-header-text':
		'Utilize o formulário abaixo para reconfigurar o mapeamento de botões para pinos.',
	'alert-text':
		'Associar botões a pinos que não estão conectados ou disponíveis pode deixar o dispositivo em um estado não funcional. Para limpar a configuração inválida, vá para a página <2>Redefinir Configurações</2>.',
	'pin-header-label': 'Pino',
	'profile-pins-warning':
		'Tente evitar alterar os botões/direções usados para seus atalhos de perfil de troca, caso contrário, ficará difícil entender qual perfil você está selecionando!',
	'socd-mode-label': 'Modo de Limpeza SOCD',
	'socd-mode-description':
		'Observação: Os modos PS4, PS3 e Nintendo Switch não suportam a configuração SOCD Cleaning como Desativado e padrão para o modo de Limpeza SOCD Neutra.',
	'socd-modes': {
		SOCD_MODE_UP_PRIORITY: 'Prioridade para Cima',
		SOCD_MODE_NEUTRAL: 'Neutra',
		SOCD_MODE_SECOND_INPUT_PRIORITY: 'Última Vitória',
		SOCD_MODE_FIRST_INPUT_PRIORITY: 'Primeira Vitória',
		SOCD_MODE_BYPASS: 'Desativado',
	},
	errors: {
		conflict: 'O Pino {{pin}} já está atribuído a {{conflictedMappings}}',
		required: '{{button}} é obrigatório',
		invalid: '{{pin}} é inválido para esta placa',
		used: '{{pin}} já está atribuído a outra função',
	},
};
