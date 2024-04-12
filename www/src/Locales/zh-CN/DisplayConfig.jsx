export default {
	'header-text': '显示屏配置',
	'sub-header-text':
		'单色显示屏可用于显示控制器状态和按键活动。确保您的显示模块具有以下属性：',
	'list-text':
		'<1>分辨率为 128x64 的单色显示屏</1> <1>使用 I2C 协议驱动的SSD1306, SH1106, SH1107 或其他可兼容的显示屏 IC </1> <1>支持 3.3v 电压运行</1>',
    section: {
        'hardware-header': '硬件选项',
        'screen-header': '屏幕选项',
        'layout-header': '布局选项',
    },
	table: {
		header:
			'根据配置的 SDA 和 SCL 引脚，使用这些表格确定要选择的 I2C 区块：',
		'sda-scl-pins-header': 'SDA/SCL 引脚',
		'i2c-block-header': 'I2C 区块',
	},
	form: {
		'i2c-block-label': 'I2C 区块',
		'sda-pin-label': 'SDA 引脚',
		'scl-pin-label': 'SCL 引脚',
		'i2c-address-label': 'I2C 地址',
		'i2c-speed-label': 'I2C 速度',
		'flip-display-label': '翻转显示',
		'invert-display-label': '反相显示',
		'button-layout-label': '按键布局 (左)',
		'button-layout-right-label': '按键布局 (右)',
		'button-layout-custom-header': '自定义按键布局参数',
		'button-layout-custom-left-label': '左侧布局',
		'button-layout-custom-right-label': '右侧布局',
		'button-layout-custom-start-x-label': '起始点 X',
		'button-layout-custom-start-y-label': '起始点 Y',
		'button-layout-custom-button-radius-label': '按键半径',
		'button-layout-custom-button-padding-label': '按键填充',
		'splash-mode-label': '启动画面',
		'splash-duration-label': '启动画面延迟(秒，0表示永远开启)',
		'display-saver-timeout-label': '屏保超时(分钟)',
		'inverted-label': '反相',
		'power-management-header': '电源管理',
		'turn-off-when-suspended': '暂停时关闭'
	},
};
