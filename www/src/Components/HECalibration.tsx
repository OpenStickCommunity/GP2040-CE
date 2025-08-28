import { useEffect, useState } from 'react';
import { Button, Modal, Row, Col, ProgressBar, Form, Spinner } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';

import FormControl from '../Components/FormControl';
import FormCheck from '../Components/FormCheck';
import WebApi from '../Services/WebApi';
import useHETriggerStore, { Trigger } from '../Store/useHETriggerStore';

import './HECalibration.scss';

type HECalibrationProps = {
	values: any;
	showModal: boolean;
	setShowModal: (show: boolean) => void;
	triggers: Trigger[];
	target: number;
	title: string;
}

const HECalibration = ({
	values,
	showModal,
	setShowModal,
	triggers,
	target,
	title,
}: HECalibrationProps) => {
	const { t } = useTranslation('');
	const setHETrigger = useHETriggerStore((state) => state.setHETrigger);
	const setAllHETriggers = useHETriggerStore((state) => state.setAllHETriggers);
	const [timerId, setTimerId] = useState<number>();
	const [calibrationStep, setCalibrationStep] = useState(0);
	const [previousStep, setPreviousStep] = useState(0);
	const [voltage, setVoltage] = useState(0);
	const [voltageIdle, setVoltageIdle] = useState(20);
	const [voltageMax, setVoltageMax] = useState(3500);
	const [voltageActive, setVoltageActive] = useState(2000);
	const [polarity, setPolarity] = useState(0);

	const saveCalibration = () => {
		// Set to Trigger Store
		setHETrigger({
			id: target,
			action: triggers[target].action,
			idle: voltageIdle,
			active: voltageActive,
			max: voltageMax,
			polarity
		})
		stopCalibration();
	};

	const overwriteAllCalibration = () => {
			setAllHETriggers({
				idle: voltageIdle,
				active: voltageActive,
				max: voltageMax,
				polarity
			});
		stopCalibration();
	};

	const stopCalibration = async () => {
		console.log("Closing modal");
		setShowModal(false);
		setCalibrationStep(0);
		if (timerId)
			clearInterval(timerId);
	};

	const startReadingCalibrationLoop = async (showModal: boolean, step: number) => {
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
			});
			// Begin reading
			if (timerId)
				clearInterval(timerId);

			const intervalId = setInterval(() => {readHallEffect(step)}, 50);
			setTimerId(intervalId);
		}
	}

	// Start Capturing on Modal Show
	const startCalibration = async() => {
		startReadingCalibrationLoop(showModal, calibrationStep);
		console.log("Trigger Target: ", triggers[target]);
		console.log("Channels: ", values['muxChannels']);
		{Array.from({ length: 4 }, (_, i) => (
			console.log(`Select ${i} Pin: `, values[`muxSelectPin${i}`])
		))}
		{Array.from({ length: 4 }, (_, i) => (
			console.log(`Mux ${i} ADC Pin: `, values[`muxADCPin${i}`])
		))}
		setVoltageIdle(triggers[target].idle);
		setVoltageActive(triggers[target].active);
		setVoltageMax(triggers[target].max);
		setPolarity(triggers[target].polarity);
	};

	const restartCalibration = () => {
		setCalibrationStep(0);
		setPreviousStep(0);
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

	const calculateVoltActiveDefault = () => {
		return (Math.floor(voltageMax*0.8));
	};

	const readHallEffect = async (step:number) => {
		// read hall effect trigger if we're not at opening
		const result = await WebApi.getHETriggerCalibration({
			targetId: target
		});

		if (!result || !result.data) {
			console.error("Could not get hall-effect trigger calibration!");
			return;
		}

		const data = result.data;

		// For Web-Testing Debug Only
		if (data.debug && data.debug === true) {
			if ( step === 0 ) {
				setVoltage(150); // min we'll set to 20
			} else if ( step === 1 ) {
				setVoltage(3500); // max we'll set to 3500
			} else if ( step === 2 || step === 3 ) {
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
		startReadingCalibrationLoop(showModal, calibrationStep);
	}, [calibrationStep]);

	return (
		<>
			<Modal className="modal-lg" contentClassName="he-modal" centered show={showModal}
				onClose={() => stopCalibration()}
				onHide={() => stopCalibration()}
				onShow={() => startCalibration()}
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
						setCalibrationStep(1);
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
						setVoltageActive(calculateVoltActiveDefault());
						setCalibrationStep(2);
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
						{t(`HETrigger:finish-calibration-${calibrationStep}-text`)}
					</Button>
					<Button onClick={() => {
							setCalibrationStep(previousStep);
						}}
						hidden={calibrationStep !== 3}>
						{t(`HETrigger:calibration-back-button`)}
					</Button>
					<Button onClick={() => {
							setPreviousStep(calibrationStep);
							setCalibrationStep(3);
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
