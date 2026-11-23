import { useContext, useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import {
	Button,
	Col,
	Form,
	FormCheck,
	FormControl,
	Row,
} from 'react-bootstrap';
import * as yup from 'yup';
import { NFCTagActionType } from '@proto/enums';

import { AppContext } from '../Contexts/AppContext';
import Section from '../Components/Section';
import WebApi from '../Services/WebApi';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';
import { BUTTONS, BUTTON_MASKS_OPTIONS } from '../Data/Buttons';
import { HOTKEY_ACTIONS } from '../Data/HotkeyActions';

const NFC_TAG_COUNT = 8;
const DEFAULT_HOLD_MS = 100;

type NFCReaderStatus = {
	enabled: boolean;
	detected: boolean;
	firmware: string;
	tagPresent: boolean;
	uid: string;
	amiibo: boolean;
	hasSignature: boolean;
	i2cScan: string;
	note: string;
};

type NFCTag = {
	name: string;
	uid: string;
	actionType: NFCTagActionType;
	hotkey: number;
	buttonMask: number;
	holdMs: number;
};

const ACTION_TYPES = [
	{
		labelKey: 'nfc-reader-action-none',
		value: NFCTagActionType.NFC_TAG_ACTION_NONE,
	},
	{
		labelKey: 'nfc-reader-action-hotkey',
		value: NFCTagActionType.NFC_TAG_ACTION_HOTKEY,
	},
	{
		labelKey: 'nfc-reader-action-buttons',
		value: NFCTagActionType.NFC_TAG_ACTION_BUTTONS,
	},
];

const TAG_BUTTON_MASKS = BUTTON_MASKS_OPTIONS.filter(
	(mask) => mask.value !== 0,
);

export const nfcReaderScheme = {
	NFCReaderAddonEnabled: yup
		.number()
		.required()
		.label('NFC Reader Add-On Enabled'),
};

export const nfcReaderState = {
	NFCReaderAddonEnabled: 0,
};

const newTag = (uid = ''): NFCTag => ({
	name: '',
	uid,
	actionType: NFCTagActionType.NFC_TAG_ACTION_NONE,
	hotkey: 0,
	buttonMask: 0,
	holdMs: DEFAULT_HOLD_MS,
});

const NFCReader = ({
	values,
	handleChange,
	handleCheckbox,
}: AddonPropTypes) => {
	const { t } = useTranslation();
	const { buttonLabels } = useContext(AppContext);
	const { buttonLabelType } = buttonLabels;
	const [status, setStatus] = useState<NFCReaderStatus>();
	const [tags, setTags] = useState<NFCTag[]>([]);
	const [message, setMessage] = useState('');
	const [saving, setSaving] = useState(false);

	useEffect(() => {
		if (!values.NFCReaderAddonEnabled) return;
		let active = true;
		const refresh = async () => {
			const data = await WebApi.getNFCReaderStatus();
			if (active) setStatus(data);
		};
		void refresh();
		const timer = setInterval(refresh, 1000);
		return () => {
			active = false;
			clearInterval(timer);
		};
	}, [values.NFCReaderAddonEnabled]);

	useEffect(() => {
		if (!values.NFCReaderAddonEnabled) return;
		WebApi.getNFCReaderOptions().then((data) => setTags(data?.tags ?? []));
	}, [values.NFCReaderAddonEnabled]);

	const statusText = () => {
		if (!status) return t('AddonsConfig:nfc-reader-status-unknown');
		if (!status.enabled) return t('AddonsConfig:nfc-reader-status-reboot');
		if (!status.detected)
			return t('AddonsConfig:nfc-reader-status-missing', {
				scan: status.i2cScan || '…',
			});
		if (!status.tagPresent)
			return t('AddonsConfig:nfc-reader-status-ready', {
				firmware: status.firmware,
			});
		if (status.amiibo)
			return t(
				status.hasSignature
					? 'AddonsConfig:nfc-reader-status-amiibo'
					: 'AddonsConfig:nfc-reader-status-amiibo-nosig',
				{ uid: status.uid },
			);
		return t('AddonsConfig:nfc-reader-status-tag', { uid: status.uid });
	};

	const updateTag = (index: number, changes: Partial<NFCTag>) =>
		setTags(tags.map((tag, i) => (i === index ? { ...tag, ...changes } : tag)));

	const saveTags = async () => {
		setSaving(true);
		setMessage(t('AddonsConfig:nfc-reader-tags-saving'));
		const success = await WebApi.setNFCReaderOptions({
			tags: tags.map((tag) => ({ ...tag, uid: tag.uid.toUpperCase() })),
		});
		setMessage(
			t(
				success
					? 'AddonsConfig:nfc-reader-tags-saved'
					: 'AddonsConfig:nfc-reader-tags-failed',
			),
		);
		setSaving(false);
	};

	const buttonLabel = (label: string) =>
		(buttonLabelType && BUTTONS[buttonLabelType][label]) || label;

	return (
		<Section title={t('AddonsConfig:nfc-reader-header-text')}>
			<div id="NFCReaderAddonOptions" hidden={!values.NFCReaderAddonEnabled}>
				<div className="alert alert-info" role="alert">
					{t('AddonsConfig:nfc-reader-sub-header-text')}
				</div>
				<div role="status" aria-live="polite" className="mb-3">
					{statusText()}
					{status?.note && (
						<div className="small text-muted">{status.note}</div>
					)}
				</div>
				<legend className="h6">
					<strong>{t('AddonsConfig:nfc-reader-tags-title')}</strong>
				</legend>
				<p>{t('AddonsConfig:nfc-reader-tags-help')}</p>
				{tags.map((tag, index) => (
					<div className="border rounded p-2 mb-2" key={`nfc-tag-${index}`}>
						<Row className="g-2 align-items-end">
							<Col sm={3}>
								<Form.Label htmlFor={`nfc-tag-name-${index}`}>
									{t('AddonsConfig:nfc-reader-tag-name')}
								</Form.Label>
								<FormControl
									id={`nfc-tag-name-${index}`}
									size="sm"
									maxLength={16}
									value={tag.name}
									onChange={(e) => updateTag(index, { name: e.target.value })}
								/>
							</Col>
							<Col sm={4}>
								<Form.Label htmlFor={`nfc-tag-uid-${index}`}>
									{t('AddonsConfig:nfc-reader-tag-uid')}
								</Form.Label>
								<div className="d-flex gap-2">
									<FormControl
										id={`nfc-tag-uid-${index}`}
										size="sm"
										maxLength={20}
										value={tag.uid}
										isInvalid={!/^([0-9a-fA-F]{2}){1,10}$/.test(tag.uid)}
										onChange={(e) => updateTag(index, { uid: e.target.value })}
									/>
									<Button
										size="sm"
										variant="outline-secondary"
										disabled={!status?.tagPresent}
										onClick={() => updateTag(index, { uid: status?.uid ?? '' })}
									>
										{t('AddonsConfig:nfc-reader-tag-use-current')}
									</Button>
								</div>
							</Col>
							<Col sm={3}>
								<Form.Label htmlFor={`nfc-tag-action-${index}`}>
									{t('AddonsConfig:nfc-reader-tag-action')}
								</Form.Label>
								<Form.Select
									id={`nfc-tag-action-${index}`}
									size="sm"
									value={tag.actionType}
									onChange={(e) =>
										updateTag(index, { actionType: Number(e.target.value) })
									}
								>
									{ACTION_TYPES.map((o) => (
										<option key={o.value} value={o.value}>
											{t(`AddonsConfig:${o.labelKey}`)}
										</option>
									))}
								</Form.Select>
							</Col>
							<Col sm={2}>
								<Form.Label htmlFor={`nfc-tag-hold-${index}`}>
									{t('AddonsConfig:nfc-reader-tag-hold')}
								</Form.Label>
								<FormControl
									id={`nfc-tag-hold-${index}`}
									type="number"
									size="sm"
									min={0}
									max={10000}
									value={tag.holdMs}
									onChange={(e) =>
										updateTag(index, { holdMs: Number(e.target.value) })
									}
								/>
							</Col>
						</Row>
						{tag.actionType === NFCTagActionType.NFC_TAG_ACTION_HOTKEY && (
							<Form.Select
								className="mt-2"
								size="sm"
								aria-label={t('AddonsConfig:nfc-reader-action-hotkey')}
								value={tag.hotkey}
								onChange={(e) =>
									updateTag(index, { hotkey: Number(e.target.value) })
								}
							>
								{HOTKEY_ACTIONS.map((o) => (
									<option key={o.value} value={o.value}>
										{t(`SettingsPage:${o.labelKey}`)}
									</option>
								))}
							</Form.Select>
						)}
						{tag.actionType === NFCTagActionType.NFC_TAG_ACTION_BUTTONS && (
							<div className="d-flex flex-wrap gap-3 mt-2">
								{TAG_BUTTON_MASKS.map((mask) => (
									<FormCheck
										key={mask.value}
										id={`nfc-tag-${index}-button-${mask.value}`}
										label={buttonLabel(mask.label)}
										checked={(tag.buttonMask & mask.value) !== 0}
										onChange={() =>
											updateTag(index, {
												buttonMask: tag.buttonMask ^ mask.value,
											})
										}
									/>
								))}
							</div>
						)}
						<Button
							className="mt-2"
							size="sm"
							variant="outline-danger"
							onClick={() => setTags(tags.filter((_, i) => i !== index))}
						>
							{t('AddonsConfig:nfc-reader-tag-remove')}
						</Button>
					</div>
				))}
				<div className="d-flex gap-2 mb-2">
					<Button
						size="sm"
						variant="outline-primary"
						disabled={tags.length >= NFC_TAG_COUNT}
						onClick={() =>
							setTags([...tags, newTag(status?.tagPresent ? status.uid : '')])
						}
					>
						{t('AddonsConfig:nfc-reader-tag-add')}
					</Button>
					<Button size="sm" disabled={saving} onClick={saveTags}>
						{t('AddonsConfig:nfc-reader-tags-save')}
					</Button>
				</div>
				{message && (
					<div role="status" aria-live="polite" className="small mb-3">
						{message}
					</div>
				)}
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="NFCReaderAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.NFCReaderAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('NFCReaderAddonEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default NFCReader;
