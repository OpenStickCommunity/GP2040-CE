export default {
	rgb: {
		'header-text': 'RGB LED Yapılandırması',
		'data-pin-label': 'Veri GPIO Pin (-1 devre dışı)',
		'led-format-label': 'LED Formatı',
		'led-layout-label': 'LED Düzeni',
		'leds-per-button-label': 'Düğme Başına LED Sayısı',
		'led-brightness-maximum-label': 'Max Parlaklık',
		'led-brightness-steps-label': 'Parlaklık Adımları',
	},
	player: {
		'header-text': 'Oyuncu Ledleri',
		'pwm-sub-header-text':
			'Parlaklık Adımları',
		'rgb-sub-header-text':
			'RGB LEDler için, indeksler <1>RGB LED Düğme Sırası</1> bölümünde tanımlanan son LED düğmesinden sonra olmalı ve büyük olasılıkla <3>{{rgbLedStartIndex}} indeksinden başlamalıdır.</3>',
		'pled-type-label': 'Oyuncu LED Tipi',
		'pled-type-off': 'Kapalı',
		'pled-type-pwm': 'PWM',
		'pled-type-rgb': 'RGB',
		'pled-color-label': 'RGB PLED Rengi',
	},
	case: {
		'header-text': 'Kasa RGB Ledleri',
		'sub-header-text':
			'Kasa RGB LEDleri için bir başlangıç ​​indeksi ve kasa RGB sayısı belirleyin. İndeks, tanımlanan son LED düğmesinden sonra olmalıdır, ancak oynatıcı LEDlerinden ve turbo LEDinden önce veya sonra da olabilir.',
		'case-index-label': 'RGB LED Index',
		'case-count-label': 'RGB LED Sayısı',
		'case-type-label': 'Renk Tipi',
		'case-type-off': 'Kapalı',
		'case-type-ambient': 'Ambiyans',
		'case-type-linked': 'Bağlantılı',
	},
	'pled-pin-label': 'PLED #{{pin}} GPIO Pin',
	'pled-index-label': 'PLED #{{index}} Index',
	'rgb-order': {
		'header-text': 'RGB LED Buton Sıralaması',
		'sub-header-text':
			'Burada, hangi düğmelerin RGB LEDlere sahip olacağını ve kontrol panelinden hangi sırayla çalışacaklarını belirleyebilirsiniz. Bu, belirli LED animasyonları ve statik tema desteği için gereklidir.',
		'sub-header1-text':
			'RGB LEDleri atamak ve yeniden sıralamak için liste öğelerini sürükleyip bırakın.',
		'available-header-text': 'Mevcut Düğmeler',
		'assigned-header-text': 'Atanmış Düğmeler',
	},
	'turn-off-when-suspended': 'Askıya Alındığında Kapat',
};
