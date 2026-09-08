import { useEffect, useRef, useState } from 'react';
import {
	Alert,
	Button,
	Col,
	Modal,
	ProgressBar,
	Row,
	Spinner,
} from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import invert from 'lodash/invert';

import WebApi from '../Services/WebApi';
import { BUTTON_ACTIONS } from '../Data/Pins';
import { HE_PRESETS, DEFAULT_PRESET_LEVEL, getPreset } from '../Data/HEPresets';
import useHETriggerStore from '../Store/useHETriggerStore';
import useHEProfileStore from '../Store/useHEProfileStore';

import './HECalibration.scss';

const POLL_INTERVAL_MS = 100;

type CalChannel = {
	id: number;
	raw: number;
	idle: number;
	stdDev: number;
	maxDeviation: number;
	moved: boolean;
	unstable: boolean;
};

type CalStatus = {
	mode: 'off' | 'idle' | 'press' | 'done';
	elapsedMs: number;
	idleDurationMs: number;
	channels: CalChannel[];
	assignedCount: number;
	movedCount: number;
	error?: string;
};

// The addon form values, as supplied by the parent's Formik state.
type HEFormValues = {
	muxChannels: number;
	muxSelectPin0: number;
	muxSelectPin1: number;
	muxSelectPin2: number;
	muxSelectPin3: number;
	muxADCPin0: number;
	muxADCPin1: number;
	muxADCPin2: number;
	muxADCPin3: number;
	heTriggerSmoothing: number;
	heTriggerSmoothingFactor: number;
};

type Props = {
	showModal: boolean;
	setShowModal: (show: boolean) => void;
	values: HEFormValues;
};

// Wizard steps
const STEP_READY = 0;
const STEP_BASELINE = 1;
const STEP_PRESS = 2;
const STEP_FEEL = 3;

const actionLabel = (actionId: number) => {
	const label = invert(BUTTON_ACTIONS)[actionId];
	return label ? label.replace('BUTTON_PRESS_', '') : `#${actionId}`;
};

