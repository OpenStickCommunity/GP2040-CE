import { Button, Col, Container, Form, Row } from 'react-bootstrap';
import { ReactNode, useContext, useEffect } from 'react';
import Section from './Section';
import { useBootModesStore, NUM_PINS } from '../Store/useBootModesStore';
import { INPUT_MODE_OPTIONS, InputModeOptions } from '../Data/InputBootModes';
import { AppContext } from '../Contexts/AppContext';
import CustomSelect from './CustomSelect';
import { useTranslation } from 'react-i18next';
import { ActionMeta, MultiValue, SingleValue } from 'react-select';

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

	const getOptionLabel = (option: InputModeOptions) => t(`SettingsPage:${option.labelKey}`);

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
	const onChange = (_selected: any, action: ActionMeta<PinOption>) => {
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
		<CustomSelect
			isClearable={true}
			isMulti={true}
			options={pins}
			isDisabled={false}
			onChange={onChange}
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
		<Row className="gx-2 mb-3">
			<Col sm={3}>{col0}</Col>
			<Col>{col1}</Col>
			<Col sm={3}>{col2}</Col>
			<Col sm={1}>{col3}</Col>
		</Row>
	);
}

export default function BootModeMappingPage() {
	const fetchBootModes = useBootModesStore((state) => state.fetchBootModes);
	const addBootMode = useBootModesStore((state) => state.addBootMode);
	const removeBootMode = useBootModesStore((state) => state.removeBootMode);
	const bootModes = useBootModesStore((state) => state.bootModes);

	const addWebConfigPin = useBootModesStore((state) => state.addWebConfigPin);
	const removeWebConfigPin = useBootModesStore((state) => state.removeWebConfigPin);
	const addUsbModePin = useBootModesStore((state) => state.addUsbModePin);
	const removeUsbModePin = useBootModesStore((state) => state.removeUsbModePin);
	const setPin = useBootModesStore((state) => state.setPin);
	const clearPin = useBootModesStore((state) => state.clearPin);

	useEffect(() => {
		fetchBootModes();
	}, []);

	return (
		<div id="BootModeSelect">
			<Form>
				<Container>
					<BootModeRow
						col0={<Form.Text className="muted ms-2">Mode</Form.Text>}
						col1={<Form.Text className="muted ms-2">GPIO Pins</Form.Text>}
						col2={<Form.Text className="muted ms-2">Profile</Form.Text>}
						col3={undefined}
					/>
					<hr />
					<BootModeRow
						col0={<label className="ms-2">Web-Config</label>}
						col1={<PinSelect onSelect={addWebConfigPin} onRemove={removeWebConfigPin} />}
						col2={undefined}
						col3={undefined}
					/>
					<BootModeRow
						col0={<label className="ms-2">USB (BOOTSEL)</label>}
						col1={<PinSelect onSelect={addUsbModePin} onRemove={removeUsbModePin} />}
						col2={undefined}
						col3={undefined}
					/>
					<hr />
					{bootModes.map((_, index) => (
						<div key={`boot-mode-${index}`}>
							<BootModeRow
								col0={<BootModeSelect mappingIndex={index} />}
								col1={
									<PinSelect
										onSelect={(pin: number) => setPin(index, pin)}
										onRemove={(pin: number) => clearPin(index, pin)}
									/>
								}
								col2={
									<PinSelect
										onSelect={(pin: number) => setPin(index, pin)}
										onRemove={(pin: number) => clearPin(index, pin)}
									/>
								}
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
						<BootModeRow
							col0={undefined}
							col1={
								<div className="text-center">
									<Button type="button" className="mt-1" variant="outline" onClick={addBootMode}>
										+ Add Mode
									</Button>
								</div>
							}
							col2={undefined}
							col3={undefined}
						/>
					)}
					<Button>Save</Button>
				</Container>
			</Form>
		</div>
	);
}
