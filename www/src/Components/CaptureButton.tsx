import { useEffect, useRef, useState } from 'react';
import { Button, Modal } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import WebApi from '../Services/WebApi';

type CaptureButtonTypes = {
	labels: string[];
	onChange: (label: string, pin: number) => void;
	small?: boolean;
	buttonLabel?: string;
};

const CaptureButton = ({
	labels,
	onChange,
	small = false,
	buttonLabel,
}: CaptureButtonTypes) => {
	const { t } = useTranslation('');
	const controller = useRef<null | AbortController>(null);
	const stopRef = useRef(false);
	const [showModal, setShowModal] = useState(false);
	const [labelIndex, setLabelIndex] = useState(0);
	const [triggerCapture, setTriggerCapture] = useState(false);

	useEffect(() => () => controller?.current?.abort(), []);

	const currentLabel = labels[labelIndex] || '';
	const hasNext = Boolean(labels[labelIndex + 1]);

	const timeout = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

	const closeAndReset = () => {
		stopRef.current = false;
		setShowModal(false);
		setLabelIndex(0);
	};

	const getHeldPins = async () => {
		setTriggerCapture(false);
		controller.current = new AbortController();

		const data = await WebApi.getHeldPins(controller.current.signal);
		const pin = data?.heldPins?.at(0);

		if (!isNaN(pin)) onChange(currentLabel, pin);

		// Gets called for both skip and stop
		if (data.canceled) {
			await timeout(50);
			await WebApi.abortGetHeldPins();
			await timeout(50);
		}

		if (stopRef.current || !hasNext) return closeAndReset();

		setLabelIndex((index) => index + 1);
		setTriggerCapture(true);
	};

	const stopCapture = async () => {
		await timeout(50);
		stopRef.current = true;
		controller?.current?.abort();
	};

	const skipButton = async () => {
		await timeout(50);
		controller?.current?.abort();
	};

	useEffect(() => {
		if (triggerCapture) {
			setShowModal(true);
			getHeldPins();
		}
	}, [triggerCapture]);

	return (
		<>
			<Modal centered show={showModal} onHide={() => stopCapture()}>
				<Modal.Header closeButton>
					<Modal.Title className="me-auto">{`${t(
						'CaptureButton:capture-button-modal-title',
					)} ${currentLabel}`}</Modal.Title>
				</Modal.Header>
				<Modal.Body className="row">
					<span className="col-sm-10">
						{t('CaptureButton:capture-button-modal-content')}
					</span>
					<span className="col-sm-1">
						<span className="spinner-border" />
					</span>
				</Modal.Body>
				<Modal.Footer>
					{hasNext && (
						<Button onClick={() => skipButton()}>
							{t('CaptureButton:capture-button-modal-skip')}
						</Button>
					)}
					<Button variant="danger" onClick={() => stopCapture()}>
						{t('CaptureButton:capture-button-modal-stop')}
					</Button>
				</Modal.Footer>
			</Modal>
			<Button onClick={() => setTriggerCapture(true)}>
				{small
					? '🎮'
					: `${
							buttonLabel
								? buttonLabel
								: t('CaptureButton:capture-button-button-label')
						} 🎮`}
			</Button>
		</>
	);
};

export default CaptureButton;
