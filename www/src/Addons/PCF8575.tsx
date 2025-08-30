import { useCallback, useContext, useEffect, useState } from 'react';

import { AppContext } from '../Contexts/AppContext';
import useExpansionPinStore from '../Store/useExpansionPinStore';

import { useTranslation } from 'react-i18next';
import { Alert, Button, FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';
import invert from 'lodash/invert';
import omit from 'lodash/omit';

import Section from '../Components/Section';
import CustomSelect from '../Components/CustomSelect';

import { getButtonLabels } from '../Data/Buttons';
import './PCF8575.scss';

import {
	BUTTON_ACTIONS,
	PIN_DIRECTIONS,
	//	NON_SELECTABLE_BUTTON_ACTIONS,
	PinActionValues,
	PinDirectionValues,
} from '../Data/Pins';
// currently using only the gamepad values here
const NON_SELECTABLE_BUTTON_ACTIONS = [
	-5, 0, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
	37, 38, 39,
];

export const pcf8575Scheme = {
	PCF8575AddonEnabled: yup
		.number()
		.required()
		.label('PCF8575 IO Add-On Enabled'),
};

export const pcf8575State = {
	PCF8575AddonEnabled: 0,
};

const getOption = (foo, actionId) => {
	return {
		label: invert(BUTTON_ACTIONS)[actionId],
		value: actionId,
	};
};

const getPinDirection = (dir) => ({
	label: invert(PIN_DIRECTIONS)[dir],
	value: dir,
});

const isNonSelectable = (value) =>
	NON_SELECTABLE_BUTTON_ACTIONS.includes(value);

const pinDirections = Object.entries(PIN_DIRECTIONS).map(([key, value]) => ({
	label: key,
	value,
}));

const options = Object.entries(BUTTON_ACTIONS)
	.filter(([, value]) => !isNonSelectable(value))
	.map(([key, value]) => ({
		label: key,
		value,
	}));

type ExpansionPinsFormTypes = {
	savePins: () => void;
	pins: {
		[key: string]: [
			{
				[key: string]: {
					option: PinActionValues;
					direction: PinDirectionValues;
				};
			},
		];
	};
	setPinAction: (
		expansion: string,
		index: number,
		pin: string,
		action: PinActionValues,
	) => void;
	setPinDirection: (
		expansion: string,
		index: number,
		pin: string,
		dir: PinDirectionValues,
	) => void;
	expansionType: string;
};

const ExpansionPinsForm = ({
	savePins,
	pins,
	setPinAction,
	setPinDirection,
	expansionType,
}: ExpansionPinsFormTypes) => {
	const { buttonLabels } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');

	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const CURRENT_BUTTONS = getButtonLabels(buttonLabelType, swapTpShareLabels);
	const buttonNames = omit(CURRENT_BUTTONS, ['label', 'value']);
	const { t } = useTranslation('');

	const handleSave = async (e) => {
		e.preventDefault();
		e.stopPropagation();
		try {
			await savePins();
			setSaveMessage(t('Common:saved-success-message'));
		} catch (error) {
			setSaveMessage(t('Common:saved-error-message'));
		}
	};
	return (
		<div>
			<div className="pin-grid-PCF8575 gap-3 mt-2">
				{Object.entries(pins[expansionType][0]).map(([pin, pinData], index) => (
					<div
						key={`select-${index}`}
						className="d-flex align-items-center gap-2"
					>
						<div className="d-flex flex-shrink-0" style={{ width: '3rem' }}>
							<label htmlFor={pin}>GP{index}</label>
						</div>
						<div className="w-20">
							<CustomSelect
								inputId={`pinMode${pin}`}
								isSearchable={false}
								isClearable={false}
								options={pinDirections}
								value={getPinDirection(pinData.direction)}
								getOptionLabel={(option) => {
									const labelKey = option.label.replace('_', '.').toLowerCase();
									return t(`PCF8575:label-${labelKey}`);
								}}
								onChange={(change) =>
									setPinDirection(
										expansionType,
										0,
										pin,
										change?.value === undefined ? -10 : change.value, // On clear set to -10
									)
								}
							/>
						</div>
						<CustomSelect
							inputId={pin}
							isClearable
							isSearchable
							options={options}
							value={getOption(pinData, pinData.option)}
							isDisabled={isNonSelectable(pinData.option)}
							getOptionLabel={(option) => {
								const labelKey = option.label.split('BUTTON_PRESS_').pop();
								// Need to fallback as some button actions are not part of button names
								return (
									(labelKey && buttonNames[labelKey]) ||
									t(`PinMapping:actions.${option.label}`)
								);
							}}
							onChange={(change) =>
								setPinAction(
									expansionType,
									0,
									pin,
									change?.value === undefined ? -10 : change.value, // On clear set to -10
								)
							}
						/>
					</div>
				))}
			</div>
			<Button type="button" onClick={handleSave} className="my-4">
				{t('Common:button-save-label')}
			</Button>
			{saveMessage && <Alert variant="secondary">{saveMessage}</Alert>}
		</div>
	);
};

const PCF8575 = ({ values, errors, handleChange, handleCheckbox }) => {
	const { getAvailablePeripherals } = useContext(AppContext);
	const { fetchPins, pins, savePins, setPinAction, setPinDirection } =
		useExpansionPinStore();
	const { t } = useTranslation();

	const expansionType = 'pcf8575';

	useEffect(() => {
		fetchPins();
	}, []);

	const saveAll = useCallback(() => {
		savePins();
	}, [savePins]);

	return (
		<Section title={t('PCF8575:header-text')}>
			<div
				id="PCF8575AddonOptions"
				hidden={!(values.PCF8575AddonEnabled && getAvailablePeripherals('i2c'))}
			>
				<Row className="mb-2">
					<ExpansionPinsForm
						pins={pins}
						expansionType={expansionType}
						savePins={saveAll}
						setPinAction={setPinAction}
						setPinDirection={setPinDirection}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="PCF8575AddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.PCF8575AddonEnabled)}
				onChange={(e) => {
					handleCheckbox('PCF8575AddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default PCF8575;
