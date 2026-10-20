import { useEffect, useState, useRef } from 'react';
import {
	Button,
	Modal,
	Row,
	Col,
	ProgressBar,
	Form,
	Spinner,
} from 'react-bootstrap';
import { useTranslation } from 'react-i18next';

import FormControl from '../Components/FormControl';
import FormCheck from '../Components/FormCheck';
import HEValueInput from '../Components/HEValueInput';
import WebApi from '../Services/WebApi';
import useHETriggerStore, { Trigger } from '../Store/useHETriggerStore';

import './HECalibration.scss';

import { BUTTON_ACTIONS } from '../Data/Pins';
import invert from 'lodash/invert';
import {
	createRapidTriggerState,
	formatReading,
	canConvert,
	fromDepth,
	toDepth,
	travelSpan,
	updateRapidTrigger,
} from '../Addons/HERapidTrigger';

const ADC_MAX = 4096;

type HECalibrationProps = {
	calibrateAllLoop: boolean;
	calibrationTarget: number;
	muxChannels: number;
	setShowModal: (show: boolean) => void;
	showModal: boolean;
	triggers: Trigger[];
	values: any;
};

const getOption = (e, actionId) => {
	return {
		label: invert(BUTTON_ACTIONS)[actionId],
		value: actionId,
	};
};

