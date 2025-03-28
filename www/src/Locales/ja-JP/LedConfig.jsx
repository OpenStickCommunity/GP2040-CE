export default {
	rgb: {
		'header-text': 'RGB LED 設定',
		'data-pin-label': 'データ端子 (-1 で無効)',
		'led-format-label': 'LED フォーマット',
		'led-layout-label': 'LED レイアウト',
		'leds-per-button-label': 'ボタン毎のLEDの数',
		'led-brightness-maximum-label': '最大輝度',
		'led-brightness-steps-label': '輝度ステップ',
	},
	player: {
		'header-text': 'プレイヤLED (XInput)',
		'pwm-sub-header-text':
			'PWM LEDの場合は各LED毎に専用のGPIO端子を割り当ててください',
		'rgb-sub-header-text':
			'RGB LED の場合、<1>RGB LEDボタン順序</1>で設定した最後のLEDボタンの次の値にしてください。<3>おそらく {{rgbLedStartIndex}}から開始。</3>',
		'pled-type-label': 'プレイヤLED種別',
		'pled-type-off': 'オフ',
		'pled-type-pwm': 'PWM',
		'pled-type-rgb': 'RGB',
		'pled-color-label': 'RGB PLED カラー',
	},
	case: {
		'header-text': 'ケース RGB LED',
		'sub-header-text':
			'ケースRGB LEDでは開始インデックスとLED数を設定してください。開始インデックスはボタンLEDの以降のインデックスであればプレイヤLEDおよびターボLEDの前後どちらも指定可能です。',
		'case-index-label': 'LED開始インデックス',
		'case-count-label': 'LED数',
		'case-type-label': '色種別',
		'case-type-off': 'オフ',
		'case-type-ambient': 'アンビエント',
		'case-type-linked': '同期',
	},
	'pled-pin-label': 'PLED #{{pin}} GPIO 端子',
	'pled-index-label': 'PLED #{{index}} インデックス',
	'rgb-order': {
		'header-text': 'RGB LED ボタン順序',
		'sub-header-text':
			'ここではどのボタンにRGB LEDが実装されているか及び制御基板からどの順序で配線されているかを設定できます。この設定は一部のLEDアニメーションや静的テーマの機能で利用されます。',
		'sub-header1-text':
			'アイテムをドラッグ＆ドロップで順序変更してRGB LEDの有効化と順序を設定してください。',
		'available-header-text': '利用可能ボタン',
		'assigned-header-text': '割当済みボタン',
	},
	'turn-off-when-suspended': 'サスペンド中はオフにする',
};
