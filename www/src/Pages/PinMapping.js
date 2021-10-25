import React, { useEffect, useState } from 'react';
import { Button, Form } from 'react-bootstrap';
import axios from 'axios';

import pinMapping from '../Data/PinMapping.json'
import boardParams from '../Data/BoardParams.json'

import './PinMappings.scss';

const baseButtonMappings = {
	Up:    { pin: -1, button: 'Up', xinput: 'Up', switch: 'Up', ps3: 'Up', dinput: 'Up', arcade: 'Up', error: null },
	Down:  { pin: -1, button: 'Down', xinput: 'Down', switch: 'Down', ps3: 'Down', dinput: 'Down', arcade: 'Down', error: null },
	Left:  { pin: -1, button: 'Left', xinput: 'Left', switch: 'Left', ps3: 'Left', dinput: 'Left', arcade: 'Left', error: null },
	Right: { pin: -1, button: 'Right', xinput: 'Right', switch: 'Right', ps3: 'Right', dinput: 'Right', arcade: 'Right', error: null },
	B1:    { pin: -1, button: 'B1', xinput: 'A', switch: 'B', ps3: 'Cross', dinput: '2', arcade: 'K1', error: null },
	B2:    { pin: -1, button: 'B2', xinput: 'B', switch: 'A', ps3: 'Circle', dinput: '3', arcade: 'K2', error: null },
	B3:    { pin: -1, button: 'B3', xinput: 'X', switch: 'Y', ps3: 'Square', dinput: '1', arcade: 'P1', error: null },
	B4:    { pin: -1, button: 'B4', xinput: 'Y', switch: 'X', ps3: 'Triangle', dinput: '4', arcade: 'P2', error: null },
	L1:    { pin: -1, button: 'L1', xinput: 'LB', switch: 'L', ps3: 'L1', dinput: '5', arcade: 'L1', error: null },
	R1:    { pin: -1, button: 'R1', xinput: 'RB', switch: 'R', ps3: 'R1', dinput: '6', arcade: 'R1', error: null },
	L2:    { pin: -1, button: 'L2', xinput: 'LT', switch: 'ZL', ps3: 'L2', dinput: '7', arcade: 'L2', error: null },
	R2:    { pin: -1, button: 'R2', xinput: 'RT', switch: 'ZR', ps3: 'R2', dinput: '8', arcade: 'R2', error: null },
	S1:    { pin: -1, button: 'S1', xinput: 'Back', switch: 'Minus', ps3: 'Select', dinput: '9', arcade: 'Coin', error: null },
	S2:    { pin: -1, button: 'S2', xinput: 'Start', switch: 'Plus', ps3: 'Start', dinput: '10', arcade: 'Start', error: null },
	L3:    { pin: -1, button: 'L3', xinput: 'LS', switch: 'LS', ps3: 'L3', dinput: '11', arcade: 'L3', error: null },
	R3:    { pin: -1, button: 'R3', xinput: 'RS', switch: 'RS', ps3: 'R3', dinput: '12', arcade: 'R3', error: null },
	A1:    { pin: -1, button: 'A1', xinput: 'Guide', switch: 'Home', ps3: 'N/A', dinput: '13', arcade: 'Home', error: null },
	A2:    { pin: -1, button: 'A2', xinput: 'N/A', switch: 'Capture', ps3: 'N/A', dinput: '14', arcade: 'N/A', error: null },
};

const buttonLabelOptions = [
	{ label: 'GP2040', value: 'button' },
	{ label: 'Arcade', value: 'arcade' },
	{ label: 'XInput', value: 'xinput' },
	{ label: 'Nintendo Switch', value: 'switch' },
	{ label: 'PS3', value: 'ps3' },
	{ label: 'DirectInput', value: 'dinput' },
];

const requiredButtons = ['B1', 'B2', 'B3', 'S2'];

