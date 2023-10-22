import React, { useContext, useEffect, useState } from 'react';
import Select from 'react-select';
import { NavLink } from 'react-router-dom';
import { Alert, Button, Form } from 'react-bootstrap';
import { Trans, useTranslation } from 'react-i18next';
import invert from 'lodash/invert';
import map from 'lodash/map';
import omit from 'lodash/omit';

import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import usePinStore, {
	BUTTON_ACTIONS,
	NON_SELECTABLE_BUTTON_ACTIONS,
} from '../Store/usePinStore';

import './PinMappings.scss';
import CaptureButton from '../Components/CaptureButton';
import { getButtonLabels } from '../Data/Buttons';

const isNonSelectable = (value) =>
	NON_SELECTABLE_BUTTON_ACTIONS.includes(value);

const options = Object.entries(BUTTON_ACTIONS)
	.filter(([_, value]) => !isNonSelectable(value))
	.map(([key, value]) => ({
		label: key,
		value,
	}));

const getOption = (actionId) => ({
	label: invert(BUTTON_ACTIONS)[actionId],
	value: actionId,
});

export default function PinMappingPage() {
	const { pins, setPinAction, fetchPins, savePins } = usePinStore();
	const { buttonLabels, updateUsedPins } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');

	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const CURRENT_BUTTONS = getButtonLabels(buttonLabelType, swapTpShareLabels);
	const buttonNames = omit(CURRENT_BUTTONS, ['label', 'value']);

	const { t } = useTranslation('');

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
				<CaptureButton
					labels={Object.values(buttonNames)}
					onChange={(label, pin) =>
						setPinAction(
							// Convert getHeldPins format to setPinMappings format
							parseInt(pin) < 10 ? `pin0${pin}` : `pin${pin}`,
							// Maps current mode buttons to actions
							BUTTON_ACTIONS[
								`BUTTON_PRESS_${invert(buttonNames)[label].toUpperCase()}`
							],
						)
					}
				/>
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
								value={getOption(pinAction)}
								isDisabled={isNonSelectable(pinAction)}
								getOptionLabel={(option) => {
									const labelKey = option.label.split('BUTTON_PRESS_').pop();
									// Need to fallback as some button actions are not part of button names
									return (
										buttonNames[labelKey] ||
										t(`PinMapping:actions.${option.label}`)
									);
								}}
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
