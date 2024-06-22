export default {
	'header-text': '引脚映射',
	'sub-header-text': '在这里，您可以配置哪个引脚对应哪个动作。如果您不确定哪个按钮连接到哪个引脚，请尝试使用引脚查看器。',
	'alert-text':
		'将按键映射到未连接或不可用的引脚可能会导致设备无法正常工作。要清除无效配置，请转到 <2>恢复默认设置</2> 页面。',
    'pin-viewer': '引脚查看器',
	'pin-pressed': '按下的引脚：{{pressedPin}}',
	'pin-header-label': '引脚',
	'profile-pins-warning':
		'尽量避免修改已设置为切换档案快捷键的按键或方向键，否则之后将很难理解你选择的档案配置！',
    'profile-copy-base': '复制基础档案',
	errors: {
		conflict: '引脚 {{pin}} 已分配给 {{conflictedMappings}}',
		required: '{{button}} 是必需的',
		invalid: '{{pin}} 对该电路板无效',
		used: '{{pin}} 已分配给另一个功能',
	},
	actions: {
		NONE: '无',
		RESERVED: '预留引脚',
		ASSIGNED_TO_ADDON: '已分配给插件',
		BUTTON_PRESS_UP: '上',
		BUTTON_PRESS_DOWN: '下',
		BUTTON_PRESS_LEFT: '左',
		BUTTON_PRESS_RIGHT: '右',
		BUTTON_PRESS_B1: 'B1',
		BUTTON_PRESS_B2: 'B2',
		BUTTON_PRESS_B3: 'B3',
		BUTTON_PRESS_B4: 'B4',
		BUTTON_PRESS_L1: 'L1',
		BUTTON_PRESS_R1: 'R1',
		BUTTON_PRESS_L2: 'L2',
		BUTTON_PRESS_R2: 'R2',
		BUTTON_PRESS_S1: 'S1',
		BUTTON_PRESS_S2: 'S2',
		BUTTON_PRESS_A1: 'A1',
		BUTTON_PRESS_A2: 'A2',
		BUTTON_PRESS_L3: 'L3',
		BUTTON_PRESS_R3: 'R3',
		BUTTON_PRESS_FN: 'Function',
		BUTTON_PRESS_DDI_UP: 'DDI 上',
		BUTTON_PRESS_DDI_DOWN: 'DDI 下',
		BUTTON_PRESS_DDI_LEFT: 'DDI 左',
		BUTTON_PRESS_DDI_RIGHT: 'DDI 右',
		SUSTAIN_DP_MODE_DP: '方向模式: 十字键',
		SUSTAIN_DP_MODE_LS: '方向模式: 左摇杆',
		SUSTAIN_DP_MODE_RS: '方向模式: 右摇杆',
		SUSTAIN_SOCD_MODE_UP_PRIO: '上优先 SOCD 覆盖',
		SUSTAIN_SOCD_MODE_NEUTRAL: '回中 SOCD 覆盖',
		SUSTAIN_SOCD_MODE_SECOND_WIN: '后输入优先 SOCD 覆盖',
		SUSTAIN_SOCD_MODE_FIRST_WIN: '先输入优先 SOCD 覆盖',
		SUSTAIN_SOCD_MODE_BYPASS: '关闭 SOCD 覆盖',
		BUTTON_PRESS_TURBO: '连发',
		BUTTON_PRESS_MACRO: '宏按钮',
		BUTTON_PRESS_MACRO_1: '宏 1',
		BUTTON_PRESS_MACRO_2: '宏 2',
		BUTTON_PRESS_MACRO_3: '宏 3',
		BUTTON_PRESS_MACRO_4: '宏 4',
		BUTTON_PRESS_MACRO_5: '宏 5',
		BUTTON_PRESS_MACRO_6: '宏 6',
	},
};
