export default {
	'header-text': 'ディスプレイ設定',
	'sub-header-text':
		'モノクロのディスプレイを利用し、コントローラの状態やボタン操作状態を表示できます。利用しているディスプレイは以下の要件を満たすことを確認してください：',
	'list-text':
		'<1>解像度128x64のモノクロディスプレイ</1> <1>SSD1306, SH1106, SH1107 のいずれか、または互換のI2C ディスプレイICを採用</1> <1>3.3v での動作に対応</1>',
	section: {
		'hardware-header': 'ハードウェア設定',
		'screen-header': 'スクリーン設定',
		'layout-header': 'レイアウト設定',
		'mode-header': 'モード設定',
		'button-layout-header': 'ボタンレイアウト',
		'status-layout-header': 'ステータスバーレイアウト',
		'history-layout-header': '入力履歴レイアウト',
	},
	table: {
		header:
			'以下の表を参照してSDAとSCL端子割当から利用I2Cブロックを確認してください',
		'sda-scl-pins-header': 'SDA/SCL 端子',
		'i2c-block-header': 'I2C ブロック',
	},
	form: {
		'i2c-block-label': 'I2Cブロック',
		'sda-pin-label': 'SDA端子',
		'scl-pin-label': 'SCL端子',
		'i2c-address-label': 'I2C アドレス',
		'i2c-speed-label': 'I2C 速度',
		'flip-display-label': '表示反転',
		'invert-display-label': '白黒反転',
		'button-layout-label': 'ボタンレイアウト（左）',
		'button-layout-right-label': 'ボタンレイアウト（右）',
		'button-layout-orientation': 'ボタンレイアウト',
		'button-layout-custom-header': 'カスタムボタンレイアウトパラメータ',
		'button-layout-custom-left-label': '左側レイアウト',
		'button-layout-custom-right-label': '右側レイアウト',
		'button-layout-custom-start-x-label': 'X座標開始位置',
		'button-layout-custom-start-y-label': 'Y座標開始位置',
		'button-layout-custom-button-radius-label': 'ボタン半径',
		'button-layout-custom-button-padding-label': 'ボタンパディング',
		'splash-mode-label': 'スプラッシュモード',
		'splash-duration-label': 'スプラッシュ表示時間 (秒, 0=常時)',
		'display-saver-timeout-label': 'スクリーンセーバータイムアウト時間 (分)',
		'screen-saver-mode-label': 'スクリーンセーバーモード',
		'inverted-label': '白黒反転',
		'power-management-header': '電源管理',
		'turn-off-when-suspended': 'サスペンド中OFFにする',
		'input-history-label': '入力履歴',
		'display-state': {
			disabled: '無効',
			enabled: '有効',
		},
		'flip-display': {
			none: '無し',
			flip: '180度回転',
			mirror: '左右反転',
			'flip-mirror': '回転して左右反転',
		},
		'splash-modes': {
			enabled: '有効 (カスタムスプラッシュ)',
			'close-in': 'ロゴズームイン',
			'close-in-custom': 'ロゴズームインカスタム',
			disabled: '無効',
		},
		'saver-modes': {
			'display-off': 'ディスプレイオフ',
			snow: '雪',
			bounce: 'ロゴ',
			pipes: 'パイプ',
			toast: 'トースター',
		},
		'layout-modes': {
			standard: 'デフォルト',
			southpaw: '左利き',
			switched: '左右入れ替え',
		},
		'status-header': {
			'input-mode': '入力モード',
			'turbo-mode': 'ターボ',
			'dpad-mode': '十字キーモード',
			'socd-mode': 'SOCDモード',
			'macro-mode': 'マクロ',
			'profile-mode': 'プロファイル',
		},
	},
};
