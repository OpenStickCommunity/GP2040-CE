import React, { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Button, Row, Col, Alert } from 'react-bootstrap';

import Section from '../Components/Section';
import WebApi from '../Services/WebApi';

export default function BluetoothSettingsPage() {
	const { t } = useTranslation('');
	const [loading, setLoading] = useState(true);
	const [bluetoothData, setBluetoothData] = useState({
		bluetoothEnabled: false,
		switchBtPaired: false,
		switchBtMac: '',
		hidBtPaired: false,
		hidBtMac: '',
	});

	useEffect(() => {
		async function fetchData() {
			setLoading(true);
			const data = await WebApi.getBluetoothSettings();
			setBluetoothData(data);
			setLoading(false);
		}
		fetchData();
	}, []);

	const handleClearSwitchPairing = async () => {
		if (window.confirm(t('BluetoothSettings:confirm-clear-switch'))) {
			await WebApi.clearSwitchBtPairing();
			// Refresh data
			const data = await WebApi.getBluetoothSettings();
			setBluetoothData(data);
		}
	};

	const handleClearHidPairing = async () => {
		if (window.confirm(t('BluetoothSettings:confirm-clear-hid'))) {
			await WebApi.clearHidBtPairing();
			// Refresh data
			const data = await WebApi.getBluetoothSettings();
			setBluetoothData(data);
		}
	};

	if (loading) {
		return <div>Loading...</div>;
	}

	if (!bluetoothData.bluetoothEnabled) {
		return (
			<Section title={t('BluetoothSettings:header-text')}>
				<Alert variant="info">
					{t('BluetoothSettings:bluetooth-not-enabled')}
				</Alert>
			</Section>
		);
	}

	return (
		<Section title={t('BluetoothSettings:header-text')}>
			<p className="card-text mb-4">
				{t('BluetoothSettings:sub-header-text')}
			</p>

			{/* Nintendo Switch Bluetooth */}
			<div className="mb-4 p-3 border rounded">
				<h5>{t('BluetoothSettings:switch-bt-title')}</h5>
				<Row className="align-items-center">
					<Col md={8}>
						{bluetoothData.switchBtPaired ? (
							<>
								<span className="text-success me-2">●</span>
								{t('BluetoothSettings:paired-to')} <code>{bluetoothData.switchBtMac}</code>
							</>
						) : (
							<>
								<span className="text-muted me-2">○</span>
								{t('BluetoothSettings:not-paired')}
							</>
						)}
					</Col>
					<Col md={4} className="text-end">
						{bluetoothData.switchBtPaired && (
							<Button
								variant="outline-danger"
								size="sm"
								onClick={handleClearSwitchPairing}
							>
								{t('BluetoothSettings:clear-pairing')}
							</Button>
						)}
					</Col>
				</Row>
			</div>

			{/* HID Bluetooth */}
			<div className="p-3 border rounded">
				<h5>{t('BluetoothSettings:hid-bt-title')}</h5>
				<Row className="align-items-center">
					<Col md={8}>
						{bluetoothData.hidBtPaired ? (
							<>
								<span className="text-success me-2">●</span>
								{t('BluetoothSettings:paired-to')} <code>{bluetoothData.hidBtMac}</code>
							</>
						) : (
							<>
								<span className="text-muted me-2">○</span>
								{t('BluetoothSettings:not-paired')}
							</>
						)}
					</Col>
					<Col md={4} className="text-end">
						{bluetoothData.hidBtPaired && (
							<Button
								variant="outline-danger"
								size="sm"
								onClick={handleClearHidPairing}
							>
								{t('BluetoothSettings:clear-pairing')}
							</Button>
						)}
					</Col>
				</Row>
			</div>
		</Section>
	);
}

