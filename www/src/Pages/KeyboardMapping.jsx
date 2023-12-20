import React, { useContext, useEffect, useState } from 'react';
import { Button, Form } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';

import { AppContext } from '../Contexts/AppContext';
import KeyboardMapper, { validateMappings } from '../Components/KeyboardMapper';
import Section from '../Components/Section';
import WebApi, { baseButtonMappings } from '../Services/WebApi';

export default function KeyboardMappingPage() {
	const { buttonLabels } = useContext(AppContext);
	const [validated, setValidated] = useState(false);
	const [saveMessage, setSaveMessage] = useState('');
	const [keyMappings, setKeyMappings] = useState(baseButtonMappings);
	const [selectedController] = useState(import.meta.env.VITE_GP2040_CONTROLLER);
	const { setLoading } = useContext(AppContext);

	const { t } = useTranslation('');

	useEffect(() => {
		async function fetchData() {
			setKeyMappings(await WebApi.getKeyMappings(setLoading));
		}

		fetchData();
	}, [setKeyMappings, selectedController]);

	const handleKeyChange = (value, button) => {
		const newMappings = { ...keyMappings };
		newMappings[button].key = value;
		const mappings = validateMappings(newMappings, t);
		setKeyMappings(mappings);
		setValidated(true);
	};

	const handleSubmit = async (e) => {
		e.preventDefault();
		e.stopPropagation();

		let mappings = { ...keyMappings };
		mappings = validateMappings(mappings, t);
		setKeyMappings(mappings);
		setValidated(true);

		if (Object.keys(mappings).filter((p) => !!mappings[p].error).length) {
			setSaveMessage(t('Common:errors.validation-error'));
			return;
		}

		const success = await WebApi.setKeyMappings(mappings);
		setSaveMessage(
			success
				? t('Common:saved-success-message')
				: t('Common:saved-error-message'),
		);
	};

	const getKeyMappingForButton = (button) => keyMappings[button];

	return (
		<Section title={t('KeyboardMapping:header-text')}>
			<Form noValidate validated={validated} onSubmit={handleSubmit}>
				<p>{t('KeyboardMapping:sub-header-text')}</p>
				<KeyboardMapper
					buttonLabels={buttonLabels}
					handleKeyChange={handleKeyChange}
					validated={validated}
					getKeyMappingForButton={getKeyMappingForButton}
				/>
				<Button type="submit">{t('Common:button-save-label')}</Button>
				{saveMessage ? <span className="alert">{saveMessage}</span> : null}
			</Form>
		</Section>
	);
}
