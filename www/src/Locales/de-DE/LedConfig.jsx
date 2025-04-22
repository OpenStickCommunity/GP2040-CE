export default {
	rgb: {
		'header-text': 'RGB LED Konfiguration',
		'data-pin-label': 'Daten GPIO Pin (-1 für deaktiviert)',
		'led-format-label': 'LED Format',
		'led-layout-label': 'LED Layout',
		'leds-per-button-label': 'LEDs Pro Taste',
		'led-brightness-maximum-label': 'Maximale Helligkeit',
		'led-brightness-steps-label': 'Helligkeitsstufen',
	},
	player: {
		'header-text': 'Spieler LEDs (XInput)',
		'pwm-sub-header-text':
			'Für PWM-LEDs, setzen Sie jede LED auf einen dedizierten GPIO Pin.',
		'rgb-sub-header-text':
			'Für RGB-LEDs müssen die Indizes nach der zuletzt definierten LED-Taste liegen.<1>RGB LED Tasten Reihenfolge</1> Sektion und wahrscheinlich <3>beginnt bei Index {{rgbLedStartIndex}}</3>.',
		'pled-type-label': 'Spieler LED Typ',
		'pled-type-off': 'Aus',
		'pled-type-pwm': 'PWM',
		'pled-type-rgb': 'RGB',
		'pled-color-label': 'RGB SLED Farbe',
	},
	'pled-pin-label': 'SLED #{{pin}} GPIO Pin',
	'pled-index-label': 'SLED #{{index}} Index',
	'rgb-order': {
		'header-text': 'RGB LED Tasten Reihenfolge',
		'sub-header-text':
			'Hier können Sie festlegen, welche Tasten RGB-LEDs haben und in welcher Reihenfolge sie vom Steuerungsboard aus gesteuert werden. Dies ist für bestimmte LED-Animationen und die Unterstützung statischer Themen erforderlich.',
		'sub-header1-text':
			'Ziehen Sie die Listenelemente per Drag-and-Drop, um die RGB-LEDs zuzuweisen und neu anzuordnen.',
		'available-header-text': 'Verfügbare Tasten',
		'assigned-header-text': 'Zugeordnete Tasten',
	},
	'turn-off-when-suspended': 'Ausschalten im Ruhezustand',
};
