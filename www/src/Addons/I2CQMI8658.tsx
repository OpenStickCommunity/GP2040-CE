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

export const i2cQMI8658Scheme = {
	I2CQMI8658InputEnabled: yup.number().label('I2C QMI8658 Input Enabled'),
};

export const i2cQMI8658State = {
	I2CQMI8658InputEnabled: 0,
};

const I2CQMI8658 = ({ values, errors, handleChange, handleCheckbox }) => {
	const { t } = useTranslation();
	const { getAvailablePeripherals, getSelectedPeripheral } =
		useContext(AppContext);

	const handlePeripheralChange = (e) => {
		let device = getSelectedPeripheral('i2c', e.target.value);
		handleChange(e);
	};

	return (
		<Section title={t('AddonsConfig:i2c-qmi8658-header-text')}>
			<div
				id="I2CQMI8658InputOptions"
				hidden={
					!(values.I2CQMI8658InputEnabled && getAvailablePeripherals('i2c'))
				}
			>
			</div>
			{getAvailablePeripherals('i2c') ? (
				<FormCheck
					label={t('Common:switch-enabled')}
					type="switch"
					id="I2CQMI8658InputButton"
					reverse
					isInvalid={false}
					checked={
						Boolean(values.I2CQMI8658InputEnabled) &&
						getAvailablePeripherals('i2c')
					}
					onChange={(e) => {
						handleCheckbox('I2CQMI8658InputEnabled', values);
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

export default I2CQMI8658;
