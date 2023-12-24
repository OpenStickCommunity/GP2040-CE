import React, { useContext, useEffect, useState } from 'react';
import { Button, Form } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import * as yup from 'yup';
import { useTranslation } from 'react-i18next';

import get from 'lodash/get';
import set from 'lodash/set';

import { AppContext } from '../Contexts/AppContext';

import WebApi from '../Services/WebApi';
import Analog, { analogScheme, analogState } from '../Addons/Analog';
import Bootsel, { bootselScheme, bootselState } from '../Addons/Bootsel';
import Buzzer, { buzzerScheme, buzzerState } from '../Addons/Buzzer';
import DualDirection, {
	dualDirectionScheme,
	dualDirectionState,
} from '../Addons/DualDirection';
import I2CAnalog1219, { i2cAnalogScheme, i2cAnalogState } from '../Addons/I2CAnalog1219';
import Joystick, { joystickScheme, joystickState } from '../Addons/Joystick';
import OnBoardLed, {
	onBoardLedScheme,
	onBoardLedState,
} from '../Addons/OnBoardLed';
import PlayerNumber, {
	playerNumberScheme,
	playerNumberState,
} from '../Addons/PlayerNumber';
import Reverse, { reverseScheme, reverseState } from '../Addons/Reverse';
import SOCD, { socdScheme, socdState } from '../Addons/SOCD';
import Tilt, { tiltScheme, tiltState } from '../Addons/Tilt';
import Turbo, { turboScheme, turboState } from '../Addons/Turbo';
import Ps4, { ps4Scheme, ps4State } from '../Addons/Ps4';
import PSPassthrough, {
	psPassthroughScheme,
	psPassthroughState,
} from '../Addons/PSPassthrough';
import Wii, { wiiScheme, wiiState } from '../Addons/Wii';
import SNES, { snesState } from '../Addons/SNES';
import FocusMode, {
	focusModeScheme,
	focusModeState,
} from '../Addons/FocusMode';
import Keyboard, { keyboardScheme, keyboardState } from '../Addons/Keyboard';
import InputHistory, { inputHistoryScheme, inputHistoryState } from '../Addons/InputHistory';
import XBOnePassthrough, {
	xbonePassthroughScheme,
	xbonePassthroughState,
} from '../Addons/XBOnePassthrough';

const schema = yup.object().shape({
	...analogScheme,
	...bootselScheme,
	...onBoardLedScheme,
	...turboScheme,
	...joystickScheme,
	...reverseScheme,
	...i2cAnalogScheme,
	...dualDirectionScheme,
	...tiltScheme,
	...buzzerScheme,
	...playerNumberScheme,
	...socdScheme,
	...ps4Scheme,
	...psPassthroughScheme,
	...xbonePassthroughScheme,
	...wiiScheme,
	...focusModeScheme,
	...keyboardScheme,
	...inputHistoryScheme,
});

const defaultValues = {
	...analogState,
	...bootselState,
	...onBoardLedState,
	...turboState,
	...joystickState,
	...reverseState,
	...i2cAnalogState,
	...dualDirectionState,
	...tiltState,
	...buzzerState,
	...playerNumberState,
	...socdState,
	...ps4State,
	...psPassthroughState,
	...xbonePassthroughState,
	...wiiState,
	...snesState,
	...focusModeState,
	...keyboardState,
	...inputHistoryState,
};

const ADDONS = [
	Bootsel,
	OnBoardLed,
	Analog,
	Turbo,
	Joystick,
	Reverse,
	I2CAnalog1219,
	DualDirection,
	Tilt,
	Buzzer,
	PlayerNumber,
	SOCD,
	Ps4,
	PSPassthrough,
	XBOnePassthrough,
	Wii,
	SNES,
	FocusMode,
	Keyboard,
	InputHistory
];

const FormContext = ({ setStoredData }) => {
	const { values, setValues } = useFormikContext();
	const { setLoading } = useContext(AppContext);

	useEffect(() => {
		async function fetchData() {
			const data = await WebApi.getAddonsOptions(setLoading);

			setValues(data);
			setStoredData(JSON.parse(JSON.stringify(data))); // Do a deep copy to keep the original
		}
		fetchData();
	}, [setValues]);

	useEffect(() => {
		sanitizeData(values);
	}, [values, setValues]);

	return null;
};

const sanitizeData = (values) => {
	for (const prop in Object.keys(values).filter(
		(key) => !!!key.includes('keyboardHostMap'),
	)) {
		if (!!values[prop]) values[prop] = parseInt(values[prop]);
	}
};

function flattenObject(object) {
	var toReturn = {};

	for (var i in object) {
		if (!object.hasOwnProperty(i)) continue;

		if (typeof object[i] == 'object' && object[i] !== null) {
			var flatObject = flattenObject(object[i]);
			for (var x in flatObject) {
				if (!flatObject.hasOwnProperty(x)) continue;

				toReturn[i + '.' + x] = flatObject[x];
			}
		} else {
			toReturn[i] = object[i];
		}
	}
	return toReturn;
}

export default function AddonsConfigPage() {
	const { updateUsedPins, updatePeripherals } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');
	const [storedData, setStoredData] = useState({});

	const { t } = useTranslation();

    useEffect(() => {
        updatePeripherals();
    }, []);

	const onSuccess = async (values) => {
		const flattened = flattenObject(storedData);
		const valuesCopy = schema.cast(values); // Strip invalid values

		// Compare what's changed and set it to resultObject
		let resultObject = {};
		Object.entries(flattened)?.map((entry) => {
			const [key, oldVal] = entry;
			const newVal = get(valuesCopy, key);
			if (newVal !== oldVal) {
				set(resultObject, key, newVal);
			}
		});
		sanitizeData(resultObject);
		const success = await WebApi.setAddonsOptions(resultObject);
		setStoredData(JSON.parse(JSON.stringify(values))); // Update to reflect saved data
		setSaveMessage(
			success
				? t('Common:saved-success-message')
				: t('Common:saved-error-message'),
		);
		if (success) updateUsedPins();
	};

	const handleCheckbox = async (name, values) => {
		values[name] = values[name] === 1 ? 0 : 1;
	};

	return (
		<Formik
			enableReinitialize={true}
			validationSchema={schema}
			onSubmit={onSuccess}
			initialValues={defaultValues}
		>
			{({ handleSubmit, handleChange, values, errors, setFieldValue }) => 
			console.log('errors', errors) || (
				<Form noValidate onSubmit={handleSubmit}>
					<h1>{t('AddonsConfig:header-text')}</h1>
					<p>{t('AddonsConfig:sub-header-text')}</p>
					{ADDONS.map((Addon, index) => (
						<Addon
							key={`addon-${index}`}
							values={values}
							errors={errors}
							handleChange={handleChange}
							handleCheckbox={handleCheckbox}
							setFieldValue={setFieldValue}
						/>
					))}

					<div className="mt-3">
						<Button type="submit" id="save">
							{t('Common:button-save-label')}
						</Button>
						{saveMessage ? <span className="alert">{saveMessage}</span> : null}
					</div>
					<FormContext setStoredData={setStoredData} />
				</Form>
			)}
		</Formik>
	);
}
