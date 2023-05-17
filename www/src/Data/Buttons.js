export const BUTTONS = {
	gp2040: {
		label: "GP2040",
		value: "gp2040",
		Up: "Up",
		Down: "Down",
		Left: "Left",
		Right: "Right",
		B1: "B1",
		B2: "B2",
		B3: "B3",
		B4: "B4",
		L1: "L1",
		R1: "R1",
		L2: "L2",
		R2: "R2",
		S1: "S1",
		S2: "S2",
		L3: "L3",
		R3: "R3",
		A1: "A1",
		A2: "A2"
	},
	arcade: {
		label: "Arcade",
		value: "arcade",
		Up: "Up",
		Down: "Down",
		Left: "Left",
		Right: "Right",
		B1: "K1",
		B2: "K2",
		B3: "P1",
		B4: "P2",
		L1: "P4",
		R1: "P3",
		L2: "K4",
		R2: "K3",
		S1: "Select",
		S2: "Start",
		L3: "L3",
		R3: "R3",
		A1: "Home",
		A2: "-"
	},
	xinput: {
		label: "XInput",
		value: "xinput",
		Up: "Up",
		Down: "Down",
		Left: "Left",
		Right: "Right",
		B1: "A",
		B2: "B",
		B3: "X",
		B4: "Y",
		L1: "LB",
		R1: "RB",
		L2: "LT",
		R2: "RT",
		S1: "Back",
		S2: "Start",
		L3: "LS",
		R3: "RS",
		A1: "Guide",
		A2: "-"
	},
	switch: {
		label: "Nintendo Switch",
		value: "switch",
		Up: "Up",
		Down: "Down",
		Left: "Left",
		Right: "Right",
		B1: "B",
		B2: "A",
		B3: "Y",
		B4: "X",
		L1: "L",
		R1: "R",
		L2: "ZL",
		R2: "ZR",
		S1: "Minus",
		S2: "Plus",
		L3: "LS",
		R3: "RS",
		A1: "Home",
		A2: "Capture"
	},
	ps3: {
		label: "PS3/PS4",
		value: "ps3",
		Up: "Up",
		Down: "Down",
		Left: "Left",
		Right: "Right",
		B1: "Cross",
		B2: "Circle",
		B3: "Square",
		B4: "Triangle",
		L1: "L1",
		R1: "R1",
		L2: "L2",
		R2: "R2",
		S1: "Select",
		S2: "Start",
		L3: "L3",
		R3: "R3",
		A1: "PS",
		A2: "-"
	},
	dinput: {
		label: "DirectInput",
		value: "dinput",
		Up: "Up",
		Down: "Down",
		Left: "Left",
		Right: "Right",
		B1: "2",
		B2: "3",
		B3: "1",
		B4: "4",
		L1: "5",
		R1: "6",
		L2: "7",
		R2: "8",
		S1: "9",
		S2: "10",
		L3: "11",
		R3: "12",
		A1: "13",
		A2: "14"
	}
};

export const AUX_BUTTONS = [ 'S1', 'S2', 'L3', 'R3', 'A1', 'A2' ];
export const MAIN_BUTTONS = [ 'Up', 'Down', 'Left', 'Right', 'B1', 'B2', 'B3', 'B4', 'L1', 'R1', 'L2', 'R2' ];

export const STICK_LAYOUT = [
	[null, 'Left', null],
	['Up', null, 'Down'],
	[null, 'Right', null],
	['B3', null, 'B1'],
	['B4', null, 'B2'],
	['R1', null, 'R2'],
	['L1', null, 'L2'],
];

export const STICKLESS_LAYOUT = [
	['Left', null, null],
	['Down', null, null],
	['Right', null, null],
	[null, 'Up', null],
	['B3', 'B1', null],
	['B4', 'B2', null],
	['R1', 'R2', null],
	['L1', 'L2', null],
];

export const KEYBOARD_LAYOUT = [
	[null, 'Left'],
	['Up', 'Down'],
	[null, 'Right'],
	['B3', 'B1'],
	['B4', 'B2'],
	['R1', 'R2'],
	['L1', 'L2'],
];
