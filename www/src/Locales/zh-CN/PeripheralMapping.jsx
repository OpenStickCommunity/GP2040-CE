export default {
	'header-text': '外设映射',
	'sub-header-text': '',
	'pins-label': '引脚',
	'pin-in-use': '使用中',
	'i2c-desc-header': 'I\u00B2C 接口信息',
	'i2c-description':
		'I\u00B2C 是一种双线制串行接口，通过 SDA 和 SCL 线连接各种配件设备（如显示屏、GPIO 扩展器和传感器）。RP2040 芯片支持两个 I\u00B2C 控制器模块：I2C0 和 I2C1。下表列出了每个模块支持的 GPIO 引脚：',
	'spi-desc-header': 'SPI 接口信息',
	'spi-description':
		'SPI 是一种四线制串行接口，通过 RX 和 TX 数据线、SCK 时钟线以及 CS 同步/片选线连接各种配件设备（如显示屏、存储器和传感器）。RP2040 芯片支持两个 SPI 控制器模块：SPI0 和 SPI1。下表列出了每个模块支持的 GPIO 引脚：',
	'usb-desc-header': 'USB Host 信息',
	'usb-description':
		'USB Host 模式允许使用受支持的 USB 客户端设备，例如键盘、手柄或其他外部认证设备。',
	'peripheral-toggle-unavailable':
		'无法启用。外设 "{{name}}" 当前不可用。请在 <0>外设映射</0> 栏目中先配置并启用它。',
	'option-speed-label': '速率',
	'option-speed-choice-100000-label': '标准 (Normal)',
	'option-speed-choice-400000-label': '快速 (Fast)',
	'option-speed-choice-1000000-label': '超快 (Fast Plus)',
	'pin-enable5v-label': '启用 5V 电源',
	'option-order-label': 'GPIO 引脚顺序',
};