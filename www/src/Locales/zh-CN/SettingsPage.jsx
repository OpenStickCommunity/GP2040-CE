export default {
    'auth-settings-label': '认证设置',
	'settings-header-text': '输入模式设置',
	'gamepad-settings-header-text': '游戏控制器设置',
	'input-mode-label': '输入模式',
	'current-input-mode-label': '当前输入模式',
	'input-mode-extra-label': '触摸板(Touchpad)和分享键(Share)互换',
	'ps4-report-hack': 'PS4 回报率破解',
	'input-mode-options': {
		none: '模式未选择',
		xinput: 'XInput',
		'nintendo-switch': 'Nintendo Switch',
		ps3: 'PS3/DirectInput',
		keyboard: '键盘',
		ps4: 'PS4',
		ps5: 'PS5',
		neogeo: 'NEOGEO mini',
		mdmini: 'Sega Genesis/MegaDrive Mini',
		pcemini: 'PC Engine/Turbografx 16 Mini',
		egret: 'EGRET II mini',
		astro: 'ASTROCITY Mini',
		psclassic: 'Playstation Classic',
		xboxoriginal: 'Original Xbox',
		xbone: 'Xbox One',
	},
    'input-mode-group': {
		primary: '主要输入模式',
		mini: '迷你/经典主机模式',
	},
	'boot-input-mode-label': '引导输入模式',
	'ps4-mode-options': {
		controller: '游戏控制器',
		arcadestick: '街机摇杆',
	},
    'input-mode-authentication': {
		none: '无',
		key: '上传的密钥文件',
		usb: 'Host USB',
		i2c: 'Attached I2C',
	},
	'd-pad-mode-label': '方向模式',
	'd-pad-mode-options': {
		'd-pad': '十字键',
		'left-analog': '左模拟摇杆',
		'right-analog': '右模拟摇杆',
	},
	'socd-cleaning-mode-label': 'SOCD 覆盖模式',
	'socd-cleaning-mode-note':
		'注意：PS4、PS3、Nintendo Switch以及迷你系列模式不支持将 SOCD 覆盖模式设置为关闭，默认设置为回中模式。',
	'socd-cleaning-mode-options': {
		'up-priority': '上优先',
		neutral: '回中',
		'last-win': '后输入优先',
		'first-win': '先输入优先',
		off: '关闭',
	},
	'profile-number-label': '档案编号',
    'debounce-delay-label': '去抖动延迟 (以毫秒为单位)',
	'ps4-compatibility-label':
		'对于 <strong>兼容PS5</strong>, 请使用“街机摇杆”模式并启用PS直通验证插件<br/>对于 <strong>支持PS4</strong>, 如果你已经上传了必要文件，请使用“游戏控制器”模式并启用PS4模式插件。',
	'hotkey-settings-label': '快捷键设置',
	'hotkey-settings-sub-header':
		"<1>Fn</1> 组合映射在<4 exact='true' to='/pin-mapping'>引脚映射</4>页面提供了一个可映射的Function按键。 要选择 <1>Fn</1> 组合映射选项，请同时按住 Function 按键和其他快捷键。<5 />此外, 可从下拉列表中选择 <1>None</1> 以取消分配按钮",
	'hotkey-settings-warning': 
        'Function 按键未映射引脚时，Fn 组合映射将会被禁用',
	'hotkey-actions': {
		'no-action': '无',
		'dpad-digital': '十字键模式',
		'dpad-left-analog': '左摇杆模式',
		'dpad-right-analog': '右摇杆模式',
		'home-button': 'Home 键',
		'capture-button': 'Capture 键',
		'socd-up-priority': 'SOCD 上优先',
		'socd-neutral': 'SOCD 回中模式',
		'socd-last-win': 'SOCD 后输入优先',
		'socd-first-win': 'SOCD 先输入优先',
		'socd-off': 'SOCD 关闭',
		'invert-x': '反转 X 轴',
		'invert-y': '反转 Y 轴',
		'toggle-4way-joystick-mode': '切换4向摇杆模式',
		'toggle-ddi-4way-joystick-mode': '切换DDI-4向摇杆模式',
		'b1-button': 'B1 键',
		'b2-button': 'B2 键',
		'b3-button': 'B3 键',
		'b4-button': 'B4 键',
		'l1-button': 'L1 键',
		'r1-button': 'R1 键',
		'l2-button': 'L2 键',
		'r2-button': 'R2 键',
		'l3-button': 'L3 键',
		'r3-button': 'R3 键',
		's1-button': 'S1 键',
		's2-button': 'S2 键',
		'a1-button': 'A1 键',
		'a2-button': 'A2 键',
		'touchpad-button': '触摸板',
		'load-profile-1': '加载 档案 #1',
		'load-profile-2': '加载 档案 #2',
		'load-profile-3': '加载 档案 #3',
		'load-profile-4': '加载 档案 #4',
		'reboot-default': '重启 GP2040-CE',
        'next-profile': '下一个档案',
	},
	'forced-setup-mode-label': '强制设置模式',
	'forced-setup-mode-options': {
		off: '关闭',
		'disable-input-mode': '禁用输入模式',
		'disable-web-config': '禁用网页配置',
		'disable-both': '全部禁用',
	},
	'forced-setup-mode-modal-title': '强制设置模式警告',
	'forced-setup-mode-modal-body':
		'如果保存后重新启动到控制器模式，您将无法再访问网页配置。 如果你完全理解并打算解锁保存按钮，请在下方键入 "<strong>{{warningCheckText}}</strong>" 。 单击取消将还原此设置并保存。',
	'4-way-joystick-mode-label': '4向摇杆模式',
	'lock-hotkeys-label': '锁定快捷键',
	'keyboard-mapping-header-text': '键盘映射',
	'keyboard-mapping-sub-header-text': 
        '使用下方的表格重新配置按键映射。',
};
