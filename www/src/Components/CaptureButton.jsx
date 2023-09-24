import React, { useEffect, useRef, useState } from 'react';
import { Button, Modal } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import WebApi from '../Services/WebApi';

const CaptureButton = ({ buttonName, onChange, abortSignalRef, triggerCapture, onTriggeredCaptureComplete, onStopCaptureSequence, size }) => {
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
		const data = await WebApi.getHeldPins(setModalVisible, newAbortSignal);
        const pin = data?.heldPins?.at(0);
		if (!isNaN(pin))
			onChange(pin);
		if (data.canceled) {
			await timeout(50);
			await WebApi.abortGetHeldPins();
			await timeout(50);
		}
        if (triggerNextRef.current) {
			triggerNextRef.current = false;
            onTriggeredCaptureComplete();
        }
	};

	useEffect(() => () => controller?.current?.abort(), []);

	const signalAbort = async () => {
		await timeout(50);
		controller?.current?.abort();
		setModalVisible(false);
	};

    useEffect(() => {
        if (triggerCapture) {
            triggerNextRef.current = true;
            getHeldPins().then();
        }
    }, [triggerCapture]);

	return (
		<>
			<Modal centered show={modalVisible} onHide={() => signalAbort()}>
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
					<Button variant="secondary" onClick={() => signalAbort()}>
						Cancel
					</Button>
					{triggerCapture && onStopCaptureSequence && 
                    <Button variant="danger"
                        onClick={async () => {
                            triggerNextRef.current = false;
                            onStopCaptureSequence();
                            await signalAbort();
                        }}>
						Stop Capture
					</Button>}
				</Modal.Footer>
			</Modal>
			<Button
				className="ms-3"
				size={size}
				variant="secondary"
				onClick={() => getHeldPins()}
			>
				{t('CaptureButton:capture-button-button-label')}
			</Button>
		</>
	);
};

export default CaptureButton;