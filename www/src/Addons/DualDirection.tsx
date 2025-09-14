import { useTranslation, Trans } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';

import { DUAL_COMBINE_MODES, DUAL_STICK_MODES } from '../Data/Addons';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

export const dualDirectionScheme = {
	DualDirectionalInputEnabled: yup
		.number()
		.required()
		.label('Dual Directional Input Enabled'),
	dualDirDpadMode: yup
		.number()
		.label('Dual Stick Mode')
		.validateSelectionWhenValue(
			'DualDirectionalInputEnabled',
			DUAL_STICK_MODES,
		),
	dualDirCombineMode: yup
		.number()
		.label('Dual Combination Mode')
		.validateSelectionWhenValue(
			'DualDirectionalInputEnabled',
			DUAL_COMBINE_MODES,
		),
	dualDirFourWayMode: yup
		.number()
		.label('Dual Directional 4-Way Joystick Mode')
		.validateRangeWhenValue('DualDirectionalInputEnabled', 0, 1),
};

export const dualDirectionState = {
	DualDirectionalInputEnabled: 0,
	dualDirDpadMode: 0,
	dualDirCombineMode: 0,
	dualDirFourWayMode: 0,
};

const DualDirection = ({ values, errors, handleChange, handleCheckbox }: AddonPropTypes) => {
	const { t } = useTranslation();
	return (
		<Section title={
			<a
				href="https://gp2040-ce.info/add-ons/dual-direction-input"
				target="_blank"
				className="text-reset text-decoration-none"
			>
				{t('AddonsConfig:dual-directional-input-header-text')}
			</a>
		}
		>
			<div
				id="DualDirectionalInputOptions"
				hidden={!values.DualDirectionalInputEnabled}
			>
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
					<FormSelect
						label={t('AddonsConfig:dual-directional-input-dpad-mode-label')}
						name="dualDirDpadMode"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.dualDirDpadMode}
						error={errors.dualDirDpadMode}
						isInvalid={Boolean(errors.dualDirDpadMode)}
						onChange={handleChange}
					>
						{DUAL_STICK_MODES.map((o, i) => (
							<option
								key={`button-dualDirDpadMode-option-${i}`}
								value={o.value}
							>
								{t(`AddonsConfig:dual-directional-input-dual-dpad-mode-${i}`)}
							</option>
						))}
					</FormSelect>
					<FormSelect
						label={t('AddonsConfig:dual-directional-input-combine-mode-label')}
						name="dualDirCombineMode"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.dualDirCombineMode}
						error={errors.dualDirCombineMode}
						isInvalid={Boolean(errors.dualDirCombineMode)}
						onChange={handleChange}
					>
						{DUAL_COMBINE_MODES.map((o, i) => (
							<option
								key={`button-dualDirCombineMode-option-${i}`}
								value={o.value}
							>
								{t(`AddonsConfig:dual-directional-input-combine-mode-${i}`)}
							</option>
						))}
					</FormSelect>
				</Row>
				<Row className="mb-3">
					<FormCheck
						label={t(
							'AddonsConfig:dual-directional-input-four-way-joystick-mode-label',
						)}
						type="switch"
						id="DualDirFourWayMode"
						className="col-sm-3 ms-3"
						isInvalid={false}
						checked={Boolean(values.dualDirFourWayMode)}
						onChange={(e) => {
							handleCheckbox('dualDirFourWayMode');
							handleChange(e);
						}}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="DualDirectionalInputButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.DualDirectionalInputEnabled)}
				onChange={(e) => {
					handleCheckbox('DualDirectionalInputEnabled');
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default DualDirection;
