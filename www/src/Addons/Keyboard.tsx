import { useContext } from 'react';
import { Trans, useTranslation } from 'react-i18next';
import { FormCheck, Row, FormLabel, Form } from 'react-bootstrap';
import { NavLink } from 'react-router-dom';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import KeyboardMapper from '../Components/KeyboardMapper';
import { baseButtonMappings } from '../Services/WebApi';
import { AppContext } from '../Contexts/AppContext';

import { BUTTON_ACTIONS } from '../Data/Pins';
import { BUTTON_MASKS_OPTIONS } from '../Data/Buttons';

export const keyboardScheme = {
	KeyboardHostAddonEnabled: yup
		.number()
		.required()
		.label('Keyboard Host Add-On Enabled'),
	keyboardHostMouseLeft: yup
		.number()
		.label('Left Mouse Button')
		.validateSelectionWhenValue(
			'KeyboardHostAddonEnabled',
			BUTTON_MASKS_OPTIONS,
		),
	keyboardHostMouseMiddle: yup
		.number()
		.label('Middle Mouse Button')
		.validateSelectionWhenValue(
			'KeyboardHostAddonEnabled',
			BUTTON_MASKS_OPTIONS,
		),
	keyboardHostMouseRight: yup
		.number()
		.label('Right Mouse Button')
		.validateSelectionWhenValue(
			'KeyboardHostAddonEnabled',
			BUTTON_MASKS_OPTIONS,
		),
	keyboardHostMouseSensitivity: yup.number().required().min(1).max(100),
	keyboardHostMouseMovement: yup.string().required().oneOf(['0', '1', '2']),
};

export const keyboardState = {
	keyboardHostMap: baseButtonMappings,
	keyboardHostMouseLeft: 0,
	keyboardHostMouseMiddle: 0,
	keyboardHostMouseRight: 0,
	KeyboardHostAddonEnabled: 0,
	keyboardHostMouseSensitivity: 0,
	keyboardHostMouseMovement: 0,
};

const excludedButtons = [
	'E1',
	'E2',
	'E3',
	'E4',
	'E5',
	'E6',
	'E7',
	'E8',
	'E9',
	'E10',
	'E11',
	'E12',
];

