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
	MPU6050SDAPin: yup
		.number()
		.label('MPU6050 SDA Pin')
		.validatePinWhenValue('MPU6050InputEnabled'),
	MPU6050SCLPin: yup
		.number()
		.label('MPU6050 SCL Pin')
		.validatePinWhenValue('MPU6050InputEnabled'),
};

export const MPU6050State = {
    MPU6050InputEnabled: 0,
	MPU6050SDAPin: -1,
	MPU6050SCLPin: -1,
};

const MPU6050 = ({ values, errors, handleChange, handleCheckbox }) => {
    const { t } = useTranslation();
    return (
        <Section title={t('AddonsConfig:mpu6050-header-text')}>
            <div id="MPU6050AddonOptions" hidden={!values.MPU6050InputEnabled}>
				<Row className="mb-3">
					<FormControl
						type="number"
						label={t('AddonsConfig:mpu6050-sda-pin-label')}
						name="MPU6050SDAPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.MPU6050SDAPin}
						error={errors.MPU6050SDAPin}
						isInvalid={errors.MPU6050SDAPin}
						onChange={handleChange}
						min={-1}
						max={29}
					/>
					<FormControl
						type="number"
						label={t('AddonsConfig:mpu6050-scl-pin-label')}
						name="MPU6050SCLPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.MPU6050SCLPin}
						error={errors.MPU6050SCLPin}
						isInvalid={errors.MPU6050SCLPin}
						onChange={handleChange}
						min={-1}
						max={29}
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