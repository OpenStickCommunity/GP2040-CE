import { useCallback, useContext, useEffect, useState } from 'react';

import { useTranslation } from 'react-i18next';
import * as yup from 'yup';

import HECalibration from '../Components/HECalibration';

import useHETriggerStore, { Trigger } from '../Store/useHETriggerStore';

import { Alert, Button, FormCheck, Row, Table } from 'react-bootstrap';
import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import { ANALOG_PINS } from '../Data/Buttons';
import invert from 'lodash/invert';
import omit from 'lodash/omit';
import CustomSelect from '../Components/CustomSelect';

import AnalogPinOptions from '../Components/AnalogPinOptions';

import { getButtonLabels } from '../Data/Buttons';

import './HETrigger.scss';

import {
	BUTTON_ACTIONS,
	//	NON_SELECTABLE_BUTTON_ACTIONS,
	PinActionValues,
} from '../Data/Pins';

// Only provide gamepad inputs for now
const SELECTABLE_BUTTON_ACTIONS = [
	-10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 59, 60, 61, 62,
	63, 64, 65, 66, 72, 73, 74, 75, 76, 77, 78
];

const isSelectable = (value) =>
	SELECTABLE_BUTTON_ACTIONS.includes(value);

export const HETriggerScheme = {
	HETriggerEnabled: yup.number().required().label('Hall Effect Triggers Enabled')
};

export const HETriggerState = {
	HETriggerEnabled: 0,
};

const CHANNEL_SELECT = {
	"Direct (No Mux)": 1,
	"4-Channels": 4,
	"8-Channels": 8,
	"16-Channels": 16
};

const getOption = (e, actionId) => {
	return {
		label: invert(BUTTON_ACTIONS)[actionId],
		value: actionId,
	};
};

const options = Object.entries(BUTTON_ACTIONS)
	.filter(([, value]) => isSelectable(value))
	.map(([key, value]) => ({
		label: key,
		value,
	}));

type TriggerActionsFormTypes = {
	triggers: Trigger[];
	values: {};
	muxChannels: number;
	handleChange: (
		e: Event,
	) => void;
};