const Keyboard = ({
	values,
	errors,
	handleChange,
	handleCheckbox,
	setFieldValue,
}) => {
	const { buttonLabels, getAvailablePeripherals } = useContext(AppContext);
	const { t } = useTranslation();

	const handleKeyChange = (values, setFieldValue) => (value, button) => {
		const newMappings = { ...values.keyboardHostMap };
		newMappings[button].key = value;
		setFieldValue('keyboardHostMap', newMappings);
	};

	const getKeyMappingForButton = (values) => (button) => {
		return values.keyboardHostMap[button];
	};

	return (
		<Section
			title={
				<a
					href="https://gp2040-ce.info/add-ons/keyboard-host"
					target="_blank"
					className="text-reset text-decoration-none"
				>
					{t('AddonsConfig:keyboard-host-header-text')}
				</a>
			}
		>
			<div
				id="KeyboardHostAddonOptions"
				hidden={
					!(values.KeyboardHostAddonEnabled && getAvailablePeripherals('usb'))
				}
			>
				<div className="alert alert-info" role="alert">
					The D+ and Enable 5V pins and GPIO Pin Order are configured in{' '}
					<a href="../peripheral-mapping" className="alert-link">
						Peripheral Mapping
					</a>
				</div>
				<Row className="mb-3">
					<p>{t('AddonsConfig:keyboard-host-sub-header-text')}</p>
					<div className="mb-2">
						<KeyboardMapper
							buttonLabels={buttonLabels}
							excludeButtons={excludedButtons}
							handleKeyChange={handleKeyChange(values, setFieldValue)}
							getKeyMappingForButton={getKeyMappingForButton(values)}
						/>
					</div>
				</Row>
				<Row className="mb-3">
					<p>{t('AddonsConfig:keyboard-host-mouse-header-text')}</p>
					<div className="col-sm-12 col-md-6 col-lg-2 mb-2">
						<FormSelect
							label={t(`AddonsConfig:keyboard-host-left-mouse`)}
							name="keyboardHostMouseLeft"
							className="form-select-sm"
							id={`keyboardHostMouseLeft`}
							value={values.keyboardHostMouseLeft}
							error={errors.keyboardHostMouseLeft}
							onChange={handleChange}
						>
							{BUTTON_MASKS_OPTIONS.map((o, i) => (
								<option key={`keyboardHostMouseLeft-${i}`} value={o.value}>
									{o.label}
								</option>
							))}
						</FormSelect>
					</div>
					<div className="col-sm-12 col-md-6 col-lg-2 mb-2">
						<FormSelect
							label={t(`AddonsConfig:keyboard-host-middle-mouse`)}
							name="keyboardHostMouseMiddle"
							className="form-select-sm"
							id={`keyboardHostMouseMiddle`}
							value={values.keyboardHostMouseMiddle}
							error={errors.keyboardHostMouseLeft}
							onChange={handleChange}
						>
							{BUTTON_MASKS_OPTIONS.map((o, i) => (
								<option key={`keyboardHostMouseMiddle-${i}`} value={o.value}>
									{o.label}
								</option>
							))}
						</FormSelect>
					</div>
					<div className="col-sm-12 col-md-6 col-lg-2 mb-2">
						<FormSelect
							label={t(`AddonsConfig:keyboard-host-right-mouse`)}
							name="keyboardHostMouseRight"
							className="form-select-sm"
							id={`keyboardHostMouseRight`}
							value={values.keyboardHostMouseRight}
							error={errors.keyboardHostMouseRight}
							onChange={handleChange}
						>
							{BUTTON_MASKS_OPTIONS.map((o, i) => (
								<option key={`keyboardHostMouseRight-${i}`} value={o.value}>
									{o.label}
								</option>
							))}
						</FormSelect>
					</div>
					<div className="col-sm-12 mb-2">
						<Form.Label>{`${t('AddonsConfig:keyboard-host-mouse-sensitivity')}: ${values.keyboardHostMouseSensitivity}%`}</Form.Label>
						<Form.Range
							name="keyboardHostMouseSensitivity"
							id={`keyboardHostMouseSensitivity`}
							min={1}
							max={100}
							step={1}
							value={values.keyboardHostMouseSensitivity}
							onChange={handleChange}
						/>
					</div>
					<div className="col-sm-12 mb-2">
						<Form.Label>
							{t('AddonsConfig:keyboard-host-mouse-movement')}
						</Form.Label>
						<div className="d-flex gap-3">
							<FormCheck
								type="radio"
								id="mouseMovementNone"
								label={t('AddonsConfig:keyboard-host-mouse-movement-none')}
								name="keyboardHostMouseMovement"
								value={0}
								checked={values.keyboardHostMouseMovement === 0}
								onChange={(e) => {
									setFieldValue(
										'keyboardHostMouseMovement',
										parseInt(e.target.value),
									);
								}}
							/>
							<FormCheck
								type="radio"
								id="mouseMovementLeftAnalog"
								label={t(
									'AddonsConfig:keyboard-host-mouse-movement-left-analog',
								)}
								name="keyboardHostMouseMovement"
								value={1}
								checked={values.keyboardHostMouseMovement === 1}
								onChange={(e) => {
									setFieldValue(
										'keyboardHostMouseMovement',
										parseInt(e.target.value),
									);
								}}
							/>
							<FormCheck
								type="radio"
								id="mouseMovementRightAnalog"
								label={t(
									'AddonsConfig:keyboard-host-mouse-movement-right-analog',
								)}
								name="keyboardHostMouseMovement"
								value={2}
								checked={values.keyboardHostMouseMovement === 2}
								onChange={(e) => {
									setFieldValue(
										'keyboardHostMouseMovement',
										parseInt(e.target.value),
									);
								}}
							/>
						</div>
					</div>
				</Row>
			</div>
			{getAvailablePeripherals('usb') ? (
				<FormCheck
					label={t('Common:switch-enabled')}
					type="switch"
					id="KeyboardHostAddonButton"
					reverse
					isInvalid={false}
					checked={Boolean(values.KeyboardHostAddonEnabled)}
					onChange={(e) => {
						handleCheckbox('KeyboardHostAddonEnabled', values);
						handleChange(e);
					}}
				/>
			) : (
				<FormLabel>
					<Trans
						ns="PeripheralMapping"
						i18nKey="peripheral-toggle-unavailable"
						values={{ name: 'USB' }}
					>
						<NavLink to="/peripheral-mapping">
							{t('PeripheralMapping:header-text')}
						</NavLink>
					</Trans>
				</FormLabel>
			)}
		</Section>
	);
};

export default Keyboard;
