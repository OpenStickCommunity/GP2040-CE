import { AppContext } from '../Contexts/AppContext';
import { useContext } from 'react';
import { Trans, useTranslation } from 'react-i18next';
import { FormCheck, FormLabel } from 'react-bootstrap';
import { NavLink } from 'react-router-dom';
import * as yup from 'yup';

import Section from '../Components/Section';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';

export const i2cAnalogScheme = {
	I2CAnalog1219InputEnabled: yup.number().label('I2C Analog1219 Input Enabled'),
};

export const i2cAnalogState = {
	I2CAnalog1219InputEnabled: 0,
};

const I2CAnalog1219 = ({ values, errors, handleChange, handleCheckbox }: AddonPropTypes) => {
	const { t } = useTranslation();
	const { getAvailablePeripherals, getSelectedPeripheral } =
		useContext(AppContext);

	const handlePeripheralChange = (e) => {
		let device = getSelectedPeripheral('i2c', e.target.value);
		handleChange(e);
	};

	return (
		<Section title={
			<a
				href="https://gp2040-ce.info/add-ons/i2c-analog-ads1219"
				target="_blank"
				className="text-reset text-decoration-none"
			>
				{t('AddonsConfig:i2c-analog-ads1219-header-text')}
			</a>
		}
		>
			<div
				id="I2CAnalog1219InputOptions"
				hidden={
					!(values.I2CAnalog1219InputEnabled && getAvailablePeripherals('i2c'))
				}
			>
				<div className="alert alert-info" role="alert">
					The SDA and SCL pins and Speed are configured in <a href="../peripheral-mapping" className="alert-link">Peripheral Mapping</a>
				</div>
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
						handleCheckbox('I2CAnalog1219InputEnabled');
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
