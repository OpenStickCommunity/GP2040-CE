import React, { useContext, useEffect, useState } from 'react';
import { NavLink } from 'react-router-dom';
import { Button, Form } from 'react-bootstrap';
import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import WebApi, { baseButtonMappings } from '../Services/WebApi';
import boards from '../Data/Boards.json';
import { BUTTONS } from '../Data/Buttons';
import './PinMappings.scss';
import { Trans, useTranslation } from 'react-i18next';

const requiredButtons = ['S2'];
const errorType = {
	required: 'errors.required',
	conflict: 'errors.conflict',
	invalid: 'errors.invalid',
	used: 'errors.used',
};

export default function PinMappingPage() {
	const { buttonLabels, setButtonLabels, usedPins, updateUsedPins } =
		useContext(AppContext);
	const [validated, setValidated] = useState(false);
	const [saveMessage, setSaveMessage] = useState('');
	const [buttonMappings, setButtonMappings] = useState(baseButtonMappings);
	const [selectedController] = useState(import.meta.env.VITE_GP2040_CONTROLLER);
	const [selectedBoard] = useState(import.meta.env.VITE_GP2040_BOARD);
	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const { setLoading } = useContext(AppContext);

	const { t } = useTranslation('');

	const translatedErrorType = Object.keys(errorType).reduce(
		(a, k) => ({ ...a, [k]: t(`PinMapping:${errorType[k]}`) }),
		{},
	);

	useEffect(() => {
		async function fetchData() {
			setButtonMappings(await WebApi.getPinMappings(setLoading));
			const options = await WebApi.getGamepadOptions(setLoading);
			setButtonLabels({
				swapTpShareLabels:
					options.switchTpShareForDs4 && options.inputMode === 4,
			});
		}

		fetchData();
	}, [setButtonMappings, selectedController]);

	const handlePinChange = (e, prop) => {
		const newMappings = { ...buttonMappings };
		if (e.target.value) newMappings[prop].pin = parseInt(e.target.value);
		else newMappings[prop].pin = '';

		validateMappings(newMappings);
	};

	const handleSubmit = async (e) => {
		e.preventDefault();
		e.stopPropagation();

		let mappings = { ...buttonMappings };
		validateMappings(mappings);

		if (Object.keys(mappings).filter((p) => mappings[p].error).length > 0) {
			setSaveMessage(t('Common:errors.validation-error'));
			return;
		}

		const success = await WebApi.setPinMappings(mappings);
		if (success) updateUsedPins();
		setSaveMessage(
			success
				? t('Common:saved-success-message')
				: t('Common:saved-error-message'),
		);
	};

	const validateMappings = (mappings) => {
		const buttons = Object.keys(mappings);

		// Create some mapped pin groups for easier error checking
		const mappedPins = buttons
			.filter((p) => mappings[p].pin > -1)
			.reduce((a, p) => {
				a.push(mappings[p].pin);
				return a;
			}, []);
		const mappedPinCounts = mappedPins.reduce(
			(a, p) => ({ ...a, [p]: (a[p] || 0) + 1 }),
			{},
		);
		const uniquePins = mappedPins.filter((p, i, a) => a.indexOf(p) === i);
		const conflictedPins = Object.keys(mappedPinCounts)
			.filter((p) => mappedPinCounts[p] > 1)
			.map(parseInt);
		const invalidPins = uniquePins.filter(
			(p) => boards[selectedBoard].invalidPins.indexOf(p) > -1,
		);
		const otherPins = usedPins.filter((p) => uniquePins.indexOf(p) === -1);

		for (let button of buttons) {
			mappings[button].error = '';

			// Validate required button
			if (
				(mappings[button].pin < boards[selectedBoard].minPin ||
					mappings[button].pin > boards[selectedBoard].maxPin) &&
				requiredButtons.filter((b) => b === button).length
			)
				mappings[button].error = translatedErrorType.required;
			// Identify conflicted pins
			else if (conflictedPins.indexOf(mappings[button].pin) > -1)
				mappings[button].error = translatedErrorType.conflict;
			// Identify invalid pin assignments
			else if (invalidPins.indexOf(mappings[button].pin) > -1)
				mappings[button].error = translatedErrorType.invalid;
			// Identify used pins
			else if (otherPins.indexOf(mappings[button].pin) > -1)
				mappings[button].error = translatedErrorType.used;
		}

		setButtonMappings(mappings);
		setValidated(true);
	};

	const renderError = (button) => {
		if (buttonMappings[button].error === translatedErrorType.required) {
			return (
				<span key="required" className="error-message">
					{t('PinMapping:errors.required', {
						button: BUTTONS[buttonLabelType][button],
					})}
				</span>
			);
		} else if (buttonMappings[button].error === translatedErrorType.conflict) {
			const conflictedMappings = Object.keys(buttonMappings)
				.filter((b) => b !== button)
				.filter((b) => buttonMappings[b].pin === buttonMappings[button].pin)
				.map((b) => BUTTONS[buttonLabelType][b]);

			return (
				<span key="conflict" className="error-message">
					{t('PinMapping:errors.conflict', {
						pin: buttonMappings[button].pin,
						conflictedMappings: conflictedMappings.join(', '),
					})}
				</span>
			);
		} else if (buttonMappings[button].error === translatedErrorType.invalid) {
			console.log(buttonMappings[button].pin);
			return (
				<span key="invalid" className="error-message">
					{t('PinMapping:errors.invalid', {
						pin: buttonMappings[button].pin,
					})}
				</span>
			);
		} else if (buttonMappings[button].error === translatedErrorType.used) {
			return (
				<span key="used" className="error-message">
					{t('PinMapping:errors.used', {
						pin: buttonMappings[button].pin,
					})}
				</span>
			);
		}

		return <></>;
	};

	return (
		<Section title={t('PinMapping:header-text')}>
			<Form noValidate validated={validated} onSubmit={handleSubmit}>
				<p>{t('PinMapping:sub-header-text')}</p>
				{/* <div className="alert alert-warning">
					Mapping buttons to pins that aren't connected or available can leave the device in non-functional state. To clear the
					the invalid configuration go to the <NavLink exact="true" to="/reset-settings">Reset Settings</NavLink> page.
				</div> */}
				<div className="alert alert-warning">
					<Trans ns="PinMapping" i18nKey="alert-text">
						Mapping buttons to pins that aren&apos;t connected or available can
						leave the device in non-functional state. To clear the the invalid
						configuration go to the{' '}
						<NavLink exact="true" to="/reset-settings">
							Reset Settings
						</NavLink>{' '}
						page.
					</Trans>
				</div>
				<table className="table table-sm pin-mapping-table">
					<thead className="table">
						<tr>
							<th className="table-header-button-label">
								{BUTTONS[buttonLabelType].label}
							</th>
							<th>{t('PinMapping:pin-header-label')}</th>
						</tr>
					</thead>
					<tbody>
						{Object.keys(BUTTONS[buttonLabelType])
							?.filter((p) => p !== 'label' && p !== 'value')
							.map((button, i) => {
								let label = BUTTONS[buttonLabelType][button];
								if (
									button === 'S1' &&
									swapTpShareLabels &&
									buttonLabelType === 'ps4'
								) {
									label = BUTTONS[buttonLabelType]['A2'];
								}
								if (
									button === 'A2' &&
									swapTpShareLabels &&
									buttonLabelType === 'ps4'
								) {
									label = BUTTONS[buttonLabelType]['S1'];
								}
								return (
									<tr
										key={`button-map-${i}`}
										className={
											validated && !!buttonMappings[button].error
												? 'table-danger'
												: ''
										}
									>
										<td>{label}</td>
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
								);
							})}
					</tbody>
				</table>
				<Button type="submit">{t('Common:button-save-label')}</Button>
				{saveMessage ? <span className="alert">{saveMessage}</span> : null}
			</Form>
		</Section>
	);
}
