import React, { useCallback, useContext, useEffect, useState } from 'react';
import { NavLink } from 'react-router-dom';
import { Alert, Button, Col, Form, Nav, Row, Tab } from 'react-bootstrap';
import { Trans, useTranslation } from 'react-i18next';
import invert from 'lodash/invert';
import omit from 'lodash/omit';

import { AppContext } from '../Contexts/AppContext';
import usePinStore from '../Store/usePinStore';
import useProfilesStore from '../Store/useProfilesStore';

import Section from '../Components/Section';
import CustomSelect from '../Components/CustomSelect';
import CaptureButton from '../Components/CaptureButton';

import { BUTTON_MASKS, DPAD_MASKS, getButtonLabels } from '../Data/Buttons';
import { BUTTON_ACTIONS, PinActionValues } from '../Data/Pins';
import './PinMapping.scss';
import { MultiValue, SingleValue } from 'react-select';

type OptionType = {
	label: string;
	value: PinActionValues;
	type: string;
	customButtonMask: number;
	customDpadMask: number;
};

type ProfilePinSectionType = {
	title: string;
	profilePins: { [key: string]: PinActionValues };
	profileIndex: number;
};

const disabledOptions = [
	BUTTON_ACTIONS.RESERVED,
	BUTTON_ACTIONS.ASSIGNED_TO_ADDON,
	BUTTON_ACTIONS.BUTTON_PRESS_TURBO,
	BUTTON_ACTIONS.BUTTON_PRESS_MACRO,
];

const getMask = (maskArr, key) =>
	maskArr.find(
		({ label }) => label?.toUpperCase() === key.split('BUTTON_PRESS_')?.pop(),
	);

const isNonSelectable = (action) =>
	[
		BUTTON_ACTIONS.NONE,
		BUTTON_ACTIONS.CUSTOM_BUTTON_COMBO,
		...disabledOptions,
	].includes(action);

const isDisabled = (action) => disabledOptions.includes(action);

const options = Object.entries(BUTTON_ACTIONS)
	.filter(([, value]) => !isNonSelectable(value))
	.map(([key, value]) => {
		const buttonMask = getMask(BUTTON_MASKS, key);
		const dpadMask = getMask(DPAD_MASKS, key);

		return {
			label: key,
			value,
			type: buttonMask
				? 'customButtonMask'
				: dpadMask
				? 'customDpadMask'
				: 'action',
			customButtonMask: buttonMask?.value || 0,
			customDpadMask: dpadMask?.value || 0,
		};
	});

const groupedOptions = [
	{
		label: 'Buttons',
		options: options.filter(({ type }) => type !== 'action'),
	},
	{
		label: 'Actions',
		options: options.filter(({ type }) => type === 'action'),
	},
];

const getMultiValue = (pinData) => {
	if (pinData.action === BUTTON_ACTIONS.NONE) return;
	if (isDisabled(pinData.action)) {
		const actionKey = invert(BUTTON_ACTIONS)[pinData.action];
		return [{ label: actionKey, ...pinData }];
	}

	return pinData.action === BUTTON_ACTIONS.CUSTOM_BUTTON_COMBO
		? options.filter(
				({ type, customButtonMask, customDpadMask }) =>
					(pinData.customButtonMask & customButtonMask &&
						type === 'customButtonMask') ||
					(pinData.customDpadMask & customDpadMask &&
						type === 'customDpadMask'),
		  )
		: options.filter((option) => option.value === pinData.action);
};

const getSingleValue = (pinData) => {
	const actionKey = invert(BUTTON_ACTIONS)[pinData];
	return { label: actionKey, value: pinData };
};

