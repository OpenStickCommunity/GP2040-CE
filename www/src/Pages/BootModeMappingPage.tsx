import { Alert, Button, Col, Container, Form, NavItem, Row } from 'react-bootstrap';
import { NavLink } from 'react-router-dom';
import { memo, ReactNode, useContext, useEffect } from 'react';
import Section from '../Components/Section';
import {
	useBootModeStore,
	useBootModeStoreActions,
	NUM_PINS,
} from '../Store/useBootModesStore';
import { INPUT_MODE_OPTIONS, InputModeOptions } from '../Data/InputBootModes';
import { AppContext } from '../Contexts/AppContext';
import CustomSelect from '../Components/CustomSelect';
import { useTranslation } from 'react-i18next';
import { ActionMeta, MultiValue, SingleValue } from 'react-select';
import CaptureButton from '../Components/CaptureButton';
import useProfilesStore from '../Store/useProfilesStore';

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
	const { setInputMode, clearErrors } = useBootModeStoreActions();

	const { getAvailablePeripherals } = useContext(AppContext);
	const { t } = useTranslation('');

	const value = INPUT_MODE_OPTIONS.find(({ value }) => value === inputMode);
	const usb_available: boolean = getAvailablePeripherals('usb');

	const isOptionDisabled = (option: InputModeOptions) => {
		return option.required.includes('usb') && !usb_available;
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

	const { addPin, removePin, validatePins, clearErrors } = useBootModeStoreActions();

	const values = PIN_OPTIONS.filter(({ value }) => pins.has(value));

	const onChange = (_: MultiValue<PinOption>, action: ActionMeta<PinOption>) => {
		if (action.action === 'select-option' && action.option !== undefined) {
			addPin(mappingKey, action.option.value);
		} else if (action.action === 'remove-value') {
			removePin(mappingKey, action.removedValue.value);
		}
		clearErrors();
		validatePins();
	};
	const isInvalid =
		modesWithDuplicates.includes(mappingKey) || (saveAttempted && values.length == 0);

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
			/>
			<CaptureButton
				labels={['']}
				onChange={(_, pin) => {
					addPin(mappingKey, pin);
					clearErrors();
					validatePins();
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
	const { setProfileIndex } = useBootModeStoreActions();
	const value = profileOptions.find(({ value }) => value === profileIndex);
	console.log(`${mappingKey} value:`, value);

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
	const { removeBootMode, clearErrors } = useBootModeStoreActions();

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
	const errorMessage = useBootModeStore((state) => state.errorMessage);

	const {
		addBootMode,
		fetchBootModeOptions,
		saveBootModeOptions,
		clearErrors,
		setEnabled,
	} = useBootModeStoreActions();

	const loadingProfiles = useProfilesStore((state) => state.loadingProfiles);
	const fetchProfiles = useProfilesStore((state) => state.fetchProfiles);
	const { t } = useTranslation('');

	useEffect(() => {
		fetchBootModeOptions();
		fetchProfiles();
	}, []);

	// The delete-able input mode keys (i.e. not web-config or usb mode)
	const inputModeKeys = Object.keys(bootModes).filter((k) =>
		k.startsWith('inputMode-'),
	);

	return (
		<Section title={t('SettingsPage:boot-input-mode-label')}>
			<Form.Check
				label="Use GPIO Pins"
				type="switch"
				className="text my-auto mb-4"
				checked={enabled}
				onChange={(e) => {
					setEnabled(e.target.checked);
				}}
			/>
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
									}}
								>
									+ Add Mode
								</Button>
							</div>
						)}
						<div className="d-flex align-items-center gap-2">
							<Button
								onClick={saveBootModeOptions}
								disabled={errorMessage !== undefined}
							>
								{t('Common:button-save-label')}
							</Button>
							{errorMessage && (
								<div className="invalid-feedback d-block">
									{errorMessage
										? errorMessage
										: t('Common:saved-error-message')}
								</div>
							)}
						</div>
						{saveAttempted && errorMessage === undefined && (
							<Alert className="mt-2" variant="info">
								{t('Common:saved-success-message')}
							</Alert>
						)}
					</Container>
				))}
		</Section>
	);
}
