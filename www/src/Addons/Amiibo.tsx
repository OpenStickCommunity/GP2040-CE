import { useCallback, useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Button, FormCheck, FormControl } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import WebApi from '../Services/WebApi';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

const AMIIBO_FILE_SIZES = [540, 572];

type AmiiboKeyType = 'unfixed' | 'locked';
type AmiiboKeys = { unfixed: boolean; locked: boolean; ready: boolean };

type AmiiboSlot = {
	filled: boolean;
	name: string;
	size: number;
	randomizeSerial: boolean;
};

export const amiiboScheme = {
	AmiiboAddonEnabled: yup.number().required().label('Amiibo Add-On Enabled'),
};

export const amiiboState = {
	AmiiboAddonEnabled: 0,
};

const toBase64 = (bytes: Uint8Array) => {
	let binary = '';
	bytes.forEach((b) => (binary += String.fromCharCode(b)));
	return btoa(binary);
};

const Amiibo = ({ values, handleChange, handleCheckbox }: AddonPropTypes) => {
	const { t } = useTranslation();
	const [keys, setKeys] = useState<AmiiboKeys>();
	const [keysLoading, setKeysLoading] = useState(true);
	const [keysBusy, setKeysBusy] = useState(false);
	const [keyMessage, setKeyMessage] = useState('');
	const [keyStatusFailed, setKeyStatusFailed] = useState(false);
	const refreshKeys = useCallback(async () => {
		setKeysLoading(true);
		const data = await WebApi.getAmiiboKeys();
		const valid =
			data &&
			typeof data.unfixed === 'boolean' &&
			typeof data.locked === 'boolean' &&
			typeof data.ready === 'boolean';
		setKeys(valid ? data : undefined);
		setKeyStatusFailed(!valid);
		setKeysLoading(false);
	}, []);
	const uploadKey = async (type: AmiiboKeyType, file?: File) => {
		if (!file) return;
		if (file.size !== 80) {
			setKeyMessage(
				t('AddonsConfig:amiibo-key-invalid-size', { size: file.size }),
			);
			return;
		}
		setKeysBusy(true);
		setKeyMessage(t('AddonsConfig:amiibo-keys-saving'));
		try {
			const data = toBase64(new Uint8Array(await file.arrayBuffer()));
			const success = await WebApi.setAmiiboKey({ type, data });
			setKeyMessage(
				t(
					success
						? 'AddonsConfig:amiibo-key-saved'
						: 'AddonsConfig:amiibo-key-failed',
				),
			);
			await refreshKeys();
		} catch {
			setKeyMessage(t('AddonsConfig:amiibo-key-failed'));
		} finally {
			setKeysBusy(false);
		}
	};
	const clearKeys = async () => {
		setKeysBusy(true);
		setKeyMessage(t('AddonsConfig:amiibo-keys-clearing'));
		try {
			const success = await WebApi.clearAmiiboKeys();
			setKeyMessage(
				t(
					success
						? 'AddonsConfig:amiibo-keys-cleared'
						: 'AddonsConfig:amiibo-keys-clear-failed',
				),
			);
			await refreshKeys();
		} finally {
			setKeysBusy(false);
		}
	};
	const [slots, setSlots] = useState<AmiiboSlot[]>([]);
	const [slotBusy, setSlotBusy] = useState(false);
	const [messages, setMessages] = useState<Record<number, string>>({});

	const refresh = useCallback(async () => {
		const data = await WebApi.getAmiiboSlots();
		if (data) setSlots(data);
	}, []);

	useEffect(() => {
		refresh();
		refreshKeys();
	}, [refresh, refreshKeys]);

	const setMessage = (slot: number, message: string) =>
		setMessages((current) => ({ ...current, [slot]: message }));

	const saveSlotOptions = async (slot: number, randomizeSerial: boolean) => {
		setSlotBusy(true);
		setMessage(slot, t('AddonsConfig:amiibo-option-saving'));
		try {
			const success = await WebApi.setAmiiboSlotOptions({
				slot,
				randomizeSerial,
			});
			if (success)
				setSlots((current) =>
					current.map((item, index) =>
						index === slot ? { ...item, randomizeSerial } : item,
					),
				);
			setMessage(
				slot,
				t(
					success
						? 'AddonsConfig:amiibo-option-saved'
						: 'AddonsConfig:amiibo-option-failed',
				),
			);
		} finally {
			setSlotBusy(false);
		}
	};

	const upload = async (slot: number, file?: File) => {
		if (!file) return;
		setSlotBusy(true);
		setMessage(slot, t('AddonsConfig:amiibo-option-saving'));
		try {
			const bytes = new Uint8Array(await file.arrayBuffer());
			if (!AMIIBO_FILE_SIZES.includes(bytes.length)) {
				setMessage(
					slot,
					t('AddonsConfig:amiibo-invalid-size', { size: bytes.length }),
				);
				return;
			}
			const name = file.name.replace(/\.[^.]+$/, '').substring(0, 31);
			const success = await WebApi.setAmiiboSlot({
				slot,
				name,
				data: toBase64(bytes),
			});
			if (success) await refresh();
			setMessage(
				slot,
				t(
					success
						? 'AddonsConfig:amiibo-saved'
						: 'AddonsConfig:amiibo-save-failed',
				),
			);
		} catch {
			setMessage(slot, t('AddonsConfig:amiibo-save-failed'));
		} finally {
			setSlotBusy(false);
		}
	};

	const clear = async (slot: number) => {
		setSlotBusy(true);
		setMessage(slot, t('AddonsConfig:amiibo-option-saving'));
		try {
			const success = await WebApi.clearAmiiboSlot(slot);
			if (success) await refresh();
			setMessage(slot, success ? '' : t('AddonsConfig:amiibo-clear-failed'));
		} finally {
			setSlotBusy(false);
		}
	};

	return (
		<Section title={t('AddonsConfig:amiibo-header-text')}>
			<div id="AmiiboAddonOptions" hidden={!values.AmiiboAddonEnabled}>
				<div className="alert alert-info" role="alert">
					<p className="mb-4">{t('AddonsConfig:amiibo-sub-header-text')}</p>
					<p className="mb-0">{t('AddonsConfig:amiibo-assign-help')}</p>
				</div>
				<div className="alert alert-success" role="alert">
					{t('AddonsConfig:amiibo-mode-warning')}
				</div>
				<fieldset
					className="mb-4"
					disabled={keysBusy || keysLoading || slotBusy}
				>
					<legend className="h6"><strong>{t('AddonsConfig:amiibo-keys-title')}</strong></legend>
					<p>{t('AddonsConfig:amiibo-keys-help')}</p>
					<p><strong><em>{t('AddonsConfig:amiibo-keys-warning')}</em></strong></p>
					{(['unfixed', 'locked'] as const).map((type) => (
						<div className="mb-2" key={type}>
							<label htmlFor={`amiibo-key-${type}`}>
								{t(`AddonsConfig:amiibo-key-${type}`)}
								{' — '}
								{t(
									!keys
										? 'AddonsConfig:amiibo-key-unknown'
										: keys[type]
											? 'AddonsConfig:amiibo-key-loaded'
											: 'AddonsConfig:amiibo-key-missing',
								)}
							</label>
							<FormControl
								id={`amiibo-key-${type}`}
								type="file"
								size="sm"
								accept=".bin"
								style={{ maxWidth: '28rem' }}
								onChange={(e) => {
									const input = e.target as HTMLInputElement;
									const file = input.files?.[0];
									input.value = '';
									void uploadKey(type, file);
								}}
							/>
						</div>
					))}
					<div className="d-flex gap-2 mt-2">
						<Button size="sm" variant="outline-danger" onClick={clearKeys}>
							{t('AddonsConfig:amiibo-keys-clear')}
						</Button>
						<Button size="sm" variant="outline-secondary" onClick={refreshKeys}>
							{t('AddonsConfig:amiibo-keys-retry')}
						</Button>
					</div>
				</fieldset>
				<div role="status" aria-live="polite" className="small mb-3">
					{keysLoading && <div>{t('AddonsConfig:amiibo-keys-loading')}</div>}
					{keyStatusFailed && <div>{t('AddonsConfig:amiibo-keys-status-failed')}</div>}
					{keys?.ready && <div>{t('AddonsConfig:amiibo-keys-ready')}</div>}
					{keyMessage && <div>{keyMessage}</div>}
				</div>
				{slots.map((slot, index) => (
					<div className="mb-3" key={`amiibo-slot-${index}`}>
						<div className="mb-1">
							<strong>
								{t('AddonsConfig:amiibo-slot-label', { slot: index + 1 })}
							</strong>{' '}
							{slot.filled
								? `${slot.name || t('AddonsConfig:amiibo-unnamed')} (${slot.size} bytes)`
								: t('AddonsConfig:amiibo-empty')}
						</div>
						<div
							className="d-flex gap-2 align-items-center"
							style={{ maxWidth: '28rem' }}
						>
							<FormControl
								type="file"
								size="sm"
								accept=".bin"
								disabled={slotBusy || keysBusy}
								onChange={(e) =>
									upload(index, (e.target as HTMLInputElement).files?.[0])
								}
							/>
							<Button
								size="sm"
								variant="outline-danger"
								disabled={!slot.filled || slotBusy || keysBusy}
								onClick={() => clear(index)}
							>
								{t('AddonsConfig:amiibo-clear')}
							</Button>
						</div>
						<FormCheck
							type="switch"
							id={`amiibo-randomize-${index}`}
							className="mt-2"
							label={t('AddonsConfig:amiibo-randomize-label')}
							checked={slot.randomizeSerial}
							disabled={!slot.filled || slotBusy || keysBusy}
							onChange={(e) => saveSlotOptions(index, e.target.checked)}
						/>
						{messages[index] && (
							<div role="status" aria-live="polite" className="small mt-1">
								{messages[index]}
							</div>
						)}
					</div>
				))}
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="AmiiboAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.AmiiboAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('AmiiboAddonEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Amiibo;
