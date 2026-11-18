import { useContext, useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import AnalogPinOptions from '../Components/AnalogPinOptions';
import { BUTTON_MASKS_OPTIONS, getButtonLabels } from '../Data/Buttons';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

import useBoardDefinition from '../Store/useBoardDefinitionStore';

const SHMUP_MIXED_MODES = [
	{ label: 'Turbo Priority', value: 0 },
	{ label: 'Charge Priority', value: 1 },
];

const TURBO_MASKS = [
	{ label: 'None', value: 0 },
	{ label: 'B1', value: 1 << 0 },
	{ label: 'B2', value: 1 << 1 },
	{ label: 'B3', value: 1 << 2 },
	{ label: 'B4', value: 1 << 3 },
	{ label: 'L1', value: 1 << 4 },
	{ label: 'R1', value: 1 << 5 },
	{ label: 'L2', value: 1 << 6 },
	{ label: 'R2', value: 1 << 7 },
	{ label: 'L3', value: 1 << 10 },
	{ label: 'R3', value: 1 << 11 },
];

const TURBO_TOGGLE_SLOTS = [
	{ slot: 1, pinKey: 'turboToggle1Pin', buttonKey: 'turboToggle1ButtonMask' },
	{ slot: 2, pinKey: 'turboToggle2Pin', buttonKey: 'turboToggle2ButtonMask' },
	{ slot: 3, pinKey: 'turboToggle3Pin', buttonKey: 'turboToggle3ButtonMask' },
	{ slot: 4, pinKey: 'turboToggle4Pin', buttonKey: 'turboToggle4ButtonMask' },
	{ slot: 5, pinKey: 'turboToggle5Pin', buttonKey: 'turboToggle5ButtonMask' },
	{ slot: 6, pinKey: 'turboToggle6Pin', buttonKey: 'turboToggle6ButtonMask' },
	{ slot: 7, pinKey: 'turboToggle7Pin', buttonKey: 'turboToggle7ButtonMask' },
	{ slot: 8, pinKey: 'turboToggle8Pin', buttonKey: 'turboToggle8ButtonMask' },
] as const;

const SHMUP_SLOTS = [
	{ alwaysOnKey: 'shmupAlwaysOn1', chargeKey: 'shmupBtnMask1' },
	{ alwaysOnKey: 'shmupAlwaysOn2', chargeKey: 'shmupBtnMask2' },
	{ alwaysOnKey: 'shmupAlwaysOn3', chargeKey: 'shmupBtnMask3' },
	{ alwaysOnKey: 'shmupAlwaysOn4', chargeKey: 'shmupBtnMask4' },
] as const;

export const turboScheme = {
	TurboInputEnabled: yup.number().required().label('Turbo Input Enabled'),
	turboPinLED: yup
		.number()
		.label('Turbo Pin LED')
		.validatePinWhenValue('TurboInputEnabled'),
	pinShmupBtn1: yup
		.number()
		.label('Charge Shot 1 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	pinShmupBtn2: yup
		.number()
		.label('Charge Shot 2 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	pinShmupBtn3: yup
		.number()
		.label('Charge Shot 3 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	pinShmupBtn4: yup
		.number()
		.label('Charge Shot 4 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	pinShmupDial: yup
		.number()
		.label('Shmup Dial Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	turboShotCount: yup
		.number()
		.label('Turbo Shot Count')
		.validateRangeWhenValue('TurboInputEnabled', 2, 30),
	shmupMode: yup
		.number()
		.label('Shmup Mode Enabled')
		.validateRangeWhenValue('TurboInputEnabled', 0, 1),
	shmupMixMode: yup
		.number()
		.label('Shmup Mix Priority')
		.validateSelectionWhenValue('TurboInputEnabled', SHMUP_MIXED_MODES),
	shmupAlwaysOn1: yup
		.number()
		.label('Turbo-Button 1 (Always On)')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS_OPTIONS),
	shmupAlwaysOn2: yup
		.number()
		.label('Turbo-Button 2 (Always On)')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS_OPTIONS),
	shmupAlwaysOn3: yup
		.number()
		.label('Turbo-Button 3 (Always On)')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS_OPTIONS),
	shmupAlwaysOn4: yup
		.number()
		.label('Turbo-Button 4 (Always On)')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS_OPTIONS),
	shmupBtnMask1: yup
		.number()
		.label('Charge Shot Button 1 Map')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS_OPTIONS),
	shmupBtnMask2: yup
		.number()
		.label('Charge Shot Button 2 Map')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS_OPTIONS),
	shmupBtnMask3: yup
		.number()
		.label('Charge Shot Button 3 Map')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS_OPTIONS),
	shmupBtnMask4: yup
		.number()
		.label('Charge Shot Button 4 Map')
		.validateSelectionWhenValue('TurboInputEnabled', BUTTON_MASKS_OPTIONS),
	turboLedType: yup.number().required().label('Turbo LED Type'),
	turboHoldToClear: yup
		.number()
		.label('Hold to Clear')
		.validateRangeWhenValue('TurboInputEnabled', 0, 1),
	turboHoldToClearTime: yup
		.number()
		.label('Hold Time (seconds)')
		.validateRangeWhenValue('TurboInputEnabled', 1, 10),
	turboToggle1Pin: yup
		.number()
		.label('Turbo Toggle 1 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	turboToggle1ButtonMask: yup
		.number()
		.label('Turbo Toggle 1 Button')
		.validateSelectionWhenValue('TurboInputEnabled', TURBO_MASKS),
	turboToggle2Pin: yup
		.number()
		.label('Turbo Toggle 2 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	turboToggle2ButtonMask: yup
		.number()
		.label('Turbo Toggle 2 Button')
		.validateSelectionWhenValue('TurboInputEnabled', TURBO_MASKS),
	turboToggle3Pin: yup
		.number()
		.label('Turbo Toggle 3 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	turboToggle3ButtonMask: yup
		.number()
		.label('Turbo Toggle 3 Button')
		.validateSelectionWhenValue('TurboInputEnabled', TURBO_MASKS),
	turboToggle4Pin: yup
		.number()
		.label('Turbo Toggle 4 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	turboToggle4ButtonMask: yup
		.number()
		.label('Turbo Toggle 4 Button')
		.validateSelectionWhenValue('TurboInputEnabled', TURBO_MASKS),
	turboToggle5Pin: yup
		.number()
		.label('Turbo Toggle 5 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	turboToggle5ButtonMask: yup
		.number()
		.label('Turbo Toggle 5 Button')
		.validateSelectionWhenValue('TurboInputEnabled', TURBO_MASKS),
	turboToggle6Pin: yup
		.number()
		.label('Turbo Toggle 6 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	turboToggle6ButtonMask: yup
		.number()
		.label('Turbo Toggle 6 Button')
		.validateSelectionWhenValue('TurboInputEnabled', TURBO_MASKS),
	turboToggle7Pin: yup
		.number()
		.label('Turbo Toggle 7 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	turboToggle7ButtonMask: yup
		.number()
		.label('Turbo Toggle 7 Button')
		.validateSelectionWhenValue('TurboInputEnabled', TURBO_MASKS),
	turboToggle8Pin: yup
		.number()
		.label('Turbo Toggle 8 Pin')
		.validatePinWhenValue('TurboInputEnabled'),
	turboToggle8ButtonMask: yup
		.number()
		.label('Turbo Toggle 8 Button')
		.validateSelectionWhenValue('TurboInputEnabled', TURBO_MASKS),
};

export const turboState = {
	pinShmupBtn1: -1,
	pinShmupBtn2: -1,
	pinShmupBtn3: -1,
	pinShmupBtn4: -1,
	pinShmupDial: -1,
	shmupAlwaysOn1: 0,
	shmupAlwaysOn2: 0,
	shmupAlwaysOn3: 0,
	shmupAlwaysOn4: 0,
	shmupBtnMask1: 0,
	shmupBtnMask2: 0,
	shmupBtnMask3: 0,
	shmupBtnMask4: 0,
	shmupMixMode: 0,
	shmupMode: 0,
	TurboInputEnabled: 0,
	turboPinLED: -1,
	turboShotCount: 5,
	turboLedType: 0,
	turboHoldToClear: 0,
	turboHoldToClearTime: 5,
	turboToggle1Pin: -1,
	turboToggle1ButtonMask: 0,
	turboToggle2Pin: -1,
	turboToggle2ButtonMask: 0,
	turboToggle3Pin: -1,
	turboToggle3ButtonMask: 0,
	turboToggle4Pin: -1,
	turboToggle4ButtonMask: 0,
	turboToggle5Pin: -1,
	turboToggle5ButtonMask: 0,
	turboToggle6Pin: -1,
	turboToggle6ButtonMask: 0,
	turboToggle7Pin: -1,
	turboToggle7ButtonMask: 0,
	turboToggle8Pin: -1,
	turboToggle8ButtonMask: 0,
};

const Turbo = ({
	values,
	errors,
	handleChange,
	handleCheckbox,
	setFieldValue,
}: AddonPropTypes) => {
	const { t } = useTranslation();

	const { usedPins, buttonLabels } = useContext(AppContext);
	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const CURRENT_BUTTONS = getButtonLabels(buttonLabelType, swapTpShareLabels);
	const { boardDefinition } = useBoardDefinition();

	const ANALOG_PINS = boardDefinition.analogPins;

	const availableAnalogPins = ANALOG_PINS.filter(
		(pin) => !usedPins?.includes(pin),
	);

	useEffect(() => {
		SHMUP_SLOTS.forEach(({ alwaysOnKey, chargeKey }) => {
			const alwaysOn = Number(values[alwaysOnKey]);
			if (Number(values[chargeKey]) !== alwaysOn) {
				setFieldValue(chargeKey, alwaysOn);
			}
		});
	}, [
		values.shmupAlwaysOn1,
		values.shmupAlwaysOn2,
		values.shmupAlwaysOn3,
		values.shmupAlwaysOn4,
		values.shmupBtnMask1,
		values.shmupBtnMask2,
		values.shmupBtnMask3,
		values.shmupBtnMask4,
	]);

	const [showLed, setShowLed] = useState(false);
	const ledEnabled = showLed || Number(values.turboLedType) !== -1;
	const [showDial, setShowDial] = useState(false);
	const dialEnabled = showDial || Number(values.pinShmupDial) !== -1;
	const [showToggles, setShowToggles] = useState(false);
	const togglesEnabled =
		showToggles ||
		TURBO_TOGGLE_SLOTS.some(({ pinKey }) => Number(values[pinKey]) !== -1);

	return (
		<Section
			title={
				<a
					href="https://gp2040-ce.info/add-ons/turbo"
					target="_blank"
					className="text-reset text-decoration-none"
				>
					{t('AddonsConfig:turbo-header-text')}
				</a>
			}
		>
			<div id="TurboInputOptions" hidden={!values.TurboInputEnabled}>
				<div className="alert alert-success" role="alert">
					{t('AddonsConfig:turbo-available-pins-text', {
						pins: availableAnalogPins.join(', '),
					})}
				</div>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:turbo-shot-count-label')}
						name="turboShotCount"
						className="form-control-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.turboShotCount}
						error={errors.turboShotCount}
						isInvalid={Boolean(errors.turboShotCount)}
						onChange={handleChange}
						min={2}
						max={30}
					/>
				</Row>
				<Row className="mb-3">
					<FormCheck
						label={t('AddonsConfig:turbo-led-label')}
						type="switch"
						id="TurboLed"
						className="col-sm-3 ms-3"
						isInvalid={false}
						checked={ledEnabled}
						onChange={(e) => {
							setShowLed(e.target.checked);
							if (!e.target.checked) setFieldValue('turboLedType', -1);
							else if (Number(values.turboLedType) === -1)
								setFieldValue('turboLedType', 0);
						}}
					/>
					<div id="TurboLedOptions" hidden={!ledEnabled}>
						<Row className="mb-3">
							<FormSelect
								label={t('AddonsConfig:turbo-led-type-label')}
								name="turboLedType"
								className="form-select-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.turboLedType}
								error={errors.turboLedType}
								isInvalid={Boolean(errors.turboLedType)}
								onChange={(e) =>
									setFieldValue('turboLedType', parseInt(e.target.value))
								}
							>
								<option value="0">
									{t('AddonsConfig:turbo-led-type-label-pwm')}
								</option>
								<option value="1">
									{t('AddonsConfig:turbo-led-type-label-rgb')}
								</option>
							</FormSelect>
							<FormControl
								type="number"
								label={t('AddonsConfig:turbo-led-pin-label')}
								name="turboPinLED"
								hidden={values.turboLedType !== 0}
								className="form-select-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.turboPinLED}
								error={errors.turboPinLED}
								isInvalid={Boolean(errors.turboPinLED)}
								onChange={handleChange}
								min={-1}
								max={boardDefinition.maxPin}
							/>
						</Row>
					</div>
				</Row>
				<Row className="mb-3">
					<FormCheck
						label={t('AddonsConfig:turbo-hold-to-clear-label')}
						type="switch"
						id="TurboHoldToClear"
						className="col-sm-3 ms-3"
						isInvalid={false}
						checked={Boolean(values.turboHoldToClear)}
						onChange={(e) => {
							handleCheckbox('turboHoldToClear');
							handleChange(e);
						}}
					/>
					<div id="TurboHoldToClearOptions" hidden={!values.turboHoldToClear}>
						<Row className="mb-3">
							<FormControl
								type="number"
								label={t('AddonsConfig:turbo-hold-to-clear-time-label')}
								name="turboHoldToClearTime"
								className="form-control-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.turboHoldToClearTime}
								error={errors.turboHoldToClearTime}
								isInvalid={Boolean(errors.turboHoldToClearTime)}
								onChange={handleChange}
								min={1}
								max={10}
							/>
						</Row>
					</div>
				</Row>
				<Row className="mb-3">
					<FormCheck
						label={t('AddonsConfig:turbo-dial-label')}
						type="switch"
						id="TurboDial"
						className="col-sm-3 ms-3"
						isInvalid={false}
						checked={dialEnabled}
						onChange={(e) => {
							setShowDial(e.target.checked);
							if (!e.target.checked) setFieldValue('pinShmupDial', -1);
						}}
					/>
					<div id="TurboDialOptions" hidden={!dialEnabled}>
						<Row className="mb-3">
							<FormSelect
								label={t('AddonsConfig:turbo-shmup-dial-pin-label')}
								name="pinShmupDial"
								className="form-select-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.pinShmupDial}
								error={errors.pinShmupDial}
								isInvalid={Boolean(errors.pinShmupDial)}
								onChange={handleChange}
							>
								<AnalogPinOptions />
							</FormSelect>
						</Row>
					</div>
				</Row>
				<Row className="mb-3">
					<FormCheck
						label={t('AddonsConfig:turbo-toggle-header')}
						type="switch"
						id="TurboGpioToggles"
						className="col-sm-3 ms-3"
						isInvalid={false}
						checked={togglesEnabled}
						onChange={(e) => {
							setShowToggles(e.target.checked);
							if (!e.target.checked) {
								TURBO_TOGGLE_SLOTS.forEach(({ pinKey, buttonKey }) => {
									setFieldValue(pinKey, -1);
									setFieldValue(buttonKey, 0);
								});
							}
						}}
					/>
					<div id="TurboGpioTogglesOptions" hidden={!togglesEnabled}>
						<p className="text-body-secondary">
							{t('AddonsConfig:turbo-toggle-description')}
						</p>
						{[TURBO_TOGGLE_SLOTS.slice(0, 4), TURBO_TOGGLE_SLOTS.slice(4)].map(
							(rowSlots, rowIndex) => (
								<Row key={`turbo-toggle-row-${rowIndex}`} className="mb-3">
									{rowSlots.map(({ slot, pinKey, buttonKey }) => {
										return (
											<div
												key={`turbo-toggle-${slot}`}
												className="col-sm-2 mb-3"
											>
												<FormControl
													type="number"
													label={t('AddonsConfig:turbo-toggle-pin-label', {
														index: slot,
													})}
													name={pinKey}
													className="form-control-sm"
													groupClassName="mb-2"
													value={values[pinKey]}
													error={errors[pinKey]}
													isInvalid={Boolean(errors[pinKey])}
													onChange={handleChange}
													min={-1}
													max={boardDefinition.maxPin}
												/>
												<FormSelect
													label={t('AddonsConfig:turbo-toggle-button-label', {
														index: slot,
													})}
													name={buttonKey}
													className="form-select-sm"
													value={values[buttonKey]}
													error={errors[buttonKey]}
													isInvalid={Boolean(errors[buttonKey])}
													onChange={(e) =>
														setFieldValue(buttonKey, parseInt(e.target.value))
													}
												>
													{TURBO_MASKS.map((o, i) => (
														<option
															key={`${buttonKey}-option-${i}`}
															value={o.value}
														>
															{CURRENT_BUTTONS[o.label] ?? o.label}
														</option>
													))}
												</FormSelect>
											</div>
										);
									})}
								</Row>
							),
						)}
					</div>
				</Row>
				<Row className="mb-3">
					<FormCheck
						label={t('AddonsConfig:turbo-shmup-mode-label')}
						type="switch"
						id="ShmupMode"
						className="col-sm-3 ms-3"
						isInvalid={false}
						checked={Boolean(values.shmupMode)}
						onChange={(e) => {
							handleCheckbox('shmupMode');
							handleChange(e);
						}}
					/>
					<div id="ShmupOptions" hidden={!values.shmupMode}>
						<p className="text-body-secondary">
							{t('AddonsConfig:turbo-shmup-description')}
						</p>
						<Row className="mb-3">
							<FormSelect
								label={t('AddonsConfig:turbo-shmup-always-on-1-label')}
								name="shmupAlwaysOn1"
								className="form-select-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.shmupAlwaysOn1}
								error={errors.shmupAlwaysOn1}
								isInvalid={Boolean(errors.shmupAlwaysOn1)}
								onChange={handleChange}
							>
								{TURBO_MASKS.map((o, i) => (
									<option key={`shmupAlwaysOn1-option-${i}`} value={o.value}>
										{CURRENT_BUTTONS[o.label] ?? o.label}
									</option>
								))}
							</FormSelect>
							<FormSelect
								label={t('AddonsConfig:turbo-shmup-always-on-2-label')}
								name="shmupAlwaysOn2"
								className="form-select-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.shmupAlwaysOn2}
								error={errors.shmupAlwaysOn2}
								isInvalid={Boolean(errors.shmupAlwaysOn2)}
								onChange={handleChange}
							>
								{TURBO_MASKS.map((o, i) => (
									<option key={`shmupAlwaysOn2-option-${i}`} value={o.value}>
										{CURRENT_BUTTONS[o.label] ?? o.label}
									</option>
								))}
							</FormSelect>
							<FormSelect
								label={t('AddonsConfig:turbo-shmup-always-on-3-label')}
								name="shmupAlwaysOn3"
								className="form-select-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.shmupAlwaysOn3}
								error={errors.shmupAlwaysOn3}
								isInvalid={Boolean(errors.shmupAlwaysOn3)}
								onChange={handleChange}
							>
								{TURBO_MASKS.map((o, i) => (
									<option key={`shmupAlwaysOn3-option-${i}`} value={o.value}>
										{CURRENT_BUTTONS[o.label] ?? o.label}
									</option>
								))}
							</FormSelect>
							<FormSelect
								label={t('AddonsConfig:turbo-shmup-always-on-4-label')}
								name="shmupAlwaysOn4"
								className="form-select-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.shmupAlwaysOn4}
								error={errors.shmupAlwaysOn4}
								isInvalid={Boolean(errors.shmupAlwaysOn4)}
								onChange={handleChange}
							>
								{TURBO_MASKS.map((o, i) => (
									<option key={`shmupAlwaysOn4-option-${i}`} value={o.value}>
										{CURRENT_BUTTONS[o.label] ?? o.label}
									</option>
								))}
							</FormSelect>
						</Row>
						<Row className="mb-3" style={{ whiteSpace: 'pre-line' }}>
							<FormControl
								type="number"
								label={t('AddonsConfig:turbo-shmup-button-1-label')}
								name="pinShmupBtn1"
								className="form-control-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.pinShmupBtn1}
								error={errors.pinShmupBtn1}
								isInvalid={Boolean(errors.pinShmupBtn1)}
								onChange={handleChange}
								min={-1}
								max={boardDefinition.maxPin}
							/>
							<FormControl
								type="number"
								label={t('AddonsConfig:turbo-shmup-button-2-label')}
								name="pinShmupBtn2"
								className="form-control-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.pinShmupBtn2}
								error={errors.pinShmupBtn2}
								isInvalid={Boolean(errors.pinShmupBtn2)}
								onChange={handleChange}
								min={-1}
								max={boardDefinition.maxPin}
							/>
							<FormControl
								type="number"
								label={t('AddonsConfig:turbo-shmup-button-3-label')}
								name="pinShmupBtn3"
								className="form-control-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.pinShmupBtn3}
								error={errors.pinShmupBtn3}
								isInvalid={Boolean(errors.pinShmupBtn3)}
								onChange={handleChange}
								min={-1}
								max={boardDefinition.maxPin}
							/>
							<FormControl
								type="number"
								label={t('AddonsConfig:turbo-shmup-button-4-label')}
								name="pinShmupBtn4"
								className="form-control-sm"
								groupClassName="col-sm-2 mb-3"
								value={values.pinShmupBtn4}
								error={errors.pinShmupBtn4}
								isInvalid={Boolean(errors.pinShmupBtn4)}
								onChange={handleChange}
								min={-1}
								max={boardDefinition.maxPin}
							/>
						</Row>
						<FormSelect
							label={t('AddonsConfig:turbo-shmup-mix-mode-label')}
							name="shmupMixMode"
							className="form-select-sm"
							groupClassName="col-sm-2 mb-3"
							value={values.shmupMixMode}
							error={errors.shmupMixMode}
							isInvalid={Boolean(errors.shmupMixMode)}
							onChange={handleChange}
						>
							{SHMUP_MIXED_MODES.map((o, i) => (
								<option
									key={`button-shmupMixedMode-option-${i}`}
									value={o.value}
								>
									{o.label}
								</option>
							))}
						</FormSelect>
					</div>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="TurboInputButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.TurboInputEnabled)}
				onChange={(e) => {
					handleCheckbox('TurboInputEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Turbo;
