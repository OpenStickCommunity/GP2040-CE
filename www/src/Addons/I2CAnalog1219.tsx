import { AppContext } from '../Contexts/AppContext';
import { useContext } from 'react';
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
};

export const i2cAnalogState = {
	I2CAnalog1219InputEnabled: 0,
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
			></div>
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
