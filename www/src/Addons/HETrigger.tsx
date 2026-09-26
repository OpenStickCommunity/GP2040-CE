import { useContext, useEffect, useState } from 'react';
import { Alert, Button, FormCheck, Row, Table } from 'react-bootstrap';


import { useTranslation } from 'react-i18next';
import * as yup from 'yup';
import omit from 'lodash/omit';

import HECalibrationWizard from '../Components/HECalibrationWizard';
import HEProfileSelector from '../Components/HEProfileSelector';
import HEMonitor from '../Components/HEMonitor';

import useHETriggerStore, { Trigger } from '../Store/useHETriggerStore';
import useHEProfileStore from '../Store/useHEProfileStore';
import useBoardDefinition from '../Store/useBoardDefinitionStore';

import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import AnalogPinOptions from '../Components/AnalogPinOptions';
import { getButtonLabels } from '../Data/Buttons';
import { AddonPropTypes, DEFAULT_VALUES } from '../Pages/AddonsConfigPage';

import './HETrigger.scss';

import {
	BUTTON_ACTIONS,
	//	NON_SELECTABLE_BUTTON_ACTIONS,
	PinActionValues,
} from '../Data/Pins';

// Only provide gamepad inputs for now
const SELECTABLE_BUTTON_ACTIONS = [
	-10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 41,
	42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 59, 60, 61, 62, 63, 64,
	65, 66, 72, 73, 74, 75, 76, 77, 78,
];

// HE-local pseudo-actions for switching binding profiles. These live above
// GpioAction's range so they can share the action field, and they are offered
// here ONLY -- they must never appear in the GPIO pin mapping UI, where they
// would be meaningless. Must match the HE_ACTION_PROFILE_* defines in
// headers/addons/he_trigger.h.
export const HE_ACTION_PROFILE_CYCLE = 1000;
export const HE_ACTION_PROFILE_1 = 1001;

const HE_PROFILE_ACTIONS = [
	HE_ACTION_PROFILE_CYCLE,
	HE_ACTION_PROFILE_1,
	HE_ACTION_PROFILE_1 + 1,
	HE_ACTION_PROFILE_1 + 2,
	HE_ACTION_PROFILE_1 + 3,
];

const heProfileActionLabel = (actionId: number) => {
	if (actionId === HE_ACTION_PROFILE_CYCLE) return 'HE_PROFILE_CYCLE';
	return `HE_PROFILE_${actionId - HE_ACTION_PROFILE_1 + 1}`;
};

const isSelectable = (value) => SELECTABLE_BUTTON_ACTIONS.includes(value);

export const HETriggerScheme = {
	HETriggerEnabled: yup
		.number()
		.required()
		.label('Hall Effect Triggers Enabled'),
	muxChannels: yup
		.number()
		.label('Multiplexer Channels')
		.validateRangeWhenValue('HETriggerEnabled', 0, 16),
	muxADCPin0: yup
		.number()
		.label('Multiplexer ADC 0 Pin')
		.validatePinWhenValue('HETriggerEnabled'),
	muxADCPin1: yup
		.number()
		.label('Multiplexer ADC 1 Pin')
		.validatePinWhenValue('HETriggerEnabled'),
	muxADCPin2: yup
		.number()
		.label('Multiplexer ADC 2 Pin')
		.validatePinWhenValue('HETriggerEnabled'),
	muxADCPin3: yup
		.number()
		.label('Multiplexer ADC 3 Pin')
		.validatePinWhenValue('HETriggerEnabled'),
	separateSelectPins: yup
		.number()
		.label('Separate Select Pins')
		.validateRangeWhenValue('HETriggerEnabled', 0, 1),
	muxes: yup.array().of(
		yup.object().shape({
			selectPin0: yup.number().label('Mux Select 0 Pin'),
			selectPin1: yup.number().label('Mux Select 1 Pin'),
			selectPin2: yup.number().label('Mux Select 2 Pin'),
			selectPin3: yup.number().label('Mux Select 3 Pin'),
		}),
	),
	heTriggerSmoothing: yup
		.number()
		.label('EMA Smoothing')
		.validateRangeWhenValue('HETriggerEnabled', 0, 1),
	heTriggerSmoothingFactor: yup
		.number()
		.label('EMA Smoothing Factor')
		.validateRangeWhenValue('HETriggerEnabled', 1, 99),
};

