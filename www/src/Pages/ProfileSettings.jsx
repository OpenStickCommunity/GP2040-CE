import React, { useContext, useEffect, useState } from 'react';
import { NavLink } from 'react-router-dom';
import { Button, Form } from 'react-bootstrap';
import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import WebApi, {
	baseProfileOptions,
	baseButtonMappings,
} from '../Services/WebApi';
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

export default function ProfileOptionsPage() {
	const { buttonLabels, setButtonLabels, usedPins, updateUsedPins } =
		useContext(AppContext);
	const [validated, setValidated] = useState(false);
	const [saveMessage, setSaveMessage] = useState('');
	const [buttonMappings, setButtonMappings] = useState(baseButtonMappings);
	const [profileOptions, setProfileOptions] = useState(baseProfileOptions);
	const [selectedBoard] = useState(import.meta.env.VITE_GP2040_BOARD);
	const { buttonLabelType } = buttonLabels;
	const { setLoading } = useContext(AppContext);

	const { t } = useTranslation('');

	const translatedErrorType = Object.keys(errorType).reduce(
		(a, k) => ({ ...a, [k]: t(`ProfileOptions:${errorType[k]}`) }),
		{},
	);

	useEffect(() => {
		async function fetchData() {
			setButtonMappings(await WebApi.getPinMappings(setLoading));
			setProfileOptions(await WebApi.getProfileOptions(setLoading));
			setButtonLabels({});
		}

		fetchData();
	}, [setButtonMappings, setProfileOptions]);

	const handlePinChange = (e, index, key) => {
		const newProfileOptions = { ...profileOptions };
		if (e.target.value)
			newProfileOptions['alternativePinMappings'][index][key].pin = parseInt(
				e.target.value,
			);
		else newProfileOptions['alternativePinMappings'][index][key].pin = '';

		validateMappings(newProfileOptions);
	};

	const handleSubmit = async (e) => {
		e.preventDefault();
		e.stopPropagation();

		let mappings = { ...profileOptions };
		validateMappings(mappings);

		if (Object.keys(mappings).filter((p) => mappings[p].error).length > 0) {
			setSaveMessage(t('Common:errors.validation-error'));
			return;
		}

		const success = await WebApi.setProfileOptions(mappings);
		if (success) updateUsedPins();
		setSaveMessage(
			success
				? t('Common:saved-success-message')
				: t('Common:saved-error-message'),
		);
	};

	const validateMappings = (mappings) => {
		profileOptions['alternativePinMappings'].forEach((altMappings) => {
			const buttons = Object.keys(altMappings);

			// Create some mapped pin groups for easier error checking
			const mappedPins = buttons
				.filter((p) => altMappings[p].pin > -1)
				.reduce((a, p) => {
					a.push(altMappings[p].pin);
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
				altMappings[button].error = '';

				// Validate required button
				if (
					(altMappings[button].pin < boards[selectedBoard].minPin ||
						altMappings[button].pin > boards[selectedBoard].maxPin) &&
					requiredButtons.filter((b) => b === button).length
				)
					altMappings[button].error = translatedErrorType.required;
				// Identify conflicted pins
				else if (conflictedPins.indexOf(altMappings[button].pin) > -1)
					altMappings[button].error = translatedErrorType.conflict;
				// Identify invalid pin assignments
				else if (invalidPins.indexOf(altMappings[button].pin) > -1)
					altMappings[button].error = translatedErrorType.invalid;
				// Identify used pins
				else if (otherPins.indexOf(altMappings[button].pin) > -1)
					altMappings[button].error = translatedErrorType.used;
			}
		});

		setProfileOptions(mappings);
		setValidated(true);
	};

	const pinCell = (profile, button) => {
		return (
			<td>
				<Form.Control
					type="number"
					className="pin-input form-control-sm"
					value={profileOptions['alternativePinMappings'][profile][button].pin}
					min={-1}
					max={boards[selectedBoard].maxPin}
					isInvalid={
						profileOptions['alternativePinMappings'][profile][button].error
					}
					onChange={(e) => handlePinChange(e, profile, button)}
				></Form.Control>
				<Form.Control.Feedback type="invalid">
					{renderError(profile, button)}
				</Form.Control.Feedback>
			</td>
		);
	};

	const renderError = (index, button) => {
		if (
			profileOptions['alternativePinMappings'][index][button].error ===
			translatedErrorType.required
		) {
			return (
				<span key="required" className="error-message">
					{t('PinMapping:errors.required', {
						button: BUTTONS[buttonLabelType][button],
					})}
				</span>
			);
		} else if (
			profileOptions['alternativePinMappings'][index][button].error ===
			translatedErrorType.conflict
		) {
			const conflictedMappings = Object.keys(
				profileOptions['alternativePinMappings'][index],
			)
				.filter((b) => b !== button)
				.filter(
					(b) =>
						profileOptions['alternativePinMappings'][index][b].pin ===
						profileOptions['alternativePinMappings'][index][button].pin,
				)
				.map((b) => BUTTONS[buttonLabelType][b]);

			return (
				<span key="conflict" className="error-message">
					{t('PinMapping:errors.conflict', {
						pin: profileOptions['alternativePinMappings'][index][button].pin,
						conflictedMappings: conflictedMappings.join(', '),
					})}
				</span>
			);
		} else if (
			profileOptions['alternativePinMappings'][index][button].error ===
			translatedErrorType.invalid
		) {
			console.log(profileOptions['alternativePinMappings'][index][button].pin);
			return (
				<span key="invalid" className="error-message">
					{t('PinMapping:errors.invalid', {
						pin: profileOptions['alternativePinMappings'][index][button].pin,
					})}
				</span>
			);
		} else if (
			profileOptions['alternativePinMappings'][index][button].error ===
			translatedErrorType.used
		) {
			return (
				<span key="used" className="error-message">
					{t('PinMapping:errors.used', {
						pin: profileOptions['alternativePinMappings'][index][button].pin,
					})}
				</span>
			);
		}

		return <></>;
	};

	return (
		<Section title={t('ProfileSettings:header-text')}>
			<p>{t('ProfileSettings:profile-pins-desc')}</p>
			<pre>
				&nbsp;&nbsp;{String(buttonMappings['Up'].pin).padStart(2)}
				<br />
				{String(buttonMappings['Left'].pin).padStart(2)}&nbsp;&nbsp;
				{String(buttonMappings['Right'].pin).padStart(2)}
				&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				{String(buttonMappings['B3'].pin).padStart(2)}{' '}
				{String(buttonMappings['B4'].pin).padStart(2)}{' '}
				{String(buttonMappings['R1'].pin).padStart(2)}{' '}
				{String(buttonMappings['L1'].pin).padStart(2)}
				<br />
				&nbsp;&nbsp;{String(buttonMappings['Down'].pin).padStart(2)}
				&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				{String(buttonMappings['B1'].pin).padStart(2)}{' '}
				{String(buttonMappings['B2'].pin).padStart(2)}{' '}
				{String(buttonMappings['R2'].pin).padStart(2)}{' '}
				{String(buttonMappings['L2'].pin).padStart(2)}
			</pre>
			<p>
				<b>{t('ProfileSettings:profile-pins-warning')}</b>
			</p>
			<Form noValidate validated={validated} onSubmit={handleSubmit}>
				<table className="table table-sm pin-mapping-table">
					<thead>
						<tr>
							<th className="table-header-button-label">
								{BUTTONS[buttonLabelType].label}
							</th>
							<th>{t('ProfileSettings:profile-1')}</th>
							<th>{t('ProfileSettings:profile-2')}</th>
							<th>{t('ProfileSettings:profile-3')}</th>
							<th>{t('ProfileSettings:profile-4')}</th>
						</tr>
					</thead>
					<tbody>
						{console.log(
							Object.keys(profileOptions['alternativePinMappings'][0]),
						)}
						{Object.keys(profileOptions['alternativePinMappings'][0]).map(
							(key) => (
								<tr key={key}>
									<td>{BUTTONS[buttonLabelType][key]}</td>
									<td>{buttonMappings[key].pin}</td>
									{pinCell(0, key)}
									{pinCell(1, key)}
									{pinCell(2, key)}
								</tr>
							),
						)}
					</tbody>
				</table>
				<Button type="submit">{t('Common:button-save-label')}</Button>
				{saveMessage ? <span className="alert">{saveMessage}</span> : null}
			</Form>
		</Section>
	);
}
