import React, { useContext, useEffect, useState } from 'react';
import { AppContext } from '../Contexts/AppContext';
import { Button, Form } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import * as yup from 'yup';

import Section from '../Components/Section';
import WebApi from '../Services/WebApi';
import BUTTONS from '../Data/Buttons.json';

const INPUT_MODES = [
	{ label: 'XInput', value: 0 },
	{ label: 'Nintendo Switch', value: 1 },
	{ label: 'PS3/DirectInput', value: 2 },
	{ label: 'Keyboard', value: 3 },
	{ label: 'PS4', value: 4 }
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
});

const FormContext = () => {
	const { values, setValues } = useFormikContext();

	useEffect(() => {
		async function fetchData() {
			setValues(await WebApi.getGamepadOptions());
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
	const { buttonLabels } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');

	const onSuccess = async (values) => {
		const success = await WebApi.setGamepadOptions(values);
		setSaveMessage(success ? 'Saved! Please Restart Your Device' : 'Unable to Save');
	};

	return (
		<Formik validationSchema={schema} onSubmit={onSuccess} initialValues={{}}>
			{({
				handleSubmit,
				handleChange,
				values,
				errors,
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
					</Section>
					<Section title="Hotkey Settings">
						<div className='row'>
							<Form.Label className='col'>{BUTTONS[buttonLabels]["S1"] + " + " + BUTTONS[buttonLabels]["S2"]}</Form.Label>
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
							<Form.Label className='col'>{BUTTONS[buttonLabels]["S2"] + " + " + BUTTONS[buttonLabels]["A1"]}</Form.Label>
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
					</Section>
					<Button type="submit">Save</Button>
					{saveMessage ? <span className="alert">{saveMessage}</span> : null}
					<FormContext />
					</Form>
				</div>
			)}
		</Formik>
	);
}
