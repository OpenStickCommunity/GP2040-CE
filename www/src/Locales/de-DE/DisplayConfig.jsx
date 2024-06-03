export default {
	'header-text': 'Display Konfiguration',
	'sub-header-text':
		'Ein monochromes Display kann dazu verwendet werden, um den Status des Controllers und die Aktivität der Tasten anzuzeigen. Stellen Sie sicher, dass Ihr Display-Modul die folgenden Eigenschaften hat:',
	'list-text':
		'<1>Monochromes Display mit 128x64 Auflösung</1> <1>Verwendet I2C mit einem SSD1306-, SH1106-, SH1107- oder einem anderen kompatiblen Anzeige-IC</1> <1>Unterstützt den Betrieb mit 3,3V</1>',
	section: {
		'hardware-header': 'Hardware Optionen',
		'screen-header': 'Bildschirm Optionen',
		'layout-header': 'Layout Optionen',
	},
	table: {
		header:
			'Verwenden Sie diese Tabellen, um zu bestimmen, welchen I2C-Block Sie basierend auf den konfigurierten SDA- und SCL-Pins auswählen haben:',
		'sda-scl-pins-header': 'SDA/SCL Pins',
		'i2c-block-header': 'I2C Block',
	},
	form: {
		'i2c-block-label': 'I2C Block',
		'sda-pin-label': 'SDA Pin',
		'scl-pin-label': 'SCL Pin',
		'i2c-address-label': 'I2C Addresse',
		'i2c-speed-label': 'I2C Geschwindigkeit',
		'flip-display-label': 'Display umdrehen',
		'invert-display-label': 'Display invertieren',
		'button-layout-label': 'Tasten Layout (Links)',
		'button-layout-right-label': 'Tasten Layout (Rechts)',
		'button-layout-custom-header': 'Benutzerdefinierte Tastenlayout-Parameter',
		'button-layout-custom-left-label': 'Layout Links',
		'button-layout-custom-right-label': 'Layout Rechts',
		'button-layout-custom-start-x-label': 'Start X',
		'button-layout-custom-start-y-label': 'Start Y',
		'button-layout-custom-button-radius-label': 'Tasten Radius',
		'button-layout-custom-button-padding-label': 'Tastenabstand',
		'splash-mode-label': 'Splash Modus',
		'splash-duration-label':
			'Splash Dauer (Sekunden, 0 für immer eingeschaltet)',
		'display-saver-timeout-label':
			'Timeout für den Bildschirmschoner (Minuten)',
		'inverted-label': 'Invertiert',
		'turn-off-when-suspended': 'Ausschalten im Ruhezustand',
	},
};
