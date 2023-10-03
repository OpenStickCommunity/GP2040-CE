import React, { useContext, useEffect, useState } from 'react';
import { AppContext } from '../Contexts/AppContext';
import { Button, Form } from 'react-bootstrap';
import { Formik, useFormikContext } from 'formik';
import * as yup from 'yup';
import { Trans, useTranslation } from 'react-i18next';

import Section from '../Components/Section';
import CaptureButton from '../Components/CaptureButton';
import FormControl from '../Components/FormControl';
import WebApi from '../Services/WebApi';
import { BUTTONS, BUTTON_MASKS } from '../Data/Buttons';

const MACRO_TYPES = [
	{ label: 'InputMacroAddon:input-macro-type.release', value: 1 },
	{ label: 'InputMacroAddon:input-macro-type.hold', value: 2 },
	{ label: 'InputMacroAddon:input-macro-type.hold-repeat', value: 3 },
	{ label: 'InputMacroAddon:input-macro-type.release-toggle', value: 4 },
];

const schema = yup.object().shape({
	macroList: yup.array().of(
		yup.object().shape({
			macroType: yup.number(),
			macroLabel: yup.string(),
			enabled: yup.number(),
			exclusive: yup.number(),
			interruptible: yup.number(),
			useMacroTriggerButton: yup.number(),
			macroTriggerPin: yup.number().checkUsedPins(),
			macroTriggerButton: yup.number(),
			macroInputs: yup.array().of(
				yup.object().shape({
					buttonMask: yup.number(),
					duration: yup.number(),
					waitDuration: yup.number(),
				}),
			),
		}),
	),
	macroPin: yup.number().checkUsedPins(),
	InputMacroAddonEnabled: yup.number(),
});

const MACRO_INPUTS_MAX = 50;

const defaultMacroInput = { buttonMask: 0, duration: 16, waitDuration: 0 };

const defaultValues = {
	macroList: Array(BUTTON_MASKS.length).fill(
		{
			macroType: 1,
			macroLabel: '',
			enabled: 1,
			exclusive: 1,
			interruptible: 1,
			useMacroTriggerButton: 0,
			macroTriggerPin: -1,
			macroTriggerButton: 0,
			macroInputs: [
				defaultMacroInput,
			],
		}),
	macroPin: -1,
	InputMacroAddonEnabled: 1,
};

const EMPTY_INPUT = {};

const FormContext = () => {
	const { values, setValues } = useFormikContext();
	const { setLoading } = useContext(AppContext);

	useEffect(() => {
		async function fetchData() {
			const options = await WebApi.getMacroAddonOptions(setLoading);
			setValues(options);
		}
		fetchData();
	}, [setValues]);

	useEffect(() => {}, [values, setValues]);

	return null;
};

const ButtonMasksComponent = (props) => {
	const { id: key, value, onChange, error, isInvalid, className, buttonLabelType } = props;
	return (
		<div key={key} className={className}>
			<Form.Select
				size="sm"
				name={`${key}.buttonMask`}
				className="form-control col-sm-auto"
				groupClassName="col-sm-1"
				value={value}
				error={error}
				isInvalid={isInvalid}
				onChange={onChange}>
				{BUTTON_MASKS.map((o, i2) => (
					<option key={`${key}.mask[${i2}]`}
						value={o.value}>
						{(buttonLabelType && BUTTONS[buttonLabelType][o.label]) || o.label}
					</option>
				))}
			</Form.Select>
		</div>)
}

const MacroInputComponent = (props) => {
	const { value: { duration, buttonMask, waitDuration },
			buttonLabelType,
			errors, handleChange,
			id: key, translation: t,
			setFieldValue } = props;

	return (
		<div key={key} className="row">
			<div className="col-sm-1">
				<Form.Control
					size="sm"
					type="number"
					placeholder={t('InputMacroAddon:input-macro-duration-label')}
					name={`${key}.duration`}
					value={duration}
					error={errors?.duration}
					isInvalid={errors?.duration}
					onChange={handleChange}
					min={0} />
			</div>
			<div key={`${key}.buttons`}
				 className="col-sm-11 row mb-2">
				{BUTTON_MASKS.map((mask, i1) =>
					buttonMask & mask.value ? (
						<ButtonMasksComponent
							key={`${key}.buttonMask[${i1}]`}
							id={`${key}.buttonMask[${i1}]`}
							className="px-1 col-sm-auto"
							value={ buttonMask & mask.value }
							onChange={(e) => { setFieldValue(`${key}.buttonMask`, (buttonMask ^ mask.value) | e.target.value,); }}
							error={errors?.buttonMask}
							isInvalid={errors?.buttonMask}
							translation={t}
							buttonLabelType={buttonLabelType} />
					) : (
						<></>
					),
				)}
				<div key={`${key}.buttonMask[placeholder]`}
					className="px-1 col-sm-auto">
					<ButtonMasksComponent
						key={`${key}.buttonMaskPlaceholder`}
						id={`${key}.buttonMaskPlaceholder`}
						className="col-sm-auto"
						value={0}
						onChange={(e) => { setFieldValue(`${key}.buttonMask`, buttonMask | e.target.value,); }}
						error={errors?.buttonMask}
						isInvalid={errors?.buttonMask}
						translation={t}
						buttonLabelType={buttonLabelType} />
				</div>
				<div className="col-auto">|</div>
				<div className="col-sm-1">
					<Form.Control
						size="sm"
						type="number"
						placeholder={t('InputMacroAddon:input-macro-wait-duration-label')}
						name={`${key}.waitDuration`}
						value={waitDuration}
						error={errors?.waitDuration}
						isInvalid={errors?.waitDuration}
						onChange={handleChange}
						min={0} />
				</div>
				<Button variant="transparent" className="col-sm-auto" size="sm" onDoubleClick={(e) => { setFieldValue(key, EMPTY_INPUT)}}>&times;</Button>
			</div>
		</div>
	)
}

