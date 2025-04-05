import { useContext, useState } from 'react';
import { Nav, NavDropdown, Navbar, Button, Modal } from 'react-bootstrap';
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

const Navigation = () => {
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

	// eventKey prop is required on NavLink components in order for mobile menu
	// to autoclose, so just auto increment as we build the menu
	let eventKey = 0;

	return (
		<Navbar collapseOnSelect expand="md" fixed="top">
			<Navbar.Brand title={`GP2040-CE ${t('Navigation:home-label')}`}>
				<Nav.Link as={NavLink} to="/" eventKey={eventKey++}>
					<img
						src="images/logo.png"
						className="title-logo"
						alt="GP2040-CE logo"
					/>
				</Nav.Link>
			</Navbar.Brand>
			<Navbar.Toggle aria-controls="responsive-navbar-nav" />
			<Navbar.Collapse id="basic-navbar-nav">
				<Nav className="me-auto nav-menu">
					<Nav.Link as={NavLink} to="/settings" eventKey={eventKey++}>
						{t('Navigation:settings-label')}
					</Nav.Link>
					<NavDropdown title={t('Navigation:config-label')}>
						<NavDropdown.Item
							as={NavLink}
							eventKey={eventKey++}
							to="/pin-mapping"
						>
							{t('Navigation:pin-mapping-label')}
						</NavDropdown.Item>
						<NavDropdown.Item
							as={NavLink}
							eventKey={eventKey++}
							to="/peripheral-mapping"
						>
							{t('Navigation:peripheral-mapping-label')}
						</NavDropdown.Item>
						<NavDropdown.Item
							as={NavLink}
							eventKey={eventKey++}
							to="/led-config"
						>
							{t('Navigation:led-config-label')}
						</NavDropdown.Item>
						<NavDropdown.Item
							as={NavLink}
							eventKey={eventKey++}
							to="/custom-theme"
						>
							{t('Navigation:custom-theme-label')}
						</NavDropdown.Item>
						<NavDropdown.Item
							as={NavLink}
							eventKey={eventKey++}
							to="/display-config"
						>
							{t('Navigation:display-config-label')}
						</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} eventKey={eventKey++} to="/add-ons">
							{t('Navigation:add-ons-label')}
						</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} eventKey={eventKey++} to="/macro">
							{t('Navigation:macro-label')}
						</NavDropdown.Item>
						<NavDropdown.Item as={NavLink} eventKey={eventKey++} to="/backup">
							{t('Navigation:backup-label')}
						</NavDropdown.Item>
						<NavDropdown.Item
							as={NavLink}
							eventKey={eventKey++}
							to="/reset-settings"
						>
							<span className="reset-settings-link">
								{t('Navigation:resetSettings-label')}
							</span>
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
				</Nav>
				<Nav className="navbar-actions">
					<Button variant="success" onClick={handleShow}>
						{t('Navigation:reboot-label')}
					</Button>
					<ColorScheme />
					<LanguageSelector />
					<div className="navbar-label-select">
						<FormSelect
							name="buttonLabels"
							className="form-select"
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
