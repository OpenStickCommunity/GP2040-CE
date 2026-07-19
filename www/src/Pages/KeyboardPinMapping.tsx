import { FormEvent, useContext, useEffect, useState } from 'react';
import { Alert, Button, Col, Form, Row } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import invert from 'lodash/invert';

import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import CustomSelect from '../Components/CustomSelect';
import CaptureButton from '../Components/CaptureButton';
import WebApi from '../Services/WebApi';
import { KEY_CODES } from '../Data/Keyboard';
import { BUTTON_ACTIONS, PinActionValues } from '../Data/Pins';
import './PinMapping.scss';

type KeyOption = { label: string; value: number };

const disabledActions = [
	BUTTON_ACTIONS.RESERVED,
	BUTTON_ACTIONS.ASSIGNED_TO_ADDON,
] as PinActionValues[];

const isDisabled = (action: PinActionValues) =>
	disabledActions.includes(action);

const keyOptions = KEY_CODES as KeyOption[];

const toPinName = (index: number) => `pin${index < 10 ? '0' : ''}${index}`;

const emptyPins = Object.fromEntries(
	Array.from({ length: 30 }, (_, index) => [toPinName(index), 0]),
);

const pickPins = (source: object) => {
	const pins: { [pin: string]: number } = { ...emptyPins };
	Object.entries(source || {}).forEach(([key, value]) => {
		if (/^pin\d\d$/.test(key)) pins[key] = parseInt(value) || 0;
	});
	return pins;
};

export default function KeyboardPinMapping() {
	const { setLoading } = useContext(AppContext);
	const { t } = useTranslation('');

	const [keyMappings, setKeyMappings] = useState<{ [pin: string]: number }>(
		emptyPins,
	);
	const [pinActions, setPinActions] = useState<{
		[pin: string]: PinActionValues;
	}>({});
	const [pressedPin, setPressedPin] = useState<number | null>(null);
	const [saveMessage, setSaveMessage] = useState('');

	useEffect(() => {
		async function fetchData() {
			const pinMappings = await WebApi.getPinMappings();
			const actions: { [pin: string]: PinActionValues } = {};
			Object.entries(pinMappings || {}).forEach(([key, value]) => {
				if (/^pin\d\d$/.test(key)) {
					actions[key] = (value as { action: PinActionValues }).action;
				}
			});
			setPinActions(actions);

			const mappings = await WebApi.getKeyMappings(setLoading);
			if (mappings) {
				setKeyMappings(pickPins(mappings));
			}
		}
		fetchData();
	}, []);

	const handleSubmit = async (e: FormEvent<HTMLFormElement>) => {
		e.preventDefault();
		e.stopPropagation();
		const success = await WebApi.setKeyMappings(keyMappings);
		setSaveMessage(
			success
				? t('Common:saved-success-message')
				: t('Common:saved-error-message'),
		);
	};

	return (
		<Row>
			<Col md={3}>
				<p className="text-center">{t('KeyboardMapping:sub-header-text')}</p>
				<div className="d-flex justify-content-center pb-3">
					<CaptureButton
						buttonLabel={t('PinMapping:pin-viewer')}
						labels={['']}
						onChange={(_, pin) => setPressedPin(pin)}
					/>
				</div>
				{pressedPin !== null && (
					<div className="alert alert-info mt-3">
						<strong>{t('PinMapping:pin-pressed', { pressedPin })}</strong>
					</div>
				)}
			</Col>
			<Col md={9}>
				<Section title={t('KeyboardMapping:header-text')}>
					<div className="alert alert-info">
						{t('KeyboardMapping:duplicates-text')}
					</div>
					<Form onSubmit={handleSubmit}>
						<div className="pin-grid gap-3 mt-2">
							{Object.keys(keyMappings).map((pin, index) => {
								const action = pinActions[pin];
								const disabled = action !== undefined && isDisabled(action);
								return (
									<div
										key={`key-select-${pin}`}
										className="d-flex align-items-center"
									>
										<div
											className="d-flex flex-shrink-0"
											style={{ width: '3.5rem' }}
										>
											<label>GP{index}</label>
										</div>
										<CustomSelect
											isDisabled={disabled}
											options={keyOptions}
											value={
												disabled
													? {
															label: t(
																`Proto:GpioAction.${invert(BUTTON_ACTIONS)[action]}`,
															),
															value: -1,
														}
													: keyOptions.find(
															(option) => option.value === keyMappings[pin],
														) || keyOptions[0]
											}
											onChange={(selected) =>
												setKeyMappings((mappings) => ({
													...mappings,
													[pin]: (selected as KeyOption)?.value ?? 0,
												}))
											}
										/>
									</div>
								);
							})}
						</div>
						<Button type="submit" className="my-3">
							{t('Common:button-save-label')}
						</Button>
						{saveMessage && <Alert variant="info">{saveMessage}</Alert>}
					</Form>
				</Section>
			</Col>
		</Row>
	);
}
