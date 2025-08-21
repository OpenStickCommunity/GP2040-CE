import { useContext, useRef, useEffect, useState } from 'react';
import { Button, Modal, Row, Col, ProgressBar, FormLabel, Form } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import FormControl from '../Components/FormControl';
import FormCheck from '../Components/FormCheck';
import WebApi from '../Services/WebApi';
import Spinner from 'react-bootstrap/Spinner';

import './HECalibration.scss';
import { useAsyncValue } from 'react-router-dom';

const HECalibration = ({
	values,
	showModal,
	setShowModal,
	triggers,
	setHETrigger,
	target,
	title,
	handleChange,
	...props
}) => {
	const { t } = useTranslation('');
	const [timerId, setTimerId] = useState<NodeJS.Timeout>();
	const [calibrationStep, setCalibrationStep] = useState(0);
	const [voltage, setVoltage] = useState(0);
	const [voltageIdle, setVoltageIdle] = useState(20);
	const [voltageMax, setVoltageMax] = useState(3500);
	const [voltageActive, setVoltageActive] = useState(2000);
	const [polarity, setPolarity] = useState(0);

	const saveCalibration = () => {
		// Set to Trigger Store
		setHETrigger(target, triggers[target].action, voltageIdle, voltageActive, voltageMax, polarity);
		stopCalibration();
	};

	const overwriteAllCalibration = () => {
		Object.keys(triggers).map((key, index) => {
			setHETrigger(key, triggers[key].action, voltageIdle, voltageActive, voltageMax, polarity);
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

	// 
	const startReadingCalibrationLoop = (showModal, step) => {
		if (showModal) {	
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

	const readHallEffect = async (step) => {
		// read hall effect trigger if we're not at opening
		var result = await WebApi.getHETriggerCalibration({
			targetId: target,
			muxChannels: values['muxChanels'],
			muxSelectPin0: values['muxSelectPin0'],
			muxSelectPin1: values['muxSelectPin1'],
			muxSelectPin2: values['muxSelectPin2'],
			muxSelectPin3: values['muxSelectPin3'],
			muxADCPin0: values['muxADCPin0'],
			muxADCPin1: values['muxADCPin1'],
			muxADCPin2: values['muxADCPin2'],
			muxADCPin3: values['muxADCPin3'],
		});

		if ( !result || !result.data ) {
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
					<Modal.Title className="me-auto">Hall-Effect Calibration - {title}</Modal.Title>
				</Modal.Header>
				<Modal.Body>
					<Row className="mb-3" hidden={calibrationStep !== 0}>
						<Col xs={12} className="mb-3">
							We need to calibrate the idle voltage and full press voltage of the hall-effect switch.
							After calibration, we can adjust the trigger-activation point to our desired depth.
						</Col>
						<Col xs={12} className="mb-3"></Col>
						<Col xs={12} className="mb-3">
							First, let's calibrate the idle voltage. Leave the hall-effect button untouched and click the "Calibrate Idle" button.
						</Col>
						<Col xs={12} className="mb-3"></Col>
						<Col xs={12} className="mb-3">
							Idle Voltage Reading:
						</Col>
						<Col xs={12} className="mb-3 text-center">
							<ProgressBar>
								<ProgressBar variant="info" now={voltage/40} key={1} />
							</ProgressBar>
						</Col>
						<Col xs={12} className="mb-3">
							<h3>{voltage}</h3>
						</Col>
					</Row>
					<Row className="mb-3" hidden={calibrationStep !== 1}>
						<span className="col-sm-12">
							Next, <b>press</b> the button fully to reach our maximum depth. Activation position can be
							adjusted after calibration.
						</span>
						<Col xs={12} className="mb-3"></Col>
						<Col xs={12} className="mb-3">
							Maximum Voltage Reading:
						</Col>
						<Col xs={12} className="mb-3 text-center">
							<ProgressBar>
								<ProgressBar striped variant="info" now={voltageIdle/40} key={1} />
								<ProgressBar variant="warning" now={voltage/40} key={2} />
							</ProgressBar>
						</Col>
						<Col xs={12} className="mb-3">
							<h3>{voltage}</h3>
						</Col>
					</Row>
					<Row className="mb-3" hidden={calibrationStep !== 2}>
						<span className="col-sm-12">
							Finally, let's adjust our current activation point and set the desired trigger point.
							Once adjusted, press the button and verify it activates at the desired position.
						</span>
						<Col xs={12} className="mb-3"></Col>
						<Col xs={12} className="mb-3">
							Activation Point Reading:
						</Col>
						<Col xs={12} className="mb-3 text-center">
							<ProgressBar>
								<ProgressBar striped variant="info" now={voltageIdle/40} key={1} />
								<ProgressBar variant={voltage>voltageActive?"success":"warning"} now={voltage/40} key={2} />
							</ProgressBar>
						</Col>
						<Col xs={12} className="mb-3">
							{voltage} {voltage>voltageActive?"Pressed!":""}
						</Col>
						<Col xs={12} className="mb-3">
							Activation Set-Point:
						</Col>
						<Col xs={12} className="mb-3">
							<Form.Range
								min={voltageIdle}
								max={voltageMax}
								step={50}
								value={voltageActive}
								onChange={(e) => {
									setVoltageActive(e.target.value);
								}}
							></Form.Range>
						</Col>
						<Col xs={12} className="mb-3">
							<FormControl
								type="number"
								label='Activation Trigger Voltage'
								name="voltageActive"
								className="form-select-sm"
								value={voltageActive}
								onChange={(e) => {
									setVoltageActive(e.target.value);
								}}
								min={0}
								max={4096}
							/>
						</Col>
					</Row>
					<Row className="mb-3" hidden={calibrationStep !== 3}>
						<Col xs={12} className="mb-3">
							Please adjust the following attributes of the hall effect button to the desired amounts.
							Once the desired values have been found, you can copy these values and set all triggers
							on the device.
						</Col>
						<Col xs={4} className="mb-3">
							<FormControl
								type="number"
								label='Current Idle Voltage'
								name="voltageIdle"
								className="form-select-sm"
								value={voltageIdle}
								onChange={(e) => {
									setVoltageIdle(e.target.value);
								}}
								min={0}
								max={4096}
							/>
						</Col>
						<Col xs={4} className="mb-3">
							<FormControl
								type="number"
								label='Current Max Voltage'
								name="voltageMax"
								className="form-select-sm"
								value={voltageMax}
								onChange={(e) => {
									setVoltageMax(e.target.value);
								}}
								min={0}
								max={4096}
							/>
						</Col>
						<Col xs={4} className="mb-3">
							<FormControl
								type="number"
								label='Current Activation Voltage'
								name="voltageActive"
								className="form-select-sm"
								value={voltageActive}
								onChange={(e) => {
									setVoltageActive(e.target.value);
								}}
								min={0}
								max={4096}
							/>
						</Col>
						<Col xs={12} className="mb-3">
							<FormCheck
								label='Flip Polarization'
								type="switch"
								name="polarity"
								id="HETriggerPolarize"
								isInvalid={false}
								checked={polarity}
								onChange={(e) => {
									setPolarity(polarity == 0 ? 1 : 0);
								}}
							/>
						</Col>
						<Col xs={12} className="mb-3">
							Activation Point Reading:
						</Col>
						<Col xs={12} className="mb-3 text-center">
							<ProgressBar>
								<ProgressBar striped variant="info" now={voltageIdle/40} key={1} />
								<ProgressBar variant={voltage>voltageActive?"success":"warning"} now={voltage/40} key={2} />
							</ProgressBar>
						</Col>
						<Col xs={12} className="mb-3">
							{voltage} {voltage>voltageActive?"Pressed!":""}
						</Col>
						<Col xs={12} className="mb-3" />
						<Col xs={12} className="mb-3 text-center">
							<Button 
								variant="danger"
								onClick={() => overwriteAllCalibration()}
								className="col-sm-4"
							>Overwrite All Triggers
							</Button>
						</Col>
					</Row>
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
						/> Calibrate Idle
					</Button>
					<Button onClick={() => {
						setVoltageMax(voltage);
						setVoltageActive(Math.floor(voltage*0.7));
						setCalibrationStep(2);
					}} hidden={calibrationStep !== 1}>
						<Spinner
							as="span"
							animation="grow"
							size="sm"
							role="status"
							aria-hidden="true"
							variant="success"
						/> Calibrate Press
					</Button>
					<Button 
						variant="success"
						onClick={() => saveCalibration()}
						hidden={calibrationStep < 2}
					>
						Finish Calibration
					</Button>
					<Button onClick={() => setCalibrationStep(3)}>
						Manual Adjustments
					</Button>
					<Button onClick={() => setCalibrationStep(0)}
						variant="danger"
						hidden={calibrationStep === 0}>
						Restart
					</Button>
				</Modal.Footer>
			</Modal>
		</>
	);
};

export default HECalibration;
