import { useCallback, useContext, useEffect, useState } from 'react';

import { useTranslation } from 'react-i18next';
import * as yup from 'yup';

import HECalibration from '../Components/HECalibration';

import useHETriggerStore from '../Store/useHETriggerStore';

import { Alert, Button, FormCheck, Row, Table } from 'react-bootstrap';
import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import { BUTTON_MASKS_OPTIONS } from '../Data/Buttons';
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

// currently using only the gamepad values here
const NON_SELECTABLE_BUTTON_ACTIONS = [
	-5, 0, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
	37, 38, 39,
];


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


const isNonSelectable = (value) =>
	NON_SELECTABLE_BUTTON_ACTIONS.includes(value);

const options = Object.entries(BUTTON_ACTIONS)
	.filter(([, value]) => !isNonSelectable(value))
	.map(([key, value]) => ({
		label: key,
		value,
	}));

type TriggerActionsFormTypes = {
	saveHETriggers: () => void;
	triggers: {
		[key: string]: {
			action: PinActionValues;
			idle: number;
			active: number;
			max: number,
			polarity: number;
		};
	};
	values: {};
	muxChannels: number;
	setHETrigger: (
		key: string,
		action: PinActionValues,
		idle: number,
		active: number,
		max: number,
		polarity: number
	) => void;
	handleChange: (
		e: Event,
	) => void;
};

const TriggerActionsForm = ({
	triggers,
	values,
	muxChannels,
	handleChange,
	saveHETriggers,
	setHETrigger
}: TriggerActionsFormTypes) => {
	const { buttonLabels } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');
	const [showModal, setShowModal] = useState(false);
	const [modalTitle, setModalTitle] = useState('');
	const [calibrationTarget, setCalibrationTarget] = useState('');
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
											inputId={key}
											isClearable
											isSearchable
											options={options}
											value={getOption(triggers[key], triggers[key].action)}
											isDisabled={isNonSelectable(triggers[key].action)}
											getOptionLabel={(option) => {
												const labelKey = option.label.split('BUTTON_PRESS_').pop();
												// Need to fallback as some button actions are not part of button names
												return (
													(labelKey && buttonNames[labelKey]) ||
													t(`PinMapping:actions.${option.label}`)
												);
											}}
											onChange={(change) =>
												setHETrigger( key,
													change?.value === undefined ? -10 : change.value,
													triggers[key].idle,
													triggers[key].active,
													triggers[key].max,
													triggers[key].polarity
												)
											}
										/>
										<Button type="button"
											key={`select-button-he-${index}`}
											onClick={(e) => {
												setShowModal(true);
												setCalibrationTarget(key);
												if (muxChannels > 1)
													setModalTitle(`Mux ${i} - Channel ${index}`);
												else 
													setModalTitle(`Direct - ADC ${values[`muxADCPin${i}`]}`);
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
						setHETrigger={setHETrigger}
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
												<td>{triggers[key].polarity}</td>
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
				{t('Common:button-save-label')}
			</Button>
			{saveMessage && <Alert variant="secondary">{saveMessage}</Alert>}
		</div>
	);
};

const HETrigger = ({ values, errors, handleChange, handleCheckbox }) => {
	const { fetchHETriggers, triggers, saveHETriggers, setHETrigger} =
		useHETriggerStore();
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
						saveHETriggers={saveAll}
						setHETrigger={setHETrigger}
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
