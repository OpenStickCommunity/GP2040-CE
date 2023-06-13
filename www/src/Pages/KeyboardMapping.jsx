import React, { useContext, useEffect, useState } from 'react';
import { NavLink } from "react-router-dom";
import { Button, Form } from 'react-bootstrap';
import { AppContext } from '../Contexts/AppContext';
import FormSelect from '../Components/FormSelect';
import Section from '../Components/Section';
import WebApi, { baseButtonMappings } from '../Services/WebApi';
import boards from '../Data/Boards.json';
import { BUTTONS } from '../Data/Buttons';
import './PinMappings.scss';

const KEY_CODES = [
	{ label: "None", value: 0x00 },
	{ label: "Alt Left", value: 0xe2 },
	{ label: "Alt Right", value: 0xe6 },
	{ label: "Arrow Down", value: 0x51 },
	{ label: "Arrow Left", value: 0x50 },
	{ label: "Arrow Right", value: 0x4f },
	{ label: "Arrow Up", value: 0x52 },
	{ label: "Backquote", value: 0x35 },
	{ label: "Backslash", value: 0x31 },
	{ label: "Backspace", value: 0x2a },
	{ label: "Bracket Left", value: 0x2f },
	{ label: "Bracket Right", value: 0x30 },
	{ label: "CapsLock", value: 0x39 },
	{ label: "Comma", value: 0x36 },
	{ label: "Control Left", value: 0xe0 },
	{ label: "Delete", value: 0x4c },
	{ label: "0", value: 0x27 },
	{ label: "1", value: 0x1e },
	{ label: "2", value: 0x1f },
	{ label: "3", value: 0x20 },
	{ label: "4", value: 0x21 },
	{ label: "5", value: 0x22 },
	{ label: "6", value: 0x23 },
	{ label: "7", value: 0x24 },
	{ label: "8", value: 0x25 },
	{ label: "9", value: 0x26 },
	{ label: "End", value: 0x4d },
	{ label: "Enter", value: 0x28 },
	{ label: "Equal", value: 0x2e },
	{ label: "Escape", value: 0x29 },
	{ label: "F1", value: 0x3a },
	{ label: "F2", value: 0x3b },
	{ label: "F3", value: 0x3c },
	{ label: "F4", value: 0x3d },
	{ label: "F5", value: 0x3e },
	{ label: "F6", value: 0x3f },
	{ label: "F7", value: 0x40 },
	{ label: "F8", value: 0x41 },
	{ label: "F9", value: 0x42 },
	{ label: "F10", value: 0x43 },
	{ label: "F11", value: 0x44 },
	{ label: "F12", value: 0x45 },
	{ label: "Home", value: 0x4a },
	{ label: "Intl Backslash", value: 0x31 },
	{ label: "A", value: 0x04 },
	{ label: "B", value: 0x05 },
	{ label: "C", value: 0x06 },
	{ label: "D", value: 0x07 },
	{ label: "E", value: 0x08 },
	{ label: "F", value: 0x09 },
	{ label: "G", value: 0x0a },
	{ label: "H", value: 0x0b },
	{ label: "I", value: 0x0c },
	{ label: "J", value: 0x0d },
	{ label: "K", value: 0x0e },
	{ label: "L", value: 0x0f },
	{ label: "M", value: 0x10 },
	{ label: "N", value: 0x11 },
	{ label: "O", value: 0x12 },
	{ label: "P", value: 0x13 },
	{ label: "Q", value: 0x14 },
	{ label: "R", value: 0x15 },
	{ label: "S", value: 0x16 },
	{ label: "T", value: 0x17 },
	{ label: "U", value: 0x18 },
	{ label: "V", value: 0x19 },
	{ label: "W", value: 0x1a },
	{ label: "X", value: 0x1b },
	{ label: "Y", value: 0x1c },
	{ label: "Z", value: 0x1d },
	{ label: "Meta Left", value: 0xe3 },
	{ label: "Meta Right", value: 0xe7 },
	{ label: "Minus", value: 0x2d },
	{ label: "Numpad Enter", value: 0x58 },
	{ label: "Page Down", value: 0x4e },
	{ label: "Page Up", value: 0x4b },
	{ label: "Period", value: 0x37 },
	{ label: "Quote", value: 0x34 },
	{ label: "Semicolon", value: 0x33 },
	{ label: "Shift Left", value: 0xe1 },
	{ label: "Shift Right", value: 0xe5 },
	{ label: "Slash", value: 0x38 },
	{ label: "Space", value: 0x2c },
	{ label: "Tab", value: 0x2b }
];

