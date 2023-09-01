import React from 'react';
import { useTranslation } from 'react-i18next';
import { FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';

import FormControl from '../Components/FormControl';
import { I2C_BLOCKS } from '../Data/Addons';

export const i2cScheme = {
	I2CAnalog1219InputEnabled: yup.number().label('I2C Analog1219 Input Enabled'),
	i2cAnalog1219SDAPin: yup
		.number()
		.label('I2C Analog1219 SDA Pin')
		.validatePinWhenValue('I2CAnalog1219InputEnabled'),
	i2cAnalog1219SCLPin: yup
		.number()
		.label('I2C Analog1219 SCL Pin')
		.validatePinWhenValue('I2CAnalog1219InputEnabled'),
	i2cAnalog1219Block: yup
		.number()
		.label('I2C Analog1219 Block')
		.validateSelectionWhenValue('I2CAnalog1219InputEnabled', I2C_BLOCKS),
	i2cAnalog1219Speed: yup
		.number()
		.label('I2C Analog1219 Speed')
		.validateNumberWhenValue('I2CAnalog1219InputEnabled'),
	i2cAnalog1219Address: yup
		.number()
		.label('I2C Analog1219 Address')
		.validateNumberWhenValue('I2CAnalog1219InputEnabled'),
};

export const i2cState = {
	I2CAnalog1219InputEnabled: 0,
	i2cAnalog1219SDAPin: -1,
	i2cAnalog1219SCLPin: -1,
	i2cAnalog1219Block: 0,
	i2cAnalog1219Speed: 400000,
	i2cAnalog1219Address: 0x40,
};

const I2c = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	return (
		<Section title={t('AddonsConfig:i2c-analog-ads1219-header-text')}>
			<div
				id="I2CAnalog1219InputOptions"
				hidden={!values.I2CAnalog1219InputEnabled}
			>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:i2c-analog-ads1219-sda-pin-label')}
						name="i2cAnalog1219SDAPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.i2cAnalog1219SDAPin}
						error={errors.i2cAnalog1219SDAPin}
						isInvalid={errors.i2cAnalog1219SDAPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:i2c-analog-ads1219-scl-pin-label')}
						name="i2cAnalog1219SCLPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.i2cAnalog1219SCLPin}
						error={errors.i2cAnalog1219SCLPin}
						isInvalid={errors.i2cAnalog1219SCLPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormSelect
						label={t('AddonsConfig:i2c-analog-ads1219-block-label')}
						name="i2cAnalog1219Block"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.i2cAnalog1219Block}
						error={errors.i2cAnalog1219Block}
						isInvalid={errors.i2cAnalog1219Block}
						onChange={handleChange}
					>
						{I2C_BLOCKS.map((o, i) => (
							<option key={`i2cBlock-option-${i}`} value={o.value}>
								{o.label}
							</option>
						))}
					</FormSelect>
					<FormControl
						label={t('AddonsConfig:i2c-analog-ads1219-speed-label')}
						name="i2cAnalog1219Speed"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.i2cAnalog1219Speed}
						error={errors.i2cAnalog1219Speed}
						isInvalid={errors.i2cAnalog1219Speed}
						onChange={handleChange}
						min={100000}
					/>
				</Row>
				<Row className="mb-3">
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
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="I2CAnalog1219InputButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.I2CAnalog1219InputEnabled)}
				onChange={(e) => {
					handleCheckbox('I2CAnalog1219InputEnabled', values);
					handleChange(e);
				}}
			/>
		</Section>
	);
};

export default I2c;