const HECalibration = ({
	calibrationTarget,
	calibrateAllLoop,
	muxChannels,
	setShowModal,
	showModal,
	triggers,
	values,
}: HECalibrationProps) => {
	const { t } = useTranslation('');
	const setHETrigger = useHETriggerStore((state) => state.setHETrigger);
	const setAllHETriggers = useHETriggerStore((state) => state.setAllHETriggers);
	const timerId = useRef<number>();
	const [title, setTitle] = useState('');
	const target = useRef(-1);
	const [nextTarget, setNextTarget] = useState(-1);
	const previousStep = useRef(0);
	const [calibrationStep, setCalibrationStep] = useState(0);
	const [voltage, setVoltage] = useState(0);
	const [activationState, setActivationState] = useState(false);
	const rapidTriggerState = useRef(createRapidTriggerState());
	const [voltageIdle, setVoltageIdle] = useState(20);
	const [voltagePressed, setVoltagePressed] = useState(3500);
	const [voltageActive, setVoltageActive] = useState(2000);
	const [polarity, setPolarity] = useState(false);
	const [noise, setNoise] = useState(30);
	const [rapidTrigger, setRapidTrigger] = useState(false);
	const [rtPressSensitivity, setRtPressSensitivity] = useState(100);
	const [rtReleaseSensitivity, setRtReleaseSensitivity] = useState(100);
	const [rtSeparateSensitivity, setRtSeparateSensitivity] = useState(false);
	const [rtContinuous, setRtContinuous] = useState(false);

	const travelMm = Number(values?.heTriggerSwitchTravel) || 0;
	const calibration = {
		idle: voltageIdle,
		pressed: voltagePressed,
		is_polarized: polarity,
	};
	const useMm = canConvert(travelMm, calibration);
	const rawLabel = (label: string) =>
		useMm ? `${label} (${t('HETrigger:unit-raw')})` : label;
	const span = Math.max(1, travelSpan(calibration));

	const minActuationDepth = Math.min(noise + 1, span);
	const minSensitivity = Math.min(Math.max(noise, 1), span);

	useEffect(() => {
		rapidTriggerState.current = createRapidTriggerState();
	}, [
		voltageIdle,
		voltagePressed,
		voltageActive,
		polarity,
		noise,
		rapidTrigger,
		rtPressSensitivity,
		rtReleaseSensitivity,
		rtSeparateSensitivity,
		rtContinuous,
	]);

	// Keep the actuation point reachable and releasable after calibration edits.
	useEffect(() => {
		const depth = toDepth(voltageActive, calibration);
		const clamped = Math.min(Math.max(depth, minActuationDepth), span);
		if (clamped !== depth) setVoltageActive(fromDepth(clamped, calibration));
	}, [voltageIdle, voltagePressed, polarity, noise]);

	useEffect(() => {
		const active = updateRapidTrigger(
			rapidTriggerState.current,
			{
				actuation: toDepth(voltageActive, calibration),
				pressSensitivity: rtPressSensitivity,
				releaseSensitivity: rtSeparateSensitivity
					? rtReleaseSensitivity
					: rtPressSensitivity,
				noise,
				travel: toDepth(voltagePressed, calibration),
				rapidTrigger,
				continuous: rtContinuous,
			},
			toDepth(voltage, calibration),
		);
		setActivationState(active);
	}, [voltage]);

	const currentTriggerValues = () => ({
		idle: voltageIdle,
		active: voltageActive,
		pressed: voltagePressed,
		is_polarized: polarity,
		noise,
		rapidTrigger,
		rtPressSensitivity,
		rtReleaseSensitivity,
		rtSeparateSensitivity,
		rtContinuous,
	});

	const loadTriggerValues = (trigger: Trigger) => {
		setVoltageIdle(trigger.idle);
		setVoltageActive(trigger.active);
		setVoltagePressed(trigger.pressed);
		setNoise(trigger.noise);
		setRapidTrigger(trigger.rapidTrigger);
		setRtPressSensitivity(trigger.rtPressSensitivity);
		setRtReleaseSensitivity(trigger.rtReleaseSensitivity);
		setRtSeparateSensitivity(Boolean(trigger.rtSeparateSensitivity));
		setRtContinuous(Boolean(trigger.rtContinuous));
		setPolarity(trigger.is_polarized);
	};

	const saveCalibration = () => {
		// Set to Trigger Store
		setHETrigger({
			id: target.current,
			action: triggers[target.current].action,
			...currentTriggerValues(),
		});
		stopCalibration();
		if (calibrateAllLoop) {
			checkNextTarget();
		} else {
			setShowModal(false);
		}
	};

	const checkNextTarget = () => {
		if (nextTarget !== -1) {
			target.current = nextTarget;
			setNextTarget(getNextTarget());
			updateTitle();
			restartCalibration();
		} else {
			setShowModal(false);
		}
	};

	const updateTitle = () => {
		if (target.current !== -1) {
			// set title
			const option = getOption(
				triggers[target.current],
				triggers[target.current].action,
			);
			const actionTitle = t(`PinMapping:actions.${option.label}`);
			if (muxChannels > 1) {
				const muxNum = Math.floor(target.current / muxChannels);
				const channelNum = target.current % muxChannels;
				setTitle(`${actionTitle} - Mux ${muxNum} - Channel ${channelNum}`);
			} else {
				setTitle(
					`${actionTitle} - Direct - ADC ${values[`muxADCPin${target.current}` as keyof typeof values]}`,
				);
			}
		}
	};

	const getNextTarget = () => {
		// Find our next
		for (var i = target.current + 1; i < 32; i++) {
			if (triggers[i].action !== -10) {
				return i;
			}
		}
		return -1;
	};

	const overwriteAllCalibration = () => {
		setAllHETriggers(currentTriggerValues());
		closeModal();
	};

	const stopCalibration = async () => {
		setCalibrationStep(0);
		target.current = -1;
		if (timerId) clearInterval(timerId.current);
	};

	const closeModal = async () => {
		stopCalibration();
		setShowModal(false);
	};

	const startReadingCalibrationLoop = async () => {
		if (showModal) {
			// Set the Hall Effect configuration pins
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
			updateCalibrationRead(0);
		}
	};

	const updateCalibrationRead = (step: number) => {
		setCalibrationStep(step);
		// Begin reading
		if (timerId.current) clearInterval(timerId.current);
		const intervalId = setInterval(() => {
			readHallEffect(step);
		}, 50);
		timerId.current = intervalId;
	};

	// Start Capturing on Modal Show
	const startCalibration = async () => {
		if (calibrateAllLoop) {
			target.current = getNextTarget();
			setNextTarget(getNextTarget());
		} else {
			target.current = calibrationTarget;
		}
		loadTriggerValues(triggers[target.current]);
	};

	const restartCalibration = () => {
		previousStep.current = 0;
		updateCalibrationRead(0);
		loadTriggerValues(triggers[target.current]);
	};

	const calculateVoltagePercentage = () => {
		return voltage / (ADC_MAX / 100.0);
	};

	const calculateVoltPressedPercentage = () => {
		return (voltage - voltageIdle) / ((voltagePressed - voltageIdle) / 100.0);
	};

	const readHallEffect = async (calibrationStep: number) => {
		const result = await WebApi.getHETriggerVoltage({
			targetId: target.current,
		});

		if (!result || !result.data) {
			console.error('Could not get hall-effect trigger calibration!');
			return;
		}

		const data = result.data;

		// For Web-Testing Debug Only
		if (data.debug && data.debug === true) {
			if (calibrationStep === 0) {
				setVoltage(150); // min we'll set to 20
			} else if (calibrationStep === 1) {
				setVoltage(3500); // max we'll set to 3500
			} else if (calibrationStep === 2 || calibrationStep === 3) {
				let time = new Date().getTime();
				const V =
					150 +
					Math.floor(
						(Math.cos((((time / 10) % 365) * Math.PI) / 180) + 1.0) * 1500,
					);
				setVoltage(V);
			}
		} else {
			// set the read voltage from real HE
			setVoltage(data.voltage);
		}
	};

	const firstStep = () => {
		return (
			<Row className="mb-3" hidden={calibrationStep !== 0}>
				<Col xs={12} className="mb-3">
					{t(`HETrigger:calibration-first-step`)}
				</Col>
				<Col xs={12} className="mb-3"></Col>
				<Col xs={12} className="mb-3">
					{rawLabel(t(`HETrigger:calibration-idle-text`))}
				</Col>
				<Col xs={12} className="mb-3 text-center">
					<ProgressBar>
						<ProgressBar
							variant="info"
							now={calculateVoltagePercentage()}
							key={1}
						/>
					</ProgressBar>
				</Col>
				<Col xs={12} className="mb-3">
					<h3>{voltage}</h3>
				</Col>
			</Row>
		);
	};

	const secondStep = () => {
		return (
			<Row className="mb-3" hidden={calibrationStep !== 1}>
				<span className="col-sm-12">
					{t(`HETrigger:calibration-second-step`)}
				</span>
				<Col xs={12} className="mb-3"></Col>
				<Col xs={12} className="mb-3">
					{rawLabel(t(`HETrigger:calibration-pressed-text`))}
				</Col>
				<Col xs={12} className="mb-3 text-center">
					<ProgressBar>
						<ProgressBar
							variant="info"
							now={calculateVoltagePercentage()}
							key={1}
						/>
					</ProgressBar>
				</Col>
				<Col xs={12} className="mb-3">
					<h3>{voltage}</h3>
				</Col>
				<Col xs={3} className="mb-3">
					<Button
						onClick={() => {
							restartCalibration();
						}}
						variant="danger"
					>
						{t(`HETrigger:restart-text`)}
					</Button>
				</Col>
			</Row>
		);
	};

	const polarityToggle = (idPrefix: string) => (
		<FormCheck
			label={t('HETrigger:calibration-flip-polarity')}
			type="switch"
			name="is_polarized"
			id={`${idPrefix}HETriggerPolarize`}
			isInvalid={false}
			checked={polarity}
			onChange={(e) => {
				setPolarity(e.target.checked);
				if (e.target.checked) {
					setVoltageIdle(Math.max(voltageIdle, voltagePressed));
					setVoltagePressed(Math.min(voltageIdle, voltagePressed));
				} else {
					setVoltageIdle(Math.min(voltageIdle, voltagePressed));
					setVoltagePressed(Math.max(voltageIdle, voltagePressed));
				}
			}}
		/>
	);

	const actuationControls = () => (
		<>
			<Col xs={12} className="mb-3">
				<HEValueInput
					label={
						useMm
							? t('HETrigger:actuation-point-text')
							: t('HETrigger:activation-input-text')
					}
					name="voltageActive"
					kind="reading"
					value={voltageActive}
					calibration={calibration}
					travelMm={travelMm}
					min={fromDepth(minActuationDepth, calibration)}
					max={voltagePressed}
					onChange={setVoltageActive}
				/>
			</Col>
		</>
	);

	const sensitivitySlider = (
		name: string,
		value: number,
		onChange: (v: number) => void,
	) => (
		<Form.Range
			name={name}
			min={minSensitivity}
			max={span}
			step={1}
			value={value}
			onChange={(e) => onChange(parseInt(e.target.value))}
		/>
	);

	const rapidTriggerControls = (idPrefix: string) => (
		<>
			<Col xs={12} className="mb-2">
				<FormCheck
					label={t('HETrigger:calibration-flip-rapid-trigger')}
					type="switch"
					name="rapidTrigger"
					id={`${idPrefix}HETriggerRapidTrigger`}
					isInvalid={false}
					checked={rapidTrigger}
					onChange={(e) => setRapidTrigger(e.target.checked)}
				/>
				<Form.Text muted>{t('HETrigger:rapid-trigger-description')}</Form.Text>
			</Col>
			{rapidTrigger && (
				<>
					<Col xs={12} md={6} className="mb-3">
						<HEValueInput
							label={
								rtSeparateSensitivity
									? t('HETrigger:rt-press-sensitivity-input-text')
									: t('HETrigger:rt-sensitivity-input-text')
							}
							name="rtPressSensitivity"
							kind="distance"
							value={rtPressSensitivity}
							calibration={calibration}
							travelMm={travelMm}
							min={minSensitivity}
							max={span}
							onChange={setRtPressSensitivity}
						/>
						{sensitivitySlider(
							'rtPressSensitivityRange',
							rtPressSensitivity,
							setRtPressSensitivity,
						)}
						<Form.Text muted>
							{rtSeparateSensitivity
								? t('HETrigger:rt-press-sensitivity-description')
								: t('HETrigger:rt-sensitivity-description')}
						</Form.Text>
					</Col>
					{rtSeparateSensitivity && (
						<Col xs={12} md={6} className="mb-3">
							<HEValueInput
								label={t('HETrigger:rt-release-sensitivity-input-text')}
								name="rtReleaseSensitivity"
								kind="distance"
								value={rtReleaseSensitivity}
								calibration={calibration}
								travelMm={travelMm}
								min={minSensitivity}
								max={span}
								onChange={setRtReleaseSensitivity}
							/>
							{sensitivitySlider(
								'rtReleaseSensitivityRange',
								rtReleaseSensitivity,
								setRtReleaseSensitivity,
							)}
							<Form.Text muted>
								{t('HETrigger:rt-release-sensitivity-description')}
							</Form.Text>
						</Col>
					)}
					<Col xs={12} md={6} className="mb-2">
						<FormCheck
							label={t('HETrigger:rt-separate-sensitivity-label')}
							type="switch"
							name="rtSeparateSensitivity"
							id={`${idPrefix}HETriggerRtSeparate`}
							isInvalid={false}
							checked={rtSeparateSensitivity}
							onChange={(e) => {
								setRtSeparateSensitivity(e.target.checked);
								if (e.target.checked)
									setRtReleaseSensitivity(rtPressSensitivity);
							}}
						/>
						<Form.Text muted>
							{t('HETrigger:rt-separate-sensitivity-description')}
						</Form.Text>
					</Col>
					<Col xs={12} md={6} className="mb-2">
						<FormCheck
							label={t('HETrigger:rt-continuous-label')}
							type="switch"
							name="rtContinuous"
							id={`${idPrefix}HETriggerRtContinuous`}
							isInvalid={false}
							checked={rtContinuous}
							onChange={(e) => setRtContinuous(e.target.checked)}
						/>
						<Form.Text muted>
							{t('HETrigger:rt-continuous-description')}
						</Form.Text>
					</Col>
				</>
			)}
			<Col xs={12} md={6} className="mb-3">
				<HEValueInput
					label={t('HETrigger:hysteresis-input-text')}
					name="noise"
					kind="distance"
					value={noise}
					calibration={calibration}
					travelMm={travelMm}
					min={0}
					max={span}
					onChange={setNoise}
				/>
				<Form.Text muted>{t('HETrigger:hysteresis-description')}</Form.Text>
			</Col>
		</>
	);

	const livePreview = () => (
		<>
			<Col xs={12} className="mb-3">
				{t(`HETrigger:activation-reading-text`)}
			</Col>
			<Col xs={12} className="mb-3 text-center">
				<ProgressBar>
					<ProgressBar
						variant={activationState ? 'success' : 'warning'}
						now={calculateVoltPressedPercentage()}
						key={1}
					/>
				</ProgressBar>
			</Col>
			<Col xs={12} className="mb-3">
				<Form.Range
					min={minActuationDepth}
					max={span}
					step={1}
					value={toDepth(voltageActive, calibration)}
					onChange={(e) =>
						setVoltageActive(fromDepth(parseInt(e.target.value), calibration))
					}
				/>
			</Col>
			<Col xs={12} className="mb-3">
				{formatReading(voltage, travelMm, calibration)}
				{useMm ? ` (${t('HETrigger:unit-raw')} ${voltage})` : ''}{' '}
				{activationState ? t('HETrigger:pressed-text') : ''}
			</Col>
		</>
	);

	const thirdStep = () => {
		return (
			<Row className="mb-3" hidden={calibrationStep !== 2}>
				<span className="col-sm-12">
					{t(`HETrigger:calibration-third-step`)}
				</span>
				<Col xs={12} className="mb-3"></Col>
				{actuationControls()}
				<Col xs={12} className="mb-3">
					{polarityToggle('step-')}
				</Col>
				{rapidTriggerControls('step-')}
				{livePreview()}
				<Col xs={3} className="mb-3">
					<Button onClick={() => restartCalibration()} variant="danger">
						{t(`HETrigger:restart-text`)}
					</Button>
				</Col>
			</Row>
		);
	};

	const manualAdjustments = () => {
		return (
			<Row className="mb-3" hidden={calibrationStep !== 3}>
				<Col xs={12} className="mb-3">
					{t(`HETrigger:calibration-manual-step`)}
				</Col>
				<Col xs={6} className="mb-3">
					<FormControl
						type="number"
						label={rawLabel(t(`HETrigger:idle-input-text`))}
						name="voltageIdle"
						className="form-select-sm"
						value={voltageIdle}
						onChange={(e) => {
							setVoltageIdle(parseInt((e.target as HTMLInputElement).value));
						}}
						min={0}
						max={ADC_MAX}
					/>
				</Col>
				<Col xs={6} className="mb-3">
					<FormControl
						type="number"
						label={rawLabel(t(`HETrigger:pressed-input-text`))}
						name="voltagePressed"
						className="form-select-sm"
						value={voltagePressed}
						onChange={(e) => {
							setVoltagePressed(parseInt((e.target as HTMLInputElement).value));
						}}
						min={0}
						max={ADC_MAX}
					/>
				</Col>
				{actuationControls()}
				<Col xs={12} className="mb-3">
					{polarityToggle('manual-')}
				</Col>
				{rapidTriggerControls('manual-')}
				{livePreview()}
				<Col xs={12} className="mb-3" />
				<Col xs={12} className="mb-3 text-center">
					<Button
						variant="danger"
						onClick={() => {
							if (window.confirm(t(`HETrigger:overwrite-confirm`))) {
								overwriteAllCalibration();
							}
						}}
						className="col-sm-4"
					>
						{t(`HETrigger:overwrite-all-warning`)}
					</Button>
				</Col>
			</Row>
		);
	};

	useEffect(() => {
		if (showModal === true) {
			startCalibration();
			startReadingCalibrationLoop();
			updateTitle();
		}
	}, [showModal]);

	return (
		<>
			<Modal
				className="modal-lg"
				contentClassName="he-modal"
				centered
				show={showModal}
				onClose={() => closeModal()}
				onHide={() => closeModal()}
			>
				<Modal.Header closeButton>
					<Modal.Title className="me-auto">
						{t(`HETrigger:calibration-header-text`)} - {title}
					</Modal.Title>
				</Modal.Header>
				<Modal.Body>
					{firstStep()}
					{secondStep()}
					{thirdStep()}
					{manualAdjustments()}
				</Modal.Body>
				<Modal.Footer>
					<Button
						onClick={() => {
							setVoltageIdle(voltage);
							updateCalibrationRead(1);
						}}
						hidden={calibrationStep !== 0}
					>
						<Spinner
							as="span"
							animation="grow"
							size="sm"
							role="status"
							aria-hidden="true"
						/>{' '}
						{t(`HETrigger:calibrate-idle-button`)}
					</Button>
					<Button
						onClick={() => {
							setVoltagePressed(voltage);
							setPolarity(voltage < voltageIdle);
							setVoltageActive(
								voltageIdle + Math.floor((voltage - voltageIdle) * 0.625),
							);
							updateCalibrationRead(2);
						}}
						hidden={calibrationStep !== 1}
					>
						<Spinner
							as="span"
							animation="grow"
							size="sm"
							role="status"
							aria-hidden="true"
							variant="success"
						/>{' '}
						{t(`HETrigger:calibrate-pressed-button`)}
					</Button>
					<Button
						variant="success"
						onClick={() => saveCalibration()}
						hidden={calibrationStep < 2}
					>
						{nextTarget !== -1
							? t(`HETrigger:next-calibration-text`)
							: t(`HETrigger:finish-calibration-text`)}
					</Button>
					<Button
						onClick={() => {
							updateCalibrationRead(previousStep.current);
						}}
						hidden={calibrationStep !== 3}
					>
						{t(`HETrigger:calibration-back-button`)}
					</Button>
					<Button
						onClick={() => {
							previousStep.current = calibrationStep;
							updateCalibrationRead(3);
						}}
						hidden={calibrationStep === 3}
					>
						{t(`HETrigger:manual-text`)}
					</Button>
				</Modal.Footer>
			</Modal>
		</>
	);
};

export default HECalibration;
