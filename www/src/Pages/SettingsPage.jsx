import React, { useContext, useEffect, useState } from 'react';
import { AppContext } from '../Contexts/AppContext';
import { Button, Form, Modal } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import * as yup from 'yup';

import Section from '../Components/Section';
import WebApi from '../Services/WebApi';
import { BUTTONS } from '../Data/Buttons';

const PS4Mode = 4;
const INPUT_MODES = [
	{ label: 'XInput', value: 0 },
	{ label: 'Nintendo Switch', value: 1 },
	{ label: 'PS3/DirectInput', value: 2 },
	{ label: 'Keyboard', value: 3 },
	{ label: 'PS4', value: PS4Mode }
];

const DPAD_MODES = [
	{ label: 'D-pad', value: 0 },
	{ label: 'Left Analog', value: 1 },
	{ label: 'Right Analog', value: 2 },
];

const SOCD_MODES = [
	{ label: 'Up Priority', value: 0 },
	{ label: 'Neutral', value: 1 },
	{ label: 'Last Win', value: 2 },
	{ label: 'First Win', value: 3 },
	{ label: 'Off', value: 4 },
];

const HOTKEY_MASKS = [
	{ label: 'Up', value: 1<<0 },
	{ label: 'Down', value: 1<<1 },
	{ label: 'Left', value: 1<<2 },
	{ label: 'Right', value: 1<<3 },
];

const HOTKEY_ACTIONS = [
	{ label: 'No Action', value: 0 },
	{ label: 'Dpad Digital', value: 1 },
	{ label: 'Dpad Left Analog', value: 2 },
	{ label: 'Dpad Right Analog', value: 3 },
	{ label: 'Home Button', value: 4 },
	{ label: 'Capture Button', value: 5 },
	{ label: 'SOCD UP Priority', value: 6 },
	{ label: 'SOCD Neutral', value: 7 },
	{ label: 'SOCD Last Win', value: 8 },
	{ label: 'SOCD First Win', value: 11 },
	{ label: 'SOCD Cleaning Off', value: 12 },
	{ label: 'Invert X Axis', value: 9 },
	{ label: 'Invert Y Axis', value: 10 },
];

const FORCED_SETUP_MODES = [
	{ label: 'Off', value: 0 },
	{ label: 'Disable Input-Mode Switch', value: 1 },
	{ label: 'Disable Web-Config', value: 2 },
	{ label: 'Disable Input-Mode Switch and Web-Config', value: 3 },
];

const schema = yup.object().shape({
	dpadMode : yup.number().required().oneOf(DPAD_MODES.map(o => o.value)).label('D-Pad Mode'),
	hotkeyF1 : yup.array().of(yup.object({
		action: yup.number().required().oneOf(HOTKEY_ACTIONS.map(o => o.value)).label('Hotkey action'),
		mask: yup.number().required().oneOf(HOTKEY_MASKS.map(o => o.value)).label('Hotkey action')
	})),
	hotkeyF2 : yup.array().of(yup.object({
		action: yup.number().required().oneOf(HOTKEY_ACTIONS.map(o => o.value)).label('Hotkey action'),
		mask: yup.number().required().oneOf(HOTKEY_MASKS.map(o => o.value)).label('Hotkey action')
	})),
	inputMode: yup.number().required().oneOf(INPUT_MODES.map(o => o.value)).label('Input Mode'),
	socdMode : yup.number().required().oneOf(SOCD_MODES.map(o => o.value)).label('SOCD Cleaning Mode'),
	switchTpShareForDs4: yup.number().required().label('Switch Touchpad and Share'),
	forcedSetupMode : yup.number().required().oneOf(FORCED_SETUP_MODES.map(o => o.value)).label('SOCD Cleaning Mode'),
	lockHotkeys: yup.number().required().label('Lock Hotkeys'),
});

const FormContext = ({ setButtonLabels }) => {
	const { values, setValues } = useFormikContext();

	useEffect(() => {
		async function fetchData() {
			const options = await WebApi.getGamepadOptions()
			setValues(options);
			setButtonLabels({ swapTpShareLabels: (options.switchTpShareForDs4 === 1) && (options.inputMode === 4) });
		}
		fetchData();
	}, [setValues]);

	useEffect(() => {
		if (!!values.dpadMode)
			values.dpadMode = parseInt(values.dpadMode);
		if (!!values.inputMode)
			values.inputMode = parseInt(values.inputMode);
		if (!!values.socdMode)
			values.socdMode = parseInt(values.socdMode);
		if (!!values.switchTpShareForDs4)
			values.switchTpShareForDs4 = parseInt(values.switchTpShareForDs4);
		if (!!values.forcedSetupMode)
			values.forcedSetupMode = parseInt(values.forcedSetupMode);
		if (!!values.lockHotkeys)
			values.lockHotkeys = parseInt(values.lockHotkeys);

		setButtonLabels({ swapTpShareLabels: (values.switchTpShareForDs4 === 1) && (values.inputMode === 4) });

		values.hotkeyF1 = values.hotkeyF1?.map( i => ({
			action: parseInt(i.action),
			mask: parseInt(i.mask)
		}));
		values.hotkeyF2 = values.hotkeyF2?.map( i => ({
			action: parseInt(i.action),
			mask: parseInt(i.mask)
		}));
	}, [values, setValues]);

	return null;
};

