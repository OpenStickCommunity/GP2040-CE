import React from 'react';
import { useTranslation } from 'react-i18next';
import { Alert, FormCheck, Row } from 'react-bootstrap';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormSelect from '../Components/FormSelect';

import FormControl from '../Components/FormControl';

export const MPU6050Scheme = {
    MPU6050InputEnabled: yup.number().label('MPU6050 Input Enabled'),
};

export const MPU6050State = {
    MPU6050InputEnabled: 0,
};

const MPU6050 = ({ values, errors, handleChange, handleCheckbox }) => {
    const { t } = useTranslation();
    return (
        <Section title={t('AddonsConfig:mpu6050-header-text')}>
            <div id="MPU6050AddonOptions" hidden={!values.MPU6050InputEnabled}>
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