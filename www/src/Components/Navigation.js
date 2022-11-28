import React, { useContext } from 'react';
import { Nav, NavDropdown, Navbar } from 'react-bootstrap';
import { NavLink } from "react-router-dom";
import { AppContext } from '../Contexts/AppContext';
import FormSelect from './FormSelect';
import { saveButtonLabels } from '../Services/Storage';
import BUTTONS from '../Data/Buttons.json';
import './Navigation.scss';

const Navigation = (props) => {
	const { buttonLabels, setButtonLabels } = useContext(AppContext);

	const updateButtonLabels = (e) => {
		saveButtonLabels(e.target.value);
		setButtonLabels(e.target.value);
	};

	return (
		<Navbar collapseOnSelect bg="dark" variant="dark" expand="md" fixed="top">
			<Navbar.Brand href="/">
				<img src="images/logo.png" className="title-logo" alt="logo" />{' '}GP2040
			</Navbar.Brand>
			<Navbar.Collapse id="basic-navbar-nav">
				<Nav className="me-auto">
					<Nav.Link as={NavLink} exact={true} to="/">Home</Nav.Link>
					<Nav.Link as={NavLink} exact={true} to="/settings">Settings</Nav.Link>
					<NavDropdown title="Configuration">
						<NavDropdown.Item as={NavLink} exact={true} to="/pin-mapping">Pin Mapping</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact={true} to="/led-config">LED Configuration</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact={true} to="/display-config">Display Configuration</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact={true} to="/add-ons">Add-Ons Configuration</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact={true} to="/backup">Data Backup and Restoration</NavDropdown.Item>
					</NavDropdown>
					<NavDropdown title="Links">
						<NavDropdown.Item as={NavLink} to={{ pathname: "https://gp2040-ce.info/" }} target="_blank">Documentation</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} to={{ pathname: "https://github.com/OpenStickFoundation/GP2040-CE" }} target="_blank">Github</NavDropdown.Item>
					</NavDropdown>
					<NavDropdown title="DANGER ZONE" className="btn-danger danger-zone">
						<NavDropdown.Item as={NavLink} exact={true} to="/reset-settings">Reset Settings</NavDropdown.Item>
					</NavDropdown>
				</Nav>
			</Navbar.Collapse>
			<div className="navbar-tools">
				<FormSelect
					name="buttonLabels"
					className="form-select-sm"
					value={buttonLabels}
					onChange={updateButtonLabels}
				>
					{Object.keys(BUTTONS).map((b, i) =>
						<option key={`button-label-option-${i}`} value={BUTTONS[b].value}>{BUTTONS[b].label}</option>
					)}
				</FormSelect>
				<Navbar.Toggle aria-controls="basic-navbar-nav" />
			</div>
		</Navbar>
	);
};

export default Navigation;
