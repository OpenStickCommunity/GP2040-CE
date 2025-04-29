import { useContext, useEffect, useState } from 'react';
import { Button, Form, Modal, Nav, Row, Col, Tab } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import { NavLink } from 'react-router-dom';
import * as yup from 'yup';
import { Trans, useTranslation } from 'react-i18next';
import JSEncrypt from 'jsencrypt';
import isNil from 'lodash/isNil';

import useProfilesStore from '../Store/useProfilesStore';
import { AppContext } from '../Contexts/AppContext';

import ContextualHelpOverlay from '../Components/ContextualHelpOverlay';
import KeyboardMapper from '../Components/KeyboardMapper';
import Section from '../Components/Section';
import WebApi, { baseButtonMappings } from '../Services/WebApi';
import { BUTTON_MASKS_OPTIONS, getButtonLabels } from '../Data/Buttons';

import { hexToInt } from '../Services/Utilities';

import './SettingsPage.scss';

const SHA256 = (ascii) => {
	function rightRotate(value, amount) {
		return (value >>> amount) | (value << (32 - amount));
	}

	const mathPow = Math.pow;
	const maxWord = mathPow(2, 32);
	const lengthProperty = 'length';
	let i; // Used as a counter across the whole file
	let j; // Used as a counter across the whole file
	var result = '';

	const words = [];
	const asciiBitLength = ascii[lengthProperty] * 8;

	//* caching results is optional - remove/add slash from front of this line to toggle
	// Initial hash value: first 32 bits of the fractional parts of the square roots of the first 8 primes
	// (we actually calculate the first 64, but extra values are just ignored)
	var hash = (SHA256.h = SHA256.h || []);
	// Round constants: first 32 bits of the fractional parts of the cube roots of the first 64 primes
	const k = (SHA256.k = SHA256.k || []);
	let primeCounter = k[lengthProperty];
	/*/
		var hash = [], k = [];
		var primeCounter = 0;
		//*/

	const isComposite = {};
	for (let candidate = 2; primeCounter < 64; candidate++) {
		if (!isComposite[candidate]) {
			for (i = 0; i < 313; i += candidate) {
				isComposite[i] = candidate;
			}
			hash[primeCounter] = (mathPow(candidate, 0.5) * maxWord) | 0;
			k[primeCounter++] = (mathPow(candidate, 1 / 3) * maxWord) | 0;
		}
	}

	ascii += '\x80'; // Append Ƈ' bit (plus zero padding)
	while ((ascii[lengthProperty] % 64) - 56) ascii += '\x00'; // More zero padding
	for (i = 0; i < ascii[lengthProperty]; i++) {
		j = ascii.charCodeAt(i);
		if (j >> 8) return; // ASCII check: only accept characters in range 0-255
		words[i >> 2] |= j << (((3 - i) % 4) * 8);
	}
	words[words[lengthProperty]] = (asciiBitLength / maxWord) | 0;
	words[words[lengthProperty]] = asciiBitLength;

	// process each chunk
	for (j = 0; j < words[lengthProperty]; ) {
		const w = words.slice(j, (j += 16)); // The message is expanded into 64 words as part of the iteration
		const oldHash = hash;
		// This is now the undefinedworking hash", often labelled as variables a...g
		// (we have to truncate as well, otherwise extra entries at the end accumulate
		hash = hash.slice(0, 8);

		for (i = 0; i < 64; i++) {
			const i2 = i + j;
			// Expand the message into 64 words
			// Used below if
			const w15 = w[i - 15];
			const w2 = w[i - 2];

			// Iterate
			const a = hash[0];
			const e = hash[4];
			const temp1 =
				hash[7] +
				(rightRotate(e, 6) ^ rightRotate(e, 11) ^ rightRotate(e, 25)) + // S1
				((e & hash[5]) ^ (~e & hash[6])) + // ch
				k[i] +
				// Expand the message schedule if needed
				(w[i] =
					i < 16
						? w[i]
						: (w[i - 16] +
								(rightRotate(w15, 7) ^ rightRotate(w15, 18) ^ (w15 >>> 3)) + // s0
								w[i - 7] +
								(rightRotate(w2, 17) ^ rightRotate(w2, 19) ^ (w2 >>> 10))) | // s1
							0);
			// This is only used once, so *could* be moved below, but it only saves 4 bytes and makes things unreadble
			const temp2 =
				(rightRotate(a, 2) ^ rightRotate(a, 13) ^ rightRotate(a, 22)) + // S0
				((a & hash[1]) ^ (a & hash[2]) ^ (hash[1] & hash[2])); // maj

			hash = [(temp1 + temp2) | 0].concat(hash); // We don't bother trimming off the extra ones, they're harmless as long as we're truncating when we do the slice()
			hash[4] = (hash[4] + temp1) | 0;
		}

		for (i = 0; i < 8; i++) {
			hash[i] = (hash[i] + oldHash[i]) | 0;
		}
	}

	for (i = 0; i < 8; i++) {
		for (j = 3; j + 1; j--) {
			const b = (hash[i] >> (j * 8)) & 255;
			result += (b < 16 ? 0 : '') + b.toString(16);
		}
	}
	return result;
};

const INPUT_MODES = [
	{
		labelKey: 'input-mode-options.xinput',
		value: 0,
		group: 'primary',
		optional: ['usb'],
		authentication: ['none', 'usb'],
	},
	{
		labelKey: 'input-mode-options.nintendo-switch',
		value: 1,
		group: 'primary',
	},
	{ labelKey: 'input-mode-options.ps3', value: 2, group: 'primary' },
	{ labelKey: 'input-mode-options.keyboard', value: 3, group: 'primary' },
	{
		labelKey: 'input-mode-options.ps4',
		value: 4,
		group: 'primary',
		optional: ['usb'],
		authentication: ['none', 'key', 'usb'],
	},
	{
		labelKey: 'input-mode-options.ps5',
		value: 13,
		group: 'primary',
		optional: ['usb'],
		authentication: ['none', 'usb'],
	},
	{
		labelKey: 'input-mode-options.xbone',
		value: 5,
		group: 'primary',
		required: ['usb'],
	},
	{ labelKey: 'input-mode-options.generic', value: 14, group: 'primary' },
	{ labelKey: 'input-mode-options.mdmini', value: 6, group: 'mini' },
	{ labelKey: 'input-mode-options.neogeo', value: 7, group: 'mini' },
	{ labelKey: 'input-mode-options.pcemini', value: 8, group: 'mini' },
	{ labelKey: 'input-mode-options.egret', value: 9, group: 'mini' },
	{ labelKey: 'input-mode-options.astro', value: 10, group: 'mini' },
	{ labelKey: 'input-mode-options.psclassic', value: 11, group: 'mini' },
	{ labelKey: 'input-mode-options.xboxoriginal', value: 12, group: 'primary' },
];

const INPUT_BOOT_MODES = [
	{ labelKey: 'input-mode-options.none', value: -1, group: 'primary' },
	{ labelKey: 'input-mode-options.xinput', value: 0, group: 'primary' },
	{
		labelKey: 'input-mode-options.nintendo-switch',
		value: 1,
		group: 'primary',
	},
	{ labelKey: 'input-mode-options.ps3', value: 2, group: 'primary' },
	{ labelKey: 'input-mode-options.keyboard', value: 3, group: 'primary' },
	{
		labelKey: 'input-mode-options.ps4',
		value: 4,
		group: 'primary',
		optional: ['usb'],
	},
	{
		labelKey: 'input-mode-options.ps5',
		value: 13,
		group: 'primary',
		optional: ['usb'],
	},
	{
		labelKey: 'input-mode-options.xbone',
		value: 5,
		group: 'primary',
		required: ['usb'],
	},
	{ labelKey: 'input-mode-options.generic', value: 14, group: 'primary' },
	{ labelKey: 'input-mode-options.mdmini', value: 6, group: 'mini' },
	{ labelKey: 'input-mode-options.neogeo', value: 7, group: 'mini' },
	{ labelKey: 'input-mode-options.pcemini', value: 8, group: 'mini' },
	{ labelKey: 'input-mode-options.egret', value: 9, group: 'mini' },
	{ labelKey: 'input-mode-options.astro', value: 10, group: 'mini' },
	{ labelKey: 'input-mode-options.psclassic', value: 11, group: 'mini' },
	{ labelKey: 'input-mode-options.xboxoriginal', value: 12, group: 'primary' },
];

