import React, { useContext, useEffect, useState } from 'react';
import { Button, Form } from 'react-bootstrap';
import { AppContext } from '../Contexts/AppContext';
import KeyboardMapper, { validateMappings } from '../Components/KeyboardMapper';
import Section from '../Components/Section';
import WebApi, { baseButtonMappings } from '../Services/WebApi';
import './PinMappings.scss';

export default function KeyboardMappingPage() {
	const { buttonLabels } = useContext(AppContext);
	const [validated, setValidated] = useState(false);
	const [saveMessage, setSaveMessage] = useState('');
	const [keyMappings, setKeyMappings] = useState(baseButtonMappings);
	const [selectedController] = useState(import.meta.env.VITE_GP2040_CONTROLLER);

	useEffect(() => {
		async function fetchData() {
			setKeyMappings(await WebApi.getKeyMappings());
		}

		fetchData();
	}, [setKeyMappings, selectedController]);

	const handleKeyChange = (value, button) => {
		const newMappings = {...keyMappings};
		newMappings[button].key = value;
		const mappings = validateMappings(newMappings);
		setKeyMappings(mappings);
		setValidated(true);
	};

	const handleSubmit = async (e) => {
		e.preventDefault();
		e.stopPropagation();

		let mappings = {...keyMappings};
		mappings = validateMappings(mappings);
		setKeyMappings(mappings);
		setValidated(true);

		if (Object.keys(mappings).filter(p => !!mappings[p].error).length) {
			setSaveMessage('Validation errors, see above');
			return;
		}

		const success = await WebApi.setKeyMappings(mappings);
		setSaveMessage(success ? 'Saved! Please Restart Your Device' : 'Unable to Save');
	};

	const getKeyMappingForButton = (button) => keyMappings[button];

	return (
		<Section title="Keyboard Mapping">
			<Form noValidate validated={validated} onSubmit={handleSubmit}>
				<p>Use the form below to reconfigure your button-to-key mapping.</p>
				<KeyboardMapper buttonLabels={buttonLabels}
								handleKeyChange={handleKeyChange}
								validated={validated}
								getKeyMappingForButton={getKeyMappingForButton} />
				<Button type="submit">Save</Button>
				{saveMessage ? <span className="alert">{saveMessage}</span> : null}
			</Form>
		</Section>
	);
}