const PinMappingWarning = () => {
	const { t } = useTranslation('');
	return (
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
	);
};
const BasePinSection = () => {
	const { pins, savePins, setPin } = usePinStore();
	const { buttonLabels, updateUsedPins } = useContext(AppContext);
	const { t } = useTranslation('');
	const [saveMessage, setSaveMessage] = useState('');

	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const CURRENT_BUTTONS = getButtonLabels(buttonLabelType, swapTpShareLabels);
	const buttonNames = omit(CURRENT_BUTTONS, ['label', 'value']);

	const onChange = useCallback(
		(pin: string) =>
			(selected: MultiValue<OptionType> | SingleValue<OptionType>) => {
				// Handle clearing
				if (!selected || (Array.isArray(selected) && !selected.length)) {
					setPin(pin, {
						action: BUTTON_ACTIONS.NONE,
						customButtonMask: 0,
						customDpadMask: 0,
					});
				} else if (Array.isArray(selected) && selected.length > 1) {
					const lastSelected = selected[selected.length - 1];
					// Revert to single option if choosing action type
					if (lastSelected.type === 'action') {
						setPin(pin, {
							action: lastSelected.value,
							customButtonMask: 0,
							customDpadMask: 0,
						});
					} else {
						setPin(
							pin,
							selected.reduce(
								(masks, option) => ({
									...masks,
									customButtonMask:
										option.type === 'customButtonMask'
											? masks.customButtonMask ^ option.customButtonMask
											: masks.customButtonMask,
									customDpadMask:
										option.type === 'customDpadMask'
											? masks.customDpadMask ^ option.customDpadMask
											: masks.customDpadMask,
								}),
								{
									action: BUTTON_ACTIONS.CUSTOM_BUTTON_COMBO,
									customButtonMask: 0,
									customDpadMask: 0,
								},
							),
						);
					}
				} else {
					setPin(pin, {
						action: selected[0].value,
						customButtonMask: 0,
						customDpadMask: 0,
					});
				}
			},
		[],
	);

	const getOptionLabel = useCallback(
		(option: OptionType) => {
			const labelKey = option.label?.split('BUTTON_PRESS_')?.pop();
			// Need to fallback as some button actions are not part of button names
			return (
				(labelKey && buttonNames[labelKey]) ||
				t(`PinMapping:actions.${option.label}`)
			);
		},
		[buttonNames],
	);

	const handleSubmit = useCallback(async (e) => {
		e.preventDefault();
		e.stopPropagation();
		try {
			await savePins();
			updateUsedPins();
			setSaveMessage(t('Common:saved-success-message'));
		} catch (error) {
			setSaveMessage(t('Common:saved-error-message'));
		}
	}, []);

	return (
		<>
			<PinMappingWarning />
			<Section title={`Base(Profile 1) - Pin Mapping`}>
				<Form onSubmit={handleSubmit}>
					<div className="pin-grid gap-3">
						{Object.entries(pins).map(([pin, pinData], index) => (
							<div
								key={`select-${index}`}
								className="d-flex col align-items-center"
							>
								<div className="d-flex flex-shrink-0" style={{ width: '4rem' }}>
									<label>{pin.toUpperCase()}</label>
								</div>
								<CustomSelect
									isClearable
									isMulti={!isDisabled(pinData.action)}
									options={groupedOptions}
									isDisabled={isDisabled(pinData.action)}
									getOptionLabel={getOptionLabel}
									onChange={onChange(pin)}
									value={getMultiValue(pinData)}
								/>
							</div>
						))}
					</div>
					<CaptureButton
						labels={Object.values(buttonNames)}
						onChange={(label, pin) =>
							setPin(
								// Convert getHeldPins format to setPinMappings format
								pin < 10 ? `pin0${pin}` : `pin${pin}`,
								{
									// Maps current mode buttons to actions
									action:
										BUTTON_ACTIONS[
											`BUTTON_PRESS_${invert(buttonNames)[label].toUpperCase()}`
										],
									customButtonMask: 0,
									customDpadMask: 0,
								},
							)
						}
					/>
					<Button type="submit" className="m-4">
						{t('Common:button-save-label')}
					</Button>
					{saveMessage && <Alert variant="info">{saveMessage}</Alert>}
				</Form>
			</Section>
		</>
	);
};

