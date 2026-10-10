import { useEffect, useState } from 'react';
import { Alert, Button, Form, FormCheck, Nav, Tab } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';

import CustomSelect from './CustomSelect';
import useHEProfileStore, {
	HE_PROFILE_COUNT,
	RT_UNSET,
	RT_OFF,
	RT_ON,
} from '../Store/useHEProfileStore';
import useHETriggerStore, { Trigger } from '../Store/useHETriggerStore';
import {
	HE_PRESETS,
	DEFAULT_PRESET_LEVEL,
	getPreset,
	matchPreset,
} from '../Data/HEPresets';

// Shown when a profile's stored tuning matches no level. Negative so it can
// collide with neither a real level nor RT_UNSET.
const CUSTOM_LEVEL = -1;

type Option = { label: string; value: number };
type OptionGroup = { label: string; options: Option[] };

type Props = {
	options: OptionGroup[];
	muxChannels: number;
	// ADC pin per multiplexer, in board order. An entry of -1 means that
	// multiplexer has no pin assigned and is therefore not connected.
	connectedMuxes: number[];
	getOptionLabel: (option: Option) => string;
};

// The one place bindings are edited. The base profile lives on the triggers
// themselves and additionally carries the per-channel tuning (rapid trigger,
// calibration); the alternate profiles carry bindings only.
const HEProfileSelector = ({
	options,
	muxChannels,
	connectedMuxes,
	getOptionLabel,
}: Props) => {
	const { t } = useTranslation('');
	const {
		profiles,
		activeProfile,
		fetchHEProfiles,
		setProfileAction,
		toggleProfileEnabled,
		setProfileTuning,
		copyProfile,
		exportProfile,
		importProfile,
		saveHEProfiles,
	} = useHEProfileStore();
	const [copyFrom, setCopyFrom] = useState<number[]>([]);
	const [codeText, setCodeText] = useState<string[]>([]);
	const [codeMessage, setCodeMessage] = useState('');
	const { triggers, setHETrigger, saveHETriggers } = useHETriggerStore();
	const [saveMessage, setSaveMessage] = useState('');

	useEffect(() => {
		fetchHEProfiles();
	}, [fetchHEProfiles]);

	const handleSave = async () => {
		try {
			// Bindings and per-channel tuning live in two different stores but are
			// edited on one screen, so one button commits both. Sequential, not
			// concurrent: both endpoints touch the same trigger records, and the
			// firmware saves to flash on each one.
			await saveHETriggers();
			await saveHEProfiles();
			setSaveMessage(t('Common:saved-success-message'));
		} catch (error) {
			setSaveMessage(t('Common:saved-error-message'));
		}
	};

	const profileName = (index: number) =>
		index === 0
			? t('HETrigger:profile-base-label')
			: t('HETrigger:profile-label', { number: index + 1 });

	const handleCopy = (toIndex: number) => {
		const fromIndex = copyFrom[toIndex];
		if (fromIndex === undefined) return;
		copyProfile(fromIndex, toIndex);
		setCodeMessage(
			t('HETrigger:profile-copied', {
				from: profileName(fromIndex),
				to: profileName(toIndex),
			}),
		);
	};

	const handleExport = async (index: number) => {
		const code = exportProfile(index);
		setCodeText((prev) => {
			const next = [...prev];
			next[index] = code;
			return next;
		});
		try {
			await navigator.clipboard.writeText(code);
			setCodeMessage(t('HETrigger:profile-code-copied'));
		} catch {
			// Clipboard access is not always granted; the code is shown in the field
			// either way, so this is not worth surfacing as an error.
			setCodeMessage(t('HETrigger:profile-code-ready'));
		}
	};

	const handleImport = (index: number) => {
		try {
			importProfile(index, codeText[index] ?? '');
			setCodeMessage(t('HETrigger:profile-code-applied'));
		} catch (e) {
			setCodeMessage(String(e instanceof Error ? e.message : e));
		}
	};

	const channelLabel = (channel: number) => {
		if (muxChannels > 1) {
			return `${t('HETrigger:multiplexer-label')} ${Math.floor(
				channel / muxChannels,
			)} · ${t('HETrigger:channel-label')} ${channel % muxChannels}`;
		}
		return `${t('HETrigger:channel-label')} ${channel}`;
	};

	// Options arrive grouped, so a plain find() no longer works.
	const findOption = (value: number) => {
		for (const group of options) {
			const match = group.options.find((option) => option.value === value);
			if (match) return match;
		}
		return null;
	};

	const toggleRapidTrigger = (channel: number) => {
		const trigger = triggers[channel] as Trigger;
		if (!trigger) return;
		setHETrigger({
			id: channel,
			...trigger,
			rapidTrigger: !trigger.rapidTrigger,
		});
	};

	// Applies a whole sensitivity level to one switch. The level itself is not
	// stored -- only the values it resolves to -- so hand tuning stays possible and
	// the firmware keeps a single code path.
	const setSensitivity = (channel: number, level: number) => {
		const trigger = triggers[channel] as Trigger;
		if (!trigger) return;
		const preset = getPreset(level);
		setHETrigger({
			id: channel,
			...trigger,
			actuationPoint: preset.actuationPoint,
			rtPressSensitivity: preset.rtPressSensitivity,
			rtReleaseSensitivity: preset.rtReleaseSensitivity,
			continuousRapidTrigger: preset.continuousRapidTrigger,
		});
	};

	// Which level a switch currently sits on, or 0 for hand-tuned values.
	const sensitivityLevel = (channel: number) => {
		const trigger = triggers[channel] as Trigger;
		if (!trigger) return 0;
		return matchPreset(trigger)?.level ?? 0;
	};

	// On a non-base profile a channel may either inherit the base switch tuning
	// (stored as 0) or override it. RT_UNSET in the dropdown means inherit, and
	// CUSTOM_LEVEL covers an override that matches no level.
	const profileSensitivityLevel = (profileIndex: number, channel: number) => {
		const profile = profiles[profileIndex];
		if (!profile) return RT_UNSET;
		const actuation = profile.actuationPoint?.[channel] ?? 0;
		if (actuation === 0) return RT_UNSET;
		// Continuous RT is not overridable per profile, so it is taken from
		// whichever level the actuation point identifies rather than matched on --
		// otherwise every level would report Custom.
		const level = HE_PRESETS.find(
			(preset) => preset.actuationPoint === actuation,
		)?.level;
		const matched = matchPreset({
			actuationPoint: actuation,
			rtPressSensitivity: profile.rtPressSensitivity?.[channel] ?? 0,
			rtReleaseSensitivity: profile.rtReleaseSensitivity?.[channel] ?? 0,
			continuousRapidTrigger: getPreset(level ?? DEFAULT_PRESET_LEVEL)
				.continuousRapidTrigger,
		});
		return matched?.level ?? CUSTOM_LEVEL;
	};

	const setProfileSensitivity = (
		profileIndex: number,
		channel: number,
		level: number,
	) => {
		// Custom is a readout, not a choice: selecting it would mean writing a
		// level that does not exist.
		if (level === CUSTOM_LEVEL) return;
		if (level === RT_UNSET) {
			setProfileTuning(profileIndex, channel, {
				actuationPoint: 0,
				rtPressSensitivity: 0,
				rtReleaseSensitivity: 0,
			});
			return;
		}
		const preset = getPreset(level);
		setProfileTuning(profileIndex, channel, {
			actuationPoint: preset.actuationPoint,
			rtPressSensitivity: preset.rtPressSensitivity,
			rtReleaseSensitivity: preset.rtReleaseSensitivity,
		});
	};

	// A channel with no measured travel has never been calibrated, and enabling
	// rapid trigger on it would do nothing useful.
	const isCalibrated = (channel: number) => {
		const trigger = triggers[channel] as Trigger;
		if (!trigger) return false;
		return Math.abs(trigger.pressed - trigger.idle) > 64;
	};

	return (
		<div className="mt-4">
			<h1>{t('HETrigger:profiles-header')}</h1>
			<Alert variant="info" className="py-2">
				{t('HETrigger:profiles-shared-tuning-note')}
			</Alert>

			<Tab.Container defaultActiveKey="he-profile-0">
				<Nav variant="pills" className="mb-3 flex-wrap gap-1">
					{Array.from({ length: HE_PROFILE_COUNT }, (_, index) => (
						<Nav.Item key={`he-profile-nav-${index}`}>
							<Nav.Link eventKey={`he-profile-${index}`}>
								{index === 0
									? t('HETrigger:profile-base-label')
									: t('HETrigger:profile-label', { number: index + 1 })}
								{index === activeProfile && ' ●'}
								{index > 0 && !profiles[index]?.enabled && ' ○'}
							</Nav.Link>
						</Nav.Item>
					))}
				</Nav>

				<Tab.Content>
					{profiles.map((profile, profileIndex) => (
						<Tab.Pane
							key={`he-profile-pane-${profileIndex}`}
							eventKey={`he-profile-${profileIndex}`}
						>
							{profileIndex > 0 && (
								<>
									<FormCheck
										type="switch"
										id={`he-profile-enabled-${profileIndex}`}
										label={t('HETrigger:profile-enabled-label')}
										checked={profile.enabled}
										onChange={() => toggleProfileEnabled(profileIndex)}
										className="mb-2"
									/>
									{!profile.enabled && (
										<Alert variant="warning" className="py-2">
											{t('HETrigger:profile-disabled-warning')}
										</Alert>
									)}
									<div className="text-muted mb-3">
										{t('HETrigger:profile-unbound-note')}
									</div>
								</>
							)}
							{profileIndex === 0 && (
								<div className="text-muted mb-3">
									{t('HETrigger:profile-base-note')}
								</div>
							)}

							{/* Copy another profile's bindings in, or move a profile between boards
							    as a pasteable code. */}
							<div className="he-profile-transfer mb-3">
								<div className="he-transfer-row">
									<Form.Select
										size="sm"
										value={copyFrom[profileIndex] ?? ''}
										onChange={(e) =>
											setCopyFrom((prev) => {
												const next = [...prev];
												next[profileIndex] = Number(e.target.value);
												return next;
											})
										}
									>
										<option value="">
											{t('HETrigger:profile-copy-placeholder')}
										</option>
										{profiles.map((_, index) =>
											index === profileIndex ? null : (
												<option
													key={`copy-${profileIndex}-${index}`}
													value={index}
												>
													{profileName(index)}
												</option>
											),
										)}
									</Form.Select>
									<Button
										size="sm"
										variant="outline-secondary"
										disabled={copyFrom[profileIndex] === undefined}
										onClick={() => handleCopy(profileIndex)}
									>
										{t('HETrigger:profile-copy-button')}
									</Button>
								</div>
								<div className="he-transfer-row">
									<Form.Control
										size="sm"
										placeholder={t('HETrigger:profile-code-placeholder')}
										value={codeText[profileIndex] ?? ''}
										onChange={(e) =>
											setCodeText((prev) => {
												const next = [...prev];
												next[profileIndex] = e.target.value;
												return next;
											})
										}
									/>
									<Button
										size="sm"
										variant="outline-secondary"
										onClick={() => handleExport(profileIndex)}
									>
										{t('HETrigger:profile-code-copy')}
									</Button>
									<Button
										size="sm"
										variant="outline-secondary"
										disabled={!codeText[profileIndex]}
										onClick={() => handleImport(profileIndex)}
									>
										{t('HETrigger:profile-code-apply')}
									</Button>
								</div>
								{codeMessage && (
									<div className="he-transfer-message">{codeMessage}</div>
								)}
							</div>
							{connectedMuxes.map((adcPin, muxIndex) =>
								adcPin === -1 ? null : (
									<div
										key={`he-profile-${profileIndex}-mux-${muxIndex}`}
										className="mb-3"
									>
										<div className="he-binding-mux-label">
											{muxChannels > 1
												? t('HETrigger:mux-group-label', {
														index: muxIndex,
														pin: adcPin,
													})
												: t('HETrigger:direct-group-label', { pin: adcPin })}
										</div>
										<div className="he-binding-grid">
											{Array.from({ length: muxChannels }, (_, offset) => {
												const channel = muxIndex * muxChannels + offset;
												return (
													<div
														key={`he-profile-${profileIndex}-ch-${channel}`}
														className="he-binding-row"
													>
														<div className="he-binding-main">
															<div className="he-binding-label">
																{channelLabel(channel)}
															</div>
															<div className="he-binding-select">
																<CustomSelect
																	inputId={`he-profile-${profileIndex}-select-${channel}`}
																	isClearable
																	isSearchable
																	options={options}
																	getOptionLabel={getOptionLabel}
																	value={findOption(profile.actions[channel])}
																	onChange={(change) =>
																		setProfileAction(
																			profileIndex,
																			channel,
																			change?.value === undefined
																				? -10
																				: change.value,
																		)
																	}
																/>
															</div>
														</div>
														{/* Rapid trigger and calibration are per switch, not per
										    profile, so they are only offered on the base tab. */}
														{profileIndex === 0 && (
															<div className="he-binding-controls">
																{/* Per-switch sensitivity: applies a level's worth of actuation
																    and rapid trigger values at once. */}
																<Form.Select
																	size="sm"
																	className="he-binding-sens"
																	value={sensitivityLevel(channel)}
																	disabled={!isCalibrated(channel)}
																	onChange={(e) =>
																		setSensitivity(
																			channel,
																			Number(e.target.value),
																		)
																	}
																	title={t('HETrigger:sensitivity-title')}
																>
																	{sensitivityLevel(channel) === 0 && (
																		<option value={0}>
																			{t('HETrigger:sensitivity-custom')}
																		</option>
																	)}
																	{HE_PRESETS.map((preset) => (
																		<option
																			key={`sens-${preset.level}`}
																			value={preset.level}
																		>
																			{t('HETrigger:sensitivity-option', {
																				level: preset.level,
																				percent: preset.actuationPoint,
																			})}
																		</option>
																	))}
																</Form.Select>
																<FormCheck
																	type="switch"
																	id={`he-rapid-${channel}`}
																	label={t(
																		'HETrigger:rapid-trigger-switch-label',
																	)}
																	checked={Boolean(
																		triggers[channel]?.rapidTrigger,
																	)}
																	disabled={!isCalibrated(channel)}
																	onChange={() => toggleRapidTrigger(channel)}
																	className="he-binding-rt"
																	title={
																		isCalibrated(channel)
																			? undefined
																			: t(
																					'HETrigger:rapid-trigger-needs-calibration',
																				)
																	}
																/>
															</div>
														)}
														{/* Other profiles can override the same tuning, or inherit whatever
														    the base profile has for this switch. */}
														{profileIndex > 0 && (
															<div className="he-binding-controls">
																<Form.Select
																	size="sm"
																	className="he-binding-sens"
																	value={profileSensitivityLevel(
																		profileIndex,
																		channel,
																	)}
																	disabled={!isCalibrated(channel)}
																	onChange={(e) =>
																		setProfileSensitivity(
																			profileIndex,
																			channel,
																			Number(e.target.value),
																		)
																	}
																	title={t('HETrigger:sensitivity-title')}
																>
																	<option value={RT_UNSET}>
																		{t('HETrigger:tuning-inherit')}
																	</option>
																	{profileSensitivityLevel(
																		profileIndex,
																		channel,
																	) === CUSTOM_LEVEL && (
																		<option value={CUSTOM_LEVEL}>
																			{t('HETrigger:sensitivity-custom')}
																		</option>
																	)}
																	{HE_PRESETS.map((preset) => (
																		<option
																			key={`psens-${preset.level}`}
																			value={preset.level}
																		>
																			{t('HETrigger:sensitivity-option', {
																				level: preset.level,
																				percent: preset.actuationPoint,
																			})}
																		</option>
																	))}
																</Form.Select>
																<Form.Select
																	size="sm"
																	className="he-binding-sens"
																	value={
																		profiles[profileIndex]?.rapidTrigger?.[
																			channel
																		] ?? RT_UNSET
																	}
																	disabled={!isCalibrated(channel)}
																	onChange={(e) =>
																		setProfileTuning(profileIndex, channel, {
																			rapidTrigger: Number(e.target.value),
																		})
																	}
																>
																	<option value={RT_UNSET}>
																		{t('HETrigger:rt-inherit')}
																	</option>
																	<option value={RT_OFF}>
																		{t('HETrigger:rt-off')}
																	</option>
																	<option value={RT_ON}>
																		{t('HETrigger:rt-on')}
																	</option>
																</Form.Select>
															</div>
														)}
													</div>
												);
											})}
										</div>
									</div>
								),
							)}
						</Tab.Pane>
					))}
				</Tab.Content>
			</Tab.Container>

			<Button type="button" onClick={handleSave} className="my-2">
				{t('HETrigger:profiles-save-button')}
			</Button>
			{saveMessage && <Alert variant="secondary">{saveMessage}</Alert>}
		</div>
	);
};

export default HEProfileSelector;