export default function PinMappingPage() {
	const [buttonMappings, setButtonMappings] = useState(baseButtonMappings);
	const [selectedBoard] = useState(process.env.REACT_APP_GP2040_BOARD);
	const [selectedButtonLabels, setSelectedButtonLabels] = useState(buttonLabelOptions[0]);
	const [validated, setValidated] = useState(false);
	const [saveMessage, setSaveMessage] = useState('');

	useEffect(() => {
		if (process.env.NODE_ENV === 'production') {
			axios.get('cgi/action?method=getPinMappings')
				.then((response) => {
					let newMappings = {...baseButtonMappings};
					for (let prop of Object.keys(response.data)) {
						if (newMappings[prop])
							newMappings[prop].pin = parseInt(response.data[prop]);
					}

					setButtonMappings(newMappings);
				})
				.catch(console.error);
		}
		else {
			// Test code
			let newMappings = {...baseButtonMappings};
			for (let prop of Object.keys(pinMapping[selectedBoard])) {
				if (newMappings[prop])
					newMappings[prop].pin = parseInt(pinMapping[selectedBoard][prop]);
			}

			setButtonMappings(newMappings);
		}
	}, [selectedBoard]);

	const buttonLabelsChanged = (e) => {
		setSelectedButtonLabels(buttonLabelOptions.filter(o => o.value === e.target.value)[0]);
	};

	const handlePinChange = (e, prop) => {
		const newMappings = {...buttonMappings};
		if (e.target.value)
			newMappings[prop].pin = parseInt(e.target.value);
		else
			newMappings[prop].pin = '';

		validateMappings(newMappings);
	};

	const handleSubmit = (e) => {
		e.preventDefault();

		let newMappings = {...buttonMappings};
		validateMappings(newMappings);

		if (Object.keys(newMappings).filter(p => !!newMappings[p].error).length) {
			setSaveMessage('Validation errors, see above');
			return;
		}

		const data = Object.keys(newMappings).reduce((last, next) => {
			let prefix = '';
			if (last)
				prefix = `${last}|`;

			return `${prefix}${next}:${newMappings[next].pin}`;
		}, '');

		if (process.env.NODE_ENV === 'production') {
			axios.get(`cgi/action?method=setPinMappings&mappings=${data}`)
				.then((response) => {
					console.log(response.data);
					setSaveMessage("Saved!");
				})
				.catch((err) => {
					console.error(err);
					setSaveMessage(err);
				});
		}
		else {
			console.log(data);
			setSaveMessage(data);
		}
	};

	const validateMappings = (mappings) => {
		const props = Object.keys(mappings);

		for (let prop of props) {
			mappings[prop].error = null;

			for (let otherProp of props) {
				if (prop === otherProp)
					continue;

				if (mappings[prop].pin === '' && !requiredButtons.filter(b => b === mappings[otherProp].button).length)
					mappings[prop].error = `${mappings[prop].button} is required`;
				else if (mappings[prop].pin === mappings[otherProp].pin)
					mappings[prop].error = `Pin ${mappings[prop].pin} is already assigned`;
				else if (boardParams[selectedBoard]?.invalid.filter(p => p === mappings[prop].pin).length > 0)
					mappings[prop].error = `Pin ${mappings[prop].pin} is invalid for this board`;
			}

		}

		setButtonMappings(mappings);
		setValidated(true);
	};

	return (
		<div>
			<Form noValidate validated={validated} onSubmit={handleSubmit}>
				<header>
					<h1>Pin Mapping</h1>
				</header>
				<p>Use the form below to reconfigure your button-to-pin mapping.</p>
				<p className="alert alert-warning">
					Mapping buttons to pins you don't have connected or available can leave your device in non-functional state.
					If this happens, you can reset your device by flashing <a href="downloads/flash_nuke.uf2">flash_nuke.uf2</a> to your
					RP2040 board. Then reflash GP2040 onto the controller to get back to defaults.
				</p>
				<Form.Group className="select-button-labels-container">
					<Form.Label>Labels</Form.Label>
					<Form.Select className="select-button-labels form-select-sm" onChange={buttonLabelsChanged}>
						{buttonLabelOptions.map((o, i) => <option key={`button-label-option-${i}`} value={o.value}>{o.label}</option>)}
					</Form.Select>
				</Form.Group>
				<table className="table table-sm table-striped pin-mapping-table">
					<thead className="table-dark">
						<tr>
							<th className="table-header-button-label">{selectedButtonLabels.label}</th>
							<th>Pin</th>
						</tr>
					</thead>
					<tbody>
						{Object.keys(buttonMappings)?.map((prop, i) =>
							<tr key={`button-map-${i}`} className={validated && !!buttonMappings[prop].error ? "table-danger" : ""}>
								<td>{buttonMappings[prop][selectedButtonLabels.value]}</td>
								<td>
									<Form.Control
										type="number"
										className="pin-input form-control-sm"
										value={buttonMappings[prop].pin}
										min={boardParams[selectedBoard]?.min}
										max={boardParams[selectedBoard]?.max}
										isInvalid={!!buttonMappings[prop].error}
										required={requiredButtons.filter(b => b === buttonMappings[prop].button).length}
										onChange={(e) => handlePinChange(e, prop)}
									></Form.Control>
									<Form.Control.Feedback type="invalid">{buttonMappings[prop].error}</Form.Control.Feedback>
								</td>
							</tr>
						)}
					</tbody>
				</table>

				<Button type="submit">Save</Button>
				{saveMessage ? <span className="alert">{saveMessage}</span> : null}
			</Form>
		</div>
	);
}