const INPUT_MODE_GROUPS = [
	{ labelKey: 'input-mode-group.primary', value: 0, group: 'primary' },
	{ labelKey: 'input-mode-group.mini', value: 1, group: 'mini' },
];

const DPAD_MODES = [
	{ labelKey: 'd-pad-mode-options.d-pad', value: 0 },
	{ labelKey: 'd-pad-mode-options.left-analog', value: 1 },
	{ labelKey: 'd-pad-mode-options.right-analog', value: 2 },
];

const SOCD_MODES = [
	{ labelKey: 'socd-cleaning-mode-options.up-priority', value: 0 },
	{ labelKey: 'socd-cleaning-mode-options.neutral', value: 1 },
	{ labelKey: 'socd-cleaning-mode-options.last-win', value: 2 },
	{ labelKey: 'socd-cleaning-mode-options.first-win', value: 3 },
	{ labelKey: 'socd-cleaning-mode-options.off', value: 4 },
];

const PS4_MODES = [
	{ labelKey: 'ps4-mode-options.controller', value: 0 },
	{ labelKey: 'ps4-mode-options.arcadestick', value: 7 },
];

const PS4_ID_MODES = [
	{ labelKey: 'ps4-id-mode-options.console', value: 0 },
	{ labelKey: 'ps4-id-mode-options.emulation', value: 1 },
];

const AUTHENTICATION_TYPES = [
	{ labelKey: 'input-mode-authentication.none', value: 0 },
	{ labelKey: 'input-mode-authentication.key', value: 1 },
	{ labelKey: 'input-mode-authentication.usb', value: 2 },
	{ labelKey: 'input-mode-authentication.i2c', value: 3 },
];

const HOTKEY_ACTIONS = [
	{ labelKey: 'hotkey-actions.no-action', value: 0 },
	{ labelKey: 'hotkey-actions.dpad-digital', value: 1 },
	{ labelKey: 'hotkey-actions.dpad-left-analog', value: 2 },
	{ labelKey: 'hotkey-actions.dpad-right-analog', value: 3 },
	{ labelKey: 'hotkey-actions.home-button', value: 4 },
	{ labelKey: 'hotkey-actions.capture-button', value: 5 },
	{ labelKey: 'hotkey-actions.socd-up-priority', value: 6 },
	{ labelKey: 'hotkey-actions.socd-neutral', value: 7 },
	{ labelKey: 'hotkey-actions.socd-last-win', value: 8 },
	{ labelKey: 'hotkey-actions.socd-first-win', value: 11 },
	{ labelKey: 'hotkey-actions.socd-off', value: 12 },
	{ labelKey: 'hotkey-actions.invert-x', value: 9 },
	{ labelKey: 'hotkey-actions.invert-y', value: 10 },
	{ labelKey: 'hotkey-actions.toggle-4way-joystick-mode', value: 13 },
	{ labelKey: 'hotkey-actions.toggle-ddi-4way-joystick-mode', value: 14 },
	{ labelKey: 'hotkey-actions.load-profile-1', value: 15 },
	{ labelKey: 'hotkey-actions.load-profile-2', value: 16 },
	{ labelKey: 'hotkey-actions.load-profile-3', value: 17 },
	{ labelKey: 'hotkey-actions.load-profile-4', value: 18 },
	{ labelKey: 'hotkey-actions.next-profile', value: 35 },
	{ labelKey: 'hotkey-actions.previous-profile', value: 42 },
	{ labelKey: 'hotkey-actions.l3-button', value: 19 },
	{ labelKey: 'hotkey-actions.r3-button', value: 20 },
	{ labelKey: 'hotkey-actions.touchpad-button', value: 21 },
	{ labelKey: 'hotkey-actions.reboot-default', value: 22 },
	{ labelKey: 'hotkey-actions.save-config', value: 43 },
	{ labelKey: 'hotkey-actions.b1-button', value: 23 },
	{ labelKey: 'hotkey-actions.b2-button', value: 24 },
	{ labelKey: 'hotkey-actions.b3-button', value: 25 },
	{ labelKey: 'hotkey-actions.b4-button', value: 26 },
	{ labelKey: 'hotkey-actions.l1-button', value: 27 },
	{ labelKey: 'hotkey-actions.r1-button', value: 28 },
	{ labelKey: 'hotkey-actions.l2-button', value: 29 },
	{ labelKey: 'hotkey-actions.r2-button', value: 30 },
	{ labelKey: 'hotkey-actions.s1-button', value: 31 },
	{ labelKey: 'hotkey-actions.s2-button', value: 32 },
	{ labelKey: 'hotkey-actions.a1-button', value: 33 },
	{ labelKey: 'hotkey-actions.a2-button', value: 34 },
	{ labelKey: 'hotkey-actions.a3-button', value: 36 },
	{ labelKey: 'hotkey-actions.a4-button', value: 37 },
	{ labelKey: 'hotkey-actions.dpad-up', value: 38 },
	{ labelKey: 'hotkey-actions.dpad-down', value: 39 },
	{ labelKey: 'hotkey-actions.dpad-left', value: 40 },
	{ labelKey: 'hotkey-actions.dpad-right', value: 41 },
	{ labelKey: 'hotkey-actions.menu-nav-up', value: 44 },
	{ labelKey: 'hotkey-actions.menu-nav-down', value: 45 },
	{ labelKey: 'hotkey-actions.menu-nav-left', value: 46 },
	{ labelKey: 'hotkey-actions.menu-nav-right', value: 47 },
	{ labelKey: 'hotkey-actions.menu-nav-select', value: 48 },
	{ labelKey: 'hotkey-actions.menu-nav-back', value: 49 },
	{ labelKey: 'hotkey-actions.menu-nav-toggle', value: 50 },
];

const FORCED_SETUP_MODES = [
	{ labelKey: 'forced-setup-mode-options.off', value: 0 },
	{ labelKey: 'forced-setup-mode-options.disable-input-mode', value: 1 },
	{ labelKey: 'forced-setup-mode-options.disable-web-config', value: 2 },
	{ labelKey: 'forced-setup-mode-options.disable-both', value: 3 },
];

const INPUT_MODES_BINDS = [
	{ value: 'B1' },
	{ value: 'B2' },
	{ value: 'B3' },
	{ value: 'B4' },
	{ value: 'L1' },
	{ value: 'L2' },
	{ value: 'R1' },
	{ value: 'R2' },
];

const hotkeySchema = {
	action: yup
		.number()
		.required()
		.oneOf(HOTKEY_ACTIONS.map((o) => o.value))
		.label('Hotkey Action'),
	buttonsMask: yup.number().required().label('Button Mask'),
	auxMask: yup.number().required().label('Function Key'),
};