export default function KeyboardMappingPage() {
	const { buttonLabels } = useContext(AppContext);
	const [validated, setValidated] = useState(false);
	const [saveMessage, setSaveMessage] = useState('');
	const [keyMappings, setKeyMappings] = useState(baseButtonMappings);
	const [selectedController] = useState(import.meta.env.VITE_GP2040_CONTROLLER);
	const [selectedBoard] = useState(import.meta.env.VITE_GP2040_BOARD);

	const { buttonLabelType, swapTpShareLabels } = buttonLabels;

	useEffect(() => {
		async function fetchData() {
			setKeyMappings(await WebApi.getKeyMappings());
		}

		fetchData();
	}, [setKeyMappings, selectedController]);

	const handleKeyChange = (e, prop) => {
		const newMappings = {...keyMappings};
		if (e.target.value)
			newMappings[prop].key = parseInt(e.target.value);
		else
			newMappings[prop].key = '';

		validateMappings(newMappings);
	};

	const handleSubmit = async (e) => {
		e.preventDefault();
		e.stopPropagation();

		let mappings = {...keyMappings};
		validateMappings(mappings);

		if (Object.keys(mappings).filter(p => !!mappings[p].error).length) {
			setSaveMessage('Validation errors, see above');
			return;
		}

		const success = await WebApi.setKeyMappings(mappings);
		setSaveMessage(success ? 'Saved! Please Restart Your Device' : 'Unable to Save');
	};

	const validateMappings = (mappings) => {
		const props = Object.keys(mappings);

		for (let prop of props) {
			mappings[prop].error = null;

			for (let otherProp of props) {
				if (prop === otherProp)
					continue;

				if (mappings[prop].key !== 0x00 && mappings[prop].key === mappings[otherProp].key)
					mappings[prop].error = `Key ${KEY_CODES.find(({label, value}) => mappings[prop].key === value).label} is already assigned`;
				else if ((boards[selectedBoard].invalidKeys || []).filter(p => p === mappings[prop].key).length > 0)
					mappings[prop].error = `Key ${KEY_CODES.find(({label, value}) => mappings[prop].key === value).label} is invalid for this board`;
			}

		}

		setKeyMappings(mappings);
		setValidated(true);
	};

	return (
		<Section title="Keyboard Mapping">
			<Form noValidate validated={validated} onSubmit={handleSubmit}>
				<p>Use the form below to reconfigure your button-to-key mapping.</p>
				<table className="table table-sm pin-mapping-table">
					<thead className="table">
						<tr>
							<th className="table-header-button-label">{BUTTONS[buttonLabelType].label}</th>
							<th>Key</th>
						</tr>
					</thead>
					<tbody>
						{Object.keys(BUTTONS[buttonLabelType])?.filter(p => p !== 'label' && p !== 'value').map((button, i) => {
							let label = BUTTONS[buttonLabelType][button];
							if (button === "S1" && swapTpShareLabels && buttonLabelType === "ps4") {
								label = BUTTONS[buttonLabelType]["A2"];
							}
							if (button === "A2" && swapTpShareLabels && buttonLabelType === "ps4") {
								label = BUTTONS[buttonLabelType]["S1"];
							}
							return <tr key={`button-map-${i}`} className={validated && !!keyMappings[button].error ? "table-danger" : ""}>
								<td>{label}</td>
								<td>
									<FormSelect
										type="number"
										className="form-select-sm sm-3"
										value={keyMappings[button].key}
										isInvalid={!!keyMappings[button].error}
										error={keyMappings[button].error}
										onChange={(e) => handleKeyChange(e, button)}
									>
									{KEY_CODES.map((o, i) => <option key={`button-key-option-${i}`} value={o.value}>{o.label}</option>)}
									</FormSelect>
								</td>
							</tr>
						})}
					</tbody>
				</table>
				<Button type="submit">Save</Button>
				{saveMessage ? <span className="alert">{saveMessage}</span> : null}
			</Form>
		</Section>
	);
}