const ProfilePinSection = ({
	title,
	profilePins,
	profileIndex,
}: ProfilePinSectionType) => {
	const { saveProfiles, setProfileAction } = useProfilesStore();
	const { buttonLabels, updateUsedPins } = useContext(AppContext);
	const { t } = useTranslation('');
	const [saveMessage, setSaveMessage] = useState('');

	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const CURRENT_BUTTONS = getButtonLabels(buttonLabelType, swapTpShareLabels);
	const buttonNames = omit(CURRENT_BUTTONS, ['label', 'value']);

	const onChange = useCallback(
		(pin: string, profileIndex: number) =>
			(selected: SingleValue<OptionType>) => {
				setProfileAction(
					profileIndex,
					pin,
					selected?.value === undefined ? BUTTON_ACTIONS.NONE : selected.value,
				);
			},
		[],
	);

	const getOptionLabel = useCallback(
		(option: OptionType) => {
			const labelKey = option.label?.split('BUTTON_PRESS_')?.pop();
			// Need to fallback as some button actions are not part of button names
			return (
				(labelKey && buttonNames[labelKey]) ||
				t(`PinMapping:actions.${option.label}`)
			);
		},
		[buttonNames],
	);

	const handleSubmit = useCallback(async (e) => {
		e.preventDefault();
		e.stopPropagation();
		try {
			await saveProfiles();
			updateUsedPins();
			setSaveMessage(t('Common:saved-success-message'));
		} catch (error) {
			setSaveMessage(t('Common:saved-error-message'));
		}
	}, []);

	return (
		<>
			<PinMappingWarning />
			<Section title={title}>
				<Form onSubmit={handleSubmit}>
					<div className="pin-grid gap-3">
						{Object.entries(profilePins).map(([pin, pinData], index) => (
							<div
								key={`select-${index}`}
								className="d-flex col align-items-center"
							>
								<div className="d-flex flex-shrink-0" style={{ width: '4rem' }}>
									<label>{pin.toUpperCase()}</label>
								</div>
								<CustomSelect
									isClearable
									options={groupedOptions}
									isDisabled={isDisabled(pinData)}
									getOptionLabel={getOptionLabel}
									onChange={onChange(pin, profileIndex)}
									value={getSingleValue(pinData)}
								/>
							</div>
						))}
					</div>
					<CaptureButton
						labels={Object.values(buttonNames)}
						onChange={(label, pin) =>
							setProfileAction(
								profileIndex,
								// Convert getHeldPins format to setPinMappings format
								pin < 10 ? `pin0${pin}` : `pin${pin}`,
								// Maps current mode buttons to actions
								BUTTON_ACTIONS[
									`BUTTON_PRESS_${invert(buttonNames)[label].toUpperCase()}`
								],
							)
						}
					/>
					<Button type="submit" className="m-4">
						{t('Common:button-save-label')}
					</Button>
					{saveMessage && <Alert variant="info">{saveMessage}</Alert>}
				</Form>
			</Section>
		</>
	);
};

export default function PinMapping() {
	const { fetchPins } = usePinStore();
	const { fetchProfiles, profiles } = useProfilesStore();
	const [pressedPin, setPressedPin] = useState<number | null>(null);
	const { t } = useTranslation('');

	useEffect(() => {
		fetchPins();
		fetchProfiles();
	}, []);

	return (
		<Tab.Container defaultActiveKey="profile-1">
			<Row>
				<Col sm={2}>
					<Nav variant="pills" className="flex-column">
						<Nav.Item>
							<Nav.Link eventKey="profile-1">Base(Profile 1)</Nav.Link>
						</Nav.Item>
						<Nav.Item>
							<Nav.Link eventKey="profile-2">Profile 2</Nav.Link>
						</Nav.Item>
						<Nav.Item>
							<Nav.Link eventKey="profile-3">Profile 3</Nav.Link>
						</Nav.Item>
						<Nav.Item>
							<Nav.Link eventKey="profile-4">Profile 4</Nav.Link>
						</Nav.Item>
					</Nav>
					<hr />
					<p className="text-center">{t('PinMapping:sub-header-text')}</p>
					<div className="d-flex justify-content-center">
						<CaptureButton
							buttonLabel={t('PinMapping:pin-viewer')}
							labels={['']}
							onChange={(_, pin) => setPressedPin(pin)}
						/>
					</div>
					{pressedPin !== null && (
						<div className="alert alert-info mt-3">
							<strong>{t('PinMapping:pin-pressed', { pressedPin })}</strong>
						</div>
					)}
				</Col>
				<Col sm={10}>
					<Tab.Content>
						<Tab.Pane eventKey="profile-1">
							<BasePinSection />
						</Tab.Pane>

						{profiles.map((profilePins, profileIndex) => (
							<Tab.Pane
								key={`profile-${profileIndex + 2}`}
								eventKey={`profile-${profileIndex + 2}`}
							>
								<ProfilePinSection
									title={`Profile ${profileIndex + 2} - Pin Mapping`}
									profilePins={profilePins}
									profileIndex={profileIndex}
								/>
							</Tab.Pane>
						))}
					</Tab.Content>
				</Col>
			</Row>
		</Tab.Container>
	);
}