const MacroComponent = (props) => {
	const { value: { macroLabel, macroType, macroInputs, enabled, exclusive, interruptible,
					 useMacroTriggerButton, macroTriggerButton, macroTriggerPin },
			errors, handleChange, id: key, translation: t, index, isMacroPinMapped, buttonLabelType,
			setFieldValue, disabled } = props;

	const filteredMacroInputs = macroInputs.filter(i => i != EMPTY_INPUT);
	return (
		<div key={key} className="row mb-2">
			<div className="row">
				<div className="col-sm-auto">
					<Form.Check
						name={`${key}.enabled`}
						label={t('InputMacroAddon:input-macro-macro-activated')}
						type="switch"
						className="form-select-sm"
						disabled={disabled}
						checked={enabled}
						onChange={(e) => { setFieldValue(`${key}.enabled`, e.target.checked ? 1 : 0); }}
						isInvalid={false} />
				</div>
				<div className="col-sm-auto">
					<Form.Control
						size="sm"
						type="text"
						placeholder={t('InputMacroAddon:input-macro-macro-label-label')}
						name={`${key}.macroLabel`}
						value={macroLabel}
						error={errors?.macroLabel}
						isInvalid={errors?.macroLabel}
						onChange={handleChange}
						maxLength={256} />
				</div>
				<div className="col-sm-auto">
					<Form.Check
						name={`${key}.interruptible`}
						label={t('InputMacroAddon:input-macro-macro-interruptible')}
						type="switch"
						className="form-select-sm"
						disabled={disabled}
						checked={interruptible}
						onChange={(e) => { setFieldValue(`${key}.interruptible`, e.target.checked ? 1 : 0); }}
						isInvalid={false} />
				</div>
				<div className="col-sm-auto">
					<Form.Check
						name={`${key}.exclusive`}
						label={t('InputMacroAddon:input-macro-macro-exclusive')}
						type="switch"
						className="form-select-sm"
						disabled={disabled}
						checked={exclusive}
						onChange={(e) => { setFieldValue(`${key}.exclusive`, e.target.checked ? 1 : 0); }}
						isInvalid={false} />
				</div>
			</div>
			<div className="row mt-2">
				<div className="col-sm-auto">
					<Form.Check
						name={`${key}.useMacroTriggerButton`}
						label={t('InputMacroAddon:input-macro-macro-uses-buttons')}
						type="switch"
						className="form-select-sm"
						disabled={disabled || !isMacroPinMapped}
						checked={useMacroTriggerButton}
						onChange={(e) => { setFieldValue(`${key}.useMacroTriggerButton`, e.target.checked ? 1 : 0); }}
						isInvalid={false} />
				</div>
				{useMacroTriggerButton ? <>
					<div className="col-sm-auto pt-1">{t('InputMacroAddon:input-macro-macro-button-pin-plus')}</div>
					<div className="col-sm-auto px-0">
						<ButtonMasksComponent
							className="col-sm-auto"
							value={macroTriggerButton}
							onChange={(e) => {
								setFieldValue(`${key}.macroTriggerButton`, parseInt(e.target.value));
							}}
							buttonLabelType={buttonLabelType}
							translation={t} />
					</div>
				</> : <>
					<div className="col-sm-auto px-0">
						<Form.Control
							type="number"
							label={t('InputMacroAddon:input-macro-pin-label')}
							className="form-control-sm"
							name="macroTriggerPin"
							value={macroTriggerPin}
							error={errors?.macroTriggerPin}
							isInvalid={errors?.macroTriggerPin}
							onChange={(e) => { setFieldValue(`${key}.macroTriggerPin`, parseInt(e.target.value)); }}
							min={-1}
							max={29} />
					</div>
					<div className="col-sm-auto px-0">
						<CaptureButton
							size="sm"
							onChange={(e) => { setFieldValue(`${key}.macroTriggerPin`, e); }}
							buttonName={`Macro Pin ${index + 1}`}/>
					</div>
				</>}
				<div className="col-sm-auto">
					<Form.Select
						name={`${key}.macroType`}
						className="form-select-sm sm-1"
						groupClassName="mb-3"
						value={macroType}
						onChange={(e) => { setFieldValue(`${key}.macroType`, parseInt(e.target.value),); }}>
						{MACRO_TYPES.map((o, i2) => (
							<option key={`${key}-macroType${i2}`} value={o.value}>
								{t(o.label)}
							</option>
						))}
					</Form.Select>
				</div>
			</div>
			<div className="row mt-2">
				<div className="ms-1">
					{filteredMacroInputs.map((input, a) => (
						<MacroInputComponent key={`${key}.macroInputs[${a}]`}
							id={`${key}.macroInputs[${a}]`}
							value={macroInputs?.at(a)}
							errors={errors?.macroInputs?.at(a)}
							translation={t}
							buttonLabelType={buttonLabelType}
							handleChange={handleChange}
							setFieldValue={setFieldValue} />
					))}
					{macroInputs.length < MACRO_INPUTS_MAX ? <Button variant="success" className="col px-2" size="sm" onClick={(e) => { setFieldValue(`${key}.macroInputs[${filteredMacroInputs.length}]`, ({...defaultMacroInput})) }}>+</Button> : <></>}
				</div>
			</div>
		</div>
	);
};

