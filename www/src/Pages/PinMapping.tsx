import {
	FormEvent,
	memo,
	useCallback,
	useContext,
	useEffect,
	useState,
} from 'react';
import { NavLink } from 'react-router-dom';
import { useShallow } from 'zustand/react/shallow';
import {
	Alert,
	Button,
	Col,
	Form,
	FormCheck,
	Nav,
	OverlayTrigger,
	Row,
	Tab,
	Tooltip,
} from 'react-bootstrap';
import { Trans, useTranslation } from 'react-i18next';
import invert from 'lodash/invert';
import omit from 'lodash/omit';

import { AppContext } from '../Contexts/AppContext';
import useProfilesStore, {
	MaskPayload,
	MAX_PROFILES,
} from '../Store/useProfilesStore';

import Section from '../Components/Section';
import CustomSelect from '../Components/CustomSelect';
import CaptureButton from '../Components/CaptureButton';

import { BUTTON_MASKS, DPAD_MASKS, getButtonLabels } from '../Data/Buttons';
import { BUTTON_ACTIONS, PinActionKeys, PinActionValues } from '../Data/Pins';
import './PinMapping.scss';
import { MultiValue, SingleValue } from 'react-select';
import InfoCircle from '../Icons/InfoCircle';

type OptionType = {
	label: string;
	value: PinActionValues;
	type: string;
	customButtonMask: number;
	customDpadMask: number;
};

const disabledOptions = [
	BUTTON_ACTIONS.RESERVED,
	BUTTON_ACTIONS.ASSIGNED_TO_ADDON,
] as PinActionValues[];

const getMask = (maskArr: { label: string; value: number }[], key: string) =>
	maskArr.find(
		({ label }) => label?.toUpperCase() === key.split('BUTTON_PRESS_')?.pop(),
	);

const isNonSelectable = (action: PinActionValues) =>
	[
		BUTTON_ACTIONS.NONE,
		BUTTON_ACTIONS.CUSTOM_BUTTON_COMBO,
		...disabledOptions,
	].includes(action);

const isDisabled = (action: PinActionValues) =>
	disabledOptions.includes(action);

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

