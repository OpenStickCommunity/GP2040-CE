import React, { useContext, useEffect, useState } from 'react';
import Select from 'react-select';
import { NavLink } from 'react-router-dom';
import { Alert, Button, Form } from 'react-bootstrap';
import { Trans, useTranslation } from 'react-i18next';
import invert from 'lodash/invert';
import map from 'lodash/map';

import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import usePinStore, {
	BUTTON_ACTIONS,
	NON_SELECTABLE_BUTTON_ACTIONS,
} from '../Store/usePinStore';

import './PinMappings.scss';

const isNonSelectable = (value) =>
	NON_SELECTABLE_BUTTON_ACTIONS.includes(value);

const createOptions = (t) =>
	Object.entries(BUTTON_ACTIONS)
		.filter(([_, value]) => !isNonSelectable(value))
		.map(([key, value]) => ({
			label: t(`PinMapping:actions.${key}`),
			value,
		}));

const getOption = (t, actionId) => ({
	label: t(`PinMapping:actions.${invert(BUTTON_ACTIONS)[actionId]}`),
	value: actionId,
});

export default function PinMappingPage() {
	const { pins, setPinAction, fetchPins, savePins } = usePinStore();
	const { updateUsedPins } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');

	const { t } = useTranslation('');
	const options = createOptions(t);

	// Todo move all required fetching above navigation to preload
	useEffect(() => {
		fetchPins();
	}, []);

	const handleSubmit = async (e) => {
		e.preventDefault();
		e.stopPropagation();
		try {
			await savePins();
			updateUsedPins();
			setSaveMessage(t('Common:saved-success-message'));
		} catch (error) {
			setSaveMessage(t('Common:saved-error-message'));
		}
	};

	return (
		<Section title={t('PinMapping:header-text')}>
			<Form onSubmit={handleSubmit}>
				<p>{t('PinMapping:sub-header-text')}</p>
				<div className="alert alert-warning">
					<Trans ns="PinMapping" i18nKey="alert-text">
						Mapping buttons to pins that aren&apos;t connected or available can
						leave the device in non-functional state. To clear the the invalid
						configuration go to the{' '}
						<NavLink to="/reset-settings">Reset Settings</NavLink> page.
					</Trans>
				</div>
				<div className="row row-cols-lg-3 row-cols-md-2 gx-3">
					{map(pins, (pinAction, pin) => (
						<div
							key={`pin-${pin}`}
							className="d-flex justify-content-center py-2"
						>
							<div className="d-flex align-items-center pe-2">
								<label htmlFor={pin}>{pin.toUpperCase()}</label>
							</div>
							<Select
								inputId={pin}
								className="flex-grow-1 text-primary"
								isClearable
								isSearchable
								options={options}
								value={getOption(t, pinAction)}
								isDisabled={isNonSelectable(pinAction)}
								onChange={(change) =>
									setPinAction(
										pin,
										change?.value === undefined ? -10 : change.value, // On clear set to -10
									)
								}
							/>
						</div>
					))}
				</div>
				<Button type="submit" className="my-4">
					{t('Common:button-save-label')}
				</Button>
				{saveMessage && <Alert variant="secondary">{saveMessage}</Alert>}
			</Form>
		</Section>
	);
}
