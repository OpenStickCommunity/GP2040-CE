import { AppContext } from '../Contexts/AppContext';
import React, { useContext } from 'react';
import { Trans, useTranslation } from 'react-i18next';
import { FormCheck, Row, FormLabel } from 'react-bootstrap';
import { NavLink } from 'react-router-dom';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';

import FormControl from '../Components/FormControl';
import { I2C_BLOCKS } from '../Data/Peripherals';

export const i2cAnalogScheme = {
	I2CAnalog1219InputEnabled: yup.number().label('I2C Analog1219 Input Enabled'),
	i2cAnalog1219Block: yup
		.number()
		.label('I2C Analog1219 Block')
		.validateSelectionWhenValue('I2CAnalog1219InputEnabled', I2C_BLOCKS),
	i2cAnalog1219Address: yup
		.number()
		.label('I2C Analog1219 Address')
		.validateNumberWhenValue('I2CAnalog1219InputEnabled'),
};

export const i2cAnalogState = {
	I2CAnalog1219InputEnabled: 0,
	i2cAnalog1219Block: 0,
	i2cAnalog1219Address: 0x40,
};

const I2CAnalog1219 = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	const { getAvailablePeripherals, getSelectedPeripheral } =
		useContext(AppContext);

	const handlePeripheralChange = (e) => {
		let device = getSelectedPeripheral('i2c', e.target.value);
		handleChange(e);
	};

	return (
		<Section title={t('AddonsConfig:i2c-analog-ads1219-header-text')}>
			<div
				id="I2CAnalog1219InputOptions"
				hidden={
					!(values.I2CAnalog1219InputEnabled && getAvailablePeripherals('i2c'))
				}
			>
				<Row className="mb-3">
					{getAvailablePeripherals('i2c') ? (
						<FormSelect
							label={t('AddonsConfig:i2c-analog-ads1219-block-label')}
							name="i2cAnalog1219Block"
							className="form-select-sm"
							groupClassName="col-sm-3 mb-3"
							value={values.i2cAnalog1219Block}
							error={errors.i2cAnalog1219Block}
							isInvalid={errors.i2cAnalog1219Block}
							onChange={handlePeripheralChange}
						>
							{getAvailablePeripherals('i2c').map((o, i) => (
								<option key={`i2cBlock-option-${i}`} value={o.value}>
									{o.label}
								</option>
							))}
						</FormSelect>
					) : (
						''
					)}
					<FormControl
						label={t('AddonsConfig:i2c-analog-ads1219-address-label')}
						name="i2cAnalog1219Address"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.i2cAnalog1219Address}
						error={errors.i2cAnalog1219Address}
						isInvalid={errors.i2cAnalog1219Address}
						onChange={handleChange}
						maxLength={4}
					/>
				</Row>
			</div>
			{getAvailablePeripherals('i2c') ? (
				<FormCheck
					label={t('Common:switch-enabled')}
					type="switch"
					id="I2CAnalog1219InputButton"
					reverse
					isInvalid={false}
					checked={
						Boolean(values.I2CAnalog1219InputEnabled) &&
						getAvailablePeripherals('i2c')
					}
					onChange={(e) => {
						handleCheckbox('I2CAnalog1219InputEnabled', values);
						handleChange(e);
					}}
				/>
			) : (
				<FormLabel>
					<Trans
						ns="PeripheralMapping"
						i18nKey="peripheral-toggle-unavailable"
						values={{ name: 'I2C' }}
					>
						<NavLink to="/peripheral-mapping">
							{t('PeripheralMapping:header-text')}
						</NavLink>
					</Trans>
				</FormLabel>
			)}
		</Section>
	);
};

export default I2CAnalog1219;
