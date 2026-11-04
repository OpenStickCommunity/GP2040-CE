import { useEffect, useRef, useState } from 'react';
import { Alert, Button, Col, Modal, ProgressBar, Row } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';

import WebApi from '../Services/WebApi';
import useHETriggerStore, {
	isTriggerScanned,
	Trigger,
} from '../Store/useHETriggerStore';
import { recalibrateTrigger } from '../Addons/HETriggerRules';

// ADC counts a key must travel before it counts as calibrated
export const MIN_CALIBRATION_SPAN = 400;
const POLL_INTERVAL_MS = 50;

type Reading = {
	// Taken with keys released; tells idle from pressed
	first: number;
	min: number;
	max: number;
	current: number;
};

type HEQuickCalibrationProps = {
	show: boolean;
	onHide: () => void;
	triggers: Trigger[];
	values: { [key: string]: unknown };
	muxChannels: number;
	getTriggerLabel: (index: number) => string;
};

// Idle is whichever extreme is closer to the first reading; the other is full press.
const getCalibration = (reading: Reading) => {
	const idleIsMin =
		Math.abs(reading.first - reading.min) <=
		Math.abs(reading.first - reading.max);
	return idleIsMin
		? { idle: reading.min, pressed: reading.max }
		: { idle: reading.max, pressed: reading.min };
};

const isCalibrated = (reading?: Reading) =>
	reading !== undefined && reading.max - reading.min >= MIN_CALIBRATION_SPAN;

