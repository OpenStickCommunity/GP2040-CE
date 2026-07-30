export default {
	rgb: {
		'header-text': 'RGB LED 配置',
		'data-pin-label': '数据 GPIO 引脚 (设为 -1 则禁用)',
		'led-format-label': 'LED 格式',
		'led-layout-label': 'LED 布局',
		'leds-per-button-label': '每个按键的 LED 数量',
		'led-brightness-maximum-label': '最大亮度',
		'led-brightness-steps-label': '亮度调节级数',
	},
	player: {
		'header-text': '玩家指示灯',
		'pwm-sub-header-text':
			'对于 PWM LEDs，请为每个 LED 分配一个专门的 GPIO 引脚。',
		'rgb-sub-header-text':
			'对于 RGB LEDs，其索引必须位于 <1>RGB LED 按键顺序</1> 部分定义的最后一个按键灯珠之后，通常 <3>从索引 {{rgbLedStartIndex}} 开始</3>。',
		'pled-type-label': '玩家指示灯类型',
		'pled-type-off': '关闭',
		'pled-color-label': 'RGB PLED 颜色',
	},
	case: {
		'header-text': '外壳 RGB LEDs',
		'sub-header-text':
			'对于外壳 RGB LEDs，请设置起始索引和灯珠数量。该索引必须位于最后一个按键灯珠之后，但可以在玩家指示灯或连发指示灯之前或之后。',
		'case-index-label': 'RGB LED 索引',
		'case-count-label': 'RGB LED 数量',
		'case-type-label': '灯效模式',
		'case-type-off': '关闭',
		'case-type-ambient': '氛围模式',
		'case-type-linked': '联动模式',
	},
	'pled-pin-label': 'PLED #{{pin}} GPIO 引脚',
	'pled-index-label': 'PLED #{{index}} 索引',
	'rgb-order': {
		'header-text': 'RGB LED 按键顺序',
		'sub-header-text':
			'在此处定义哪些按键配有 RGB LED，以及它们在控制板上的串联顺序。这对于某些 LED 动画效果和自定义静态主题的支持是必需的。',
		'sub-header1-text':
			'通过拖放列表项来分配并重新排列 RGB LED 的顺序。',
		'available-header-text': '待分配按键',
		'assigned-header-text': '已分配按键',
	},
	'turn-off-when-suspended': '设备挂起时关闭灯效',
};
