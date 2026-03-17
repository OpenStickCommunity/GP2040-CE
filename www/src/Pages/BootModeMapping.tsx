import { Alert, Button, Col, Container, Form, Row } from 'react-bootstrap';
import { ReactNode, useContext, useEffect } from 'react';
import Section from '../Components/Section';
import { useShallow } from 'zustand/react/shallow';
import { NavLink } from 'react-router-dom';
import {
	useBootModeStore,
	useBootModeStoreActions,
	NUM_PINS,
} from '../Store/useBootModesStore';
import { INPUT_MODE_OPTIONS, InputModeOptions } from '../Data/InputBootModes';
import { AppContext } from '../Contexts/AppContext';
import CustomSelect from '../Components/CustomSelect';
import { Trans, useTranslation } from 'react-i18next';
import { ActionMeta, MultiValue, SingleValue } from 'react-select';
import CaptureButton from '../Components/CaptureButton';
import useProfilesStore, { MaskPayload } from '../Store/useProfilesStore';
import omit from 'lodash/omit';
import { BUTTON_ACTIONS } from '../Data/Pins';
import ContextualHelpOverlay from '../Components/ContextualHelpOverlay';

type PinOption = {
	label: string;
	value: number;
};

type ProfileOption = {
	label: string;
	value: number;
	disabled: boolean;
};

const MAX_INPUT_MODES = 8;

const GROUPED_OPTIONS = [
	{
		label: 'Primary',
		options: INPUT_MODE_OPTIONS.filter(({ group }) => group === 'primary'),
	},
	{
		label: 'Mini',
		options: INPUT_MODE_OPTIONS.filter(({ group }) => group === 'mini'),
	},
];

const PIN_OPTIONS: PinOption[] = Array.from({ length: NUM_PINS }, (_, i) => ({
	label: `GP${i}`,
	value: i,
}));

function BootModeSelect({ mappingKey }: { mappingKey: string }) {
	const inputMode = useBootModeStore((state) => state.bootModes[mappingKey].inputMode);
	const saveAttempted = useBootModeStore((state) => state.saveAttempted);
	const { setInputMode, clearErrors, setDirty } = useBootModeStoreActions();

	const { getAvailablePeripherals } = useContext(AppContext);
	const { t } = useTranslation('');

	const value = INPUT_MODE_OPTIONS.find(({ value }) => value === inputMode);
	const usbAvailable: boolean = getAvailablePeripherals('usb');

	const isOptionDisabled = (option: InputModeOptions) => {
		return option.required.includes('usb') && !usbAvailable;
	};

	const getOptionLabel = (option: InputModeOptions) => {
		return (
			t(`SettingsPage:${option.labelKey}`) +
			(isOptionDisabled(option) ? ' (USB peripheral not enabled)' : '')
		);
	};

	const onChange = (option: SingleValue<InputModeOptions>) => {
		clearErrors();
		setInputMode(mappingKey, option?.value);
		setDirty();
	};

	const isInvalid = saveAttempted && !value;

	return (
		<CustomSelect
			isClearable={false}
			isMulti={false}
			options={GROUPED_OPTIONS}
			isOptionDisabled={isOptionDisabled}
			isDisabled={false}
			getOptionLabel={getOptionLabel}
			onChange={onChange}
			value={value}
			isInvalid={isInvalid}
		/>
	);
}

