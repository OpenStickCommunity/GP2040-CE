import { Button, Col, Container, Form, Row } from 'react-bootstrap';
import { ReactNode, useContext, useEffect } from 'react';
import Section from './Section';
import { useBootModesStore, NUM_PINS } from '../Store/useBootModesStore';
import { INPUT_MODE_OPTIONS, InputModeOptions } from '../Data/InputBootModes';
import { AppContext } from '../Contexts/AppContext';
import CustomSelect from './CustomSelect';
import { useTranslation } from 'react-i18next';
import { ActionMeta, MultiValue, SingleValue } from 'react-select';
import CaptureButton from './CaptureButton';
import useProfilesStore, { PinsType } from '../Store/useProfilesStore';

const groupedOptions = [
	{
		label: 'Primary',
		options: INPUT_MODE_OPTIONS.filter(({ group }) => group === 'primary'),
	},
	{
		label: 'Mini',
		options: INPUT_MODE_OPTIONS.filter(({ group }) => group === 'mini'),
	},
];

function BootModeSelect({ mappingIndex }: { mappingIndex: number }) {
	const setInputMode = useBootModesStore((state) => state.setInputMode);
	const { t } = useTranslation('');

	const getOptionLabel = (option: InputModeOptions) =>
		t(`SettingsPage:${option.labelKey}`);

	const onChange = (option: SingleValue<InputModeOptions>) => {
		setInputMode(mappingIndex, option?.value);
	};

	return (
		<CustomSelect
			isClearable={false}
			isMulti={false}
			options={groupedOptions}
			isDisabled={false}
			getOptionLabel={getOptionLabel}
			onChange={onChange}
		/>
	);
}

type PinOption = {
	label: string;
	value: number;
};

function PinSelect({
	onSelect,
	onRemove,
}: {
	onSelect: (pin: number) => void;
	onRemove: (pin: number) => void;
}) {
	const onChange = (_: MultiValue<PinOption>, action: ActionMeta<PinOption>) => {
		if (!action.option?.value) {
			return;
		}
		if (action.action === 'select-option') {
			onSelect(action.option.value);
		} else if (action.action === 'remove-value') {
			onRemove(action.option.value);
		}
	};
	const pins = Array.from({ length: NUM_PINS }, (_, i) => ({
		label: `GP${i}`,
		value: i,
	}));

	return (
		<div className="d-flex gap-2">
			<CustomSelect
				isClearable={false}
				isMulti={true}
				options={pins}
				isDisabled={false}
				onChange={onChange}
			/>
			<CaptureButton labels={['']} onChange={(_, pin) => onSelect(pin)} small={true} />
		</div>
	);
}

type ProfileOption = {
	label: string;
	value: number;
	disabled: boolean;
};

function ProfileSelect() {
	const profiles: ProfileOption[] = useProfilesStore((state) =>
		state.profiles.map((p, i) => ({
			label: p.profileLabel,
			value: i,
			disabled: !p.enabled,
		})),
	);

	const getLabel = (option: ProfileOption) =>
		option.label + (option.disabled ? ' (Disabled)' : '');

	const onChange = (_selected: any, action: ActionMeta<ProfileOption>) => {
		console.log(action);
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
		/>
	);
}

function BootModeRow({
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

export default function BootModeMappingPage() {
	const loadingBootModes = useBootModesStore((state) => state.loadingBootModes);
	const loadingProfiles = useProfilesStore((state) => state.loadingProfiles);

	const addBootMode = useBootModesStore((state) => state.addBootMode);
	const removeBootMode = useBootModesStore((state) => state.removeBootMode);
	const bootModes = useBootModesStore((state) => state.bootModes);

	const addWebConfigPin = useBootModesStore((state) => state.addWebConfigPin);
	const removeWebConfigPin = useBootModesStore((state) => state.removeWebConfigPin);
	const addUsbModePin = useBootModesStore((state) => state.addUsbModePin);
	const removeUsbModePin = useBootModesStore((state) => state.removeUsbModePin);
	const addPin = useBootModesStore((state) => state.addPin);
	const clearPin = useBootModesStore((state) => state.removePin);

	return (
		<div id="BootModeSelect">
			{loadingBootModes || loadingProfiles ? (
				<div className="d-flex justify-content-center">
					<span className="spinner-border" />
				</div>
			) : (
				<Container fluid className="p-0">
					<BootModeRow
						col0={<Form.Text className="muted ms-2">MODE</Form.Text>}
						col1={<Form.Text className="muted ms-2">GPIO PINS</Form.Text>}
						col2={<Form.Text className="muted ms-2">PROFILE</Form.Text>}
						col3={<Button className="invisible">{'✕'}</Button>}
					/>
					<hr />
					<BootModeRow
						col0={<label className="ms-2">Web-Config</label>}
						col1={<PinSelect onSelect={addWebConfigPin} onRemove={removeWebConfigPin} />}
						col2={<CustomSelect isDisabled={true} placeholder="N/A" />}
						col3={<Button disabled={true}>{'✕'}</Button>}
					/>
					<BootModeRow
						col0={<label className="ms-2">USB (BOOTSEL)</label>}
						col1={<PinSelect onSelect={addUsbModePin} onRemove={removeUsbModePin} />}
						col2={<CustomSelect isDisabled={true} placeholder="N/A" />}
						col3={<Button disabled={true}>{'✕'}</Button>}
					/>
					{bootModes.map((_, index) => (
						<div key={`boot-mode-${index}`}>
							<BootModeRow
								col0={<BootModeSelect mappingIndex={index} />}
								col1={
									<PinSelect
										onSelect={(pin: number) => addPin(index, pin)}
										onRemove={(pin: number) => clearPin(index, pin)}
									/>
								}
								col2={<ProfileSelect />}
								col3={
									<Button
										onClick={() => {
											removeBootMode(index);
										}}
									>
										{'✕'}
									</Button>
								}
							/>
						</div>
					))}
					{bootModes.length < 8 && (
						<div className="d-flex flex-row gap-2 justify-content-end">
							<div className="d-flex flex-grow-1 justify-content-center">
								<Button className="mt-1" variant="outline" onClick={addBootMode}>
									+ Add Mode
								</Button>
							</div>
						</div>
					)}
				</Container>
			)}
		</div>
	);
}
