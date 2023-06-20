import React, { useContext, useState } from 'react';
import { Nav, NavDropdown, Navbar, Button, Modal, Dropdown } from 'react-bootstrap';
import { NavLink } from "react-router-dom";
import { AppContext } from '../Contexts/AppContext';
import FormSelect from './FormSelect';
import { saveButtonLabels } from '../Services/Storage';
import { BUTTONS } from '../Data/Buttons';
import './Navigation.scss';
import WebApi from '../Services/WebApi';
import ColorScheme from './ColorScheme';

const BOOT_MODES = {
	GAMEPAD: 0,
	WEBCONFIG: 1,
	BOOTSEL: 2
};

const Navigation = (props) => {
	const { buttonLabels, setButtonLabels } = useContext(AppContext);

	const [show, setShow] = useState(false);
	const [isRebooting, setIsRebooting] = useState(null); // null because we want the button to assume untouched state

	const handleClose = () => setShow(false);
	const handleShow = () => { setIsRebooting(null); setShow(true); }
	const handleReboot = async (bootMode) => {
		if (isRebooting === false) { setShow(false); return; }
		setIsRebooting(bootMode);
		await WebApi.reboot(bootMode);
		setIsRebooting(-1);
	};

	const updateButtonLabels = (e) => {
		saveButtonLabels(e.target.value);
		setButtonLabels({ buttonLabelType: e.target.value });
	};

	return (
		<Navbar collapseOnSelect bg="primary" variant="dark" expand="md" fixed="top">
			<Navbar.Brand href="/">
				<img src="images/logo.png" className="title-logo" alt="logo" />{' '}GP2040
			</Navbar.Brand>
			<Navbar.Collapse id="basic-navbar-nav">
				<Nav className="me-auto">
					<Nav.Link as={NavLink} exact="true" to="/">Home</Nav.Link>
					<Nav.Link as={NavLink} exact="true" to="/settings">Settings</Nav.Link>
					<NavDropdown title="Configuration">
						<NavDropdown.Item as={NavLink} exact="true" to="/pin-mapping">Pin Mapping</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/keyboard-mapping">Keyboard Mapping</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/led-config">LED Configuration</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/custom-theme">Custom LED Theme</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/display-config">Display Configuration</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/add-ons">Add-Ons Configuration</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/backup">Data Backup and Restoration</NavDropdown.Item>
					</NavDropdown>
					<NavDropdown title="Links">
						<NavDropdown.Item href="https://gp2040-ce.info/" target="_blank">Documentation</NavDropdown.Item>
						<NavDropdown.Item href="https://github.com/OpenStickCommunity/GP2040-CE" target="_blank">Github</NavDropdown.Item>
					</NavDropdown>

					<Dropdown>
						<Dropdown.Toggle variant="danger">
							DANGER ZONE
						</Dropdown.Toggle>

						<Dropdown.Menu>
							<Dropdown.Item href="/reset-settings">Reset Settings</Dropdown.Item>
						</Dropdown.Menu>
					</Dropdown>
				</Nav>
				<Nav>
					<ColorScheme />
					<Button style={{ marginRight: "7px" }} variant="success" onClick={handleShow}>
						Reboot
					</Button>
					<div style={{ marginTop: "4px", marginRight: "10px" }}>
						<FormSelect
							name="buttonLabels"
							className="form-select-sm"
							value={buttonLabels.buttonLabelType}
							onChange={updateButtonLabels}
						>
							{Object.keys(BUTTONS).map((b, i) =>
								<option key={`button-label-option-${i}`} value={BUTTONS[b].value}>{BUTTONS[b].label}</option>
							)}
						</FormSelect>
						<Navbar.Toggle aria-controls="basic-navbar-nav" />
					</div>
				</Nav>
			</Navbar.Collapse>

			<Modal show={show} onHide={handleClose}>
				<Modal.Header closeButton>
					<Modal.Title>Reboot?</Modal.Title>
				</Modal.Header>
				<Modal.Body>{ isRebooting === -1 ? "Done rebooting, this browser tab can now be closed."
								: "Select a mode to reboot to" }</Modal.Body>
				<Modal.Footer>
					<Button variant="secondary" onClick={() => handleReboot(BOOT_MODES.BOOTSEL)}>
						{ isRebooting !== BOOT_MODES.BOOTSEL ? "USB (BOOTSEL)" : (isRebooting ? "Rebooting" : "Done!") }
					</Button>
					<Button variant="primary" onClick={() => handleReboot(BOOT_MODES.WEBCONFIG)}>
						{ isRebooting !== BOOT_MODES.WEBCONFIG ? "Web-config" : (isRebooting ? "Rebooting" : "Done!") }
					</Button>
					<Button variant="success" onClick={() => handleReboot(BOOT_MODES.GAMEPAD)}>
						{ isRebooting !== BOOT_MODES.GAMEPAD ? "Controller" : (isRebooting ? "Rebooting" : "Done!") }
					</Button>
				</Modal.Footer>
			</Modal>
		</Navbar>
	);
};

export default Navigation;
