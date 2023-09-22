import React from 'react';
import { useTranslation } from 'react-i18next';
import { Alert, FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';

import FormControl from '../Components/FormControl';
import { I2C_BLOCKS } from '../Data/Addons';

export const MPU6050Scheme = {
    MPU6050InputEnabled: yup.number().label('MPU6050 Input Enabled'),
	MPU6050i2cSDAPin: yup
		.number()
		.label('MPU6050 SDA Pin')
		.validatePinWhenValue('MPU6050InputEnabled'),
	MPU6050i2cSCLPin: yup
		.number()
		.label('MPU6050 SCL Pin')
		.validatePinWhenValue('MPU6050InputEnabled'),
	MPU6050i2cBlock: yup
		.number()
		.label('MPU6050 i2c Block')
		.validateSelectionWhenValue('MPU6050InputEnabled', I2C_BLOCKS),
	MPU6050i2cSpeed: yup
		.number()
		.label('MPU6050 i2c Speed')
		.validateRangeWhenValue('MPU6050InputEnabled', 100000, 400000),
	MPU6050i2cAddress: yup
		.number()
		.label('MPU6050 i2c Address')
		.validateNumberWhenValue('MPU6050InputEnabled'), // replace with validateSelectionWhenValue
};

export const MPU6050State = {
	MPU6050InputEnabled: 0,
	MPU6050i2cSDAPin: -1,
	MPU6050i2cSCLPin: -1,
	MPU6050i2cBlock: 0,
	MPU6050i2cSpeed: 400000,
	MPU6050i2cAddress: 0x68,
};

const MPU6050 = ({ values, errors, handleChange, handleCheckbox }) => {
    const { t } = useTranslation();
    return (
        <Section title={t('AddonsConfig:mpu6050-header-text')}>
            <div
				id="MPU6050AddonOptions"
				hidden={!values.MPU6050InputEnabled}
			>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:mpu6050-sda-pin-label')}
						name="MPU6050i2cSDAPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.MPU6050i2cSDAPin}
						error={errors.MPU6050i2cSDAPin}
						isInvalid={errors.MPU6050i2cSDAPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:mpu6050-scl-pin-label')}
						name="MPU6050i2cSCLPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.MPU6050i2cSCLPin}
						error={errors.MPU6050i2cSCLPin}
						isInvalid={errors.MPU6050i2cSCLPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormSelect
						label={t('AddonsConfig:mpu6050-i2c-block-label')}
						name="MPU6050i2cBlock"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.MPU6050i2cBlock}
						error={errors.MPU6050i2cBlock}
						isInvalid={errors.MPU6050i2cBlock}
						onChange={handleChange}
						>
							{I2C_BLOCKS.map((o, i) => (
								<option key={`i2cBlock-option-${i}`} value={o.value}>
									{o.label}
								</option>
							))}
					</FormSelect>
					<FormControl
						label={t('AddonsConfig:mpu6050-i2c-speed-label')}
						name="MPU6050i2cSpeed"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.MPU6050i2cSpeed}
						error={errors.MPU6050i2cSpeed}
						isInvalid={errors.MPU6050i2cSpeed}
						onChange={handleChange}
						min={100000}
						max={400000}
					/>
				</Row>
				<Row className="mb-3">
					<FormControl
						label={t('AddonsConfig:mpu6050-i2c-address-label')}
						name="MPU6050i2cAddress"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.MPU6050i2cAddress}
						error={errors.MPU6050i2cAddress}
						isInvalid={errors.MPU6050i2cAddress}
						onChange={handleChange}
						maxLength={4}
					/>
				</Row>
            </div>
			<FormCheck
				label={t('Common:switch-enabled')}
				type="switch"
				id="MPU6050InputButton"
				reverse
				isInvalid={false}
				checked={Boolean(values.MPU6050InputEnabled)}
				onChange={(e) => {
					handleCheckbox('MPU6050InputEnabled', values);
					handleChange(e);
				}}
			/>
        </Section>
    );
};

export default MPU6050;