const TriggerActionsForm = ({
	triggers,
	values,
	muxChannels,
	handleChange,
}: TriggerActionsFormTypes) => {
	const setHETrigger = useHETriggerStore((state) => state.setHETrigger);
	const saveHETriggers = useHETriggerStore((state) => state.saveHETriggers);

	const { buttonLabels } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');
	const [showModal, setShowModal] = useState(false);
	const [modalTitle, setModalTitle] = useState('');
	const [calibrationTarget, setCalibrationTarget] = useState(0);
	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const [showVoltTable, setShowVoltTable] = useState(false);
	const CURRENT_BUTTONS = getButtonLabels(buttonLabelType, swapTpShareLabels);
	const buttonNames = omit(CURRENT_BUTTONS, ['label', 'value']);
	const { t } = useTranslation('');

	const handleSave = async (e) => {
		e.preventDefault();
		e.stopPropagation();
		try {
			await saveHETriggers();
			setSaveMessage(t('Common:saved-success-message'));
		} catch (error) {
			setSaveMessage(t('Common:saved-error-message'));
		}
	};

	return (
		<div className="mb-3">
			<div className="mt-2">
				<div>
					<div className="mt-2">
						<h1>Hall-Effect Action Assignment</h1>
					</div>
					{Array.from({ length: Math.min(4,Math.floor(32/muxChannels)) }, (_, i) => (
						<div className="mt-3 mb-3" hidden={values[`muxADCPin${i}`] === -1}>

							<div className="d-flex flex-shrink-0">
								<label htmlFor={i}>
									{muxChannels > 1 ? `Multiplexer ${i}` : 'Direct'} (ADC {values[`muxADCPin${i}`]})
								</label>
							</div>
							{ (values['muxADCPin${i}'] !== -1) ?
							<div className={`action-grid-HE-trigger-${muxChannels} gap-3 mt-2 mb-3`}>
								{Object.keys(triggers).splice(i*muxChannels,muxChannels).map((key, index) => (
									<div
										key={`select-he-${index}`}
										className="d-flex align-items-center gap-2"
									>
										<div className="d-flex flex-shrink-0" style={{ width: '5rem' }}>
											<label htmlFor={key}>Channel {index}</label>
										</div>
										<CustomSelect
											key={`select-option-he-${index}`}
											inputId={key}
											isClearable
											isSearchable
											options={options}
											value={getOption(triggers[key], triggers[key].action)}
											getOptionLabel={(option) => {
												const labelKey = option.label.split('BUTTON_PRESS_').pop();
												// Need to fallback as some button actions are not part of button names
												return (
													(labelKey && buttonNames[labelKey]) ||
													t(`PinMapping:actions.${option.label}`)
												);
											}}
											onChange={(change) =>
												setHETrigger(
													{
														id: parseInt(key),
														action: change?.value === undefined ? -10 : change.value,
														idle: triggers[key].idle,
														active: triggers[key].active,
														max: triggers[key].max,
														polarity: triggers[key].polarity
													}
												)
											}
										/>
										<Button type="button"
											key={`select-button-he-${index}`}
											onClick={(e) => {
												setShowModal(true);
												setCalibrationTarget(parseInt(key));
												const option = getOption(triggers[key], triggers[key].action);
												const actionTitle = t(`PinMapping:actions.${option.label}`);
												if (muxChannels > 1) {
													setModalTitle(`${actionTitle} - Mux ${i} - Channel ${index}`);
												} else {
													setModalTitle(`${actionTitle} - Direct - ADC ${values[`muxADCPin${i}`]}`);
												}
											}}
											disabled={triggers[key].action === -10}
											className="d-flex flex-shrink-0">
											🧲
										</Button>
									</div>
								))}
							</div> : '' }
						</div>
					))}
					<HECalibration
						name="hallEffectCalibration"
						values={values}
						showModal={showModal}
						setShowModal={setShowModal}
						triggers={triggers}
						target={calibrationTarget}
						title={modalTitle}
						handleChange={handleChange}
					></HECalibration>
				</div>
				<Button type="button" onClick={() => {setShowVoltTable(!showVoltTable)}} className="my-4">
				    {!showVoltTable ? "Show" : "Hide"} Voltage Table⚡
			    </Button>
				<div hidden={!showVoltTable}>
					<div className="mt-2">
						<h1>Hall-Effect Voltage Table</h1>
					</div>
					<div className="mt-2">
						{Array.from({ length: Math.min(4,Math.floor(32/muxChannels)) }, (_, i) => (
							<div className="mt-3 mb-3" hidden={values[`muxADCPin${i}`] === -1}>
								<div className="d-flex flex-shrink-0">
									<label htmlFor={i}>
										{muxChannels > 1 ? `Multiplexer ${i}` : 'Direct'} (ADC {values[`muxADCPin${i}`]})
									</label>
								</div>
								{ (values['muxADCPin${i}'] !== -1) ?
								<div className={`action-grid-HE-trigger-${muxChannels} gap-0 mt-0 mb-0`}>
									<Table bordered className="mb-0 mt-0">
										<thead>
											<tr>
												<th>Channel</th>
												<th>Idle</th>
												<th>Trigger</th>
												<th>Max</th>
												<th>Polarity</th>
											</tr>
										</thead>
										<tbody>
										{Object.keys(triggers).splice(i*muxChannels,muxChannels).map((key, index) => (
											<tr>
												<td>{index} {triggers[key].action===-10?'(Disabled)':''}</td>
												<td>{triggers[key].idle}</td>
												<td>{triggers[key].active}</td>
												<td>{triggers[key].max}</td>
												<td>{triggers[key].polarity == 1 ? 'S' : 'N'}</td>
											</tr>
										))}
										</tbody>
									</Table>
								</div> : '' }
							</div>
						))}
					</div>
				</div>
			</div>
			<Button type="button" onClick={handleSave} className="my-4">
				{t('HETrigger:save-button')}
			</Button>
			{saveMessage && <Alert variant="secondary">{saveMessage}</Alert>}
		</div>
	);
};