const hotkeyFields = Array(16)
	.fill(0)
	.reduce((acc, a, i) => {
		const number = String(i + 1).padStart(2, '0');
		const newSchema = yup
			.object()
			.label('Hotkey ' + number)
			.shape({ ...hotkeySchema })
			.test(
				'duplicate-hotkeys',
				'Duplicate button combinations are not allowed',
				function (currentValue) {
					return !Object.entries(this.parent).some(
						([key, { buttonsMask, auxMask }]) => {
							if (
								!key.includes('hotkey') || // Skip non-hotkey rows
								key === 'hotkey' + number || // Skip current hotkey
								!Boolean(currentValue.buttonsMask + currentValue.auxMask) // Skip unset hotkey rows
							) {
								return false;
							}
							return (
								buttonsMask === currentValue.buttonsMask &&
								auxMask === currentValue.auxMask
							);
						},
					);
				},
			);
		acc['hotkey' + number] = newSchema;
		return acc;
	}, {});

const schema = yup.object().shape({
	dpadMode: yup
		.number()
		.required()
		.oneOf(DPAD_MODES.map((o) => o.value))
		.label('D-Pad Mode'),
	...hotkeyFields,
	inputMode: yup
		.number()
		.required()
		.oneOf(INPUT_MODES.map((o) => o.value))
		.label('Input Mode'),
	socdMode: yup
		.number()
		.required()
		.oneOf(SOCD_MODES.map((o) => o.value))
		.label('SOCD Cleaning Mode'),
	switchTpShareForDs4: yup
		.number()
		.required()
		.label('Switch Touchpad and Share'),
	ps4ControllerIDMode: yup
		.number()
		.required()
		.oneOf(PS4_ID_MODES.map((o) => o.value))
		.label('PS4 Controller Identification Mode'),
	forcedSetupMode: yup
		.number()
		.required()
		.oneOf(FORCED_SETUP_MODES.map((o) => o.value))
		.label('SOCD Cleaning Mode'),
	lockHotkeys: yup.number().required().label('Lock Hotkeys'),
	fourWayMode: yup.number().required().label('4-Way Joystick Mode'),
	profileNumber: yup.number().required().label('Profile Number'),
	ps4AuthType: yup
		.number()
		.required()
		.oneOf(AUTHENTICATION_TYPES.map((o) => o.value))
		.label('PS4 Authentication Type'),
	ps5AuthType: yup
		.number()
		.required()
		.oneOf(AUTHENTICATION_TYPES.map((o) => o.value))
		.label('PS5 Authentication Type'),
	xinputAuthType: yup
		.number()
		.required()
		.oneOf(AUTHENTICATION_TYPES.map((o) => o.value))
		.label('X-Input Authentication Type'),
	debounceDelay: yup.number().required().label('Debounce Delay'),
	miniMenuGamepadInput: yup.number().required().label('Mini Menu'),
	inputModeB1: yup
		.number()
		.required()
		.oneOf(INPUT_BOOT_MODES.map((o) => o.value))
		.label('B1 Input Mode'),
	inputModeB2: yup
		.number()
		.required()
		.oneOf(INPUT_BOOT_MODES.map((o) => o.value))
		.label('B2 Input Mode'),
	inputModeB3: yup
		.number()
		.required()
		.oneOf(INPUT_BOOT_MODES.map((o) => o.value))
		.label('B3 Input Mode'),
	inputModeB4: yup
		.number()
		.required()
		.oneOf(INPUT_BOOT_MODES.map((o) => o.value))
		.label('B4 Input Mode'),
	inputModeL1: yup
		.number()
		.required()
		.oneOf(INPUT_BOOT_MODES.map((o) => o.value))
		.label('L1 Input Mode'),
	inputModeL2: yup
		.number()
		.required()
		.oneOf(INPUT_BOOT_MODES.map((o) => o.value))
		.label('L2 Input Mode'),
	inputModeR1: yup
		.number()
		.required()
		.oneOf(INPUT_BOOT_MODES.map((o) => o.value))
		.label('R1 Input Mode'),
	inputModeR2: yup
		.number()
		.required()
		.oneOf(INPUT_BOOT_MODES.map((o) => o.value))
		.label('R2 Input Mode'),
	usbDescProduct: yup.string().label('USB Description: Product Name'),
	usbDescManufacturer: yup.string().label('USB Description: Manufacturer'),
	usbDescVersion: yup.string().label('USB Description: Version'),
	usbVendorID: yup.string().label('USB Vendor ID').validateUSBHexID(),
	usbProductID: yup.string().label('USB Product ID').validateUSBHexID(),
});

const FormContext = ({ setButtonLabels, setKeyMappings }) => {
	const { values, setValues } = useFormikContext();
	const { setLoading } = useContext(AppContext);

	useEffect(() => {
		async function fetchData() {
			const options = await WebApi.getGamepadOptions(setLoading);
			setValues(options);
			setButtonLabels({
				swapTpShareLabels:
					options.switchTpShareForDs4 === 1 && options.inputMode === 4,
			});
			setKeyMappings(await WebApi.getKeyMappings(setLoading));
		}
		fetchData();
	}, [setKeyMappings, setValues]);

	useEffect(() => {
		if (!!values.dpadMode) values.dpadMode = parseInt(values.dpadMode);
		if (!!values.inputMode) values.inputMode = parseInt(values.inputMode);
		if (!!values.socdMode) values.socdMode = parseInt(values.socdMode);
		if (!!values.switchTpShareForDs4)
			values.switchTpShareForDs4 = parseInt(values.switchTpShareForDs4);
		if (!!values.forcedSetupMode)
			values.forcedSetupMode = parseInt(values.forcedSetupMode);
		if (!!values.lockHotkeys) values.lockHotkeys = parseInt(values.lockHotkeys);
		if (!!values.fourWayMode) values.fourWayMode = parseInt(values.fourWayMode);
		if (!!values.profileNumber)
			values.profileNumber = parseInt(values.profileNumber);
		if (!!values.ps4AuthType) values.ps4AuthType = parseInt(values.ps4AuthType);
		if (!!values.ps5AuthType) values.ps5AuthType = parseInt(values.ps5AuthType);
		if (!!values.xinputAuthType)
			values.xinputAuthType = parseInt(values.xinputAuthType);
		if (!!values.ps4ControllerIDMode)
			values.ps4ControllerIDMode = parseInt(values.ps4ControllerIDMode);

		setButtonLabels({
			swapTpShareLabels:
				values.switchTpShareForDs4 === 1 && values.inputMode === 4,
		});

		Object.keys(hotkeyFields).forEach((a) => {
			const value = values[a];
			if (value) {
				values[a] = {
					action: parseInt(value.action),
					buttonsMask: parseInt(value.buttonsMask),
					auxMask: parseInt(value.auxMask),
				};
			}
		});
	}, [values, setValues]);

	return null;
};

