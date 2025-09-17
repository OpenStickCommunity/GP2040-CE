import { useEffect, useState, useRef, useContext } from 'react';
import { AppContext } from '../Contexts/AppContext';
import { Button, Form, Col } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import merge from 'lodash/merge';
import cloneDeep from 'lodash/cloneDeep';

import Section from '../Components/Section';
import WebApi from '../Services/WebApi';

const FILE_EXTENSION = '.gp2040';
const FILENAME = 'gp2040ce_backup_{DATE}' + FILE_EXTENSION;

const API_BINDING = {
	display: {
		get: WebApi.getDisplayOptions,
		set: WebApi.setDisplayOptions,
	},
	splash: {
		get: WebApi.getSplashImage,
		set: WebApi.setSplashImage,
	},
	gamepad: {
		get: WebApi.getGamepadOptions,
		set: WebApi.setGamepadOptions,
	},
	led: { get: WebApi.getLedOptions, set: WebApi.setLedOptions },
	ledTheme: {
		get: WebApi.getCustomTheme,
		set: WebApi.setCustomTheme,
	},
	macros: {
		get: WebApi.getMacroAddonOptions,
		set: WebApi.setMacroAddonOptions,
	},
	pins: {
		get: WebApi.getPinMappings,
		set: WebApi.setPinMappings,
	},
	profiles: {
		get: WebApi.getProfileOptions,
		set: WebApi.setProfileOptions,
	},
	heTrigger: {
		get: WebApi.getHETriggerOptions,
		set: WebApi.setHETriggerOptions,
	},
	addons: {
		get: WebApi.getAddonsOptions,
		set: WebApi.setAddonsOptions,
	},
	// new api, add it here
	// "example":	{get: WebApi.getNewAPI,			set: WebApi.setNewAPI},
};