export default function SettingsPage() {
	const { buttonLabels, setButtonLabels } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');
	const [warning, setWarning] = useState({ show: false, acceptText: ''});

	const WARNING_CHECK_TEXT = "GP2040-CE";

	const handleWarningClose = async (accepted, values, setFieldValue) => {
		setWarning({ show: false, acceptText: ''});
		if (accepted) await saveSettings(values);
		else setFieldValue('forcedSetupMode', 0);
	};

	const setWarningAcceptText = (e) => {
		setWarning({ ...warning, acceptText: e.target.value });
	};

	const saveSettings = async (values) => {
		const success = await WebApi.setGamepadOptions(values);
		setSaveMessage(success ? 'Saved! Please Restart Your Device' : 'Unable to Save');
	};

	const onSuccess = async (values) => {
		if (values.forcedSetupMode > 1) { setWarning({ show: true, acceptText: ''}); }
		else { await saveSettings(values); }
	};

	const { buttonLabelType, swapTpShareLabels } = buttonLabels;

	const buttonLabelS1 = BUTTONS[buttonLabelType][(swapTpShareLabels && buttonLabelType === "ps4") ? "A2" : "S1"];
	const buttonLabelS2 = BUTTONS[buttonLabelType]["S2"];
	const buttonLabelA1 = BUTTONS[buttonLabelType]["A1"];

	return (
		<Formik validationSchema={schema} onSubmit={onSuccess} initialValues={{}}>
			{({
				handleSubmit,
				handleChange,
				values,
				errors,
				setFieldValue
			}) => console.log('errors', errors) || (
				<div>
					<Form noValidate onSubmit={handleSubmit}>
					<Section title="Settings">
						<Form.Group className="row mb-3">
							<Form.Label>Input Mode</Form.Label>
							<div className="col-sm-3">
								<Form.Select name="inputMode" className="form-select-sm" value={values.inputMode} onChange={handleChange} isInvalid={errors.inputMode}>
									{INPUT_MODES.map((o, i) => <option key={`button-inputMode-option-${i}`} value={o.value}>{o.label}</option>)}
								</Form.Select>
								<Form.Control.Feedback type="invalid">{errors.inputMode}</Form.Control.Feedback>
							</div>
							<div className="col-sm-3">
								{values.inputMode === PS4Mode && <Form.Check
									label="Switch Touchpad and Share"
									type="switch"
									name="switchTpShareForDs4"
									isInvalid={false}
									checked={Boolean(values.switchTpShareForDs4)}
									onChange={(e) => { setFieldValue("switchTpShareForDs4", e.target.checked ? 1 : 0); }}
								/>}
							</div>
						</Form.Group>
						<Form.Group className="row mb-3">
							<Form.Label>D-Pad Mode</Form.Label>
							<div className="col-sm-3">
								<Form.Select name="dpadMode" className="form-select-sm" value={values.dpadMode} onChange={handleChange} isInvalid={errors.dpadMode}>
									{DPAD_MODES.map((o, i) => <option key={`button-dpadMode-option-${i}`} value={o.value}>{o.label}</option>)}
								</Form.Select>
								<Form.Control.Feedback type="invalid">{errors.dpadMode}</Form.Control.Feedback>	
							</div>
						</Form.Group>
						<Form.Group className="row mb-3">
							<Form.Label>SOCD Cleaning Mode</Form.Label>
							<div className="col-sm-3">
								<Form.Select name="socdMode" className="form-select-sm" value={values.socdMode} onChange={handleChange} isInvalid={errors.socdMode}>
									{SOCD_MODES.map((o, i) => <option key={`button-socdMode-option-${i}`} value={o.value}>{o.label}</option>)}
								</Form.Select>
								<Form.Control.Feedback type="invalid">{errors.socdMode}</Form.Control.Feedback>
							</div>
						</Form.Group>
						<p>Note: PS4, PS3 and Nintendo Switch modes do not support setting SOCD Cleaning to Off and will default to Neutral SOCD Cleaning mode.</p>
						<Form.Group className="row mb-3">
							<Form.Label>Forced Setup Mode</Form.Label>
							<div className="col-sm-3">
								<Form.Select name="forcedSetupMode" className="form-select-sm" value={values.forcedSetupMode} onChange={handleChange} isInvalid={errors.forcedSetupMode}>
									{FORCED_SETUP_MODES.map((o, i) => <option key={`button-forcedSetupMode-option-${i}`} value={o.value}>{o.label}</option>)}
								</Form.Select>
								<Form.Control.Feedback type="invalid">{errors.forcedSetupMode}</Form.Control.Feedback>
							</div>
						</Form.Group>
					</Section>
					<Section title="Hotkey Settings">
						<div id="Hotkeys"
							hidden={values.lockHotkeys}>
							<div className='row'>
								<Form.Label className='col'>{buttonLabelS1 + " + " + buttonLabelS2}</Form.Label>
							</div>
							{HOTKEY_MASKS.map((o, i) =>
								<Form.Group key={`hotkey-${i}`} className="row mb-3">
								<div className="col-sm-1">{o.label}</div>
								<div className="col-sm-2">
									<Form.Select name={`hotkeyF1[${i}].action`} className="form-select-sm" value={values?.hotkeyF1 && values?.hotkeyF1[i]?.action} onChange={handleChange} isInvalid={errors?.hotkeyF1 && errors?.hotkeyF1[i]?.action}>
										{HOTKEY_ACTIONS.map((o, i) => <option key={`f1-option-${i}`} value={o.value}>{o.label}</option>)}
									</Form.Select>
									<Form.Control.Feedback type="invalid">{errors?.hotkeyF1 && errors?.hotkeyF1[i]?.action}</Form.Control.Feedback>
								</div>
								</Form.Group>
							)}	
							<div className='row'>
								<Form.Label className='col'>{buttonLabelS2 + " + " + buttonLabelA1}</Form.Label>
							</div>
							{HOTKEY_MASKS.map((o, i) =>
								<Form.Group key={`hotkey-${i}`} className="row mb-3">
								<div className="col-sm-1">{o.label}</div>
								<div className="col-sm-2">
									<Form.Select name={`hotkeyF2[${i}].action`} className="form-select-sm" value={values?.hotkeyF2 && values?.hotkeyF2[i]?.action} onChange={handleChange} isInvalid={errors?.hotkeyF2 && errors?.hotkeyF2[i]?.action}>
										{HOTKEY_ACTIONS.map((o, i) => <option key={`f2-option-${i}`} value={o.value}>{o.label}</option>)}
									</Form.Select>
									<Form.Control.Feedback type="invalid">{errors?.hotkeyF2 && errors?.hotkeyF2[i]?.action}</Form.Control.Feedback>
								</div>
								</Form.Group>
							)}	
						</div>
						<Form.Check
							label="Lock Hotkeys"
							type="switch"
							id="LockHotkeys"
							reverse
							isInvalid={false}
							checked={Boolean(values.lockHotkeys)}
							onChange={(e) => { setFieldValue("lockHotkeys", e.target.checked ? 1 : 0); }}
						/>	
					</Section>
					<Button type="submit">Save</Button>
					{saveMessage ? <span className="alert">{saveMessage}</span> : null}
					<FormContext  setButtonLabels={setButtonLabels}/>
					</Form>
					<Modal size="lg" show={warning.show} onHide={handleWarningClose}>
						<Modal.Header closeButton>
							<Modal.Title>Forced Setup Mode Warning</Modal.Title>
						</Modal.Header>
						<Modal.Body>
							<div className='mb-3'>
								If you reboot to Controller mode after saving, you will no longer have access to the web-config.
								Please type "<strong>{WARNING_CHECK_TEXT}</strong>" below to unlock the Save button if you fully acknowledge this and intend it.
								Clicking on Dismiss will revert this setting which then is to be saved.
							</div>
							<Form.Control value={warning.acceptText} onChange={setWarningAcceptText}></Form.Control>
						</Modal.Body>
						<Modal.Footer>
							<Button disabled={warning.acceptText != WARNING_CHECK_TEXT} variant="warning" onClick={() => handleWarningClose(true, values)}>
								Save
							</Button>
							<Button variant="primary" onClick={() => handleWarningClose(false, values, setFieldValue)}>
								Dismiss
							</Button>
						</Modal.Footer>
					</Modal>
				</div>
			)}
		</Formik>
	);
}
