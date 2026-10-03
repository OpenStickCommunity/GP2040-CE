import assert from 'node:assert/strict';
import test from 'node:test';
import {
	parseBluetoothStatus,
	parseBluetoothControlResponse,
} from '../../src/Services/BluetoothApi.js';

const boardStatus = {
	supported: 1,
	enabled: 1,
	initializationFailed: 0,
	bondSaveFailed: 0,
	connected: 0,
	notifying: 0,
	pairingMode: 0,
	hasBondedPeers: 0,
	bondCount: 0,
	deviceName: 'GP2040-CE Gamepad',
};

const expectedStatus = {
	supported: true,
	enabled: true,
	initializationFailed: false,
	bondSaveFailed: false,
	connected: false,
	notifying: false,
	pairingMode: false,
	hasBondedPeers: false,
	bondCount: 0,
	deviceName: 'GP2040-CE Gamepad',
};

test('parses the status captured from the Pimoroni hardware', () => {
	assert.deepEqual(parseBluetoothStatus(boardStatus), expectedStatus);
	assert.equal(boardStatus.supported, 1);
	assert.equal(boardStatus.pairingMode, 0);
});

test('accepts boolean responses without changing their meaning', () => {
	assert.deepEqual(parseBluetoothStatus(expectedStatus), expectedStatus);
});

test('normalizes every numeric status flag in both states', () => {
	for (const flag of Object.keys(expectedStatus).filter(
		(key) => typeof expectedStatus[key] === 'boolean',
	)) {
		for (const value of [0, 1]) {
			assert.equal(
				parseBluetoothStatus({ ...boardStatus, [flag]: value })[flag],
				value === 1,
			);
		}
	}
});

test('preserves the device name and saved-device count', () => {
	const status = parseBluetoothStatus({
		...boardStatus,
		bondCount: 3,
		hasBondedPeers: 1,
	});
	assert.equal(status.bondCount, 3);
	assert.equal(status.hasBondedPeers, true);
	assert.equal(status.deviceName, boardStatus.deviceName);
});

test('normalizes numeric and boolean control success', () => {
	for (const success of [0, 1, false, true]) {
		assert.equal(
			parseBluetoothControlResponse({ ...boardStatus, success }).success,
			success === 1 || success === true,
		);
	}
});

test('a firmware command failure stays a failure and retains its error', () => {
	const response = parseBluetoothControlResponse({
		...boardStatus,
		success: 0,
		error: 'invalid-request',
	});
	assert.equal(response.success, false);
	assert.equal(response.error, 'invalid-request');
});

test('rejects missing flags and non-boolean values other than 0/1', () => {
	for (const value of [undefined, null, -1, 2, '0', '1', '', {}, []]) {
		assert.throws(() =>
			parseBluetoothStatus({ ...boardStatus, pairingMode: value }),
		);
		assert.throws(() =>
			parseBluetoothControlResponse({ ...boardStatus, success: value }),
		);
	}
});

test('rejects malformed status responses instead of inventing defaults', () => {
	for (const status of [
		null,
		undefined,
		{},
		[],
		{ error: 'unavailable' },
		{ ...boardStatus, bondCount: '0' },
		{ ...boardStatus, bondCount: -1 },
		{ ...boardStatus, bondCount: 0.5 },
		{ ...boardStatus, deviceName: null },
	]) {
		assert.throws(() => parseBluetoothStatus(status));
	}
});