export const HETriggerState = {
	HETriggerEnabled: 0,
	muxChannels: 1,
	muxADCPin0: 26,
	muxADCPin1: 27,
	muxADCPin2: 28,
	muxADCPin3: -1,
	separateSelectPins: 0,
	muxes: [
		{ selectPin0: 0, selectPin1: 1, selectPin2: 2, selectPin3: -1 },
		{ selectPin0: -1, selectPin1: -1, selectPin2: -1, selectPin3: -1 },
		{ selectPin0: -1, selectPin1: -1, selectPin2: -1, selectPin3: -1 },
		{ selectPin0: -1, selectPin1: -1, selectPin2: -1, selectPin3: -1 },
	],
	heTriggerSmoothing: 0,
	heTriggerSmoothingFactor: 5,
};

// Grouped so the analog stick directions are findable: the flat list ran to
// ~50 entries and buried them among the button presses.
const ANALOG_ACTION_MIN = 59;
const ANALOG_ACTION_MAX = 66;

const buttonOptions = Object.entries(BUTTON_ACTIONS)
	.filter(
		([, value]) =>
			isSelectable(value) &&
			!(value >= ANALOG_ACTION_MIN && value <= ANALOG_ACTION_MAX),
	)
	.map(([key, value]) => ({ label: key, value }));

// Analog directions appear twice: once as the original full-tilt binding, and
// once as a proportional variant. Making proportional a distinct choice rather
// than a separate toggle means it is selectable per profile like any other
// binding, and there is no hidden switch to discover.
const analogOptions = Object.entries(BUTTON_ACTIONS)
	.filter(
		([, value]) => value >= ANALOG_ACTION_MIN && value <= ANALOG_ACTION_MAX,
	)
	.map(([key, value]) => ({ label: key, value }));

// Offset applied to an analog action to mark it proportional. Must match
// HE_ANALOG_PROPORTIONAL_OFFSET in headers/addons/he_trigger.h.
export const HE_ANALOG_PROPORTIONAL_OFFSET = 2000;

const analogProportionalOptions = analogOptions.map((option) => ({
	label: option.label,
	value: (option.value + HE_ANALOG_PROPORTIONAL_OFFSET) as PinActionValues,
}));

const profileOptions = HE_PROFILE_ACTIONS.map((value) => ({
	label: heProfileActionLabel(value),
	value: value as PinActionValues,
}));

type TriggerActionsFormTypes = {
	triggers: Trigger[];
	values: typeof DEFAULT_VALUES;
	muxChannels: number;
};

