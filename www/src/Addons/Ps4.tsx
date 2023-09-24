import React from 'react';
import { useTranslation } from 'react-i18next';
import { Alert, Button, FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';
import JSEncrypt from 'jsencrypt';
import CryptoJS from 'crypto-js';
import isNil from 'lodash/isNil';
import WebApi from '../Services/WebApi';

import Section from '../Components/Section';

const verifyAndSavePS4 = async () => {
	let PS4Key = document.getElementById('ps4key-input');
	let PS4Serial = document.getElementById('ps4serial-input');
	let PS4Signature = document.getElementById('ps4signature-input');

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
		var out = [];
		var mask = BigInt('4294967295');
		var zero = BigInt('0');
		while (num !== zero) {
			out.push((num & mask).toString(16).padStart(8, '0'));
			num = num >> BigInt(32);
		}
		return out;
	}

	function hexToBytes(hex) {
		let bytes = [];
		for (let c = 0; c < hex.length; c += 2)
			bytes.push(parseInt(hex.substr(c, 2), 16));
		return bytes;
	}

	function mbedmpi2b64(mpi) {
		var arr = new Uint8Array(mpi.length * 4);
		var cnt = 0;
		for (let i = 0; i < mpi.length; i++) {
			let bytes = hexToBytes(mpi[i]);
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
			loadFile(PS4Key.files[0], true),
			loadFile(PS4Signature.files[0], false),
			loadFile(PS4Serial.files[0], true),
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
		const hashed = CryptoJS.SHA256(bytes);
		const signNonce = key.sign(hashed, CryptoJS.SHA256, 'sha256');

		if (signNonce === false) {
			throw new Error('Bad Private Key');
		}

		// Private key worked!

		// Translate these to BigInteger
		var N = BigInt(String(key.key.n));
		var E = BigInt(String(key.key.e));
		var P = BigInt(String(key.key.p));
		var Q = BigInt(String(key.key.q));

		let serialBin = hexToBytes(serial);

		let success = await WebApi.setPS4Options({
			N: mbedmpi2b64(int2mbedmpi(N)),
			E: mbedmpi2b64(int2mbedmpi(E)),
			P: mbedmpi2b64(int2mbedmpi(P)),
			Q: mbedmpi2b64(int2mbedmpi(Q)),
			serial: btoa(String.fromCharCode(...new Uint8Array(serialBin))),
			signature: btoa(signature),
		});

		if (success) {
			document.getElementById('ps4alert').textContent =
				'Verified and Saved PS4 Mode! Reboot to take effect';
			document.getElementById('save').click();
		} else {
			throw Error('ERROR: Failed to upload the key to the board');
		}
	} catch (e) {
		document.getElementById('ps4alert').textContent =
			`ERROR: Could not verify required files: ${e}`;
	}
};

export const ps4Scheme = {
	PS4ModeAddonEnabled: yup.number().required().label('PS4 Mode Add-on Enabled'),
};

export const ps4State = {
	PS4ModeAddonEnabled: 0,
};

const Ps4 = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:ps4-mode-header-text')}>
			<div id="PS4ModeOptions" hidden={!values.PS4ModeAddonEnabled}>
				<Alert variant="danger">
					<Alert.Heading>{t('AddonsConfig:ps4-mode-sub-header')}</Alert.Heading>
					{t('AddonsConfig:ps4-mode-sub-header-text')}
				</Alert>
				<p>{t('AddonsConfig:ps4-mode-text')}</p>
				<Row className="mb-3">
					<div className="col-sm-3 mb-3">
						{t('AddonsConfig:ps4-mode-private-key-label')}:
						<input type="file" id="ps4key-input" accept="*/*" />
					</div>
					<div className="col-sm-3 mb-3">
						{t('AddonsConfig:ps4-mode-serial-number-label')}:
						<input type="file" id="ps4serial-input" accept="*/*" />
					</div>
					<div className="col-sm-3 mb-3">
						{t('AddonsConfig:ps4-mode-signature-label')}:
						<input type="file" id="ps4signature-input" accept="*/*" />
					</div>
				</Row>
				<Row className="mb-3">
					<div className="col-sm-3 mb-3">
						<Button type="button" onClick={verifyAndSavePS4}>
							{t('Common:button-verify-save-label')}
						</Button>
					</div>
				</Row>
				<Row className="mb-3">
					<div className="col-sm-3 mb-3">
						<span id="ps4alert"></span>
					</div>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="PS4ModeAddonEnabledButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.PS4ModeAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('PS4ModeAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Ps4;
