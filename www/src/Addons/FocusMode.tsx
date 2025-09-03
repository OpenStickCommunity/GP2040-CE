import { useTranslation, Trans } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormSelect from '../Components/FormSelect';
import { BUTTON_MASKS_OPTIONS } from '../Data/Buttons';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

export const focusModeScheme = {
	FocusModeAddonEnabled: yup
		.number()
		.required()
		.label('Focus Mode Add-On Enabled'),
	focusModeButtonLockEnabled: yup
		.number()
		.label('Focus Mode Button Lock Enabled')
		.validateRangeWhenValue('FocusModeAddonEnabled', 0, 1),
	focusModeMacroLockEnabled: yup
		.number()
		.label('Focus Mode Macro Lock Enabled')
		.validateRangeWhenValue('FocusModeAddonEnabled', 0, 1),
	focusModeButtonLockMask: yup
		.number()
		.label('Focus Mode Button Lock Map')
		.validateRangeWhenValue('FocusModeAddonEnabled', 0, (1 << 20) - 1),
};

export const focusModeState = {
	FocusModeAddonEnabled: 0,
	focusModeButtonLockEnabled: 0,
	focusModeMacroLockEnabled: 0,
	focusModeButtonLockMask: 0,
};

const FocusMode = ({
	values,
	errors,
	handleChange,
	handleCheckbox,
	setFieldValue,
}: AddonPropTypes) => {
	const { t } = useTranslation();
	return (
		<Section title={
			<a
				href="https://gp2040-ce.info/add-ons/focus-mode"
				target="_blank"
				className="text-reset text-decoration-none"
			>
				{t('AddonsConfig:focus-mode-header-text')}
			</a>
		}
		>
			<div id="FocusModeAddonOptions" hidden={!values.FocusModeAddonEnabled}>
				<div className="alert alert-info" role="alert">
					<Trans
						ns="AddonsConfig"
						i18nKey='AddonsConfig:pin-config-moved-to-core-text'
						components={[
							<a
								key="0"
								href="../pin-mapping"
								className="alert-link"
								target="_blank"
							/>
						]}
					/>
				</div>
				<Row className="mb-3">
					<FormCheck
						label={t('Common:lock-macro')}
						className="col-sm-3 ms-3"
						type="switch"
						id="FocusModeAddonMacroButton"
						isInvalid={false}
						checked={Boolean(values.focusModeMacroLockEnabled)}
						onChange={(e) => {
							handleCheckbox('focusModeMacroLockEnabled');
							handleChange(e);
						}}
					/>
					<FormCheck
						label={t('Common:lock-buttons')}
						className="col-sm-3 ms-3"
						type="switch"
						id="FocusModeAddonButton"
						isInvalid={false}
						checked={Boolean(values.focusModeButtonLockEnabled)}
						onChange={(e) => {
							handleCheckbox('focusModeButtonLockEnabled');
							handleChange(e);
						}}
					/>
				</Row>
				<Row className="mb-3">
					{BUTTON_MASKS_OPTIONS.map(
						(mask) =>
							Boolean(values.focusModeButtonLockMask & mask.value) && (
								<FormSelect
									key={`focusModeButtonLockMask-${mask.label}`}
									name="focusModeButtonLockMask"
									className="form-select-sm"
									groupClassName="col-sm-3 mb-3"
									value={values.focusModeButtonLockMask & mask.value}
									error={errors.focusModeButtonLockMask}
									isInvalid={Boolean(errors.focusModeButtonLockMask)}
									onChange={(e) => {
										setFieldValue(
											'focusModeButtonLockMask',
											(values.focusModeButtonLockMask ^ mask.value) |
												Number(e.target.value),
										);
									}}
								>
									{BUTTON_MASKS_OPTIONS.map((o, i) => (
										<option
											key={`focusModeButtonLockMask-option-${i}`}
											value={o.value}
										>
											{o.label}
										</option>
									))}
								</FormSelect>
							),
					)}
					<FormSelect
						name="focusModeButtonLockMask"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={0}
						error={errors.focusModeButtonLockMask}
						isInvalid={Boolean(errors.focusModeButtonLockMask)}
						onChange={(e) => {
							setFieldValue(
								'focusModeButtonLockMask',
								values.focusModeButtonLockMask | Number(e.target.value),
							);
						}}
					>
						{BUTTON_MASKS_OPTIONS.map((o, i) => (
							<option
								key={`focusModeButtonLockMask-option-${i}`}
								value={o.value}
							>
								{o.label}
							</option>
						))}
					</FormSelect>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="FocusModeAddonButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.FocusModeAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('FocusModeAddonEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default FocusMode;
