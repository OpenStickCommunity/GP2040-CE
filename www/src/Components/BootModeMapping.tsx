import { Button, Col, Container, Form, Row } from 'react-bootstrap';
import { memo, ReactNode, useContext, useEffect } from 'react';
import Section from './Section';
import { useBootModesStore, NUM_PINS } from '../Store/useBootModesStore';
import { INPUT_MODE_OPTIONS, InputModeOptions } from '../Data/InputBootModes';
import { AppContext } from '../Contexts/AppContext';
import CustomSelect from './CustomSelect';
import { useTranslation } from 'react-i18next';
import { ActionMeta, MultiValue, SingleValue } from 'react-select';
import CaptureButton from './CaptureButton';
import useProfilesStore, { PinsType } from '../Store/useProfilesStore';

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
	const setInputMode = useBootModesStore((state) => state.setInputMode);
	const inputMode = useBootModesStore((state) => state.bootModes[mappingKey].inputMode);
	const { getAvailablePeripherals } = useContext(AppContext);
	const { t } = useTranslation('');

	const value = INPUT_MODE_OPTIONS.filter(({ value }) => value === inputMode);
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
		setInputMode(mappingKey, option?.value);
	};

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
		/>
	);
}

function PinSelect({ mappingKey }: { mappingKey: string }) {
	const pins = useBootModesStore((state) => state.bootModes[mappingKey].pins);
	const addPin = useBootModesStore((state) => state.addPin);
	const removePin = useBootModesStore((state) => state.removePin);
	const values = PIN_OPTIONS.filter(({ value }) => pins.has(value));

	const onChange = (_: MultiValue<PinOption>, action: ActionMeta<PinOption>) => {
		if (action.action === 'select-option') {
			if (action.option === undefined) {
				return;
			}
			addPin(mappingKey, action.option.value);
		} else if (action.action === 'remove-value') {
			removePin(mappingKey, action.removedValue.value);
		}
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
			/>
			<CaptureButton
				labels={['']}
				onChange={(_, pin) => addPin(mappingKey, pin)}
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

	const profileIndex = useBootModesStore(
		(state) => state.bootModes[mappingKey].profileIndex,
	);
	const setProfileIndex = useBootModesStore((state) => state.setProfileIndex);
	const value = profileOptions.find(({ value }) => value === profileIndex);

	const getLabel = (option: ProfileOption) =>
		option.label + (option.disabled ? ' (Disabled)' : '');

	const onChange = (_selected: any, action: ActionMeta<ProfileOption>) => {
		if (action.action == 'clear') {
			setProfileIndex(mappingKey, undefined);
		} else if (action.action == 'select-option') {
			setProfileIndex(mappingKey, action.option?.value);
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

const BootModeRow = memo(function BootModeRow({ mappingKey }: { mappingKey: string }) {
	const removeBootMode = useBootModesStore((state) => state.removeBootMode);
	return (
		<FormRow
			col0={<BootModeSelect mappingKey={mappingKey} />}
			col1={<PinSelect mappingKey={mappingKey} />}
			col2={<ProfileSelect mappingKey={mappingKey} />}
			col3={
				<Button
					onClick={() => {
						removeBootMode(mappingKey);
					}}
				>
					{'✕'}
				</Button>
			}
		/>
	);
});

const FixedBootModeRow = memo(function FixedBootModeRow({
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
});

export default function BootModeMapping() {
	const loadingBootModes = useBootModesStore((state) => state.loadingBootModes);
	const loadingProfiles = useProfilesStore((state) => state.loadingProfiles);

	const { t } = useTranslation('');

	const addBootMode = useBootModesStore((state) => state.addBootMode);
	const bootModes = useBootModesStore((state) => state.bootModes);

	// The delete-able input mode keys (i.e. not web-config or usb mode)
	const inputModeKeys = Object.keys(bootModes).filter((k) => k.startsWith('inputMode-'));

	return (
		<div id="BootModeSelect">
			{loadingBootModes || loadingProfiles ? (
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
							<Button className="mt-1" variant="outline" onClick={addBootMode}>
								+ Add Mode
							</Button>
						</div>
					)}
				</Container>
			)}
		</div>
	);
}
