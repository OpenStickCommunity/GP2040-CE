export default {
	rgb: {
		'header-text': 'RGB LED 설정',
		'data-pin-label': '데이터 단자 (-1 로 비활성)',
		'led-format-label': 'LED 포맷',
		'led-layout-label': 'LED 레이아웃',
		'leds-per-button-label': '버튼당 LED수',
		'led-brightness-maximum-label': '최대 밝기',
		'led-brightness-steps-label': '밝기 스택',
	},
	player: {
		'header-text': '플레이어LED (XInput)',
		'pwm-sub-header-text':
			'PWM LED의 경우에는 각 LED에 전용 GPIO 핀을 할당해 주세요',
		'rgb-sub-header-text':
			'RGB LED 의 경우 <1>RGB LED버튼 순서</1>에서 설정한 마지막 LED 버튼 다음의 값으로 설정해 주세요. <3>{{rgbLedStartIndex}}에서 시작</3>',
		'pled-type-label': '플레이어LED 타입',
		'pled-type-off': '오프',
		'pled-type-pwm': 'PWM',
		'pled-type-rgb': 'RGB',
		'pled-color-label': 'RGB PLED 컬러',
	},
	'pled-pin-label': 'PLED #{{pin}} 단자',
	'pled-index-label': 'PLED #{{index}} 인덱스',
	'rgb-order': {
		'header-text': 'RGB LED 버튼 순서',
		'sub-header-text':
			'여기에서는 어떤 버튼에 RGB LED가 이 기능이 구현되어 있는지 및 제어 기판에서 어떤 순서로 배선되어 있는지를 설정할 수 있습니다. 이 설정은 일부 LED 애니메이션이나 정적 테마 기능에서 사용됩니다',
		'sub-header1-text':
			'아이템을 드래그 앤 드롭하여 순서를 변경하고 RGB LED의 활성화 및 순서를 설정하세요.',
		'available-header-text': '사용 가능 버튼',
		'assigned-header-text': '할당 된 버튼',
	},
	'turn-off-when-suspended': '일시정지중 off 하기',
};
