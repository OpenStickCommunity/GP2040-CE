export default {
	rgb: {
		'header-text': 'RGB LED 구성',
		'data-pin-label': '데이터 핀 (-1은 비활성)',
		'led-format-label': 'LED 형식',
		'led-layout-label': 'LED 레이아웃',
		'leds-per-button-label': '버튼 당 LED 수',
		'led-brightness-maximum-label': '최대 밝기',
		'led-brightness-steps-label': '밝기 단계',
	},
	player: {
		'header-text': '플레이어 LED (XInput)',
		'pwm-sub-header-text':
			'PWM LED의 경우에는 각각의 LED에 지정된 GPIO 핀을 할당해주세요',
		'rgb-sub-header-text':
			'RGB LED의 경우 <1>RGB LED 버튼 순서</1>에서 마지막 LED 버튼 다음으로 인덱스를 설정해야하며 <3>{{rgbLedStartIndex}} 인덱스에서 시작됩니다.</3>',
		'pled-type-label': '플레이어 LED 형식',
		'pled-type-off': '끄기',
		'pled-type-pwm': 'PWM',
		'pled-type-rgb': 'RGB',
		'pled-color-label': 'RGB PLED 색상',
	},
	'pled-pin-label': 'PLED #{{pin}} GPIO 핀',
	'pled-index-label': 'PLED #{{index}} 인덱스',
	'rgb-order': {
		'header-text': 'RGB LED 버튼 순서',
		'sub-header-text':
			'여기에서 어떤 버튼에 RGB LED가 있는지 및 컨트롤 보드에서 어떤 순서로 실행되는지를 정의 할 수 있습니다. 이 설정은 일부 LED 애니메이션 및 정적 테마 기능에서 사용됩니다',
		'sub-header1-text':
			'아이템을 드래그 앤 드롭하여 RGB LED를 할당하고 순서를 설정하세요.',
		'available-header-text': '사용 가능한 버튼',
		'assigned-header-text': '할당된 버튼',
	},
	'turn-off-when-suspended': '대기모드에서 끄기',
};
