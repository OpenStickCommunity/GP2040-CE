import React from 'react';
import { Trans, useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';

import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import { I2C_BLOCKS } from '../Data/Addons';

export const wiiScheme = {
	WiiExtensionAddonEnabled: yup
		.number()
		.required()
		.label('Wii Extensions Enabled'),
	wiiExtensionSDAPin: yup
		.number()
		.required()
		.label('WiiExtension I2C SDA Pin')
		.validatePinWhenValue('WiiExtensionAddonEnabled'),
	wiiExtensionSCLPin: yup
		.number()
		.required()
		.label('WiiExtension I2C SCL Pin')
		.validatePinWhenValue('WiiExtensionAddonEnabled'),
	wiiExtensionBlock: yup
		.number()
		.required()
		.label('WiiExtension I2C Block')
		.validateSelectionWhenValue('WiiExtensionAddonEnabled', I2C_BLOCKS),
	wiiExtensionSpeed: yup
		.number()
		.label('WiiExtension I2C Speed')
		.validateNumberWhenValue('WiiExtensionAddonEnabled'),
};

export const wiiState = {
	WiiExtensionAddonEnabled: 0,
	wiiExtensionSDAPin: -1,
	wiiExtensionSCLPin: -1,
	wiiExtensionBlock: 0,
	wiiExtensionSpeed: 400000,
};

const Wii = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:wii-extension-header-text')}>
			<div
				id="WiiExtensionAddonOptions"
				hidden={!values.WiiExtensionAddonEnabled}
			>
				<Row>
					<Trans ns="AddonsConfig" i18nKey="wii-extension-sub-header-text">
						<p>
							Note: If the Display is enabled at the same time, this Addon will
							be disabled.
						</p>
						<h3>Currently Supported Controllers</h3>
						<p>
							Classic/Classic Pro: Both Analogs and D-Pad Supported. B = B1, A =
							B2, Y = B3, X = B4, L = L1, ZL = L2, R = R1, ZR = R2, Minus = S1,
							Plus = S2, Home = A1
						</p>
						<p>Nunchuck: Analog Stick Supported. C = B1, Z = B2</p>
						<p>
							Guitar Hero Guitar: Analog Stick Supported. Green = B1, Red = B2,
							Blue = B3, Yellow = B4, Orange = L1, Strum Up = Up, Strum Down =
							Down, Minus = S1, Plus = S2
						</p>
					</Trans>
				</Row>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:wii-extension-sda-pin-label')}
						name="wiiExtensionSDAPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.wiiExtensionSDAPin}
						error={errors.wiiExtensionSDAPin}
						isInvalid={errors.wiiExtensionSDAPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:wii-extension-scl-pin-label')}
						name="wiiExtensionSCLPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.wiiExtensionSCLPin}
						error={errors.wiiExtensionSCLPin}
						isInvalid={errors.wiiExtensionSCLPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormSelect
						label={t('AddonsConfig:wii-extension-block-label')}
						name="wiiExtensionBlock"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.wiiExtensionBlock}
						error={errors.wiiExtensionBlock}
						isInvalid={errors.wiiExtensionBlock}
						onChange={handleChange}
					>
						{I2C_BLOCKS.map((o, i) => (
							<option key={`wiiExtensionI2cBlock-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
					<FormControl
						label={t('AddonsConfig:wii-extension-speed-label')}
						name="wiiExtensionSpeed"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.wiiExtensionSpeed}
						error={errors.wiiExtensionSpeed}
						isInvalid={errors.wiiExtensionSpeed}
						onChange={handleChange}
						min={100000}
					/>
				</Row>
			</div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="WiiExtensionButton"
				reverse={true}
				isInvalid={false}
				checked={Boolean(values.WiiExtensionAddonEnabled)}
				onChange={(e) => {
					handleCheckbox('WiiExtensionAddonEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default Wii;