const HECalibrationWizard = ({ showModal, setShowModal, values }: Props) => {
	const { t } = useTranslation('');
	const { triggers, fetchHETriggers } = useHETriggerStore();
	const { profiles, fetchHEProfiles, saveHEProfiles } = useHEProfileStore();

	const [step, setStep] = useState(STEP_READY);
	const [status, setStatus] = useState<CalStatus | null>(null);
	const [presetLevel, setPresetLevel] = useState(DEFAULT_PRESET_LEVEL);
	const [error, setError] = useState('');
	const [busy, setBusy] = useState(false);
	const timerId = useRef<ReturnType<typeof setInterval> | undefined>(undefined);

	// A channel needs calibrating if it is bound in ANY profile, not just the base
	// one: a profile for a game that uses fewer buttons would otherwise leave the
	// rest uncalibrated even though other profiles rely on them. This mirrors
	// HETriggerAddon::isChannelAssigned, which is what the firmware actually
	// sweeps -- the two must agree or the wizard shows fewer tiles than it captures.
	const isAssignedAnywhere = (index: number) => {
		if (triggers[index]?.action !== -10) return true;
		return profiles.some(
			(profile, profileIndex) =>
				profileIndex > 0 &&
				profile.actions?.[index] !== undefined &&
				profile.actions[index] !== -10,
		);
	};

	// Label a channel by whichever profile actually binds it.
	const effectiveAction = (index: number) => {
		if (triggers[index]?.action !== -10)
			return triggers[index].action as number;
		for (let p = 1; p < profiles.length; p++) {
			const action = profiles[p]?.actions?.[index];
			if (action !== undefined && action !== -10) return action;
		}
		return -10;
	};

	const assignedChannels = triggers
		.map((trigger, index) => ({ trigger, index }))
		.filter(({ index }) => isAssignedAnywhere(index));

	// Where a channel physically lives, for the tile subtitle.
	const channelLocation = (index: number) => {
		const muxChannels = values['muxChannels'];
		if (muxChannels > 1) {
			return `M${Math.floor(index / muxChannels)}:C${index % muxChannels}`;
		}
		return `ADC ${index}`;
	};

	const stopPolling = () => {
		if (timerId.current) {
			clearInterval(timerId.current);
			timerId.current = undefined;
		}
	};

	const poll = async () => {
		try {
			const data: CalStatus = await WebApi.getHECalibrationStatus();
			if (data?.error) {
				setError(data.error);
				stopPolling();
				return;
			}
			setStatus(data);
			// The firmware advances from the idle phase on its own once the sampling
			// window elapses, so follow its reported mode rather than a local timer.
			//
			// This deliberately does not read `step`: poll() is handed to setInterval
			// once, so it closes over the values from that render forever, and a
			// `step === STEP_BASELINE` guard here would compare against the stale
			// initial value and never match. Deriving the step from the reported mode
			// inside the updater keeps it correct regardless of when poll() was made.
			if (data.mode === 'press') {
				setStep((current) =>
					current === STEP_BASELINE ? STEP_PRESS : current,
				);
			}
		} catch (e) {
			setError(String(e));
			stopPolling();
		}
	};

	const startPolling = () => {
		stopPolling();
		timerId.current = setInterval(poll, POLL_INTERVAL_MS);
	};

	// Tear down on unmount so a closed modal cannot keep polling the device.
	useEffect(() => stopPolling, []);

	useEffect(() => {
		if (!showModal) {
			stopPolling();
			setStep(STEP_READY);
			setStatus(null);
			setError('');
		}
	}, [showModal]);

	const beginCalibration = async () => {
		setBusy(true);
		setError('');
		try {
			// Push the current (possibly unsaved) mux/pin config first, so reads work
			// before the user has committed the addon form.
			await WebApi.setHETriggerOptions({
				muxChannels: values['muxChannels'],
				muxSelectPin0: values['muxSelectPin0'],
				muxSelectPin1: values['muxSelectPin1'],
				muxSelectPin2: values['muxSelectPin2'],
				muxSelectPin3: values['muxSelectPin3'],
				muxADCPin0: values['muxADCPin0'],
				muxADCPin1: values['muxADCPin1'],
				muxADCPin2: values['muxADCPin2'],
				muxADCPin3: values['muxADCPin3'],
				heTriggerSmoothing: values['heTriggerSmoothing'],
				heTriggerSmoothingFactor: values['heTriggerSmoothingFactor'],
			});

			// Commit the action assignments too. The firmware decides which channels
			// to sweep by reading the stored bindings, but assigning a button in the
			// UI only updates the browser store until it is saved -- so without this,
			// a newly assigned button is still unset as far as the sweep is concerned
			// and gets skipped entirely. Bindings live in the profile store, which is
			// what owns triggers[].action on the firmware side.
			//
			// Fetch first. This modal can be opened before anything has populated the
			// profile store, and saving an unpopulated store would post every binding
			// as NONE -- wiping the profiles on the device.
			await fetchHEProfiles();
			await saveHEProfiles();

			const result = await WebApi.startHECalibration();
			if (result?.error) {
				setError(result.error);
				return;
			}
			setStep(STEP_BASELINE);
			startPolling();
		} catch (e) {
			setError(String(e));
		} finally {
			setBusy(false);
		}
	};

	const finishPressPhase = async () => {
		setBusy(true);
		try {
			await WebApi.advanceHECalibration('finish');
			stopPolling();
			setStep(STEP_FEEL);
		} catch (e) {
			setError(String(e));
		} finally {
			setBusy(false);
		}
	};

	const applyPreset = async () => {
		setBusy(true);
		try {
			const preset = getPreset(presetLevel);
			await WebApi.applyHECalibration({
				actuationPoint: preset.actuationPoint,
				rtPressSensitivity: preset.rtPressSensitivity,
				rtReleaseSensitivity: preset.rtReleaseSensitivity,
				continuousRapidTrigger: preset.continuousRapidTrigger,
			});
			await fetchHETriggers();
			setShowModal(false);
		} catch (e) {
			setError(String(e));
		} finally {
			setBusy(false);
		}
	};

	const cancel = async () => {
		stopPolling();
		try {
			await WebApi.advanceHECalibration('abort');
		} catch {
			// Aborting is best effort; the firmware also times out on its own.
		}
		setShowModal(false);
	};

	const channelById = (id: number) =>
		status?.channels?.find((c) => c.id === id);

	const baselineProgress = status
		? Math.min(
				100,
				Math.round((status.elapsedMs / (status.idleDurationMs || 2000)) * 100),
			)
		: 0;

	const unstableChannels = (status?.channels || []).filter((c) => c.unstable);

	return (
		<Modal size="lg" show={showModal} onHide={cancel} className="he-modal">
			<Modal.Header closeButton>
				<Modal.Title>{t('HETrigger:wizard-title')}</Modal.Title>
			</Modal.Header>
			<Modal.Body>
				{error && <Alert variant="danger">{error}</Alert>}

				{step === STEP_READY && (
					<div>
						<p>{t('HETrigger:wizard-intro')}</p>
						<p className="text-muted">
							{t('HETrigger:wizard-assigned-count', {
								count: assignedChannels.length,
							})}
						</p>
						<div className="he-channel-grid">
							{assignedChannels.map(({ index }) => (
								<div key={`ready-${index}`} className="he-channel-tile">
									<div className="he-channel-name">
										{actionLabel(effectiveAction(index))}
									</div>
									<div className="he-channel-meta">
										{channelLocation(index)}
									</div>
								</div>
							))}
						</div>
					</div>
				)}

				{step === STEP_BASELINE && (
					<div>
						<h5>{t('HETrigger:wizard-baseline-heading')}</h5>
						<p>{t('HETrigger:wizard-baseline-body')}</p>
						<ProgressBar
							now={baselineProgress}
							label={`${baselineProgress}%`}
						/>
						<div className="he-channel-grid mt-3">
							{assignedChannels.map(({ index }) => {
								const channel = channelById(index);
								return (
									<div key={`base-${index}`} className="he-channel-tile">
										<div className="he-channel-name">
											{actionLabel(effectiveAction(index))}
										</div>
										<div className="he-channel-value">
											{channel?.raw ?? '—'}
										</div>
									</div>
								);
							})}
						</div>
					</div>
				)}

				{step === STEP_PRESS && (
					<div>
						<h5>{t('HETrigger:wizard-press-heading')}</h5>
						<p>{t('HETrigger:wizard-press-body')}</p>
						{unstableChannels.length > 0 && (
							<Alert variant="warning">
								{t('HETrigger:wizard-unstable-warning', {
									channels: unstableChannels.map((c) => c.id).join(', '),
								})}
							</Alert>
						)}
						<div className="he-progress-line">
							{t('HETrigger:wizard-press-progress', {
								moved: status?.movedCount ?? 0,
								total: status?.assignedCount ?? assignedChannels.length,
							})}
						</div>
						<div className="he-channel-grid mt-2">
							{assignedChannels.map(({ index }) => {
								const channel = channelById(index);
								const deviation = Math.abs(channel?.maxDeviation ?? 0);
								const captured = channel?.moved ?? false;
								return (
									<div
										key={`press-${index}`}
										className={`he-channel-tile ${captured ? 'captured' : ''}`}
									>
										<div className="he-channel-name">
											{actionLabel(effectiveAction(index))}
										</div>
										<ProgressBar
											now={Math.min(100, (deviation / 2000) * 100)}
											variant={captured ? 'success' : 'secondary'}
											className="he-channel-bar"
										/>
										<div className="he-channel-meta">
											{captured
												? t('HETrigger:wizard-captured', { span: deviation })
												: t('HETrigger:wizard-waiting')}
										</div>
									</div>
								);
							})}
						</div>
					</div>
				)}

				{step === STEP_FEEL && (
					<div>
						<h5>{t('HETrigger:wizard-feel-heading')}</h5>
						<p>{t('HETrigger:wizard-feel-body')}</p>
						<Row>
							{HE_PRESETS.map((preset) => (
								<Col xs={6} md={4} key={preset.level} className="mb-2">
									<div
										role="button"
										tabIndex={0}
										onClick={() => setPresetLevel(preset.level)}
										onKeyDown={(e) => {
											if (e.key === 'Enter' || e.key === ' ')
												setPresetLevel(preset.level);
										}}
										className={`he-preset-card ${
											presetLevel === preset.level ? 'selected' : ''
										}`}
									>
										<div className="he-preset-name">
											{t('HETrigger:preset-level', { level: preset.level })}
										</div>
										<div className="he-preset-detail">
											{t('HETrigger:preset-actuation-label', {
												percent: preset.actuationPoint,
											})}
										</div>
										<div className="he-preset-desc">
											{t('HETrigger:preset-level-desc', {
												press: preset.rtPressSensitivity,
											})}
										</div>
									</div>
								</Col>
							))}
						</Row>
					</div>
				)}
			</Modal.Body>
			<Modal.Footer>
				<Button variant="secondary" onClick={cancel} disabled={busy}>
					{t('HETrigger:wizard-cancel')}
				</Button>
				{step === STEP_READY && (
					<Button
						onClick={beginCalibration}
						disabled={busy || assignedChannels.length === 0}
					>
						{busy && <Spinner size="sm" className="me-2" />}
						{t('HETrigger:wizard-start')}
					</Button>
				)}
				{step === STEP_PRESS && (
					<Button onClick={finishPressPhase} disabled={busy}>
						{t('HETrigger:wizard-finish-press')}
					</Button>
				)}
				{step === STEP_FEEL && (
					<Button onClick={applyPreset} disabled={busy}>
						{busy && <Spinner size="sm" className="me-2" />}
						{t('HETrigger:wizard-apply')}
					</Button>
				)}
			</Modal.Footer>
		</Modal>
	);
};

export default HECalibrationWizard;
