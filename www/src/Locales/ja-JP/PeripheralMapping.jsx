export default {
	'header-text': '周辺機器設定',
	'sub-header-text':
		'ここでは周辺機器の端子割り当て及び動作に関する設定ができます',
	'pins-label': '端子',
	'pin-in-use': '割当済み',
	'i2c-desc-header': 'I\u00B2C 情報',
	'i2c-description':
		'I\u00B2C は周辺デバイス接続に利用されるSDA ラインとSCLラインによる2線式通信インターフェースで、ディスプレイやGPIO拡張、センサ等に利用されます。RP2040では2つのI\u00B2Cブロックをサポートしています（I2C0とI2C1）。以下の端子が各ブロックで利用可能となるGPIO端子となります。',
	'spi-desc-header': 'SPI 情報',
	'spi-description':
		'SPIは周辺デバイス接続に利用されるRX、TX、データラインとSCKクロックラインで構成される4線式通信インターフェースで、ディスプレイやGPIO拡張、センサ等に利用されます。RP2040では2つのSPIコントローラブロックをサポートしています（SPI0とSPI1）。以下の端子が各ブロックで利用可能となるGPIO端子となります。',
	'usb-label': 'USBホスト',
	'usb-desc-header': 'USBホスト情報',
	'usb-description':
		'USBホスト機能はUSBクライアントの接続に利用され、キーボード、ゲームコントローラ等のデバイスの接続ができます。',
	'peripheral-toggle-unavailable':
		'有効化できません。周辺機器 {{name}} は利用できません。<0>周辺機器設定</0>で有効化してください。',
	'pin-sda-label': 'SDA端子',
	'pin-scl-label': 'SCL端子',
	'option-speed-label': '速度',
	'option-speed-choice-100000-label': '標準',
	'option-speed-choice-400000-label': '高速',
	'option-speed-choice-1000000-label': '高速＋',
	'pin-rx-label': 'RX端子',
	'pin-cs-label': 'CS端子',
	'pin-sck-label': 'SCK端子',
	'pin-tx-label': 'TX端子',
	'pin-dp-label': 'D+端子',
	'pin-enable5v-label': '5V有効化',
	'option-order-label': '端子順序',
};