const HETrigger = ({ values, errors, handleChange, handleCheckbox }) => {
	const { fetchHETriggers, triggers, saveHETriggers} = useHETriggerStore();
	const { t } = useTranslation();

	const { usedPins } = useContext(AppContext);
	const availableAnalogPins = ANALOG_PINS.filter(
		(pin) => !usedPins?.includes(pin),
	);

	useEffect(() => {
		fetchHETriggers();
	}, []);

	const saveAll = useCallback(() => {
		saveHETriggers();
	}, [saveHETriggers]);

	return (
		<Section title={
			<a
				href="https://gp2040-ce.info/add-ons/he-trigger"
				target="_blank"
				className="text-reset text-decoration-none"
			>
				{t('HETrigger:header-text')}
			</a>
			}
		>
			<div
				id="HETriggerOptions"
				hidden={!(values.HETriggerEnabled)}
			>
				<div className="alert alert-info" role="alert">
					Hall Effect Trigger Supports 4-Channel, 8-Channel, and 16-Channel Multiplexers.
				</div>
				<div className="alert alert-success" role="alert">
					{t('HETrigger:available-pins-text', {
						pins: availableAnalogPins.join(', '),
					})}
				</div>
				<div className="mt-2">
					<FormSelect
						label="Channels Per Multiplexer"
						name="muxChannels"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.muxChannels}
						error={errors.muxChannels}
						isInvalid={errors.muxChannels}
						onChange={handleChange}
					>
						{Object.entries(CHANNEL_SELECT).map(([label, num], i) => (
							<option key={`channels-per-mux-option-${i}`} value={num}>
								{label}
							</option>
						))}
					</FormSelect>
				</div>
				<div className="mb-3 row">
					<FormControl
						type="number"
						label='Select Pin 0'
						name="muxSelectPin0"
						hidden = {values.muxChannels < 4}
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.muxSelectPin0}
						error={errors.muxSelectPin0}
						isInvalid={errors.muxSelectPin0}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label='Select Pin 1'
						name="muxSelectPin1"
						hidden = {values.muxChannels < 4}
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.muxSelectPin1}
						error={errors.muxSelectPin1}
						isInvalid={errors.muxSelectPin1}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label='Select Pin 2'
						name="muxSelectPin2"
						hidden = {values.muxChannels < 8}
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.muxSelectPin2}
						error={errors.muxSelectPin2}
						isInvalid={errors.muxSelectPin2}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label='Select Pin 3'
						name="muxSelectPin3"
						hidden = {values.muxChannels < 16}
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.muxSelectPin3}
						error={errors.muxSelectPin3}
						isInvalid={errors.muxSelectPin3}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
				</div>
				<div className="mb-3 row">
					<FormSelect
						label={`ADC Pin 0`}
						name='muxADCPin0'
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values['muxADCPin0']}
						error={errors['muxADCPin0']}
						isInvalid={errors['muxADCPin0']}
						onChange={handleChange}
					>
						<AnalogPinOptions />
					</FormSelect>
					<FormSelect
						label={`ADC Pin 1`}
						name='muxADCPin1'
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values['muxADCPin1']}
						error={errors['muxADCPin1']}
						isInvalid={errors['muxADCPin1']}
						onChange={handleChange}
					>
						<AnalogPinOptions />
					</FormSelect>
					<FormSelect
						label={`ADC Pin 2`}
						name='muxADCPin2'
						hidden = {values.muxChannels >= 16}
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values['muxADCPin2']}
						error={errors['muxADCPin2']}
						isInvalid={errors['muxADCPin2']}
						onChange={handleChange}
					>
						<AnalogPinOptions />
					</FormSelect>
					<FormSelect
						label={`ADC Pin 3`}
						name='muxADCPin3'
						hidden = {values.muxChannels >= 8}
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values['muxADCPin3']}
						error={errors['muxADCPin3']}
						isInvalid={errors['muxADCPin3']}
						onChange={handleChange}
					>
						<AnalogPinOptions />
					</FormSelect>
				</div>
				<Row className="mb-2">
					<TriggerActionsForm
						values={values}
						triggers={triggers}
						handleChange={handleChange}
						muxChannels={values.muxChannels}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="HETriggerButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.HETriggerEnabled)}
				onChange={(e) => {
					handleCheckbox('HETriggerEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default HETrigger;
