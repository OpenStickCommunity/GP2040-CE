import React, { useContext, useState } from 'react';
import {
	Nav,
	NavDropdown,
	Navbar,
	Button,
	Modal,
	Dropdown,
} from 'react-bootstrap';
import { NavLink } from 'react-router-dom';
import { useTranslation } from 'react-i18next';
import { AppContext } from '../Contexts/AppContext';
import FormSelect from './FormSelect';
import { saveButtonLabels } from '../Services/Storage';
import { BUTTONS } from '../Data/Buttons';
import './Navigation.scss';
import WebApi from '../Services/WebApi';
import ColorScheme from './ColorScheme';
import LanguageSelector from './LanguageSelector';

const BOOT_MODES = {
	GAMEPAD: 0,
	WEBCONFIG: 1,
	BOOTSEL: 2,
};

const Navigation = (props) => {
	const { buttonLabels, setButtonLabels } = useContext(AppContext);

	const [show, setShow] = useState(false);
	const [isRebooting, setIsRebooting] = useState(null); // null because we want the button to assume untouched state

	const handleClose = () => setShow(false);
	const handleShow = () => {
		setIsRebooting(null);
		setShow(true);
	};
	const handleReboot = async (bootMode) => {
		if (isRebooting === false) {
			setShow(false);
			return;
		}
		setIsRebooting(bootMode);
		await WebApi.reboot(bootMode);
		setIsRebooting(-1);
	};

	const updateButtonLabels = (e) => {
		saveButtonLabels(e.target.value);
		setButtonLabels({ buttonLabelType: e.target.value });
	};

	const { t } = useTranslation('');

	return (
		<Navbar
			collapseOnSelect
			bg="primary"
			variant="dark"
			expand="md"
			fixed="top"
		>
			<Navbar.Brand href="/">
				<img src="images/logo.png" className="title-logo" alt="logo" />{' '}
				{t('Common:brand-text')}
			</Navbar.Brand>
			<Navbar.Collapse id="basic-navbar-nav">
				<Nav className="me-auto">
					<Nav.Link as={NavLink} exact="true" to="/">
						{t('Navigation:home-label')}
					</Nav.Link>
					<Nav.Link as={NavLink} exact="true" to="/settings">
						{t('Navigation:settings-label')}
					</Nav.Link>
					<NavDropdown title={t('Navigation:config-label')}>
						<NavDropdown.Item as={NavLink} exact="true" to="/pin-mapping">
							{t('Navigation:pin-mapping-label')}
						</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/keyboard-mapping">
							{t('Navigation:keyboard-mapping-label')}
						</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/profile-settings">
							{t('Navigation:profile-settings-label')}
						</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/led-config">
							{t('Navigation:led-config-label')}
						</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/custom-theme">
							{t('Navigation:custom-theme-label')}
						</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/display-config">
							{t('Navigation:display-config-label')}
						</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/add-ons">
							{t('Navigation:add-ons-label')}
						</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/macro">
							{t('Navigation:macro-label')}
						</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} exact="true" to="/backup">
							{t('Navigation:backup-label')}
						</NavDropdown.Item>
					</NavDropdown>
					<NavDropdown title={t('Navigation:links-label')}>
						<NavDropdown.Item href="https://gp2040-ce.info/" target="_blank">
							{t('Navigation:docs-label')}
						</NavDropdown.Item>
						<NavDropdown.Item
							href="https://github.com/OpenStickCommunity/GP2040-CE"
							target="_blank"
						>
							{t('Navigation:github-label')}
						</NavDropdown.Item>
					</NavDropdown>

					<Dropdown>
						<Dropdown.Toggle variant="danger">
							{t('Navigation:dangerZone-label').toUpperCase()}
						</Dropdown.Toggle>

						<Dropdown.Menu>
							<Dropdown.Item href="/reset-settings">
								{t('Navigation:resetSettings-label')}
							</Dropdown.Item>
						</Dropdown.Menu>
					</Dropdown>
				</Nav>
				<Nav>
					<LanguageSelector />
					<ColorScheme />
					<Button
						style={{ marginRight: '7px' }}
						variant="success"
						onClick={handleShow}
					>
						{t('Navigation:reboot-label')}
					</Button>
					<div style={{ marginTop: '4px', marginRight: '10px' }}>
						<FormSelect
							name="buttonLabels"
							className="form-select-sm"
							value={buttonLabels.buttonLabelType}
							onChange={updateButtonLabels}
						>
							{Object.keys(BUTTONS).map((b, i) => (
								<option
									key={`button-label-option-${i}`}
									value={BUTTONS[b].value}
								>
									{BUTTONS[b].label}
								</option>
							))}
						</FormSelect>
						<Navbar.Toggle aria-controls="basic-navbar-nav" />
					</div>
				</Nav>
			</Navbar.Collapse>

			<Modal show={show} onHide={handleClose}>
				<Modal.Header closeButton>
					<Modal.Title>{t('Navigation:reboot-modal-label')}</Modal.Title>
				</Modal.Header>
				<Modal.Body>
					{isRebooting === -1
						? t('Navigation:reboot-modal-success')
						: t('Navigation:reboot-modal-body')}
				</Modal.Body>
				<Modal.Footer>
					<Button
						variant="secondary"
						onClick={() => handleReboot(BOOT_MODES.BOOTSEL)}
					>
						{isRebooting !== BOOT_MODES.BOOTSEL
							? t('Navigation:reboot-modal-button-bootsel-label')
							: isRebooting
							? t('Navigation:reboot-modal-button-progress-label')
							: t('Navigation:reboot-modal-button-success-label')}
					</Button>
					<Button
						variant="primary"
						onClick={() => handleReboot(BOOT_MODES.WEBCONFIG)}
					>
						{isRebooting !== BOOT_MODES.WEBCONFIG
							? t('Navigation:reboot-modal-button-web-config-label')
							: isRebooting
							? t('Navigation:reboot-modal-button-progress-label')
							: t('Navigation:reboot-modal-button-success-label')}
					</Button>
					<Button
						variant="success"
						onClick={() => handleReboot(BOOT_MODES.GAMEPAD)}
					>
						{isRebooting !== BOOT_MODES.GAMEPAD
							? t('Navigation:reboot-modal-button-controller-label')
							: isRebooting
							? t('Navigation:reboot-modal-button-progress-label')
							: t('Navigation:reboot-modal-button-success-label')}
					</Button>
				</Modal.Footer>
			</Modal>
		</Navbar>
	);
};

export default Navigation;