export default function SettingsPage() {
	const { buttonLabels, setButtonLabels } = useContext(AppContext);
	const [saveMessage, setSaveMessage] = useState('');

	const saveSettings = async (values) => {
		const success = await WebApi.setMacroAddonOptions(values);
		setSaveMessage(
			success
				? t('Common:saved-success-message')
				: t('Common:saved-error-message'),
		);
	};

	const onSuccess = async (values) => await saveSettings(values);

	const { buttonLabelType, swapTpShareLabels } = buttonLabels;

	const { t } = useTranslation('');

	const handleCheckbox = async (name, values) => {
		values[name] = values[name] === 1 ? 0 : 1;
	};

	return (
		<Formik validationSchema={schema}
			onSubmit={onSuccess}
			initialValues={defaultValues}>
			{({ handleSubmit, handleChange, values, errors, setFieldValue }) =>
				((window.values = values) && console.log('values', values)) || console.log('errors', errors) || (
					<div>
						<Form noValidate onSubmit={handleSubmit}>
							<Section title={t('InputMacroAddon:input-macro-header-text')}>
								<div className="mb-3">
									<Trans ns="InputMacroAddon" i18nKey="input-macro-sub-header" />
								</div>
								{values.macroPin === -1 && (
									<div className="alert alert-warning">
										{t('InputMacroAddon:input-macro-warning')}
									</div>
								)}
								<div className="row mb-3">
									<FormControl
										type="number"
										label={t('InputMacroAddon:input-macro-pin-label')}
										name="macroPin"
										className="form-select-sm"
										groupClassName="col-sm-auto mb-3"
										value={values.macroPin}
										error={errors.macroPin}
										isInvalid={errors.macroPin}
										onChange={handleChange}
										min={-1}
										max={29} />
									<Form.Check
										label={t('Common:switch-enabled')}
										className="col me-3"
										type="switch"
										id="InputMacroAddonButton"
										reverse
										isInvalid={false}
										checked={Boolean(values.InputMacroAddonEnabled)}
										onChange={(e) => {
											handleCheckbox('InputMacroAddonEnabled', values);
											handleChange(e);
										}} />
								</div>
								<div id="Macros">
									<div className="row mb-3">
										{values.macroList.map((macro, i) => <>
												<MacroComponent
													key={`macroList[${i}]`}
													id={`macroList[${i}]`}
													value={values.macroList?.at(i)}
													errors={errors?.macroList?.at(i)}
													disabled={!values.InputMacroAddonEnabled}
													translation={t}
													buttonLabelType={buttonLabelType}
													handleChange={handleChange}
													index={i}
													isMacroPinMapped={values.macroPin != -1}
													setFieldValue={setFieldValue} />
												{values.macroList.length == i + 1 ? <></> : <hr className="mt-3" /> }
											</>,
										)}
									</div>
								</div>
							</Section>
							<Button type="submit">{t('Common:button-save-label')}</Button>
							{saveMessage ? (<span className="alert">{saveMessage}</span>) : null}
							<FormContext setButtonLabels={setButtonLabels} />
						</Form>
					</div>
				)
			}
		</Formik>
	);
}