function PinSelect({ mappingKey }: { mappingKey: string }) {
	const pins = useBootModeStore((state) => state.bootModes[mappingKey].pins);
	const modesWithDuplicates = useBootModeStore((state) => state.modesWithDuplicates);
	const saveAttempted = useBootModeStore((state) => state.saveAttempted);

	const { addPin, removePin, validatePins, clearErrors, setDirty } =
		useBootModeStoreActions();

	// Need the profile pin mapping to determine which pins are assigned to addons or reserved,
	// relying on the assumption that these are the same across all profiles.
	const profilePins: { [key: string]: MaskPayload } = useProfilesStore(
		useShallow((state) => omit(state.profiles[0], ['profileLabel', 'enabled'])),
	);

	const { t } = useTranslation('');

	const pinField = (value: number) => {
		let s = '0' + value;
		return 'pin' + s.substring(s.length - 2);
	};

	const values = PIN_OPTIONS.filter(({ value }) => pins.has(value));

	const onChange = (_: MultiValue<PinOption>, action: ActionMeta<PinOption>) => {
		if (action.action === 'select-option' && action.option !== undefined) {
			addPin(mappingKey, action.option.value);
		} else if (action.action === 'remove-value') {
			removePin(mappingKey, action.removedValue.value);
		}
		clearErrors();
		validatePins(t('BootModeMapping:unique-validation-err'));
		setDirty();
	};

	const isInvalid =
		modesWithDuplicates.includes(mappingKey) || (saveAttempted && values.length == 0);

	const isOptionDisabled = (option: PinOption) => {
		return [BUTTON_ACTIONS.RESERVED, BUTTON_ACTIONS.ASSIGNED_TO_ADDON].includes(
			profilePins[pinField(option.value)].action,
		);
	};

	const getOptionLabel = (option: PinOption) => {
		if (profilePins[pinField(option.value)].action == BUTTON_ACTIONS.RESERVED) {
			return `${option.label} (Reserved)`;
		}
		if (profilePins[pinField(option.value)].action == BUTTON_ACTIONS.ASSIGNED_TO_ADDON) {
			return `${option.label} (Assigned to Add-on)`;
		}
		return option.label;
	};

	return (
		<div className="d-flex gap-2">
			<CustomSelect
				isClearable={false}
				isMulti={true}
				options={PIN_OPTIONS}
				isDisabled={false}
				onChange={onChange}
				value={values}
				isInvalid={isInvalid}
				isOptionDisabled={isOptionDisabled}
				getOptionLabel={getOptionLabel}
			/>
			<CaptureButton
				labels={['']}
				onChange={(_, pin) => {
					addPin(mappingKey, pin);
					clearErrors();
					validatePins(t('BootModeMapping:unique-validation-err'));
				}}
				small={true}
			/>
		</div>
	);
}

function ProfileSelect({ mappingKey }: { mappingKey: string }) {
	const profiles = useProfilesStore((state) => state.profiles);
	const profileOptions = profiles.map(({ profileLabel, enabled }, i) => ({
		label: profileLabel,
		value: i,
		disabled: !enabled,
	}));

	const profileIndex = useBootModeStore(
		(state) => state.bootModes[mappingKey].profileIndex,
	);
	const { setProfileIndex, setDirty } = useBootModeStoreActions();
	const value = profileOptions.find(({ value }) => value === profileIndex);

	const getLabel = (option: ProfileOption) => {
		const label = option.label ? option.label : `Profile ${option.value + 1}`;
		return label + (option.disabled ? ' (Disabled)' : '');
	};

	const onChange = (selected: any, action: ActionMeta<ProfileOption>) => {
		if (action.action === 'clear') {
			setProfileIndex(mappingKey, undefined);
		} else if (action.action === 'select-option') {
			setProfileIndex(mappingKey, selected.value);
		}
		setDirty();
	};

	return (
		<CustomSelect
			isClearable={true}
			isMulti={false}
			options={profileOptions}
			getOptionLabel={getLabel}
			isOptionDisabled={(option) => option.disabled}
			onChange={onChange}
			placeholder="Last Used"
			value={value}
		/>
	);
}

function FormRow({
	col0,
	col1,
	col2,
	col3,
}: {
	col0?: ReactNode;
	col1?: ReactNode;
	col2?: ReactNode;
	col3?: ReactNode;
}) {
	return (
		<Row className="gx-2 mb-3 align-items-center">
			<Col sm={3}>{col0}</Col>
			<Col>{col1}</Col>
			<Col sm={3}>{col2}</Col>
			<Col md="auto">{col3}</Col>
		</Row>
	);
}

function BootModeRow({ mappingKey }: { mappingKey: string }) {
	const { removeBootMode, clearErrors, setDirty } = useBootModeStoreActions();

	return (
		<FormRow
			col0={<BootModeSelect mappingKey={mappingKey} />}
			col1={<PinSelect mappingKey={mappingKey} />}
			col2={<ProfileSelect mappingKey={mappingKey} />}
			col3={
				<Button
					onClick={() => {
						removeBootMode(mappingKey);
						clearErrors();
						setDirty();
					}}
				>
					{'✕'}
				</Button>
			}
		/>
	);
}

function FixedBootModeRow({
	label,
	mappingKey,
}: {
	label: string;
	mappingKey: 'usbMode' | 'webConfig';
}) {
	return (
		<FormRow
			col0={<label className="ms-2">{label}</label>}
			col1={<PinSelect mappingKey={mappingKey} />}
			col2={<CustomSelect isDisabled={true} placeholder="N/A" />}
			col3={<Button disabled={true}>{'✕'}</Button>}
		/>
	);
}

