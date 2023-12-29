import React, { useEffect, useState, useRef, useContext } from 'react';
import { AppContext } from '../Contexts/AppContext';
import { Button, Form, Col } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';

import Section from '../Components/Section';
import WebApi from '../Services/WebApi';

const FILE_EXTENSION = '.gp2040';
const FILENAME = 'gp2040ce_backup_{DATE}' + FILE_EXTENSION;

const API_BINDING = {
	display: {
		label: 'Display',
		get: WebApi.getDisplayOptions,
		set: WebApi.setDisplayOptions,
	},
	gamepad: {
		label: 'Gamepad',
		get: WebApi.getGamepadOptions,
		set: WebApi.setGamepadOptions,
	},
	led: { label: 'LED', get: WebApi.getLedOptions, set: WebApi.setLedOptions },
	ledTheme: {
		label: 'Custom LED Theme',
		get: WebApi.getCustomTheme,
		set: WebApi.setCustomTheme,
	},
	macros: {
		label: 'Macro Mappings',
		get: WebApi.getMacroAddonOptions,
		set: WebApi.setMacroAddonOptions,
	},
	pins: {
		label: 'Pin Mappings',
		get: WebApi.getPinMappings,
		set: WebApi.setPinMappings,
	},
	profiles: {
		label: 'Profile Mappings',
		get: WebApi.getProfileOptions,
		set: WebApi.setProfileOptions,
	},
	addons: {
		label: 'Add-Ons',
		get: WebApi.getAddonsOptions,
		set: WebApi.setAddonsOptions,
	},
	// new api, add it here
	// "example":	{label: "Example",		get: WebApi.getNewAPI,			set: WebApi.setNewAPI},
};

export default function BackupPage() {
	const inputFileSelect = useRef();

	const [optionState, setOptionStateData] = useState({});
	const [checkValues, setCheckValues] = useState({}); // lazy approach

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

		// setup defaults
		function getDefaultValues() {
			let defaults = {};
			for (const [key] of Object.entries(API_BINDING)) {
				defaults[`export_${key}`] = true;
				defaults[`import_${key}`] = true;
			}
			return defaults;
		}
		setCheckValues(getDefaultValues());
	}, []);

	const validateValues = (data, nextData) => {
		if (typeof data != 'object' || typeof nextData != 'object') {
			// invalid data types
			return {};
		}

		let validated = Array.isArray(data) ? [] : {};
		const addValidated = (value, key) =>
			Array.isArray(validated)
				? validated.push(value)
				: (validated[key] = value);

		for (const [key, value] of Object.entries(data)) {
			const nextDataValue = nextData[key];
			if (
				nextDataValue !== null &&
				typeof nextDataValue !== 'undefined' &&
				typeof value == typeof nextDataValue
			) {
				if (typeof nextDataValue == 'object') {
					addValidated(validateValues(value, nextDataValue), key);
				} else {
					addValidated(nextDataValue, key);
				}
			}
		}

		return validated;
	};

	const setOptionsToAPIStorage = async (options) => {
		for (const [key, func] of Object.entries(API_BINDING)) {
			const values = options[key];
			if (values) {
				const result = await func.set(values);
				console.log(result);
			}
		}
	};

	const handleChange = (ev) => {
		const id = ev.nativeEvent.target.id;
		let nextCheckValue = {};
		nextCheckValue[id] = !checkValues[id];
		setCheckValues((checkValues) => ({ ...checkValues, ...nextCheckValue }));
	};

	const handleSave = async () => {
		let exportData = {};
		for (const [key, value] of Object.entries(checkValues)) {
			if (key.match('export_') && (value != null || value !== undefined)) {
				let skey = key.slice(7, key.length);
				if (optionState[skey] !== undefined || optionState[skey] != null) {
					exportData[skey] = optionState[skey];
				}
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
			// validate parsed data
			let newData = {};
			for (const [key, value] of Object.entries(fileData)) {
				if (optionState[key]) {
					const result = validateValues(optionState[key], value);
					newData[key] = result;
				}
			}

			if (Object.entries(newData).length > 0) {
				// filter by known values
				let filteredData = {};
				for (const [key, value] of Object.entries(checkValues)) {
					if (key.match('import_') && (value != null || value !== undefined)) {
						let skey = key.slice(7, key.length);
						if (newData[skey] !== undefined || newData[skey] != null) {
							filteredData[skey] = newData[skey];
						}
					}
				}
				const nextOptions = { ...optionState, ...filteredData };
				setOptionStateData(nextOptions);

				// write to internal storage
				setOptionsToAPIStorage(nextOptions);

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
						<div className={'col-sm-4'}>
							{Object.entries(API_BINDING).map((api) => (
								<Form.Check
									id={`export_${api[0]}`}
									key={`export_${api[0]}`}
									label={t('BackupPage:save-export-option-label', {
										api: t(`BackupPage:api-${api[0]}-text`),
									})}
									type={'checkbox'}
									checked={checkValues[`export_${api[0]}`] ?? false}
									onChange={handleChange}
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
						<div className={'col-sm-4'}>
							{Object.entries(API_BINDING).map((api) => (
								<Form.Check
									id={`import_${api[0]}`}
									key={`import_${api[0]}`}
									label={t('BackupPage:load-export-option-label', {
										api: t(`BackupPage:api-${api[0]}-text`),
									})}
									type={'checkbox'}
									checked={checkValues[`import_${api[0]}`] ?? false}
									onChange={handleChange}
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