const TriggerActionsForm = ({
	triggers,
	values,
	muxChannels,
}: TriggerActionsFormTypes) => {
	const saveHETriggers = useHETriggerStore((state) => state.saveHETriggers);
	const heProfiles = useHEProfileStore((state) => state.profiles);

	const { buttonLabels } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');
	const [showWizard, setShowWizard] = useState(false);
	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const [showVoltTable, setShowVoltTable] = useState(false);
	const CURRENT_BUTTONS = getButtonLabels(buttonLabelType, swapTpShareLabels);
	const buttonNames = omit(CURRENT_BUTTONS, ['label', 'value']);
	const { t } = useTranslation('');

	// Shared by the base grid and the per-profile grids. The HE profile
	// pseudo-actions have no PinMapping translation key, so they are resolved
	// against the HETrigger namespace instead.
	// react-select renders these as labelled groups.
	const groupedOptions = [
		{ label: t('HETrigger:option-group-buttons'), options: buttonOptions },
		{ label: t('HETrigger:option-group-analog'), options: analogOptions },
		{
			label: t('HETrigger:option-group-analog-proportional'),
			options: analogProportionalOptions,
		},
		{ label: t('HETrigger:option-group-profiles'), options: profileOptions },
	];

	// Calibration covers any channel bound in any profile, matching the firmware,
	// so the buttons must not be gated on the base profile alone.
	const anyChannelAssigned =
		triggers.some((trigger) => trigger.action !== -10) ||
		heProfiles.some(
			(profile, index) =>
				index > 0 && profile.actions?.some((action) => action !== -10),
		);

	const optionLabel = (option: { label: string; value: number }) => {
		if (
			option.value >= ANALOG_ACTION_MIN + HE_ANALOG_PROPORTIONAL_OFFSET &&
			option.value <= ANALOG_ACTION_MAX + HE_ANALOG_PROPORTIONAL_OFFSET
		) {
			return t('HETrigger:analog-proportional-option', {
				action: t(`PinMapping:actions.${option.label}`),
			});
		}
		if (HE_PROFILE_ACTIONS.includes(option.value)) {
			return t(`HETrigger:action-${option.label.toLowerCase()}`);
		}
		const labelKey = option.label.split('BUTTON_PRESS_').pop();
		// Need to fallback as some button actions are not part of button names
		return (
			(labelKey && buttonNames[labelKey]) ||
			t(`PinMapping:actions.${option.label}`)
		);
	};

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
		<div>
			<div className="mt-2">
				<div className="mt-2">
					<h1>{t('HETrigger:action-assignment-sub-header')}</h1>
				</div>
				<div className="mt-2 d-flex gap-2 flex-wrap">
					{/* Guided flow: calibrates every assigned button in one pass. */}
					<Button
						type="button"
						key={`calibrate-wizard-he`}
						onClick={() => setShowWizard(true)}
						disabled={!anyChannelAssigned}
						className="my-2"
					>
						{t('HETrigger:wizard-button')}
					</Button>
				</div>
				<HECalibrationWizard
					values={values}
					showModal={showWizard}
					setShowModal={setShowWizard}
				></HECalibrationWizard>
				<HEProfileSelector
					options={groupedOptions}
					muxChannels={muxChannels}
					// Only offer channels on multiplexers that actually have an ADC pin
					// assigned; an unset pin means that board is not connected.
					connectedMuxes={Array.from(
						{ length: Math.min(4, Math.floor(32 / muxChannels)) },
						(_, i) => values[`muxADCPin${i}` as keyof typeof values] as number,
					)}
					getOptionLabel={optionLabel}
				></HEProfileSelector>
				<HEMonitor
					muxChannels={muxChannels}
					actionForChannel={(channel) => triggers[channel]?.action ?? -10}
				></HEMonitor>
			</div>
			<div className="mt-2">
				<Button
					type="button"
					onClick={() => {
						setShowVoltTable(!showVoltTable);
					}}
					className="my-4"
				>
					{!showVoltTable
						? t('HETrigger:voltage-table-show-label')
						: t('HETrigger:voltage-table-hide-label')}{' '}
					⚡
				</Button>
			</div>
			<div hidden={!showVoltTable} className="mt-2">
				<div>
					<h1>{t('HETrigger:voltage-table-header-text')}</h1>
				</div>
				<div>
					{Array.from(
						{ length: Math.min(4, Math.floor(32 / muxChannels)) },
						(_, i) => (
							<div
								key={`voltage-table-header-${i}`}
								className="mt-3 mb-3"
								hidden={
									Number(values[`muxADCPin${i}` as keyof typeof values]) === -1
								}
							>
								<div className="d-flex flex-shrink-0">
									<label>
										{muxChannels > 1
											? `${t('HETrigger:multiplexer-label')} ${i}`
											: 'Direct'}{' '}
										(ADC{' '}
										{String(values[`muxADCPin${i}` as keyof typeof values])})
									</label>
								</div>
								{values[`muxADCPin${i}` as keyof typeof values] !== -1 ? (
									<div
										className={`action-grid-HE-trigger-${muxChannels} gap-0 mt-0 mb-0`}
									>
										<Table bordered className="mb-0 mt-0">
											<thead>
												<tr>
													<th>{t('HETrigger:channel-label')}</th>
													<th>{t('HETrigger:voltage-table-idle-text')}</th>
													<th>{t('HETrigger:voltage-table-pressed-text')}</th>
													<th>{t('HETrigger:voltage-table-span-text')}</th>
													<th>{t('HETrigger:voltage-table-actuation-text')}</th>
													<th>{t('HETrigger:voltage-table-polarity-text')}</th>
													<th>
														{t('HETrigger:voltage-table-rapid-trigger-text')}
													</th>
													<th>{t('HETrigger:voltage-table-rt-press-text')}</th>
													<th>
														{t('HETrigger:voltage-table-rt-release-text')}
													</th>
													<th>{t('HETrigger:voltage-table-noise-text')}</th>
												</tr>
											</thead>
											<tbody>
												{Object.keys(triggers)
													.slice(i * muxChannels, i * muxChannels + muxChannels)
													.map((key, index) => (
														<tr key={`table-tr-triggers-${index}`}>
															<td>
																{index}{' '}
																{triggers[key].action === -10
																	? t('HETrigger:voltage-table-disabled-label')
																	: ''}
															</td>
															<td>{triggers[key].idle}</td>
															<td>{triggers[key].pressed}</td>
															{/* Span is what makes a bad calibration obvious at a glance:
											    a near-zero span means the channel never really moved. */}
															<td>
																{Math.abs(
																	triggers[key].pressed - triggers[key].idle,
																)}
															</td>
															<td>{triggers[key].actuationPoint}%</td>
															<td>{triggers[key].is_polarized ? 'S' : 'N'}</td>
															<td>
																{triggers[key].rapidTrigger
																	? 'Enabled'
																	: 'Disabled'}
															</td>
															<td>
																{triggers[key].rapidTrigger
																	? `${triggers[key].rtPressSensitivity}%`
																	: 'N/A'}
															</td>
															<td>
																{triggers[key].rapidTrigger
																	? `${triggers[key].rtReleaseSensitivity}%`
																	: 'N/A'}
															</td>
															<td>{triggers[key].noise}</td>
														</tr>
													))}
											</tbody>
										</Table>
									</div>
								) : (
									''
								)}
							</div>
						),
					)}
				</div>
			</div>
			<div className="mt-2">
				<Button type="button" onClick={handleSave} className="my-2">
					{t('HETrigger:save-button')}
				</Button>
				{saveMessage && <Alert variant="secondary">{saveMessage}</Alert>}
			</div>
		</div>
	);
};

