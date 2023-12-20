import React, {
	useCallback,
	useContext,
	useEffect,
	useMemo,
	useState,
} from 'react';
import Select from 'react-select';
import { NavLink } from 'react-router-dom';
import { Alert, Button, Form, Tab, Tabs } from 'react-bootstrap';
import { Trans, useTranslation } from 'react-i18next';
import invert from 'lodash/invert';
import omit from 'lodash/omit';
import zip from 'lodash/zip';

import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import usePinStore from '../Store/usePinStore';

import CaptureButton from '../Components/CaptureButton';
import { getButtonLabels } from '../Data/Buttons';
import {
	BUTTON_ACTIONS,
	NON_SELECTABLE_BUTTON_ACTIONS,
	PinActionValues,
} from '../Data/Pins';
import useProfilesStore from '../Store/useProfilesStore';

type PinCell = [string, PinActionValues];
type PinRow = [PinCell, PinCell];
type PinList = [PinRow];

const isNonSelectable = (value) =>
	NON_SELECTABLE_BUTTON_ACTIONS.includes(value);

const options = Object.entries(BUTTON_ACTIONS)
	.filter(([_, value]) => !isNonSelectable(value))
	.map(([key, value]) => ({
		label: key,
		value,
	}));

const getOption = (actionId) => ({
	label: invert(BUTTON_ACTIONS)[actionId],
	value: actionId,
});

type PinsFormTypes = {
	savePins: () => void;
	pins: { [key: string]: PinActionValues };
	setPinAction: (pin: string, action: PinActionValues) => void;
};

const PinsForm = ({ savePins, pins, setPinAction }: PinsFormTypes) => {
	const { buttonLabels, updateUsedPins } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');

	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const CURRENT_BUTTONS = getButtonLabels(buttonLabelType, swapTpShareLabels);
	const buttonNames = omit(CURRENT_BUTTONS, ['label', 'value']);
	const { t } = useTranslation('');

	const handleSubmit = async (e) => {
		e.preventDefault();
		e.stopPropagation();
		try {
			await savePins();
			updateUsedPins();
			setSaveMessage(t('Common:saved-success-message'));
		} catch (error) {
			setSaveMessage(t('Common:saved-error-message'));
		}
	};

	const pinList = useMemo<PinList>(() => {
		const pinArray = Object.entries(pins);
		return zip(
			pinArray.slice(0, pinArray.length / 2),
			pinArray.slice(pinArray.length / 2, pinArray.length),
		);
	}, [pins]);

	const createCell = useCallback(
		([pin, pinAction]: PinCell) => (
			<div className="d-flex col py-2">
				<div className="d-flex align-items-center" style={{ width: '4rem' }}>
					<label htmlFor={pin}>{pin.toUpperCase()}</label>
				</div>
				<Select
					inputId={pin}
					className="text-primary flex-grow-1"
					isClearable
					isSearchable
					options={options}
					value={getOption(pinAction)}
					isDisabled={isNonSelectable(pinAction)}
					getOptionLabel={(option) => {
						const labelKey = option.label.split('BUTTON_PRESS_').pop();
						// Need to fallback as some button actions are not part of button names
						return (
							buttonNames[labelKey] || t(`PinMapping:actions.${option.label}`)
						);
					}}
					onChange={(change) =>
						setPinAction(
							pin,
							change?.value === undefined ? -10 : change.value, // On clear set to -10
						)
					}
				/>
			</div>
		),
		[],
	);

	return (
		<Form onSubmit={handleSubmit}>
			<div className="py-3">
				<CaptureButton
					labels={Object.values(buttonNames)}
					onChange={(label, pin) =>
						setPinAction(
							// Convert getHeldPins format to setPinMappings format
							parseInt(pin) < 10 ? `pin0${pin}` : `pin${pin}`,
							// Maps current mode buttons to actions
							BUTTON_ACTIONS[
								`BUTTON_PRESS_${invert(buttonNames)[label].toUpperCase()}`
							],
						)
					}
				/>
			</div>
			<div className="gx-3">
				{pinList.map(([cell1, cell2], i) => (
					<div key={`pin-row-${i}`} className="row">
						{createCell(cell1)}
						{createCell(cell2)}
					</div>
				))}
			</div>
			<Button type="submit" className="my-4">
				{t('Common:button-save-label')}
			</Button>
			{saveMessage && <Alert variant="secondary">{saveMessage}</Alert>}
		</Form>
	);
};

export default function PinMappingPage() {
	const { fetchPins, pins, savePins, setPinAction } = usePinStore();
	const { fetchProfiles, profiles, saveProfiles, setProfileAction } =
		useProfilesStore();

	const { t } = useTranslation('');

	useEffect(() => {
		fetchPins();
		fetchProfiles();
	}, []);

	const saveAll = useCallback(() => {
		savePins();
		saveProfiles();
	}, [savePins, saveProfiles]);

	return (
		<Section title={t('PinMapping:header-text')}>
			<p>{t('PinMapping:sub-header-text')}</p>
			<div className="alert alert-warning">
				<Trans ns="PinMapping" i18nKey="alert-text">
					Mapping buttons to pins that aren&apos;t connected or available can
					leave the device in non-functional state. To clear the invalid
					configuration go to the{' '}
					<NavLink to="/reset-settings">Reset Settings</NavLink> page.
				</Trans>
				<br />
				<br />
				{t(`PinMapping:profile-pins-warning`)}
			</div>
			<Tabs id="profiles">
				<Tab eventKey="Base" title="Base(Profile 1)">
					<PinsForm
						pins={pins}
						savePins={saveAll}
						setPinAction={setPinAction}
					/>
				</Tab>
				{profiles.map((profilePins, profileIndex) => (
					<Tab
						key={`Profile${profileIndex + 2}`}
						eventKey={`Profile${profileIndex + 2}`}
						title={`Profile ${profileIndex + 2}`}
					>
						<PinsForm
							pins={profilePins}
							savePins={saveAll}
							setPinAction={(pin, action) => {
								setProfileAction(profileIndex, pin, action);
							}}
						/>
					</Tab>
				))}
			</Tabs>
		</Section>
	);
}
