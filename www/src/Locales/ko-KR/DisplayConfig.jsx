
export default {
	'header-text': '디스플레이 설정',
	'sub-header-text':
		'모노크롬 모니터를 사용하여 컨트롤러의 상태와 버튼 작동 상태를 표시할 수 있습니다. 사용 중인 디스플레이가 다음 요구 사항을 충족하는지 확인하세요.：',
	'list-text':
		'<1>해상도 128x64의 모노크롬 모니터</1> <1>SSD1306, SH1106, SH1107 중 하나 또는 호환 가능한 I2C 디스플레이 IC를 채용</1> <1>3.3v 에서의 동작에 대응</1>',
	section: {
		'hardware-header': '하드웨어 설정',
		'screen-header': '스크린 설정',
		'layout-header': '레이아웃 설정',
	},
	table: {
		header:
			'아래의 표를 참조하여 SDA와 SCL단자 할당부터 사용 I2C 블럭을 확인해주세요',
		'sda-scl-pins-header': 'SDA/SCL 단자',
		'i2c-block-header': 'I2C 블럭',
	},
	form: {
		'i2c-block-label': 'I2C블럭',
		'sda-pin-label': 'SDA단자',
		'scl-pin-label': 'SCL단자',
		'i2c-address-label': 'I2C 주소',
		'i2c-speed-label': 'I2C 속도',
		'flip-display-label': '표시 반전',
		'invert-display-label': '흑백 반전',
		'button-layout-label': '버튼 레이아웃（좌）',
		'button-layout-right-label': '버튼 레이아웃（우）',
		'button-layout-custom-header': '커스텀 버튼 레이아웃 매개변수',
		'button-layout-custom-left-label': '좌측 레이아웃',
		'button-layout-custom-right-label': '우측 레이아웃',
		'button-layout-custom-start-x-label': 'X좌표 시작 위치',
		'button-layout-custom-start-y-label': 'Y좌표 시작 위치',
		'button-layout-custom-button-radius-label': '버튼 반경',
		'button-layout-custom-button-padding-label': '버튼 내부여백',
		'splash-mode-label': '스플래쉬 모드',
		'splash-duration-label': '스플래쉬 표시 시간 (초, 0이면 항상켜짐)',
		'display-saver-timeout-label': '화면보호기 타임아웃 시간 (분)',
		'inverted-label': '흑백반전',
		'power-management-header': '전원 관리',
		'turn-off-when-suspended': '서스펜드 중에 OFF로 설정',
		'flip-display-none': '없음',
		'flip-display-flip': '180도 회전',
		'flip-display-mirror': '좌우 반전',
		'flip-display-flip-mirror': '회전하여 좌우반전',
	},
};
