import React from 'react';
import { useTranslation } from 'react-i18next';
import { Alert, Button, FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';
import JSEncrypt from 'jsencrypt';
import isNil from 'lodash/isNil';
import WebApi from '../Services/WebApi';

import Section from '../Components/Section';

const SHA256 = (ascii) => {
    function rightRotate(value, amount) {
        return (value>>>amount) | (value<<(32 - amount));
    };
    
    var mathPow = Math.pow;
    var maxWord = mathPow(2, 32);
    var lengthProperty = 'length'
    var i, j; // Used as a counter across the whole file
    var result = ''

    var words = [];
    var asciiBitLength = ascii[lengthProperty]*8;
    
    //* caching results is optional - remove/add slash from front of this line to toggle
    // Initial hash value: first 32 bits of the fractional parts of the square roots of the first 8 primes
    // (we actually calculate the first 64, but extra values are just ignored)
    var hash = SHA256.h = SHA256.h || [];
    // Round constants: first 32 bits of the fractional parts of the cube roots of the first 64 primes
    var k = SHA256.k = SHA256.k || [];
    var primeCounter = k[lengthProperty];
    /*/
    var hash = [], k = [];
    var primeCounter = 0;
    //*/

    var isComposite = {};
    for (var candidate = 2; primeCounter < 64; candidate++) {
        if (!isComposite[candidate]) {
            for (i = 0; i < 313; i += candidate) {
                isComposite[i] = candidate;
            }
            hash[primeCounter] = (mathPow(candidate, .5)*maxWord)|0;
            k[primeCounter++] = (mathPow(candidate, 1/3)*maxWord)|0;
        }
    }
    
    ascii += '\x80' // Append Ƈ' bit (plus zero padding)
    while (ascii[lengthProperty]%64 - 56) ascii += '\x00' // More zero padding
    for (i = 0; i < ascii[lengthProperty]; i++) {
        j = ascii.charCodeAt(i);
        if (j>>8) return; // ASCII check: only accept characters in range 0-255
        words[i>>2] |= j << ((3 - i)%4)*8;
    }
    words[words[lengthProperty]] = ((asciiBitLength/maxWord)|0);
    words[words[lengthProperty]] = (asciiBitLength)
    
    // process each chunk
    for (j = 0; j < words[lengthProperty];) {
        var w = words.slice(j, j += 16); // The message is expanded into 64 words as part of the iteration
        var oldHash = hash;
        // This is now the undefinedworking hash", often labelled as variables a...g
        // (we have to truncate as well, otherwise extra entries at the end accumulate
        hash = hash.slice(0, 8);
        
        for (i = 0; i < 64; i++) {
            var i2 = i + j;
            // Expand the message into 64 words
            // Used below if 
            var w15 = w[i - 15], w2 = w[i - 2];

            // Iterate
            var a = hash[0], e = hash[4];
            var temp1 = hash[7]
                + (rightRotate(e, 6) ^ rightRotate(e, 11) ^ rightRotate(e, 25)) // S1
                + ((e&hash[5])^((~e)&hash[6])) // ch
                + k[i]
                // Expand the message schedule if needed
                + (w[i] = (i < 16) ? w[i] : (
                        w[i - 16]
                        + (rightRotate(w15, 7) ^ rightRotate(w15, 18) ^ (w15>>>3)) // s0
                        + w[i - 7]
                        + (rightRotate(w2, 17) ^ rightRotate(w2, 19) ^ (w2>>>10)) // s1
                    )|0
                );
            // This is only used once, so *could* be moved below, but it only saves 4 bytes and makes things unreadble
            var temp2 = (rightRotate(a, 2) ^ rightRotate(a, 13) ^ rightRotate(a, 22)) // S0
                + ((a&hash[1])^(a&hash[2])^(hash[1]&hash[2])); // maj
            
            hash = [(temp1 + temp2)|0].concat(hash); // We don't bother trimming off the extra ones, they're harmless as long as we're truncating when we do the slice()
            hash[4] = (hash[4] + temp1)|0;
        }
        
        for (i = 0; i < 8; i++) {
            hash[i] = (hash[i] + oldHash[i])|0;
        }
    }
    
    for (i = 0; i < 8; i++) {
        for (j = 3; j + 1; j--) {
            var b = (hash[i]>>(j*8))&255;
            result += ((b < 16) ? 0 : '') + b.toString(16);
        }
    }
    return result;
};

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
		const hashed = SHA256(bytes);
		const signNonce = key.sign(hashed, SHA256, 'sha256');

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
