export default {
	rgb: {
		'header-text': 'RGB LED 配置',
		'data-pin-label': '数据引脚 (-1 为禁用)',
		'led-format-label': 'LED 颜色格式',
		'led-layout-label': 'LED 布局',
		'leds-per-button-label': '每个按键 LED 数量',
		'led-brightness-maximum-label': '最大亮度',
		'led-brightness-steps-label': '亮度变化梯度',
	},
	player: {
		'header-text': '玩家 LED (XInput)',
		'pwm-sub-header-text': '对于 PWM LED，将每个LED设置为专用GPIO引脚',
		'rgb-sub-header-text':
			'对于 RGB LED，索引必须在<1>RGB LED 按键顺序</1> 部分定义的最后一个LED按键之后，比如 <4>开始于 {{rgbLedStartIndex}}</4>。',
		'pled-type-label': '玩家 LED 类型',
		'pled-type-off': '关闭',
		'pled-type-pwm': 'PWM',
		'pled-type-rgb': 'RGB',
		'pled-color-label': 'RGB PLED 颜色',
	},
	'pled-pin-label': 'PLED #{{pin}} 引脚',
	'pled-index-label': 'PLED #{{index}} 索引',
	'rgb-order': {
		'header-text': 'RGB LED 按键顺序',
		'sub-header-text':
			'在这里，你可以定义哪些按键拥有RGB LED，以及它们从控制电路板运行的顺序。这对于某些LED动画和静态主题的支持是必需的。',
		'sub-header1-text': '拖放列表项可分配RGB LED并对其重新排序。',
		'available-header-text': '可用按键',
		'assigned-header-text': '已分配按键',
	},
};
