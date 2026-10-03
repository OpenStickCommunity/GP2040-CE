import { useCallback, useEffect, useRef, useState } from 'react';
import { Alert, Badge, Button, Card, Modal, Spinner } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import WebApi from '../Services/WebApi';

type BluetoothStatus = {
	supported: boolean;
	enabled: boolean;
	initializationFailed: boolean;
	bondSaveFailed: boolean;
	connected: boolean;
	notifying: boolean;
	pairingMode: boolean;
	bondCount: number;
	deviceName: string;
};

type Props = {
	onSupportChange: (supported: boolean) => void;
};

export default function BluetoothSettings({ onSupportChange }: Props) {
	const { t } = useTranslation('SettingsPage');
	const [status, setStatus] = useState<BluetoothStatus | null>(null);
	const [statusError, setStatusError] = useState(false);
	const [actionError, setActionError] = useState('');
	const [notice, setNotice] = useState('');
	const [busy, setBusy] = useState(false);
	const [confirmForget, setConfirmForget] = useState(false);
	const mounted = useRef(false);
	const busyRef = useRef(false);
	const refreshing = useRef(false);
	const requestVersion = useRef(0);

	const refreshStatus = useCallback(async () => {
		if (busyRef.current || refreshing.current) return;
		refreshing.current = true;
		const version = ++requestVersion.current;
		try {
			const response: BluetoothStatus | null = await WebApi.getBLEHIDStatus();
			if (!mounted.current || version !== requestVersion.current) return;
			setStatusError(!response);
			onSupportChange(response?.supported === true);
			if (response) setStatus(response);
		} finally {
			refreshing.current = false;
		}
	}, [onSupportChange]);

	useEffect(() => {
		mounted.current = true;
		refreshStatus();
		const timer = setInterval(refreshStatus, 2000);
		return () => {
			mounted.current = false;
			requestVersion.current++;
			clearInterval(timer);
		};
	}, [refreshStatus]);

	const sendControl = async (
		options: { pairingMode: boolean } | { clearBonds: true },
	) => {
		if (busyRef.current) return;
		busyRef.current = true;
		requestVersion.current++;
		setBusy(true);
		setActionError('');
		setNotice('');
		try {
			const response:
				| (BluetoothStatus & { success: boolean; error?: string })
				| null = await WebApi.setBLEHIDControls(options);
			if (!mounted.current) return;
			if (response) {
				setStatus(response);
				setStatusError(false);
				onSupportChange(response.supported);
			} else {
				setStatusError(true);
				onSupportChange(false);
			}
			if (response?.success !== true) {
				const error = response?.error;
				setActionError(
					error === 'not-ready' ||
						error === 'connected' ||
						error === 'unsupported'
						? error
						: 'control-failed',
				);
				return;
			}
			if ('clearBonds' in options) {
				setConfirmForget(false);
				setNotice('forgotten');
			} else if (!options.pairingMode) {
				setNotice('canceled');
			}
		} finally {
			busyRef.current = false;
			if (mounted.current) setBusy(false);
		}
	};

	const phase = !status
		? 'loading'
		: !status.supported
			? 'unsupported'
			: status.initializationFailed
				? 'init-failed'
				: !status.enabled
					? 'initializing'
					: status.connected
						? status.notifying
							? 'connected'
							: 'connecting'
						: status.pairingMode
							? 'pairing'
							: 'ready';
	const available = status?.supported && status.enabled && !statusError;
	const canPair = available && !status?.connected && !busy;
	const canForget =
		canPair && !status?.pairingMode && (status?.bondCount ?? 0) > 0;

	return (
		<Card className="mb-3" aria-labelledby="bluetooth-settings-title">
			<Card.Body>
				<div className="d-flex justify-content-between align-items-center gap-2 mb-3">
					<h5 id="bluetooth-settings-title" className="mb-0">
						{t('ble-status.title')}
					</h5>
					<span role="status" aria-live="polite">
						<Badge
							bg={
								statusError
									? 'danger'
									: phase === 'connected'
										? 'success'
										: phase === 'pairing'
											? 'primary'
											: 'secondary'
							}
						>
							{t(`ble-status.state.${statusError ? 'unavailable' : phase}`)}
						</Badge>
					</span>
				</div>
				<p>{t('ble-status.intro')}</p>
				{statusError && (
					<Alert variant="danger">
						{t('ble-status.status-error')}
						<Button
							variant="link"
							type="button"
							onClick={refreshStatus}
							disabled={busy}
						>
							{t('ble-status.retry')}
						</Button>
					</Alert>
				)}
				{phase === 'unsupported' && (
					<Alert variant="warning">{t('ble-status.unsupported')}</Alert>
				)}
				{phase === 'init-failed' && (
					<Alert variant="danger">{t('ble-status.init-failed')}</Alert>
				)}
				{(phase === 'loading' || phase === 'initializing') && !statusError && (
					<p className="text-muted">
						<Spinner
							as="span"
							animation="border"
							size="sm"
							className="me-2"
							aria-hidden="true"
						/>
						{t('ble-status.initializing')}
					</p>
				)}
				{status?.supported && (
					<>
						<div className="mb-3">
							<div className="text-muted small">
								{t('ble-status.device-name')}
							</div>
							<strong>{status.deviceName}</strong>
						</div>
						{phase === 'pairing' && (
							<Alert variant="info">
								<Spinner
									as="span"
									animation="border"
									size="sm"
									className="me-2"
									aria-hidden="true"
								/>
								{t('ble-status.pairing-help', { name: status.deviceName })}
							</Alert>
						)}
						{phase === 'connected' && (
							<Alert variant="success">{t('ble-status.connected-help')}</Alert>
						)}
						{status.bondSaveFailed && (
							<Alert variant="danger">{t('ble-status.save-failed')}</Alert>
						)}
						{phase === 'connecting' && (
							<Alert variant="info">{t('ble-status.connecting-help')}</Alert>
						)}
						<ol className="ps-3">
							<li>{t('ble-status.step-start')}</li>
							<li>{t('ble-status.step-host', { name: status.deviceName })}</li>
							<li>{t('ble-status.step-play')}</li>
						</ol>
						<Button
							type="button"
							variant={status.pairingMode ? 'outline-secondary' : 'primary'}
							disabled={!canPair}
							onClick={() => sendControl({ pairingMode: !status.pairingMode })}
						>
							{busy && (
								<Spinner
									as="span"
									animation="border"
									size="sm"
									className="me-2"
									aria-hidden="true"
								/>
							)}
							{t(status.pairingMode ? 'ble-status.cancel' : 'ble-status.start')}
						</Button>
						<p className="text-muted small mt-2">
							{t('ble-status.webconfig-note')}
						</p>
						<hr />
						<div className="d-flex justify-content-between align-items-center gap-3 flex-wrap">
							<div>
								<h6 className="mb-1">{t('ble-status.saved-title')}</h6>
								<span className="text-muted">
									{t('ble-status.saved-count', { count: status.bondCount })}
								</span>
							</div>
							<Button
								type="button"
								variant="outline-danger"
								disabled={!canForget}
								onClick={() => setConfirmForget(true)}
							>
								{t('ble-status.forget')}
							</Button>
						</div>
						{status.connected && (
							<p className="text-muted small mt-2 mb-0">
								{t('ble-status.disconnect-help')}
							</p>
						)}
					</>
				)}
				{actionError && (
					<Alert variant="danger" className="mt-3 mb-0">
						{t(`ble-status.errors.${actionError}`)}
					</Alert>
				)}
				{notice && (
					<Alert variant="success" className="mt-3 mb-0" role="status">
						{t(`ble-status.${notice}`)}
					</Alert>
				)}
			</Card.Body>
			<Modal
				show={confirmForget}
				onHide={() => !busy && setConfirmForget(false)}
				centered
			>
				<Modal.Header closeButton={!busy}>
					<Modal.Title>{t('ble-status.forget-title')}</Modal.Title>
				</Modal.Header>
				<Modal.Body>
					{t('ble-status.forget-help')}
					{actionError && (
						<Alert variant="danger" className="mt-3">
							{t(`ble-status.errors.${actionError}`)}
						</Alert>
					)}
				</Modal.Body>
				<Modal.Footer>
					<Button
						type="button"
						variant="secondary"
						disabled={busy}
						onClick={() => setConfirmForget(false)}
					>
						{t('ble-status.keep-devices')}
					</Button>
					<Button
						type="button"
						variant="danger"
						disabled={!canForget}
						onClick={() => sendControl({ clearBonds: true })}
					>
						{t('ble-status.forget')}
					</Button>
				</Modal.Footer>
			</Modal>
		</Card>
	);
}