export default function BackupPage() {
	const inputFileSelect = useRef();

	const [optionState, setOptionStateData] = useState({});
	const [importOptions, setImportOptions] = useState({});
	const [exportOptions, setExportOptions] = useState({});

	const [noticeMessage, setNoticeMessage] = useState('');
	const [saveMessage, setSaveMessage] = useState('');
	const [loadMessage, setLoadMessage] = useState('');
	const { setLoading } = useContext(AppContext);

	const { t } = useTranslation('');

	useEffect(() => {
		async function fetchData() {
			let exportData = {};
			for (const [key, func] of Object.entries(API_BINDING)) {
				exportData[key] = await func.get(setLoading);
			}
			setOptionStateData(exportData);
		}
		fetchData();

		let defaults = {};
		for (const [key] of Object.entries(API_BINDING)) {
			defaults[key] = true;
		}
		setImportOptions(defaults);
		setExportOptions(defaults);
	}, []);

	const validateValues = (data, nextData) => {
		// Handle array cases - always use backup data for arrays to allow clearing
		if (Array.isArray(nextData)) {
			return nextData;
		}

		return merge(cloneDeep(data), nextData);
	};

	const setOptionsToAPIStorage = async (options) => {
		for (const [key, func] of Object.entries(API_BINDING)) {
			const values = options[key];
			if (values) {
				try {
					await func.set(values);
				} catch (error) {
					setNoticeMessage(`Failed to set ${key} options: ${error.message}`);
				}
			}
		}
	};

	const handleSave = async () => {
		let exportData = {};
		for (const [key, value] of Object.entries(exportOptions)) {
			if (value && optionState[key] !== undefined) {
				exportData[key] = optionState[key];
			}
		}

		const fileDate = new Date().toISOString().replace(/[^0-9]/g, '');
		const name = FILENAME.replace('{DATE}', fileDate);
		const json = JSON.stringify(exportData);
		const file = new Blob([json], { type: 'text/json;charset=utf-8' });

		let a = document.createElement('a');
		a.href = URL.createObjectURL(file);
		a.download = name;
		a.innerHTML = 'Save Backup';

		let container = document.getElementById('root');
		container.appendChild(a);

		a.click();
		a.remove();

		setSaveMessage(t('BackupPage:saved-success-message', { name }));

		setTimeout(() => {
			setSaveMessage('');
		}, 5000);
	};

	const handleFileSelect = (ev) => {
		const input = ev.target;
		if (!input) {
			setNoticeMessage(`Unknown browser error, missing event data!`);
			return;
		}
		if (input.files.length === 0) {
			setNoticeMessage(`No files are loaded.`);
			return;
		}

		const fileName = input.files[0].name;

		let reader = new FileReader();
		reader.onload = function () {
			let fileData = undefined;
			try {
				fileData = JSON.parse(reader.result);
			} catch (e) {
				// error dialog
				setNoticeMessage(`Failed to parse data for ${fileName}!`);
				return;
			}
			if (!fileData) {
				setNoticeMessage(`No file data found for ${fileName}`);
				return;
			}

			let filteredData = {};
			for (const [key, value] of Object.entries(importOptions)) {
				if (value && fileData[key] !== undefined) {
					const validData = validateValues(optionState[key], fileData[key]);
					filteredData[key] = validData;
				}
			}

			if (Object.keys(filteredData).length > 0) {
				const nextOptions = { ...optionState, ...filteredData };
				setOptionStateData(nextOptions);

				// write to internal storage
				setOptionsToAPIStorage(filteredData); // Only send the filtered data, not the entire state

				setLoadMessage(`Loaded ${fileName}`);
				setNoticeMessage('');

				setTimeout(() => {
					setLoadMessage('');
				}, 5000);
			}
		};
		reader.onerror = () => {
			setNoticeMessage(`Error occured while reading ${fileName}.`);
		};
		reader.readAsText(input.files[0]);
	};

	return (
		<>
			<Section title={t('BackupPage:save-header-text')}>
				<Col>
					<Form.Group className={'row mb-3'}>
						<div className={'col'}>
							{Object.entries(API_BINDING).map(([key]) => (
								<Form.Check
									id={`export_${key}`}
									key={`export_${key}`}
									label={t('BackupPage:save-export-option-label', {
										api: t(`BackupPage:api-${key}-text`),
									})}
									type={'checkbox'}
									checked={exportOptions[key] ?? false}
									onChange={() => {
										setExportOptions((prev) => ({
											...prev,
											[key]: !prev[key],
										}));
									}}
								/>
							))}
						</div>
					</Form.Group>
					<div
						style={{
							display: 'flex',
							flexDirection: 'row',
						}}
					>
						<Button type="submit" onClick={handleSave}>
							{t('Common:button-save-label')}
						</Button>
						<div
							style={{
								height: '100%',
								paddingLeft: 24,
								fontWeight: 600,
								color: 'darkcyan',
								alignSelf: 'center',
							}}
						>
							{saveMessage ? saveMessage : null}
						</div>
					</div>
				</Col>
			</Section>
			<Section title={t('BackupPage:load-header-text')}>
				<div className="alert alert-warning">
					{t(`BackupPage:pin-version-warning-text`)}
				</div>
				<Col>
					<Form.Group className={'row mb-3'}>
						<div className={'col'}>
							{Object.entries(API_BINDING).map(([key]) => (
								<Form.Check
									id={`import_${key}`}
									key={`import_${key}`}
									label={t('BackupPage:load-export-option-label', {
										api: t(`BackupPage:api-${key}-text`),
									})}
									type={'checkbox'}
									checked={importOptions[key] ?? false}
									onChange={() => {
										setImportOptions((prev) => ({
											...prev,
											[key]: !prev[key],
										}));
									}}
								/>
							))}
						</div>
					</Form.Group>
					<input
						ref={inputFileSelect}
						type={'file'}
						accept={FILE_EXTENSION}
						style={{ display: 'none' }}
						onChange={handleFileSelect.bind(this)}
					/>
					<div
						style={{
							display: 'flex',
							flexDirection: 'row',
						}}
					>
						<Button
							onClick={() => {
								inputFileSelect.current.click();
							}}
						>
							{t('Common:button-load-label')}
						</Button>
						<div
							style={{
								height: '100%',
								paddingLeft: 24,
								fontWeight: 600,
								color: 'darkcyan',
								alignSelf: 'center',
							}}
						>
							<span>{loadMessage ? loadMessage : null}</span>
							<span style={{ color: 'red', fontWeight: 'bold' }}>
								{noticeMessage ? noticeMessage : null}
							</span>
						</div>
					</div>
				</Col>
			</Section>
		</>
	);
}