const HETrigger = ({
	values,
	errors,
	handleChange,
	handleCheckbox,
}: AddonPropTypes) => {
	const { fetchHETriggers, triggers } = useHETriggerStore();
	const { t } = useTranslation();

	const CHANNEL_SELECT = {
		1: t('HETrigger:direct-no-mux'),
		4: t('HETrigger:4-channels'),
		8: t('HETrigger:8-channels'),
		16: t('HETrigger:16-channels'),
	};

	const { usedPins } = useContext(AppContext);
	const { boardDefinition } = useBoardDefinition();

	const ANALOG_PINS = boardDefinition.analogPins;

	const availableAnalogPins = ANALOG_PINS.filter(
		(pin) => !usedPins?.includes(pin),
	);

	useEffect(() => {
		fetchHETriggers();
	}, []);

	return (
		<Section
			title={
				<a
					href="https://gp2040-ce.info/add-ons/he-trigger"
					target="_blank"
					className="text-reset text-decoration-none"
				>
					{t('HETrigger:header-text')}
				</a>
			}
		>
			<div id="HETriggerOptions" hidden={!values.HETriggerEnabled}>
				<div className="alert alert-info" role="alert">
					{t('HETrigger:desc-header-text')}
				</div>
				<div className="alert alert-success" role="alert">
					{t('HETrigger:available-pins-text', {
						pins: availableAnalogPins.join(', '),
					})}
				</div>
				<Row className="mt-2">
					<FormSelect
						label={t('HETrigger:multiplexer-channel-select')}
						name="muxChannels"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.muxChannels}
						error={errors.muxChannels}
						isInvalid={Boolean(errors.muxChannels)}
						onChange={handleChange}
					>
						{Object.entries(CHANNEL_SELECT).map(([num, label], i) => (
							<option key={`channels-per-mux-option-${i}`} value={num}>
								{label}
							</option>
						))}
					</FormSelect>
				</Row>
				<Row className="mb-3" hidden={values.muxChannels < 4}>
					<FormCheck
						label={t('HETrigger:separate-select-pins-label')}
						type="switch"
						id="HETriggerSeparateSelectPins"
						className="col-sm-4 mt-auto mb-auto ms-3"
						isInvalid={false}
						checked={Boolean(values.separateSelectPins)}
						onChange={(e) => {
							handleCheckbox('separateSelectPins');
							handleChange(e);
						}}
					/>
				</Row>
				{/* Mux 0 always uses the primary select pins; extra muxes appear
				    when separate select pins are enabled and their ADC pin is set. */}
				{[0, 1, 2, 3].map((mux) => {
					const isExtra = mux > 0;
					if (isExtra && !values.separateSelectPins) return null;
					if (
						isExtra &&
						Number(values[`muxADCPin${mux}` as keyof typeof values]) === -1
					)
						return null;
					const muxValues = (values.muxes?.[mux] ?? {}) as Record<
						string,
						number
					>;
					const muxErrors = ((
						errors.muxes as unknown as Record<string, string>[]
					)?.[mux] ?? {}) as Record<string, string>;
					return (
						<Row
							className="mb-3"
							key={`select-pin-row-${mux}`}
							hidden={values.muxChannels < 4}
						>
							{Boolean(values.separateSelectPins) && (
								<div className="d-flex flex-shrink-0 mb-2">
									<label>
										{t('HETrigger:multiplexer-label')} {mux}
									</label>
								</div>
							)}
							{[0, 1, 2, 3].map((bit) => {
								const name = `muxes.${mux}.selectPin${bit}`;
								const minChannels = bit < 2 ? 4 : bit === 2 ? 8 : 16;
								return (
									<FormControl
										key={name}
										type="number"
										label={t(`HETrigger:select-pin-${bit}`)}
										name={name}
										hidden={values.muxChannels < minChannels}
										className="form-select-sm"
										groupClassName="col-sm-2 mb-3"
										value={muxValues[`selectPin${bit}`]}
										error={muxErrors[`selectPin${bit}`]}
										isInvalid={Boolean(muxErrors[`selectPin${bit}`])}
										onChange={handleChange}
										min={-1}
										max={boardDefinition.maxPin}
									/>
								);
							})}
						</Row>
					);
				})}
				<Row className="mb-3">
					{boardDefinition.analogPins.map((val, i) => (
						<FormSelect
							key={`muxADCPin${i}`}
							label={t(`HETrigger:adc-pin-${i}`)}
							name={`muxADCPin${i}`}
							// A wide mux covers every channel from fewer ADCs, so the
							// trailing selects would only be able to pick an unused pin.
							hidden={
								(i === 2 && values.muxChannels >= 16) ||
								(i === 3 && values.muxChannels >= 8)
							}
							className="form-select-sm"
							groupClassName="col-sm-2 mb-3"
							value={values[`muxADCPin${i}`]}
							error={errors[`muxADCPin${i}`]}
							isInvalid={Boolean(errors[`muxADCPin${i}`])}
							onChange={handleChange}
						>
							<AnalogPinOptions />
						</FormSelect>
					))}
				</Row>
				<Row className="mb-3">
					<FormCheck
						label={t('AddonsConfig:analog-smoothing')}
						type="switch"
						id="TriggerSmoothingAddonButton"
						className="col-sm-3 mt-auto mb-auto ms-3"
						isInvalid={false}
						checked={Boolean(values.heTriggerSmoothing)}
						onChange={(e) => {
							handleCheckbox('heTriggerSmoothing');
							handleChange(e);
						}}
					/>
					<FormControl
						hidden={!values.heTriggerSmoothing}
						type="number"
						label={t('AddonsConfig:smoothing-factor')}
						name="heTriggerSmoothingFactor"
						className="form-control-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.heTriggerSmoothingFactor}
						error={errors.heTriggerSmoothingFactor}
						isInvalid={Boolean(errors.heTriggerSmoothingFactor)}
						onChange={handleChange}
						min={1}
						max={99}
					/>
				</Row>
				<Row className="mb-2">
					<TriggerActionsForm
						key="triggers-actions-form"
						values={values}
						triggers={triggers}
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
					handleCheckbox('HETriggerEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default HETrigger;
