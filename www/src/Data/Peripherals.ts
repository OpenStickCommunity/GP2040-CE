export const I2C_BLOCKS = [
	{
		label: 'i2c0',
		value: 0,
		pins: { sda: [0, 4, 8, 12, 16, 20, 28, 32, 36, 40, 44], scl: [1, 5, 9, 13, 17, 21, 29, 33, 37, 41, 45] },
	},
	{
		label: 'i2c1',
		value: 1,
		pins: { sda: [2, 6, 10, 14, 18, 26, 30, 34, 38, 42, 46], scl: [3, 7, 11, 15, 19, 27, 31, 35, 39, 43, 47] },
	},
];

export const SPI_BLOCKS = [
	{
		label: 'spi0',
		value: 0,
		pins: {
			rx: [0, 4, 16, 20, 32, 36],
			cs: [1, 5, 17, 21, 33, 37],
			sck: [2, 6, 18, 22, 34, 38],
			tx: [3, 7, 19, 23, 35, 39],
		},
	},
	{
		label: 'spi1',
		value: 1,
		pins: {
			rx: [8, 12, 24, 28, 40, 44],
			cs: [9, 13, 25, 29, 41, 45],
			sck: [10, 14, 26, 30, 42, 46],
			tx: [11, 15, 27, 31, 43, 47],
		},
	},
];

export const USB_BLOCKS = [
	{ label: 'usb0', value: 0, pins: { dp: [], enable5v: [] } },
];

export const PERIPHERAL_DEVICES = [
	{
		label: 'i2c',
		value: 0,
		blocks: I2C_BLOCKS,
		pinTable: true,
		options: {
			speed: [
				{ label: 'Normal', value: 100000 },
				{ label: 'Fast', value: 400000, isDefault: true },
				{ label: 'Fast Plus', value: 1000000 },
			],
		},
	},
	{
		label: 'spi',
		value: 1,
		blocks: SPI_BLOCKS,
		pinTable: true,
		options: {},
	},
	{
		label: 'usb',
		value: 2,
		blocks: USB_BLOCKS,
		pinTable: false,
		options: {
			order: [
				{ label: 'D+/D-', value: 0, isDefault: true },
				{ label: 'D-/D+', value: 1 },
			],
		},
	},
];
