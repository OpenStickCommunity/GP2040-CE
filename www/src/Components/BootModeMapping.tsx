import { Button, Col, Container, Form, Row } from 'react-bootstrap';
import { memo, ReactNode, useContext, useEffect } from 'react';
import Section from './Section';
import { useBootModesStore, NUM_PINS, PinsKey } from '../Store/useBootModesStore';
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

function BootModeSelect({ mappingIndex }: { mappingIndex: number }) {
	const setInputMode = useBootModesStore((state) => state.setInputMode);
	const inputMode = useBootModesStore((state) => state.bootModes[mappingIndex].inputMode);
	const { getAvailablePeripherals } = useContext(AppContext);
	const { t } = useTranslation('');

	const usb_available: boolean = getAvailablePeripherals('usb');
	const value = INPUT_MODE_OPTIONS.filter(({ value }) => value === inputMode);

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
		setInputMode(mappingIndex, option?.value);
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

function PinSelect({ pinsKey, pins }: { pinsKey: PinsKey; pins: Set<number> }) {
	const addPin = useBootModesStore((state) => state.addPin);
	const removePin = useBootModesStore((state) => state.removePin);
	const values = PIN_OPTIONS.filter(({ value }) => pins.has(value));

	const onChange = (_: MultiValue<PinOption>, action: ActionMeta<PinOption>) => {
		if (action.action === 'select-option') {
			if (action.option === undefined) {
				return;
			}
			addPin(pinsKey, action.option.value);
		} else if (action.action === 'remove-value') {
			removePin(pinsKey, action.removedValue.value);
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
				onChange={(_, pin) => addPin(pinsKey, pin)}
				small={true}
			/>
		</div>
	);
}

function RequiredPinSelect({ pinsKey }: { pinsKey: 'usbModePins' | 'webConfigPins' }) {
	const pins = useBootModesStore((state) => state[pinsKey]);
	return <PinSelect pinsKey={pinsKey} pins={pins} />;
}

function OptionalPinSelect({ mappingIndex }: { mappingIndex: number }) {
	const pins = useBootModesStore((state) => state.bootModes[mappingIndex].pins);
	return <PinSelect pinsKey={mappingIndex} pins={pins} />;
}

function ProfileSelect({ mappingIndex }: { mappingIndex: number }) {
	const profiles: ProfileOption[] = useProfilesStore((state) =>
		state.profiles.map((p, i) => ({
			label: p.profileLabel,
			value: i,
			disabled: !p.enabled,
		})),
	);
	const profileIndex = useBootModesStore(
		(state) => state.bootModes[mappingIndex].profileIndex,
	);
	const setProfileIndex = useBootModesStore((state) => state.setProfileIndex);
	const value = profiles.find(({ value }) => value === profileIndex);

	const getLabel = (option: ProfileOption) =>
		option.label + (option.disabled ? ' (Disabled)' : '');

	const onChange = (_selected: any, action: ActionMeta<ProfileOption>) => {
		if (action.action == 'clear') {
			setProfileIndex(mappingIndex, undefined);
		} else if (action.action == 'select-option') {
			setProfileIndex(mappingIndex, action.option?.value);
		}
	};

	return (
		<CustomSelect
			isClearable={true}
			isMulti={false}
			options={profiles}
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

const BootModeRow = memo(function BootModeRow({
	mappingIndex,
}: {
	mappingIndex: number;
}) {
	const removeBootMode = useBootModesStore((state) => state.removeBootMode);
	return (
		<FormRow
			col0={<BootModeSelect mappingIndex={mappingIndex} />}
			col1={<OptionalPinSelect mappingIndex={mappingIndex} />}
			col2={<ProfileSelect mappingIndex={mappingIndex} />}
			col3={
				<Button
					onClick={() => {
						removeBootMode(mappingIndex);
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
	pinsKey,
}: {
	label: string;
	pinsKey: 'usbModePins' | 'webConfigPins';
}) {
	return (
		<FormRow
			col0={<label className="ms-2">{label}</label>}
			col1={<RequiredPinSelect pinsKey={pinsKey} />}
			col2={<CustomSelect isDisabled={true} placeholder="N/A" />}
			col3={<Button disabled={true}>{'✕'}</Button>}
		/>
	);
});

export default function BootModeMapping() {
	const loadingBootModes = useBootModesStore((state) => state.loadingBootModes);
	const loadingProfiles = useProfilesStore((state) => state.loadingProfiles);

	const addBootMode = useBootModesStore((state) => state.addBootMode);
	const bootModes = useBootModesStore((state) => state.bootModes);

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
					<FixedBootModeRow label="Web-Config" pinsKey="webConfigPins" />
					<FixedBootModeRow label="USB (BOOTSEL)" pinsKey="usbModePins" />
					{bootModes.map((mode, index) => (
						<BootModeRow mappingIndex={index} key={mode.key} />
					))}

					{bootModes.length < 8 && (
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
