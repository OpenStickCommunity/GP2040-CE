import { useContext, useEffect, useState } from 'react';
import { Button, Form } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import * as yup from 'yup';
import { useTranslation } from 'react-i18next';

import get from 'lodash/get';
import set from 'lodash/set';

import { AppContext } from '../Contexts/AppContext';

import { hexToInt } from '../Services/Utilities';

import WebApi from '../Services/WebApi';
import Analog, { analogScheme, analogState } from '../Addons/Analog';
import Analog1256, {
	analog1256Scheme,
	analog1256State,
} from '../Addons/Analog1256';
import Bootsel, { bootselScheme, bootselState } from '../Addons/Bootsel';
import Buzzer, { buzzerScheme, buzzerState } from '../Addons/Buzzer';
import DualDirection, {
	dualDirectionScheme,
	dualDirectionState,
} from '../Addons/DualDirection';
import I2CAnalog1219, {
	i2cAnalogScheme,
	i2cAnalogState,
} from '../Addons/I2CAnalog1219';
import OnBoardLed, {
	onBoardLedScheme,
	onBoardLedState,
} from '../Addons/OnBoardLed';
import Reverse, { reverseScheme, reverseState } from '../Addons/Reverse';
import SOCD, { socdScheme, socdState } from '../Addons/SOCD';
import Tilt, { tiltScheme, tiltState } from '../Addons/Tilt';
import Turbo, { turboScheme, turboState } from '../Addons/Turbo';
import Wii, { wiiScheme, wiiState } from '../Addons/Wii';
import SNES, { snesState } from '../Addons/SNES';
import FocusMode, {
	focusModeScheme,
	focusModeState,
} from '../Addons/FocusMode';
import Keyboard, { keyboardScheme, keyboardState } from '../Addons/Keyboard';
import GamepadUSBHost, {
	gamepadUSBHostScheme,
	gamepadUSBHostState,
} from '../Addons/GamepadUSBHost';
import Rotary, { rotaryScheme, rotaryState } from '../Addons/Rotary';
import PCF8575, { pcf8575Scheme, pcf8575State } from '../Addons/PCF8575';
import DRV8833Rumble, {
	drv8833RumbleScheme,
	drv8833RumbleState,
} from '../Addons/DRV8833';
import ReactiveLED, {
	reactiveLEDScheme,
	reactiveLEDState,
} from '../Addons/ReactiveLED';

const schema = yup.object().shape({
	...analogScheme,
	...analog1256Scheme,
	...bootselScheme,
	...onBoardLedScheme,
	...turboScheme,
	...reverseScheme,
	...i2cAnalogScheme,
	...dualDirectionScheme,
	...tiltScheme,
	...buzzerScheme,
	...socdScheme,
	...wiiScheme,
	...focusModeScheme,
	...keyboardScheme,
	...rotaryScheme,
	...pcf8575Scheme,
	...drv8833RumbleScheme,
	...reactiveLEDScheme,
	...gamepadUSBHostScheme,
});

const defaultValues = {
	...analogState,
	...analog1256State,
	...bootselState,
	...onBoardLedState,
	...turboState,
	...reverseState,
	...i2cAnalogState,
	...dualDirectionState,
	...tiltState,
	...buzzerState,
	...socdState,
	...wiiState,
	...snesState,
	...focusModeState,
	...keyboardState,
	...rotaryState,
	...pcf8575State,
	...drv8833RumbleState,
	...reactiveLEDState,
	...gamepadUSBHostState,
};

const ADDONS = [
	Bootsel,
	OnBoardLed,
	Analog,
	Turbo,
	Reverse,
	I2CAnalog1219,
	Analog1256,
	DualDirection,
	Tilt,
	Buzzer,
	SOCD,
	Wii,
	SNES,
	FocusMode,
	Keyboard,
	GamepadUSBHost,
	Rotary,
	PCF8575,
	DRV8833Rumble,
	ReactiveLED,
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

		// Convert turbo LED color if available
		const data = {
			...values,
			turboLedColor: hexToInt(values.turboLedColor || '#000000'),
		};
		const valuesSchema = schema.cast(data); // Strip invalid values

		// Compare what's changed and set it to resultObject
		let resultObject = {};
		Object.entries(flattened)?.map((entry) => {
			const [key, oldVal] = entry;
			const newVal = get(valuesSchema, key);
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
							{saveMessage ? (
								<span className="alert">{saveMessage}</span>
							) : null}
						</div>
						<FormContext setStoredData={setStoredData} />
					</Form>
				)
			}
		</Formik>
	);
}
