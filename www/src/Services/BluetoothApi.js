const STATUS_FLAGS = [
	'supported',
	'enabled',
	'initializationFailed',
	'bondSaveFailed',
	'connected',
	'notifying',
	'pairingMode',
	'hasBondedPeers',
];

function parseFlag(value) {
	// Firmware writeDoc() encodes booleans as 0/1.
	if (typeof value === 'boolean') return value;
	if (value === 0 || value === 1) return value === 1;
	throw new Error('Invalid Bluetooth flag value');
}

export function parseBluetoothStatus(status) {
	if (
		!status ||
		typeof status !== 'object' ||
		Array.isArray(status) ||
		!Number.isInteger(status.bondCount) ||
		status.bondCount < 0 ||
		typeof status.deviceName !== 'string'
	) {
		throw new Error('Invalid Bluetooth status response');
	}

	const result = { ...status };
	for (const flag of STATUS_FLAGS) {
		result[flag] = parseFlag(status[flag]);
	}
	return result;
}

export function parseBluetoothControlResponse(response) {
	return {
		...parseBluetoothStatus(response),
		success: parseFlag(response.success),
	};
}
