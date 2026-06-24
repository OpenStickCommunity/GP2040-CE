export default {
	'use-gpio-slider-label': '使用 GPIO 引脚',
	'gpio-input-mode-label': 'GPIO 输入模式选择',
	'gpio-input-mode-explanation': '<p>如果您不使用多个配置文件（Profiles），通常只需在 <0>设置页面</0> 中使用“启动输入模式”选项即可。</p><p>此页面允许您配置在插入控制器时，通过按住哪些物理按钮（GPIO 引脚）来进入特定的输入模式。这与“映射按键”不同，映射按键可能会随着配置文件的切换而改变，而物理引脚始终固定。</p><p>您还可以为每种输入模式指定启动时加载的特定配置文件。</p>',
	'alert-text': '建议您使用 🎮 按钮来通过实时检测选择引脚。映射未连接或不可用的引脚可能会导致设备进入无法正常工作的状态。如果需要清除无效配置，请前往 <0>重置设置</0> 页面。',
	'unique-validation-err': '映射的 GPIO 引脚不能重复',
	'required-validation-err': '缺少必填字段',
	'generic-save-err': '保存更改失败',
}
