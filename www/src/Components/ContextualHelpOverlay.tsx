import { useState } from 'react';
import Button from 'react-bootstrap/Button';
import Offcanvas from 'react-bootstrap/Offcanvas';

import InfoCircle from '../Icons/InfoCircle';

const ContextualHelpOverlay = ({ ...props }) => {
	const [show, setShow] = useState(false);

	const handleClose = () => setShow(false);
	const handleShow = () => setShow(true);

	return (
		<>
			<Button variant="link" onClick={handleShow} className="me-2">
				<InfoCircle />
			</Button>
			<Offcanvas show={show} onHide={handleClose} {...props}>
				<Offcanvas.Header closeButton>
					<Offcanvas.Title>{props.title}</Offcanvas.Title>
				</Offcanvas.Header>
				<Offcanvas.Body>{props.body}</Offcanvas.Body>
			</Offcanvas>
		</>
	);
};

export default ContextualHelpOverlay;