export default function SettingsPage() {
	const {
		buttonLabels,
		setButtonLabels,
		getAvailablePeripherals,
		updatePeripherals,
	} = useContext(AppContext);

	const fetchProfiles = useProfilesStore((state) => state.fetchProfiles);
	const profiles = useProfilesStore((state) => state.profiles);

	useEffect(() => {
		fetchProfiles();
		updatePeripherals();
	}, []);

	const [saveMessage, setSaveMessage] = useState('');
	const [warning, setWarning] = useState({ show: false, acceptText: '' });
	const [validated, setValidated] = useState(false);
	const [keyMappings, setKeyMappings] = useState(baseButtonMappings);

	const [message, setMessage] = useState(null);
	const [PS4Key, setPS4Key] = useState();
	const [PS4Serial, setPS4Serial] = useState();
	const [PS4Signature, setPS4Signature] = useState();

	const handlePS4Key = (event) => {
		setPS4Key(event.target.files[0]);
		setMessage(null);
	};
	const handlePS4Serial = (event) => {
		setPS4Serial(event.target.files[0]);
		setMessage(null);
	};
	const handlePS4Signature = (event) => {
		setPS4Signature(event.target.files[0]);
		setMessage(null);
	};

	const verifyAndSavePS4 = async ({
		PS4Key,
		PS4Serial,
		PS4Signature,
		setMessage,
	}) => {
		const loadFile = (file, text) => {
			return new Promise((resolve, reject) => {
				const keyReader = new FileReader();
				keyReader.onloadend = (e) => {
					if (!isNil(keyReader.error)) {
						reject(keyReader.error);
					} else {
						resolve(keyReader.result);
					}
				};
				if (text) {
					keyReader.readAsText(file);
				} else {
					keyReader.readAsBinaryString(file);
				}
			});
		};

		function int2mbedmpi(num) {
			const out = [];
			const mask = BigInt('4294967295');
			const zero = BigInt('0');
			while (num !== zero) {
				out.push((num & mask).toString(16).padStart(8, '0'));
				num = num >> BigInt(32);
			}
			return out;
		}

		function hexToBytes(hex) {
			const bytes = [];
			for (let c = 0; c < hex.length; c += 2)
				bytes.push(parseInt(hex.substr(c, 2), 16));
			return bytes;
		}

		function mbedmpi2b64(mpi) {
			const arr = new Uint8Array(mpi.length * 4);
			let cnt = 0;
			for (let i = 0; i < mpi.length; i++) {
				const bytes = hexToBytes(mpi[i]);
				for (let j = 4; j > 0; j--) {
					//arr[cnt] = bytes[j];
					// TEST: re-order from LSB to MSB
					arr[cnt] = bytes[j - 1];
					cnt++;
				}
			}

			return btoa(String.fromCharCode.apply(null, arr));
		}

		try {
			const [pem, signature, serialFileContent] = await Promise.all([
				loadFile(PS4Key, true),
				loadFile(PS4Signature, false),
				loadFile(PS4Serial, true),
			]);

			// Make sure our signature is 256 bytes
			const serialNoPadding = serialFileContent.trimRight();
			if (signature.length !== 256 || serialNoPadding.length !== 16) {
				throw new Error('Signature or serial is invalid');
			}
			const serial = serialNoPadding.padStart(32, '0'); // Add our padding

			const key = new JSEncrypt();
			key.setPrivateKey(pem);
			const bytes = new Uint8Array(256);
			for (let i = 0; i < 256; i++) {
				bytes[i] = Math.random() * 255;
			}
			const hashed = SHA256(bytes);
			const signNonce = key.sign(hashed, SHA256, 'sha256');

			if (signNonce === false) {
				throw new Error('Bad Private Key');
			}

			// Private key worked!

			// Translate these to BigInteger
			const N = BigInt(String(key.key.n));
			const E = BigInt(String(key.key.e));
			const P = BigInt(String(key.key.p));
			const Q = BigInt(String(key.key.q));

			const serialBin = hexToBytes(serial);

			const success = await WebApi.setPS4Options({
				N: mbedmpi2b64(int2mbedmpi(N)),
				E: mbedmpi2b64(int2mbedmpi(E)),
				P: mbedmpi2b64(int2mbedmpi(P)),
				Q: mbedmpi2b64(int2mbedmpi(Q)),
				serial: btoa(String.fromCharCode(...new Uint8Array(serialBin))),
				signature: btoa(signature),
			});

			if (success) {
				setMessage('Verified and Saved PS4 Keys!');
			} else {
				throw Error('ERROR: Failed to upload the key to the board');
			}
		} catch (e) {
			setMessage(`ERROR: Could not verify required files: ${e}`);
		}
	};

	const WARNING_CHECK_TEXT = 'GP2040-CE';

	const INPUT_MODE_PERMISSIONS = [
		{
			permission: 'usb',
			check: () => getAvailablePeripherals('usb') !== false,
			reason: () =>
				getAvailablePeripherals('usb') === false
					? 'USB peripheral not enabled'
					: '',
		},
	];

	const handleKeyChange = (value, button) => {
		const newMappings = { ...keyMappings };
		newMappings[button].key = value;
		setKeyMappings(newMappings);
	};

	const generateAuthSelection = (
		inputMode,
		label,
		name,
		value,
		error,
		handleChange,
	) => {
		return (
			<Row className="mb-3">
				<Col sm={4}>
					<Form.Label>{label}</Form.Label>
					<Form.Select
						name={name}
						className="form-select-sm"
						value={value}
						onChange={handleChange}
						isInvalid={error}
					>
						{inputMode.authentication.map((authType) =>
							AUTHENTICATION_TYPES.filter(
								(mode) =>
									mode.labelKey === 'input-mode-authentication.' + authType,
							).map((o) => (
								<option
									key={`button-${name}-option-${o.value}`}
									value={o.value}
								>
									{
										translatedInputModeAuthentications.find(
											(t) => o.value === t.value,
										).label
									}
								</option>
							)),
						)}
					</Form.Select>
				</Col>
			</Row>
		);
	};

	const keyboardModeSpecifics = (
		values,
		errors,
		setFieldValue,
		handleChange,
	) => {
		return (
			<div>
				<Row className="mb-3">
					<Col sm={6}>
						<div className="fs-3 fw-bold">
							{t('SettingsPage:keyboard-mapping-header-text')}
						</div>
					</Col>
				</Row>
				<Row className="mb-3">
					<Col sm={6}>
						<div>{t('SettingsPage:keyboard-mapping-sub-header-text')}</div>
					</Col>
				</Row>
				<KeyboardMapper
					buttonLabels={buttonLabels}
					handleKeyChange={handleKeyChange}
					getKeyMappingForButton={getKeyMappingForButton}
				/>
			</div>
		);
	};

	const ps4ModeSpecifics = (
		values,
		errors,
		setFieldValue,
		handleChange,
		inputMode,
	) => {
		return (
			<div className="row mb-3">
				<Row className="mb-3">
					<Col sm={10}>{t('SettingsPage:ps4-mode-explanation-text')}</Col>
				</Row>
				<Row className="mb-3">
					<Col sm={10}>
						<Form.Check
							label={t('SettingsPage:input-mode-extra-label')}
							type="switch"
							name="switchTpShareForDs4"
							isInvalid={false}
							checked={Boolean(values.switchTpShareForDs4)}
							onChange={(e) => {
								setFieldValue('switchTpShareForDs4', e.target.checked ? 1 : 0);
							}}
						/>
					</Col>
				</Row>
				<Row className="mb-3">
					<Col sm={3}>
						<Form.Label>
							{t('SettingsPage:ps4-id-mode-label')}
							<ContextualHelpOverlay
								title={t('SettingsPage:ps4-id-mode-label')}
								body={
									<Trans
										ns="SettingsPage"
										i18nKey="ps4-id-mode-explanation-text"
										components={{ ul: <ul />, li: <li /> }}
									/>
								}
							/>
						</Form.Label>
						<Form.Select
							name="ps4ControllerIDMode"
							className="form-select-sm"
							value={values.ps4ControllerIDMode}
							onChange={handleChange}
						>
							{PS4_ID_MODES.map((o) => (
								<option key={`ps4-id-option-${o.value}`} value={o.value}>
									{`${t('SettingsPage:' + o.labelKey)}`}
								</option>
							))}
						</Form.Select>
					</Col>
				</Row>
				{generateAuthSelection(
					inputMode,
					t('SettingsPage:auth-settings-label'),
					'ps4AuthType',
					values.ps4AuthType,
					errors.ps4AuthType,
					handleChange,
				)}
				{values.ps4AuthType === 0 && (
					<Row className="mb-3">
						<Col sm={10}>
							<Trans
								ns="SettingsPage"
								i18nKey="ps4-mode-warning-text"
								components={{ span: <span className="text-warning" /> }}
							/>
						</Col>
					</Row>
				)}
				{values.ps4AuthType === 1 && (
					<Row className="mb-3">
						<Row className="mb-3">
							<Col sm={5}>
								<Form.Label className="badge bg-primary fs-2">
									{t('AddonsConfig:ps4-mode-sub-header')}
								</Form.Label>
								<br />
								<Form.Label className="fw-bolder">
									{t('AddonsConfig:ps4-mode-sub-header-text')}
								</Form.Label>
							</Col>
						</Row>
						<Row className="mb-3">
							<Col sm={3}>
								<Form.Label>
									{t('AddonsConfig:ps4-mode-private-key-label')}:
								</Form.Label>
								<br />
								<input
									type="file"
									id="ps4key-input"
									onChange={handlePS4Key}
									multiple={false}
									accept="*/*"
								/>
							</Col>
							<Col sm={3}>
								<Form.Label>
									{t('AddonsConfig:ps4-mode-serial-number-label')}:
								</Form.Label>
								<br />
								<input
									type="file"
									id="ps4serial-input"
									accept="*/*"
									multiple={false}
									onChange={handlePS4Serial}
								/>
							</Col>
							<Col sm={3}>
								<Form.Label>
									{t('AddonsConfig:ps4-mode-signature-label')}:
								</Form.Label>
								<br />
								<input
									type="file"
									id="ps4signature-input"
									accept="*/*"
									multiple={false}
									onChange={handlePS4Signature}
								/>
							</Col>
						</Row>
						<Row className="mb-3">
							<Col sm={10}>
								<Button
									type="button"
									onClick={() =>
										verifyAndSavePS4({
											PS4Key,
											PS4Serial,
											PS4Signature,
											setMessage,
										})
									}
								>
									{t('Common:button-verify-save-label')}
								</Button>
								{message && <span> {message}</span>}
							</Col>
						</Row>
					</Row>
				)}
				{values.ps4AuthType === 2 && (
					<Row className="mb-3">
						<Col sm={10}>
							<Trans
								ns="SettingsPage"
								i18nKey="ps4-usb-host-mode-text"
								components={{ span: <span className="text-info" /> }}
							/>
						</Col>
					</Row>
				)}
			</div>
		);
	};

	const ps5ModeSpecifics = (
		values,
		errors,
		setFieldValue,
		handleChange,
		inputMode,
	) => {
		return (
			<div className="row mb-3">
				<Row className="mb-3">
					<Col sm={10}>{t('SettingsPage:ps5-mode-explanation-text')}</Col>
				</Row>
				<Row className="mb-3">
					<Col sm={10}>
						<Form.Check
							label={t('SettingsPage:input-mode-extra-label')}
							type="switch"
							name="switchTpShareForDs4"
							isInvalid={false}
							checked={Boolean(values.switchTpShareForDs4)}
							onChange={(e) => {
								setFieldValue('switchTpShareForDs4', e.target.checked ? 1 : 0);
							}}
						/>
					</Col>
				</Row>
				<Row className="mb-3">
					<Col sm={3}>
						<Form.Label>
							{t('SettingsPage:ps4-id-mode-label')}
							<ContextualHelpOverlay
								title={t('SettingsPage:ps4-id-mode-label')}
								body={
									<Trans
										ns="SettingsPage"
										i18nKey="ps4-id-mode-explanation-text"
										components={{ ul: <ul />, li: <li /> }}
									/>
								}
							/>
						</Form.Label>
						<Form.Select
							name="ps4ControllerIDMode"
							className="form-select-sm"
							value={values.ps4ControllerIDMode}
							onChange={handleChange}
						>
							{PS4_ID_MODES.map((o) => (
								<option key={`ps4-id-option-${o.value}`} value={o.value}>
									{`${t('SettingsPage:' + o.labelKey)}`}
								</option>
							))}
						</Form.Select>
					</Col>
				</Row>
				{generateAuthSelection(
					inputMode,
					t('SettingsPage:auth-settings-label'),
					'ps5AuthType',
					values.ps5AuthType,
					errors.ps5AuthType,
					handleChange,
				)}
				{values.ps5AuthType === 0 && (
					<Row className="mb-3">
						<Col sm={10}>
							<Trans
								ns="SettingsPage"
								i18nKey="ps5-mode-warning-text"
								components={{ span: <span className="text-warning" /> }}
							/>
						</Col>
					</Row>
				)}
				{values.ps5AuthType === 2 && (
					<Row className="mb-3">
						<Col sm={10}>
							<Trans
								ns="SettingsPage"
								i18nKey="ps5-usb-host-mode-text"
								components={{ span: <span className="text-info" /> }}
							/>
						</Col>
					</Row>
				)}
			</div>
		);
	};

	const usbOverride = (values, errors, setFieldValue, handleChange) => {
		return (
			<div>
				<Row className="mb-3">
					<Col sm={12}>
						<Form.Check
							label={t('SettingsPage:usb-override.advanced-override')}
							type="switch"
							id="usbDescOverride"
							isInvalid={false}
							checked={Boolean(values.usbDescOverride)}
							onChange={(e) => {
								setFieldValue('usbDescOverride', e.target.checked ? 1 : 0);
							}}
						/>
					</Col>
				</Row>
				{values.usbDescOverride === 1 && (
					<>
						<Row className="mb-4 mt-4">
							<Col>
								<span className="alert alert-danger">
									{t('SettingsPage:usb-override.invalid-warning-danger')}
								</span>
							</Col>
						</Row>
						<Row className="mb-3">
							<Col sm={4}>
								<Form.Label>
									{t('SettingsPage:usb-override.product-name')}
								</Form.Label>
								<Form.Control
									size="sm"
									type="text"
									placeholder={'test'}
									name="usbDescProduct"
									value={values.usbDescProduct}
									error={errors?.usbDescProduct}
									isInvalid={errors?.usbDescProduct}
									onChange={handleChange}
									maxLength={32}
								/>
							</Col>
							<Col sm={4}>
								<Form.Label>
									{t('SettingsPage:usb-override.manufacturer')}
								</Form.Label>
								<Form.Control
									size="sm"
									type="text"
									name="usbDescManufacturer"
									value={values.usbDescManufacturer}
									error={errors?.usbDescManufacturer}
									isInvalid={errors?.usbDescManufacturer}
									onChange={handleChange}
									maxLength={32}
								/>
							</Col>
							<Col sm={2}>
								<Form.Label>
									{t('SettingsPage:usb-override.version')}
								</Form.Label>
								<Form.Control
									size="sm"
									type="text"
									name="usbDescVersion"
									value={values.usbDescVersion}
									error={errors?.usbDescVersion}
									isInvalid={errors?.usbDescVersion}
									onChange={handleChange}
									maxLength={8}
								/>
							</Col>
						</Row>
						<Row className="mb-3">
							<Col sm={6}>
								<Form.Check
									label={t('SettingsPage:usb-override.physical-warning-danger')}
									type="switch"
									id="usbOverrideID"
									isInvalid={false}
									checked={Boolean(values.usbOverrideID)}
									onChange={(e) => {
										setFieldValue('usbOverrideID', e.target.checked ? 1 : 0);
									}}
								/>
							</Col>
						</Row>
						<Row className="mb-3" hidden={values.usbOverrideID !== 1}>
							<Col sm={2}>
								<Form.Label>
									{t('SettingsPage:usb-override.vendor-id')}
								</Form.Label>
								<Form.Control
									size="sm"
									type="text"
									name="usbVendorID"
									value={values.usbVendorID}
									error={errors.usbVendorID}
									isInvalid={errors?.usbVendorID}
									onChange={handleChange}
									onBlur={(e) => {
										e.target.value = e.target.value.padStart(4, '0');
										return handleChange(e);
									}}
									minLength={4}
									maxLength={4}
								/>
							</Col>
							<Col sm={2}>
								<Form.Label>
									{t('SettingsPage:usb-override.product-id')}
								</Form.Label>
								<Form.Control
									size="sm"
									type="text"
									name="usbProductID"
									value={values.usbProductID}
									error={errors?.usbProductID}
									isInvalid={errors?.usbProductID}
									onChange={handleChange}
									onBlur={(e) => {
										e.target.value = e.target.value.padStart(4, '0');
										return handleChange(e);
									}}
									minLength={4}
									maxLength={4}
								/>
							</Col>
						</Row>
					</>
				)}
			</div>
		);
	};

	const xinputModeSpecifics = (
		values,
		errors,
		setFieldValue,
		handleChange,
		inputMode,
	) => {
		return (
			<div className="row mb-3">
				{generateAuthSelection(
					inputMode,
					t('SettingsPage:auth-settings-label'),
					'xinputAuthType',
					values.xinputAuthType,
					errors.xinputAuthType,
					handleChange,
				)}
				<Row className="mb-3">
					<Col sm={10}>
						<Trans
							ns="SettingsPage"
							i18nKey="xinput-mode-text"
							components={{ span: <span className="text-success" /> }}
						/>
					</Col>
				</Row>
				{usbOverride(values, errors, setFieldValue, handleChange)}
			</div>
		);
	};

	const xboneModeSpecifics = (values, errors, setFieldValue, handleChange) => {
		return (
			<div className="row mb-3">
				<Row className="mb-3">
					<Col sm={10}>
						<Trans
							ns="SettingsPage"
							i18nKey="xbone-mode-text"
							components={{ span: <span className="text-success" /> }}
						/>
					</Col>
				</Row>
			</div>
		);
	};

	const genericHidModeSpecifics = (
		values,
		errors,
		setFieldValue,
		handleChange,
		inputMode,
	) => {
		return (
			<div className="row mb-3">
				{usbOverride(values, errors, setFieldValue, handleChange)}
			</div>
		);
	};

	const inputModeSpecifics = (values, errors, setFieldValue, handleChange) => {
		// Value hasn't been filled out yet
		if (Object.keys(values).length == 0) {
			return;
		}

		const inputMode = INPUT_MODES.find((o) => o.value == values.inputMode);
		switch (inputMode.labelKey) {
			case 'input-mode-options.keyboard':
				return keyboardModeSpecifics(
					values,
					errors,
					setFieldValue,
					handleChange,
				);
			case 'input-mode-options.ps4':
				return ps4ModeSpecifics(
					values,
					errors,
					setFieldValue,
					handleChange,
					inputMode,
				);
			case 'input-mode-options.ps5':
				return ps5ModeSpecifics(
					values,
					errors,
					setFieldValue,
					handleChange,
					inputMode,
				);
			case 'input-mode-options.generic':
				return genericHidModeSpecifics(
					values,
					errors,
					setFieldValue,
					handleChange,
				);
			case 'input-mode-options.xinput':
				return xinputModeSpecifics(
					values,
					errors,
					setFieldValue,
					handleChange,
					inputMode,
				);
			case 'input-mode-options.xbone':
				return xboneModeSpecifics(values, errors, setFieldValue, handleChange);
			default:
				return (
					<Row className="mb-3">
						<Col>
							{t('SettingsPage:no-mode-settings-text', {
								mode: t(`SettingsPage:${inputMode.labelKey}`),
								interpolation: { escapeValue: false },
							})}
						</Col>
					</Row>
				);
		}
	};

	const handleWarningClose = async (accepted, values, setFieldValue) => {
		setWarning({ show: false, acceptText: '' });
		if (accepted) await saveSettings(values);
		else setFieldValue('forcedSetupMode', 0);
	};

	const setWarningAcceptText = (e) => {
		setWarning({ ...warning, acceptText: e.target.value });
	};

	const saveSettings = async (values) => {
		const success = await WebApi.setGamepadOptions(values);
		setSaveMessage(
			success
				? t('Common:saved-success-message')
				: t('Common:saved-error-message'),
		);
	};

	const onSubmit = async (values) => {
		const isKeyboardMode = values.inputMode === 3;

		const data = {
			...values,
			usbProductID: hexToInt(values.usbProductID || '0000'),
			usbVendorID: hexToInt(values.usbVendorID || '0000'),
		};

		if (values.forcedSetupMode > 1) {
			setWarning({ show: true, acceptText: '' });
		} else {
			if (isKeyboardMode) {
				await WebApi.setKeyMappings(keyMappings);
			}
			await saveSettings(data);
		}
	};

	const translateArray = (array) => {
		return array.map(({ labelKey, ...values }) => {
			return { label: t(`SettingsPage:${labelKey}`), ...values };
		});
	};

	const checkRequiredArray = (array) => {
		return array.map(({ required, optional, ...values }) => {
			let disabledState = false;
			let disabledReason = '';
			let permissionOptions = {};
			if (required) {
				disabledState = INPUT_MODE_PERMISSIONS.filter(({ permission }) =>
					required.includes(permission),
				)
					.map((perm) => perm.check())
					.reduce((acc, val) => acc | (val === false ? 1 : 0), 0);
				disabledReason =
					INPUT_MODE_PERMISSIONS.filter(({ permission }) =>
						required.includes(permission),
					)
						.map((perm) => perm.reason())
						.find((o) => o != '') ?? '';

				permissionOptions = {
					...permissionOptions,
					disabled: disabledState,
					reason: disabledReason,
				};
			}
			if (optional) {
				// todo: define permissions behavior
				permissionOptions = { ...permissionOptions };
			}
			return { ...permissionOptions, ...values };
		});
	};

	const { buttonLabelType, swapTpShareLabels } = buttonLabels;

	const currentButtonLabels = getButtonLabels(
		buttonLabelType,
		swapTpShareLabels,
	);

	const getKeyMappingForButton = (button) => keyMappings[button];

	const { t } = useTranslation('');

	const translatedInputBootModes = translateArray(
		checkRequiredArray(INPUT_BOOT_MODES),
	);
	const translatedInputModes = translateArray(checkRequiredArray(INPUT_MODES));
	const translatedInputModeGroups = translateArray(INPUT_MODE_GROUPS);
	const translatedDpadModes = translateArray(DPAD_MODES);
	const translatedSocdModes = translateArray(SOCD_MODES);
	const translatedHotkeyActions = translateArray(HOTKEY_ACTIONS);
	const translatedForcedSetupModes = translateArray(FORCED_SETUP_MODES);
	// Not currently used but we might add the option at a later date (wheel type, etc.)
	const translatedPS4ControllerTypeModes = translateArray(PS4_MODES);
	const translatedInputModeAuthentications =
		translateArray(AUTHENTICATION_TYPES);

	return (
		<Formik validationSchema={schema} onSubmit={onSubmit} initialValues={{}}>
			{({ handleSubmit, handleChange, values, errors, setFieldValue }) =>
				console.log('errors', errors) || (
					<div>
						<Form noValidate onSubmit={handleSubmit}>
							<Tab.Container defaultActiveKey="inputmode">
								<Row>
									<Col md={3}>
										<Nav variant="pills" className="flex-column">
											<Nav.Item>
												<Nav.Link eventKey="inputmode">
													{t('SettingsPage:settings-header-text')}
												</Nav.Link>
											</Nav.Item>
											<Nav.Item>
												<Nav.Link eventKey="gamepad">
													{t('SettingsPage:gamepad-settings-header-text')}
												</Nav.Link>
											</Nav.Item>
											<Nav.Item>
												<Nav.Link eventKey="bootmode">
													{t('SettingsPage:boot-input-mode-label')}
												</Nav.Link>
											</Nav.Item>
											<Nav.Item>
												<Nav.Link eventKey="hotkey">
													{t('SettingsPage:hotkey-settings-label')}
												</Nav.Link>
											</Nav.Item>
										</Nav>
									</Col>
									<Col md={9}>
										<Tab.Content>
											<Tab.Pane eventKey="inputmode">
												<Section title={t('SettingsPage:settings-header-text')}>
													<Form.Group className="row mb-3">
														<Row className="mb-3">
															<Col sm={4}>
																<Form.Label>
																	{t('SettingsPage:current-input-mode-label')}
																</Form.Label>
																<Form.Select
																	name="inputMode"
																	className="form-select-sm"
																	value={values.inputMode}
																	onChange={handleChange}
																	isInvalid={errors.inputMode}
																>
																	{translatedInputModeGroups.map((o, i) => (
																		<optgroup
																			label={o.label}
																			key={`optgroup-inputMode-${i}`}
																		>
																			{translatedInputModes
																				.filter(({ group }) => group == o.group)
																				.map((o, i) => (
																					<option
																						key={`button-inputMode-option-${i}`}
																						value={o.value}
																						disabled={o.disabled}
																					>
																						{o.label}
																						{o.disabled && o.reason != ''
																							? ' (' + o.reason + ')'
																							: ''}
																					</option>
																				))}
																		</optgroup>
																	))}
																</Form.Select>
																<Form.Control.Feedback type="invalid">
																	{errors.inputMode}
																</Form.Control.Feedback>
															</Col>
														</Row>
														{inputModeSpecifics(
															values,
															errors,
															setFieldValue,
															handleChange,
															translatedInputModeAuthentications,
														)}
													</Form.Group>
													<Button type="submit">
														{t('Common:button-save-label')}
													</Button>
													{saveMessage ? (
														<span className="alert">{saveMessage}</span>
													) : null}
												</Section>
											</Tab.Pane>
											<Tab.Pane eventKey="gamepad">
												<Section
													title={t('SettingsPage:gamepad-settings-header-text')}
												>
													<Form.Group className="row mb-3">
														<Form.Label>
															{t('SettingsPage:d-pad-mode-label')}
														</Form.Label>
														<Col sm={3}>
															<Form.Select
																name="dpadMode"
																className="form-select-sm"
																value={values.dpadMode}
																onChange={handleChange}
																isInvalid={errors.dpadMode}
															>
																{translatedDpadModes.map((o, i) => (
																	<option
																		key={`button-dpadMode-option-${i}`}
																		value={o.value}
																	>
																		{o.label}
																	</option>
																))}
															</Form.Select>
															<Form.Control.Feedback type="invalid">
																{errors.dpadMode}
															</Form.Control.Feedback>
														</Col>
														<Col sm={3}>
															<Form.Check
																label={t(
																	'SettingsPage:4-way-joystick-mode-label',
																)}
																type="switch"
																id="fourWayMode"
																isInvalid={false}
																checked={Boolean(values.fourWayMode)}
																onChange={(e) => {
																	setFieldValue(
																		'fourWayMode',
																		e.target.checked ? 1 : 0,
																	);
																}}
															/>
														</Col>
													</Form.Group>
													<Form.Group className="row mb-3">
														<Form.Label>
															{t('SettingsPage:socd-cleaning-mode-label')}
														</Form.Label>
														<Col sm={3}>
															<Form.Select
																name="socdMode"
																className="form-select-sm"
																value={values.socdMode}
																onChange={handleChange}
																isInvalid={errors.socdMode}
															>
																{translatedSocdModes.map((o, i) => (
																	<option
																		key={`button-socdMode-option-${i}`}
																		value={o.value}
																	>
																		{o.label}
																	</option>
																))}
															</Form.Select>
															<Form.Control.Feedback type="invalid">
																{errors.socdMode}
															</Form.Control.Feedback>
														</Col>
													</Form.Group>
													<p>{t('SettingsPage:socd-cleaning-mode-note')}</p>
													<Form.Group className="row mb-3">
														<Form.Label>
															{t('SettingsPage:forced-setup-mode-label')}
														</Form.Label>
														<Col sm={3}>
															<Form.Select
																name="forcedSetupMode"
																className="form-select-sm"
																value={values.forcedSetupMode}
																onChange={handleChange}
																isInvalid={errors.forcedSetupMode}
															>
																{translatedForcedSetupModes.map((o, i) => (
																	<option
																		key={`button-forcedSetupMode-option-${i}`}
																		value={o.value}
																	>
																		{o.label}
																	</option>
																))}
															</Form.Select>
															<Form.Control.Feedback type="invalid">
																{errors.forcedSetupMode}
															</Form.Control.Feedback>
														</Col>
													</Form.Group>
													<Form.Group className="row mb-3">
														<Form.Label>
															{t('SettingsPage:profile-label')}
														</Form.Label>
														<Col sm={3}>
															<Form.Select
																name="profileNumber"
																className="form-select-sm"
																value={values.profileNumber}
																onChange={handleChange}
																isInvalid={errors.profileNumber}
															>
																{profiles.map((profile, index) => (
																	<option
																		key={`button-profileNumber-option-${
																			index + 1
																		}`}
																		value={index + 1}
																	>
																		{profile.profileLabel ||
																			t('PinMapping:profile-label-default', {
																				profileNumber: index + 1,
																			})}
																	</option>
																))}
															</Form.Select>
														</Col>
													</Form.Group>
													<Form.Group className="row mb-3">
														<Form.Label>
															{t('SettingsPage:debounce-delay-label')}
														</Form.Label>
														<Col sm={3}>
															<Form.Control
																type="number"
																name="debounceDelay"
																className="form-control-sm"
																value={values.debounceDelay}
																error={errors.debounceDelay}
																isInvalid={errors.debounceDelay}
																onChange={handleChange}
																min={0}
																max={5000}
															/>
														</Col>
													</Form.Group>
													<Form.Group className="row mb-5">
														<Col sm={5}>
															<Form.Check
																label={t('SettingsPage:mini-menu-gamepad-input')}
																type="switch"
																id="miniMenuGamepadInput"
																isInvalid={false}
																checked={Boolean(values.miniMenuGamepadInput)}
																onChange={(e) => {
																	setFieldValue(
																		'miniMenuGamepadInput',
																		e.target.checked ? 1 : 0,
																	);
																}}
															/>
														</Col>
													</Form.Group>
													<Button type="submit">
														{t('Common:button-save-label')}
													</Button>
													{saveMessage ? (
														<span className="alert">{saveMessage}</span>
													) : null}
												</Section>
											</Tab.Pane>
											<Tab.Pane eventKey="bootmode">
												<Section
													title={t('SettingsPage:boot-input-mode-label')}
												>
													<Row sm={3}>
														{INPUT_MODES_BINDS.map((mode, index) => (
															<Form.Group
																className="mb-3 col-sm-6"
																key={`input-mode-${index}`}
															>
																<Form.Label>
																	{mode.value in currentButtonLabels
																		? currentButtonLabels[mode.value]
																		: mode.value}
																</Form.Label>
																<Col sm={10}>
																	<Form.Select
																		name={`inputMode${mode.value}`}
																		className="form-select-sm"
																		value={values[`inputMode${mode.value}`]}
																		onChange={handleChange}
																		isInvalid={errors[`inputMode${mode.value}`]}
																	>
																		{translatedInputModeGroups.map((o, i) => (
																			<optgroup
																				label={o.label}
																				key={`optgroup-${o.label}-${i}`}
																			>
																				{translatedInputBootModes
																					.filter(
																						({ group }) => group == o.group,
																					)
																					.map((o, i) => (
																						<option
																							key={`button-inputMode-${mode.value
																								.toString()
																								.toLowerCase()}-option-${i}`}
																							value={o.value}
																							disabled={o.disabled}
																						>
																							{o.label}
																							{o.disabled && o.reason != ''
																								? ' (' + o.reason + ')'
																								: ''}
																						</option>
																					))}
																			</optgroup>
																		))}
																	</Form.Select>
																	<Form.Control.Feedback type="invalid">
																		{errors[`inputMode${mode.value}`]}
																	</Form.Control.Feedback>
																</Col>
															</Form.Group>
														))}
													</Row>
													<Button type="submit">
														{t('Common:button-save-label')}
													</Button>
													{saveMessage ? (
														<span className="alert">{saveMessage}</span>
													) : null}
												</Section>
											</Tab.Pane>
											<Tab.Pane eventKey="hotkey">
												<Section
													title={t('SettingsPage:hotkey-settings-label')}
												>
													<div className="mb-3">
														<Trans
															ns="SettingsPage"
															i18nKey="hotkey-settings-sub-header"
															components={{
																link_pinmap: <NavLink to="/pin-mapping" />,
															}}
														/>
													</div>
													{values.fnButtonPin === -1 && (
														<div className="alert alert-warning">
															{t('SettingsPage:hotkey-settings-warning')}
														</div>
													)}
													<div
														id="Hotkeys"
														hidden={values.lockHotkeys}
														className="d-grid gap-2"
													>
														{Object.keys(hotkeyFields).map((o, i) => (
															<Form.Group
																key={`hotkey-${i}-base`}
																className={`row row-gap-2 align-items-center gx-2`}
															>
																<Col
																	sm="auto"
																	className="d-flex align-items-center"
																>
																	<Form.Check
																		name={`${o}.auxMask`}
																		label="Fn"
																		type="switch"
																		className="text my-auto"
																		disabled={values.fnButtonPin === -1}
																		checked={values[o] && !!values[o]?.auxMask}
																		onChange={(e) => {
																			setFieldValue(
																				`${o}.auxMask`,
																				e.target.checked ? 32768 : 0,
																			);
																		}}
																		isInvalid={errors[o] || errors[o]?.auxMask}
																	/>
																	<Form.Control.Feedback type="invalid">
																		{errors[o] && errors[o]?.action}
																	</Form.Control.Feedback>
																</Col>
																<Col sm="auto">+</Col>
																{BUTTON_MASKS_OPTIONS.map((mask) =>
																	values[o] &&
																	values[o]?.buttonsMask & mask.value ? (
																		<>
																			<Col sm="auto">
																				<Form.Select
																					name={`${o}.buttonsMask`}
																					className="form-select-sm sm-1"
																					value={
																						values[o] &&
																						values[o]?.buttonsMask & mask.value
																					}
																					error={
																						errors[o] || errors[o]?.buttonsMask
																					}
																					isInvalid={
																						errors[o] || errors[o]?.buttonsMask
																					}
																					onChange={(e) => {
																						setFieldValue(
																							`${o}.buttonsMask`,
																							(values[o] &&
																								values[o]?.buttonsMask ^
																									mask.value) | e.target.value,
																						);
																					}}
																				>
																					{BUTTON_MASKS_OPTIONS.map((o, i2) => (
																						<option
																							key={`hotkey-${i}-button${i2}`}
																							value={o.value}
																						>
																							{o.label in currentButtonLabels
																								? currentButtonLabels[o.label]
																								: o.label}
																						</option>
																					))}
																				</Form.Select>
																			</Col>
																			<Col sm="auto">+</Col>
																		</>
																	) : (
																		<></>
																	),
																)}
																<Col sm="auto">
																	<Form.Select
																		name={`${o}.buttonsMask`}
																		className="form-select-sm sm-1"
																		value={0}
																		onChange={(e) => {
																			setFieldValue(
																				`${o}.buttonsMask`,
																				(values[o] && values[o]?.buttonsMask) |
																					e.target.value,
																			);
																		}}
																	>
																		{BUTTON_MASKS_OPTIONS.map((o, i2) => (
																			<option
																				key={`hotkey-${i}-buttonZero-${i2}`}
																				value={o.value}
																			>
																				{o.label in currentButtonLabels
																					? currentButtonLabels[o.label]
																					: o.label}
																			</option>
																		))}
																	</Form.Select>
																</Col>
																<Col sm="auto">=</Col>
																<Col sm="auto">
																	<Form.Select
																		name={`${o}.action`}
																		className="form-select-sm"
																		value={values[o] && values[o]?.action}
																		onChange={handleChange}
																		isInvalid={errors[o] && errors[o]?.action}
																	>
																		{translatedHotkeyActions.map((o, i) => (
																			<option
																				key={`hotkey-action-${i}`}
																				value={o.value}
																			>
																				{o.label}
																			</option>
																		))}
																	</Form.Select>
																	<Form.Control.Feedback type="invalid">
																		{errors[o] && errors[o]?.action}
																	</Form.Control.Feedback>
																</Col>
																{Boolean(
																	values[o]?.buttonsMask || values[o]?.action,
																) && (
																	<Col>
																		<Button
																			size="sm"
																			onClick={() => {
																				setFieldValue(`${o}.action`, 0);
																				setFieldValue(`${o}.buttonsMask`, 0);
																			}}
																		>
																			{'✕'}
																		</Button>
																	</Col>
																)}
																<Form.Control.Feedback
																	type="invalid"
																	className={errors[o] ? 'd-block' : ''}
																>
																	{errors[o]}
																</Form.Control.Feedback>
															</Form.Group>
														))}
													</div>
													<Form.Check
														label={t('SettingsPage:lock-hotkeys-label')}
														type="switch"
														id="LockHotkeys"
														reverse
														isInvalid={false}
														checked={Boolean(values.lockHotkeys)}
														onChange={(e) => {
															setFieldValue(
																'lockHotkeys',
																e.target.checked ? 1 : 0,
															);
														}}
													/>
													<Button type="submit">
														{t('Common:button-save-label')}
													</Button>
													{saveMessage ? (
														<span className="alert">{saveMessage}</span>
													) : null}
												</Section>
											</Tab.Pane>
										</Tab.Content>
									</Col>
								</Row>
							</Tab.Container>
							<FormContext
								setButtonLabels={setButtonLabels}
								setKeyMappings={setKeyMappings}
							/>
						</Form>
						<Modal size="lg" show={warning.show} onHide={handleWarningClose}>
							<Modal.Header closeButton>
								<Modal.Title>
									{t('SettingsPage:forced-setup-mode-modal-title')}
								</Modal.Title>
							</Modal.Header>
							<Modal.Body>
								<div className="mb-3">
									<Trans
										ns="SettingsPage"
										i18nKey="forced-setup-mode-modal-body"
										components={{ strong: <strong /> }}
										values={{ warningCheckText: WARNING_CHECK_TEXT }}
									/>
								</div>
								<Form.Control
									value={warning.acceptText}
									onChange={setWarningAcceptText}
								></Form.Control>
							</Modal.Body>
							<Modal.Footer>
								<Button
									disabled={warning.acceptText != WARNING_CHECK_TEXT}
									variant="warning"
									onClick={() => handleWarningClose(true, values)}
								>
									{t('Common:button-save-label')}
								</Button>
								<Button
									variant="primary"
									onClick={() =>
										handleWarningClose(false, values, setFieldValue)
									}
								>
									{t('Common:button-dismiss-label')}
								</Button>
							</Modal.Footer>
						</Modal>
					</div>
				)
			}
		</Formik>
	);
}
