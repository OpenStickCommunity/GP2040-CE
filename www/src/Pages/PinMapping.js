import React, { useContext, useEffect, useState } from 'react';
import { NavLink } from "react-router-dom";
import { Button, Form } from 'react-bootstrap';
import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import WebApi, { baseButtonMappings } from '../Services/WebApi';
import boards from '../Data/Boards.json';
import { BUTTONS } from '../Data/Buttons';
import './PinMappings.scss';

const requiredButtons = ['S2'];
const errorType = {
	required: 'required',
	conflict: 'conflict',
	invalid: 'invalid',
	used: 'used',
};

export default function PinMappingPage() {
	const { buttonLabels, usedPins } = useContext(AppContext);
	const [validated, setValidated] = useState(false);
	const [saveMessage, setSaveMessage] = useState('');
	const [buttonMappings, setButtonMappings] = useState(baseButtonMappings);
	const [selectedController] = useState(process.env.REACT_APP_GP2040_CONTROLLER);
	const [selectedBoard] = useState(process.env.REACT_APP_GP2040_BOARD);

	useEffect(() => {
		async function fetchData() {
			setButtonMappings(await WebApi.getPinMappings());
		}

		fetchData();
	}, [setButtonMappings, selectedController]);

	const handlePinChange = (e, prop) => {
		const newMappings = {...buttonMappings};
		if (e.target.value)
			newMappings[prop].pin = parseInt(e.target.value);
		else
			newMappings[prop].pin = '';

		validateMappings(newMappings);
	};

	const handleSubmit = async (e) => {
		e.preventDefault();
		e.stopPropagation();

		let mappings = {...buttonMappings};
		validateMappings(mappings);

		if (Object.keys(mappings).filter(p => mappings[p].error).length > 0) {
			setSaveMessage('Validation errors, see above');
			return;
		}

		const success = await WebApi.setPinMappings(mappings);
		setSaveMessage(success ? 'Saved! Please Restart Your Device' : 'Unable to Save');
	};

	const validateMappings = (mappings) => {
		const buttons = Object.keys(mappings);

		// Create some mapped pin groups for easier error checking
		const mappedPins = buttons
			.filter(p => mappings[p].pin > -1)
			.reduce((a, p) => {
				a.push(mappings[p].pin);
				return a;
			}, []);
		const mappedPinCounts = mappedPins.reduce((a, p) => ({ ...a, [p]: (a[p] || 0) + 1 }), {});
		const uniquePins = mappedPins.filter((p, i, a) => a.indexOf(p) === i);
		const conflictedPins = Object.keys(mappedPinCounts).filter(p => mappedPinCounts[p] > 1).map(parseInt);
		const invalidPins = uniquePins.filter(p => boards[selectedBoard].invalidPins.indexOf(p) > -1);
		const otherPins = usedPins.filter(p => uniquePins.indexOf(p) === -1);

		for (let button of buttons) {
			mappings[button].error = '';

			// Validate required button
			if ((mappings[button].pin < boards[selectedBoard].minPin || mappings[button].pin > boards[selectedBoard].maxPin) && requiredButtons.filter(b => b === button).length)
				mappings[button].error = errorType.required;

			// Identify conflicted pins
			else if (conflictedPins.indexOf(mappings[button].pin) > -1)
				mappings[button].error = errorType.conflict;

			// Identify invalid pin assignments
			else if (invalidPins.indexOf(mappings[button].pin) > -1)
				mappings[button].error = errorType.invalid;

			// Identify used pins
			else if (otherPins.indexOf(mappings[button].pin) > -1)
				mappings[button].error = errorType.used;
		}

		setButtonMappings(mappings);
		setValidated(true);
	};

	const renderError = (button) => {
		if (buttonMappings[button].error === errorType.required) {
			return <span key="required" className="error-message">{`${BUTTONS[buttonLabels][button]} is required`}</span>;
		}
		else if (buttonMappings[button].error === errorType.conflict) {
			const conflictedMappings = Object.keys(buttonMappings)
				.filter(b => b !== button)
				.filter(b => buttonMappings[b].pin === buttonMappings[button].pin)
				.map(b => BUTTONS[buttonLabels][b]);

			return <span key="conflict" className="error-message">{`Pin ${buttonMappings[button].pin} is already assigned to ${conflictedMappings.join(', ')}`}</span>;
		}
		else if (buttonMappings[button].error === errorType.invalid) {
			return <span key="invalid" className="error-message">{`Pin ${buttonMappings[button].pin} is invalid for this board`}</span>;
		}
		else if (buttonMappings[button].error === errorType.used) {
			return <span key="used" className="error-message">{`Pin ${buttonMappings[button].pin} is already assigned to another feature`}</span>;
		}

		return <></>;
	};

	return (
		<Section title="Pin Mapping">
			<Form noValidate validated={validated} onSubmit={handleSubmit}>
				<p>Use the form below to reconfigure your button-to-pin mapping.</p>
				<div className="alert alert-warning">
					Mapping buttons to pins that aren't connected or available can leave the device in non-functional state. To clear the
					the invalid configuration go to the <NavLink exact="true" to="/reset-settings">Reset Settings</NavLink> page.
				</div>
				<table className="table table-sm pin-mapping-table">
					<thead className="table">
						<tr>
							<th className="table-header-button-label">{BUTTONS[buttonLabels].label}</th>
							<th>Pin</th>
						</tr>
					</thead>
					<tbody>
						{Object.keys(BUTTONS[buttonLabels])?.filter(p => p !== 'label' && p !== 'value').map((button, i) =>
							<tr key={`button-map-${i}`} className={validated && !!buttonMappings[button].error ? "table-danger" : ""}>
								<td>{BUTTONS[buttonLabels][button]}</td>
								<td>
									<Form.Control
										type="number"
										className="pin-input form-control-sm"
										value={buttonMappings[button].pin}
										min={-1}
										max={boards[selectedBoard].maxPin}
										isInvalid={buttonMappings[button].error}
										onChange={(e) => handlePinChange(e, button)}
									></Form.Control>
									<Form.Control.Feedback type="invalid">
										{renderError(button)}
									</Form.Control.Feedback>
								</td>
							</tr>
						)}
					</tbody>
				</table>
				<Button type="submit">Save</Button>
				{saveMessage ? <span className="alert">{saveMessage}</span> : null}
			</Form>
		</Section>
	);
}