const HEQuickCalibration = ({
	show,
	onHide,
	triggers,
	values,
	muxChannels,
	getTriggerLabel,
}: HEQuickCalibrationProps) => {
	const { t } = useTranslation('');
	const setHETrigger = useHETriggerStore((state) => state.setHETrigger);
	const [readings, setReadings] = useState<Record<number, Reading>>({});
	const [error, setError] = useState('');
	const readingsRef = useRef<Record<number, Reading>>({});
	const timerId = useRef<ReturnType<typeof setInterval>>();
	const inFlight = useRef(false);

	const targets = triggers
		.map((trigger, index) => ({ trigger, index }))
		.filter(
			({ trigger, index }) =>
				trigger.action !== -10 && isTriggerScanned(index, muxChannels, values),
		)
		.map(({ index }) => index);
	const calibratedCount = targets.filter((i) =>
		isCalibrated(readings[i]),
	).length;

	const resetReadings = () => {
		readingsRef.current = {};
		setReadings({});
	};

	const stopPolling = () => {
		if (timerId.current) clearInterval(timerId.current);
		timerId.current = undefined;
	};

	const poll = async () => {
		if (inFlight.current) return;
		inFlight.current = true;
		try {
			const response = await WebApi.getHETriggerVoltages();
			const voltages: number[] | undefined = response?.data?.voltages;
			if (!Array.isArray(voltages)) {
				setError(response?.data?.error || t('HETrigger:quick-read-error'));
				return;
			}
			setError('');
			const next = { ...readingsRef.current };
			targets.forEach((index) => {
				const value = voltages[index];
				if (typeof value !== 'number' || value < 0) return;
				const previous = next[index];
				next[index] = previous
					? {
							first: previous.first,
							min: Math.min(previous.min, value),
							max: Math.max(previous.max, value),
							current: value,
						}
					: { first: value, min: value, max: value, current: value };
			});
			readingsRef.current = next;
			setReadings(next);
		} catch (e) {
			setError(t('HETrigger:quick-read-error'));
		} finally {
			inFlight.current = false;
		}
	};

	const startPolling = async () => {
		resetReadings();
		setError('');
		// Same pins the single-trigger calibration sends, so unsaved form values are used
		await WebApi.setHETriggerOptions({
			muxChannels: values['muxChannels'],
			separateSelectPins: values['separateSelectPins'],
			muxes: values['muxes'],
			muxADCPin0: values['muxADCPin0'],
			muxADCPin1: values['muxADCPin1'],
			muxADCPin2: values['muxADCPin2'],
			muxADCPin3: values['muxADCPin3'],
			heTriggerSmoothing: values['heTriggerSmoothing'],
			heTriggerSmoothingFactor: values['heTriggerSmoothingFactor'],
		});
		stopPolling();
		timerId.current = setInterval(poll, POLL_INTERVAL_MS);
	};

	useEffect(() => {
		if (show) startPolling();
		else stopPolling();
		return stopPolling;
	}, [show]);

	const finish = () => {
		stopPolling();
		targets.forEach((index) => {
			const reading = readingsRef.current[index];
			if (!isCalibrated(reading)) return;
			const { idle, pressed } = getCalibration(reading);
			setHETrigger({
				id: index,
				...recalibrateTrigger(triggers[index], idle, pressed),
			});
		});
		onHide();
	};

	const location = (index: number) =>
		muxChannels > 1
			? `${t('HETrigger:multiplexer-label')} ${Math.floor(index / muxChannels)} - ${t('HETrigger:channel-label')} ${index % muxChannels}`
			: `${t('HETrigger:direct-label')} - ADC ${values[`muxADCPin${index}`]}`;

	return (
		<Modal
			size="xl"
			centered
			show={show}
			onHide={() => {
				stopPolling();
				onHide();
			}}
		>
			<Modal.Header closeButton>
				<Modal.Title>
					{t('HETrigger:quick-calibration-header-text')}
				</Modal.Title>
			</Modal.Header>
			<Modal.Body>
				<p>{t('HETrigger:quick-calibration-instructions')}</p>
				{error && <Alert variant="danger">{error}</Alert>}
				<Row className="g-2">
					{targets.map((index) => {
						const reading = readings[index];
						const calibrated = isCalibrated(reading);
						const calibration = reading ? getCalibration(reading) : null;
						const range = calibration
							? calibration.pressed - calibration.idle
							: 0;
						const percent =
							reading && calibration && range !== 0
								? ((reading.current - calibration.idle) / range) * 100
								: 0;
						return (
							<Col xs={12} sm={6} lg={4} xl={3} key={`he-quick-${index}`}>
								<div
									className={`border rounded p-2 h-100 ${
										calibrated ? 'border-success bg-success-subtle' : ''
									}`}
								>
									<div className="d-flex justify-content-between">
										<strong>{getTriggerLabel(index)}</strong>
										{calibrated && <span aria-hidden="true">✅</span>}
									</div>
									<div className="small text-muted mb-1">{location(index)}</div>
									<div className="small">
										{t('HETrigger:quick-raw-label')}: {reading?.current ?? '-'}
									</div>
									<div className="small">
										{t('HETrigger:quick-min-label')}: {reading?.min ?? '-'} /{' '}
										{t('HETrigger:quick-max-label')}: {reading?.max ?? '-'}
									</div>
									<ProgressBar
										className="mt-1"
										style={{ height: '0.5rem' }}
										variant={calibrated ? 'success' : 'warning'}
										now={Math.min(Math.max(percent, 0), 100)}
									/>
								</div>
							</Col>
						);
					})}
				</Row>
				{targets.length === 0 && (
					<Alert variant="warning">{t('HETrigger:quick-no-triggers')}</Alert>
				)}
			</Modal.Body>
			<Modal.Footer>
				<span className="me-auto">
					{t('HETrigger:quick-calibrated-count', {
						count: calibratedCount,
						total: targets.length,
					})}
				</span>
				<Button variant="danger" onClick={resetReadings}>
					{t('HETrigger:restart-text')}
				</Button>
				<Button
					variant="secondary"
					onClick={() => {
						stopPolling();
						onHide();
					}}
				>
					{t('HETrigger:quick-cancel-button')}
				</Button>
				<Button variant="success" onClick={finish}>
					{t('HETrigger:quick-finish-button')}
				</Button>
			</Modal.Footer>
		</Modal>
	);
};

export default HEQuickCalibration;