const getMultiValue = (pinData: MaskPayload) => {
	if (pinData.action === BUTTON_ACTIONS.NONE) return;
	if (isDisabled(pinData.action)) {
		const actionKey = invert(BUTTON_ACTIONS)[pinData.action];
		return [
			{
				label: actionKey,
				value: pinData.action,
				type: 'action',
				customButtonMask: pinData.customButtonMask,
				customDpadMask: pinData.customDpadMask,
			},
		];
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

const ProfileLabel = memo(function ProfileLabel({
	profileIndex,
}: {
	profileIndex: number;
}) {
	const { t } = useTranslation('');
	const setProfileLabel = useProfilesStore((state) => state.setProfileLabel);
	const profileLabel = useProfilesStore(
		(state) => state.profiles[profileIndex].profileLabel,
	);

	const onLabelChange = useCallback(
		(event: React.ChangeEvent<HTMLInputElement>) =>
			setProfileLabel(
				profileIndex,
				event.target.value.replace(/[^a-zA-Z0-9\s]/g, ''),
			),
		[],
	);

	return (
		<div className="pin-grid">
			<Form.Label>{t('PinMapping:profile-label-title')}</Form.Label>
			<Form.Control
				type="text"
				value={profileLabel}
				placeholder={t('PinMapping:profile-label-default', {
					profileNumber: profileIndex + 1,
				})}
				onChange={onLabelChange}
				maxLength={16}
				pattern="[a-zA-Z0-9\s]+"
			/>
			<Form.Text muted>{t('PinMapping:profile-label-description')}</Form.Text>
		</div>
	);
});

const PinSelectList = memo(function PinSelectList({
	profileIndex,
}: {
	profileIndex: number;
}) {
	const setProfilePin = useProfilesStore((state) => state.setProfilePin);

	const pins = useProfilesStore(
		useShallow((state) =>
			omit(state.profiles[profileIndex], ['profileLabel', 'enabled']),
		),
	);
	const { t } = useTranslation('');
	const { buttonLabels } = useContext(AppContext);
	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const CURRENT_BUTTONS = getButtonLabels(buttonLabelType, swapTpShareLabels);
	const buttonNames = omit(CURRENT_BUTTONS, ['label', 'value']);

	const onChange = useCallback(
		(pin: string) =>
			(selected: MultiValue<OptionType> | SingleValue<OptionType>) => {
				// Handle clearing
				if (!selected || (Array.isArray(selected) && !selected.length)) {
					setProfilePin(profileIndex, pin, {
						action: BUTTON_ACTIONS.NONE,
						customButtonMask: 0,
						customDpadMask: 0,
					});
				} else if (Array.isArray(selected) && selected.length > 1) {
					const lastSelected = selected[selected.length - 1];
					// Revert to single option if choosing action type
					if (lastSelected.type === 'action') {
						setProfilePin(profileIndex, pin, {
							action: lastSelected.value,
							customButtonMask: 0,
							customDpadMask: 0,
						});
					} else {
						setProfilePin(
							profileIndex,
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
					setProfilePin(profileIndex, pin, {
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
	return (
		<div className="pin-grid gap-3 mt-2">
			{Object.entries(pins).map(([pin, pinData], index) => (
				<div key={`select-${index}`} className="d-flex align-items-center">
					<div className="d-flex flex-shrink-0" style={{ width: '3.5rem' }}>
						<label>GP{index}</label>
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
	);
});

const PinSection = memo(function PinSection({
	profileIndex,
}: {
	profileIndex: number;
}) {
	const { t } = useTranslation('');
	const copyBaseProfile = useProfilesStore((state) => state.copyBaseProfile);
	const setProfilePin = useProfilesStore((state) => state.setProfilePin);
	const saveProfiles = useProfilesStore((state) => state.saveProfiles);
	const toggleProfileEnabled = useProfilesStore(
		(state) => state.toggleProfileEnabled,
	);
	const enabled = useProfilesStore(
		(state) => state.profiles[profileIndex].enabled,
	);
	const profileLabel =
		useProfilesStore((state) => state.profiles[profileIndex].profileLabel) ||
		t('PinMapping:profile-label-default', {
			profileNumber: profileIndex + 1,
		});

	const { updateUsedPins, buttonLabels } = useContext(AppContext);
	const { buttonLabelType, swapTpShareLabels } = buttonLabels;
	const CURRENT_BUTTONS = getButtonLabels(buttonLabelType, swapTpShareLabels);
	const buttonNames = omit(CURRENT_BUTTONS, ['label', 'value']);

	const [saveMessage, setSaveMessage] = useState('');

	const handleSubmit = useCallback(async (e: FormEvent<HTMLFormElement>) => {
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
			<Section
				title={t('PinMapping:profile-pin-mapping-title', {
					profileLabel,
				})}
			>
				<Form onSubmit={handleSubmit}>
					<div className="d-flex justify-content-between">
						<ProfileLabel profileIndex={profileIndex} />
						{profileIndex > 0 && (
							<div className="d-flex">
								<FormCheck
									size={3}
									label={
										<OverlayTrigger
											overlay={
												<Tooltip>
													{t('PinMapping:profile-enabled-tooltip')}
												</Tooltip>
											}
										>
											<div className="d-flex gap-1">
												<label>{t('Common:switch-enabled')} </label>
												<InfoCircle />
											</div>
										</OverlayTrigger>
									}
									type="switch"
									reverse
									checked={enabled}
									onChange={() => {
										toggleProfileEnabled(profileIndex);
									}}
								/>
							</div>
						)}
					</div>
					<hr />

					<PinSelectList profileIndex={profileIndex} />
					<div className="d-flex gap-3 my-3">
						<CaptureButton
							labels={Object.values(buttonNames)}
							onChange={(label, pin) =>
								setProfilePin(
									profileIndex,
									// Convert getHeldPins format to setPinMappings format
									pin < 10 ? `pin0${pin}` : `pin${pin}`,
									{
										// Maps current mode buttons to actions
										action:
											BUTTON_ACTIONS[
												`BUTTON_PRESS_${invert(buttonNames)[
													label
												].toUpperCase()}` as PinActionKeys
											],
										customButtonMask: 0,
										customDpadMask: 0,
									},
								)
							}
						/>
						{profileIndex > 0 && (
							<Button onClick={() => copyBaseProfile(profileIndex)}>
								{t(`PinMapping:profile-copy-base`)}
							</Button>
						)}
						<Button type="submit">{t('Common:button-save-label')}</Button>
					</div>
					{saveMessage && <Alert variant="info">{saveMessage}</Alert>}
				</Form>
			</Section>
		</>
	);
});

export default function PinMapping() {
	const fetchProfiles = useProfilesStore((state) => state.fetchProfiles);
	const addProfile = useProfilesStore((state) => state.addProfile);
	const profiles = useProfilesStore((state) => state.profiles);
	const loadingProfiles = useProfilesStore((state) => state.loadingProfiles);

	const [pressedPin, setPressedPin] = useState<number | null>(null);
	const { t } = useTranslation('');

	useEffect(() => {
		fetchProfiles();
	}, []);

	return (
		<Tab.Container defaultActiveKey="profile-0">
			<Row>
				<Col md={3}>
					{loadingProfiles && (
						<div className="d-flex justify-content-center">
							<span className="spinner-border" />
						</div>
					)}
					<Nav variant="pills" className="flex-column">
						{profiles.map(({ profileLabel, enabled }, index) => (
							<Nav.Item key={`profile-${index}`}>
								<Nav.Link eventKey={`profile-${index}`}>
									{profileLabel ||
										t('PinMapping:profile-label-default', {
											profileNumber: index + 1,
										})}

									{!enabled && index > 0 && (
										<span>{t('PinMapping:profile-disabled')}</span>
									)}
								</Nav.Link>
							</Nav.Item>
						))}
						{profiles.length !== MAX_PROFILES && (
							<Button
								type="button"
								className="mt-1"
								variant="outline"
								onClick={addProfile}
							>
								{t('PinMapping:profile-add-button')}
							</Button>
						)}
					</Nav>
					<hr />
					<p className="text-center">{t('PinMapping:sub-header-text')}</p>
					<div className="d-flex justify-content-center pb-3">
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
				<Col md={9}>
					<Tab.Content>
						{profiles.map((_, index) => (
							<Tab.Pane key={`profile-${index}`} eventKey={`profile-${index}`}>
								<PinSection profileIndex={index} />
							</Tab.Pane>
						))}
					</Tab.Content>
				</Col>
			</Row>
		</Tab.Container>
	);
}
