import { useEffect, useState, useRef } from 'react';
import { Button, Modal, Row, Col, ProgressBar, Form, Spinner } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';

import FormControl from '../Components/FormControl';
import FormCheck from '../Components/FormCheck';
import WebApi from '../Services/WebApi';
import useHETriggerStore, { Trigger } from '../Store/useHETriggerStore';

import './HECalibration.scss';

import { BUTTON_ACTIONS } from '../Data/Pins';
import invert from 'lodash/invert';

type HECalibrationProps = {
	calibrateAllLoop: boolean;
	calibrationTarget: number;
	muxChannels: number;
	setShowModal: (show: boolean) => void;
	showModal: boolean;
	triggers: Trigger[];
	values: any;
}

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
	const [voltageIdle, setVoltageIdle] = useState(20);
	const [voltageMax, setVoltageMax] = useState(3500);
	const [voltageActive, setVoltageActive] = useState(2000);
	const [polarity, setPolarity] = useState(0);

	const saveCalibration = () => {
		// Set to Trigger Store
		setHETrigger({
			id: target.current,
			action: triggers[target.current].action,
			idle: voltageIdle,
			active: voltageActive,
			max: voltageMax,
			polarity
		})
		stopCalibration();
		if ( calibrateAllLoop ) {
			checkNextTarget();
		} else {
			setShowModal(false);
		}
	};

	const checkNextTarget = () => {
		if ( nextTarget!== -1 ) {
			target.current = nextTarget;
			setNextTarget(getNextTarget());
			updateTitle();
			restartCalibration();
		} else {
			setShowModal(false);
		}
	};

	const updateTitle = () => {
		if ( target.current !== -1 ) {
			// set title
			const option = getOption(triggers[target.current], triggers[target.current].action);
			const actionTitle = t(`PinMapping:actions.${option.label}`);
			if ( muxChannels > 1 ) {
				const muxNum = Math.floor(target.current/muxChannels);
				const channelNum = muxNum + Math.floor(target.current%muxChannels);
				setTitle(`${actionTitle} - Mux ${muxNum} - Channel ${channelNum}`);
			} else {
				setTitle(`${actionTitle} - Direct - ADC ${values[`muxADCPin${target.current}` as keyof typeof values]}`);
			}
		}
	};

	const getNextTarget = () => {
		// Find our next
		for(var i = target.current+1; i < 32; i++) {
			if (triggers[i].action !== -10) {
				return i;
			}
		}
		return -1;
	}

	const overwriteAllCalibration = () => {
		setAllHETriggers({
			idle: voltageIdle,
			active: voltageActive,
			max: voltageMax,
			polarity
		});
		closeModal();
	};

	const stopCalibration = async () => {
		setCalibrationStep(0);
		target.current = -1;
		if (timerId)
			clearInterval(timerId.current);
	};

	const closeModal = async () => {
		stopCalibration();
		setShowModal(false);
	};

	const startReadingCalibrationLoop = async () => {
		if (showModal) {
			// Set the Hall Effect calibration pins
			await WebApi.setHETriggerCalibration({
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
			updateCalibrationRead(0);
		}
	}

	const updateCalibrationRead = (step:number) => {
		setCalibrationStep(step);
		// Begin reading
		if (timerId.current)
			clearInterval(timerId.current);
		const intervalId = setInterval(() => {
			readHallEffect(step);
		}, 50);
		timerId.current = intervalId;
	};

	// Start Capturing on Modal Show
	const startCalibration = async() => {
		if ( calibrateAllLoop ) {
			target.current = getNextTarget();
			setNextTarget(getNextTarget());
		} else {
			target.current = calibrationTarget;
		}
		setVoltageIdle(triggers[target.current].idle);
		setVoltageActive(triggers[target.current].active);
		setVoltageMax(triggers[target.current].max);
		setPolarity(triggers[target.current].polarity);
	};

	const restartCalibration = () => {
		previousStep.current = 0;
		updateCalibrationRead(0);
	};

	const calculateVoltIdle = () => {
		return (voltage/(4096/100.0));
	};

	const calculateVoltMax = () => {
		return ((voltage-voltageIdle)/((4096-voltageIdle)/100.0));
	};

	const calculateVoltPressed = () => {
		return (Math.min(voltageMax,voltage)-voltageIdle)/((voltageMax-voltageIdle)/100.0);
	};

	const readHallEffect = async (calibrationStep:number) => {
		// read hall effect trigger if we're not at opening
		const result = await WebApi.getHETriggerCalibration({
			targetId: target.current
		});

		if (!result || !result.data) {
			console.error("Could not get hall-effect trigger calibration!");
			return;
		}

		const data = result.data;

		// For Web-Testing Debug Only
		if (data.debug && data.debug === true) {
			if ( calibrationStep === 0 ) {
				setVoltage(150); // min we'll set to 20
			} else if ( calibrationStep === 1 ) {
				setVoltage(3500); // max we'll set to 3500
			} else if ( calibrationStep === 2 || calibrationStep === 3 ) {
				let time = (new Date()).getTime();
				const V = 150+Math.floor((Math.cos((( time/10 ) % 365) * Math.PI / 180)+1.0)*1500);
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
					{t(`HETrigger:calibration-idle-text`)}
				</Col>
				<Col xs={12} className="mb-3 text-center">
					<ProgressBar>
						<ProgressBar variant="info" now={calculateVoltIdle()} key={1} />
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
					{t(`HETrigger:calibration-pressed-text`)}
				</Col>
				<Col xs={12} className="mb-3 text-center">
					<ProgressBar>
						<ProgressBar variant="info" now={calculateVoltMax()} key={1} />
					</ProgressBar>
				</Col>
				<Col xs={12} className="mb-3">
					<h3>{voltage}</h3>
				</Col>
				<Col xs={3} className="mb-3">
					<Button onClick={() => { restartCalibration(); }} variant="danger">
						{t(`HETrigger:restart-text`)}
					</Button>
				</Col>
			</Row>
		);
	};

	const thirdStep = () => {
		return (
			<Row className="mb-3" hidden={calibrationStep !== 2}>
				<span className="col-sm-12">
					{t(`HETrigger:calibration-third-step`)}
				</span>
				<Col xs={12} className="mb-3"></Col>
				<Col xs={12} className="mb-3">
					<FormControl
						type="number"
						label={t(`HETrigger:activation-input-text`)}
						name="voltageActive"
						className="form-select-sm"
						value={voltageActive}
						onChange={(e) => {
							setVoltageActive(parseInt((e.target as HTMLInputElement).value));
						}}
						min={voltageIdle}
						max={voltageMax}
					/>
				</Col>
				<Col xs={12} className="mb-3">
					{t(`HETrigger:activation-reading-text`)}
				</Col>
				<Col xs={12} className="mb-3 text-center">
					<ProgressBar>
						<ProgressBar variant={voltage>voltageActive?"success":"warning"} now={calculateVoltPressed()} key={1} />
					</ProgressBar>
				</Col>
				<Col xs={12} className="mb-3">
					<Form.Range
						min={voltageIdle}
						max={voltageMax}
						step={1}
						value={voltageActive}
						onChange={(e) => {
							setVoltageActive(parseInt((e.target as HTMLInputElement).value));
						}}
						></Form.Range>
				</Col>
				<Col xs={12} className="mb-3">
					{voltage} {voltage>voltageActive?t('HETrigger:pressed-text'):""}
				</Col>
				<Col xs={3} className="mb-3">
					<Button onClick={() => restartCalibration()} variant="danger">
						{t(`HETrigger:restart-text`)}
					</Button>
				</Col>
			</Row>
		);
	};

	const manulAdjustments = () => {
		return (
			<Row className="mb-3" hidden={calibrationStep !== 3}>
				<Col xs={12} className="mb-3">
					{t(`HETrigger:calibration-manual-step`)}
				</Col>
				<Col xs={4} className="mb-3">
					<FormControl
						type="number"
						label={t(`HETrigger:idle-input-text`)}
						name="voltageIdle"
						className="form-select-sm"
						value={voltageIdle}
						onChange={(e) => {
							setVoltageIdle(parseInt((e.target as HTMLInputElement).value));
						}}
						min={0}
						max={4096}
					/>
				</Col>
				<Col xs={4} className="mb-3">
					<FormControl
						type="number"
						label={t(`HETrigger:activation-input-text`)}
						name="voltageActive"
						className="form-select-sm"
						value={voltageActive}
						onChange={(e) => {
							setVoltageActive(parseInt((e.target as HTMLInputElement).value));
						}}
						min={0}
						max={4096}
					/>
				</Col>
				<Col xs={4} className="mb-3">
					<FormControl
						type="number"
						label={t(`HETrigger:pressed-input-text`)}
						name="voltageMax"
						className="form-select-sm"
						value={voltageMax}
						onChange={(e) => {
							setVoltageMax(parseInt((e.target as HTMLInputElement).value));
						}}
						min={0}
						max={4096}
					/>
				</Col>
				<Col xs={12} className="mb-3">
					<FormCheck
						label={t('HETrigger:calibration-flip-polarity')}
						type="switch"
						name="polarity"
						id="HETriggerPolarize"
						disabled
						isInvalid={false}
						checked={polarity}
						onChange={(e) => {
							setPolarity(polarity == 0 ? 1 : 0);
						}}
					/>
				</Col>
				<Col xs={12} className="mb-3">
					{t(`HETrigger:activation-reading-text`)}
				</Col>
				<Col xs={12} className="mb-3 text-center">
					<ProgressBar>
						<ProgressBar variant={voltage>voltageActive?"success":"warning"} now={calculateVoltPressed()} key={1} />
					</ProgressBar>
				</Col>
				<Col xs={12} className="mb-3">
					<Form.Range
						min={voltageIdle}
						max={voltageMax}
						step={1}
						value={voltageActive}
						onChange={(e) => {
							setVoltageActive(parseInt(e.target.value));
						}}
					></Form.Range>
				</Col>
				<Col xs={12} className="mb-3">
					{voltage} {voltage>voltageActive?t('HETrigger:pressed-text'):""}
				</Col>
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
					>{t(`HETrigger:overwrite-all-warning`)}
					</Button>
				</Col>
			</Row>
		);
	};

	useEffect(() => {
		if ( showModal === true ) {
			startCalibration();
			startReadingCalibrationLoop();
			updateTitle();
		}
	}, [showModal]);

	return (
		<>
			<Modal className="modal-lg" contentClassName="he-modal" centered show={showModal}
				onClose={() => closeModal()}
				onHide={() => closeModal()}
			>
				<Modal.Header closeButton>
					<Modal.Title className="me-auto">{t(`HETrigger:calibration-header-text`)} - {title}</Modal.Title>
				</Modal.Header>
				<Modal.Body>
					{firstStep()}
					{secondStep()}
					{thirdStep()}
					{manulAdjustments()}
				</Modal.Body>
				<Modal.Footer>
					<Button onClick={() => {
						setVoltageIdle(voltage);
						updateCalibrationRead(1);
					}} hidden={calibrationStep !== 0}>
						<Spinner
							as="span"
							animation="grow"
							size="sm"
							role="status"
							aria-hidden="true"
						/> {t(`HETrigger:calibrate-idle-button`)}
					</Button>
					<Button onClick={() => {
						setVoltageMax(voltage);
						setVoltageActive(voltageIdle + Math.floor((voltage-voltageIdle)*0.625));
						updateCalibrationRead(2);
					}} hidden={calibrationStep !== 1}>
						<Spinner
							as="span"
							animation="grow"
							size="sm"
							role="status"
							aria-hidden="true"
							variant="success"
						/> {t(`HETrigger:calibrate-pressed-button`)}
					</Button>
					<Button
						variant="success"
						onClick={() => saveCalibration()}
						hidden={calibrationStep < 2}
					>
						{nextTarget !== -1 ? t(`HETrigger:next-calibration-text`): t(`HETrigger:finish-calibration-text`)}
					</Button>
					<Button onClick={() => {
							updateCalibrationRead(previousStep.current);
						}}
						hidden={calibrationStep !== 3}>
						{t(`HETrigger:calibration-back-button`)}
					</Button>
					<Button onClick={() => {
							previousStep.current = calibrationStep;
							updateCalibrationRead(3);
						}}
						hidden={calibrationStep === 3}>
						{t(`HETrigger:manual-text`)}
					</Button>
				</Modal.Footer>
			</Modal>
		</>
	);
};

export default HECalibration;
