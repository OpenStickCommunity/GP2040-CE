import React, { useEffect, useRef, useState } from 'react';
import { Button, Modal } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import WebApi from '../Services/WebApi';

const CaptureButton = ({ buttonName, onChange, abortSignalRef, triggerCapture, onTriggeredCaptureComplete, onStopCaptureSequence }) => {
	const { t } = useTranslation('');
	const controller = abortSignalRef || useRef();
    const triggerNextRef = useRef(true);
	const [modalVisible, setModalVisible] = useState(false);

	const newAbortSignal = () => {
		controller.current = new AbortController();
		return controller.current.signal;
	};

	const timeout = (ms) => (new Promise(resolve => setTimeout(resolve, ms)));

	const getHeldPins = async () => {
		await timeout(50); // Workaround to prevent hardfault, probably has to do with the extra `loop` call
		const data = await WebApi.getHeldPins(setModalVisible, newAbortSignal);
        const pin = data?.heldPins?.at(0);
		if (!isNaN(pin))
			onChange(pin);
        if (triggerNextRef.current) {
            if (triggerCapture) onTriggeredCaptureComplete();
        }
	};

	useEffect(() => () => controller?.current?.abort(), []);

	const closeModal = async () => {
		controller?.current?.abort();
		await timeout(50);
		await WebApi.abortGetHeldPins();
		await timeout(50);
		setModalVisible(false);
	};

    useEffect(() => {
        console.log('triggerCapture', triggerCapture); if (triggerCapture) {
            getHeldPins().then();
            triggerNextRef.current = true;
        }
    }, [triggerCapture]);

	return (
		<>
			<Modal centered show={modalVisible} onHide={() => closeModal()}>
				<Modal.Header closeButton>
					<Modal.Title className="me-auto">{`${t('CaptureButton:capture-button-modal-title')} (${buttonName})`}</Modal.Title>
				</Modal.Header>
				<Modal.Body className="row">
                    <span className="col-sm-10">{`${t('CaptureButton:capture-button-modal-content')}`}</span>
					<span className="col-sm-1">
						<span className="spinner-border" />
					</span>
				</Modal.Body>
				<Modal.Footer>
					<Button variant="secondary" onClick={() => closeModal()}>
						Cancel
					</Button>
					{triggerCapture && onStopCaptureSequence && 
                    <Button variant="danger"
                        onClick={async () => {
                            triggerNextRef.current = false;
                            onStopCaptureSequence();
                            await closeModal();
                        }}>
						Stop Capture
					</Button>}
				</Modal.Footer>
			</Modal>
			<Button
				className="ms-3"
				variant="secondary"
				onClick={() => getHeldPins()}
			>
				{t('CaptureButton:capture-button-button-label')}
			</Button>
		</>
	);
};

export default CaptureButton;