export default function BootModeMappingPage() {
	const enabled = useBootModeStore((state) => state.enabled);
	const loadingBootModes = useBootModeStore((state) => state.loadingBootModes);
	const bootModes = useBootModeStore((state) => state.bootModes);
	const saveAttempted = useBootModeStore((state) => state.saveAttempted);
	const dirty = useBootModeStore((state) => state.dirty);
	const errorMessage = useBootModeStore((state) => state.errorMessage);

	const {
		addBootMode,
		fetchBootModeOptions,
		saveBootModeOptions,
		clearErrors,
		setEnabled,
		setDirty,
		validateRequired,
	} = useBootModeStoreActions();

	const loadingProfiles = useProfilesStore((state) => state.loadingProfiles);
	const fetchProfiles = useProfilesStore((state) => state.fetchProfiles);
	const { t } = useTranslation('');

	useEffect(() => {
		fetchBootModeOptions();
		fetchProfiles();
	}, []);

	// The delete-able input mode keys (i.e. not web-config or usb mode)
	const inputModeKeys = Object.keys(bootModes).filter((k) => k.startsWith('inputMode-'));

	const handleSubmit = () => {
		validateRequired(t('BootModeMapping:required-validation-err'));
		saveBootModeOptions(t('BootModeMapping:generic-save-err'));
	};

	const showSaveMessage = !dirty && saveAttempted && errorMessage === undefined;

	return (
		<div>
			{enabled && (
				<div className="alert alert-warning">
					<Trans
						ns="BootModeMapping"
						i18nKey="alert-text"
						components={[<NavLink to="/reset-settings" />]}
					/>
				</div>
			)}
			<Section title={t('SettingsPage:boot-input-mode-label')}>
				<div className="d-flex align-items-center mb-2">
					<Form.Check
						label="Use GPIO Pins"
						type="switch"
						className="text my-auto"
						checked={enabled}
						onChange={(e) => {
							setEnabled(e.target.checked);
							setDirty();
						}}
					/>
					<ContextualHelpOverlay
						title={t('BootModeMapping:gpio-input-mode-label')}
						body={
							<Trans
								ns="BootModeMapping"
								i18nKey="gpio-input-mode-explanation"
								components={[<NavLink to="/settings" />]}
							/>
						}
					/>
				</div>
				{enabled &&
					(loadingBootModes || loadingProfiles ? (
						<div className="d-flex justify-content-center">
							<span className="spinner-border" />
						</div>
					) : (
						<Container fluid className="p-0">
							<FormRow
								col0={<Form.Text className="muted ms-2">MODE</Form.Text>}
								col1={<Form.Text className="muted ms-2">GPIO PINS</Form.Text>}
								col2={<Form.Text className="muted ms-2">PROFILE</Form.Text>}
								col3={<Button className="invisible">{'✕'}</Button>}
							/>
							<hr />
							<FixedBootModeRow
								label={t('Navigation:reboot-modal-button-web-config-label')}
								mappingKey="webConfig"
							/>
							<FixedBootModeRow
								label={t('Navigation:reboot-modal-button-bootsel-label')}
								mappingKey="usbMode"
							/>
							{inputModeKeys.map((k, _) => (
								<BootModeRow mappingKey={k} key={k} />
							))}

							{inputModeKeys.length < MAX_INPUT_MODES && (
								<div className="d-flex justify-content-center">
									<Button
										className="mt-1"
										variant="outline"
										onClick={() => {
											addBootMode();
											clearErrors();
											setDirty();
										}}
									>
										+ Add Mode
									</Button>
								</div>
							)}
						</Container>
					))}
				<div className="d-flex align-items-center gap-2">
					<Button
						onClick={handleSubmit}
						disabled={!dirty || errorMessage !== undefined || showSaveMessage}
					>
						{t('Common:button-save-label')}
					</Button>
					{errorMessage && (
						<div className="invalid-feedback d-block">
							{errorMessage ? errorMessage : t('Common:saved-error-message')}
						</div>
					)}
				</div>
				{showSaveMessage && (
					<Alert className="mt-2" variant="info">
						{t('Common:saved-success-message')}
					</Alert>
				)}
			</Section>
		</div>
	);
}
