import { AppContext } from '../Contexts/AppContext';
import { useContext } from 'react';
import { Trans, useTranslation } from 'react-i18next';
import { FormCheck, FormLabel, Row } from 'react-bootstrap';
import FormSelect from '../Components/FormSelect';
import FormControl from '../Components/FormControl';
import { NavLink } from 'react-router-dom';
import * as yup from 'yup';

import Section from '../Components/Section';
import { AddonPropTypes } from '../Pages/AddonsConfigPage';
// import Form4BitSwitches from '../Components/FormBitConfig'


export const i2cAnalogScheme = {
	I2CAnalog1115InputEnabled: yup.number().required().label('I2C Analog1115 Input Enabled'),
	// channelEnabled: yup
	// 	.number()
	// 	.label('Channel Enabled')
	// 	.validateRangeWhenValue('I2CAnalog1115InputEnabled', 0b0000, 0b1111),
	// channelInnerDeadzoneEnable: yup
	// 	.number()
	// 	.label('Channel Inner Deadzone Enable')
	// 	.validateRangeWhenValue('I2CAnalog1115InputEnabled', 0b0000, 0b1111),
	// channelOuterDeadzoneEnable: yup
	// 	.number()
	// 	.label('Channel Outer Deadzone Enable')
	// 	.validateRangeWhenValue('I2CAnalog1115InputEnabled', 0b0000, 0b1111),
	// invertEnabled: yup
	// 	.number()
	// 	.label('Invert Enable by-channel')
	// 	.validateRangeWhenValue('I2CAnalog1115InputEnabled', 0b0000, 0b1111),
	// lxChannel: yup
	// 	.number()
	// 	.label('Analog Left Stick X-axis Channel Number')
	// 	.validateRangeWhenValue('I2CAnalog1115InputEnabled', 0, 3),
	// lyChannel: yup
	// 	.number()
	// 	.label('Analog Left Stick Y-axis Channel Number')
	// 	.validateRangeWhenValue('I2CAnalog1115InputEnabled', 0, 3),
	// rxChannel: yup
	// 	.number()
	// 	.label('Analog Right Stick X-axis Channel Number')
	// 	.validateRangeWhenValue('I2CAnalog1115InputEnabled', 0, 3),
	// ryChannel: yup
	// 	.number()
	// 	.label('Analog Right Stick Y-axis Channel Number')
	// 	.validateRangeWhenValue('I2CAnalog1115InputEnabled', 0, 3),
};

export const i2cAnalogState = {
	I2CAnalog1115InputEnabled: 0,
	// channelEnabled: 0b1111,
	// channelInnerDeadzoneEnable: 0b1111,
	// channelOuterDeadzoneEnable: 0b1111,
	// invertEnabled: 0b0000,
	lxChannel: 0,
	lyChannel: 1,
	rxChannel: 2,
	ryChannel: 3,
	channel0InnerDeadzone: 2,
	channel1InnerDeadzone: 2,
	channel2InnerDeadzone: 2,
	channel3InnerDeadzone: 2,
	channel0OuterDeadzone: 98,
	channel1OuterDeadzone: 98,
	channel2OuterDeadzone: 98,
	channel3OuterDeadzone: 98,
	leftStickDeadzoneEnable: 0,
	rightStickDeadzoneEnable: 0,
	leftStickDeadzone: 5,
	rightStickDeadzone: 5,
};

const I2CAnalog1115 = ({ values, errors, handleChange, handleCheckbox, setFieldValue }: AddonPropTypes) => {
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
				href="https://gp2040-ce.info/add-ons/i2c-analog-ads1115"
				target="_blank"
				className="text-reset text-decoration-none"
			>
				{t('AddonsConfig:i2c-analog-ads1115-header-text')}
			</a>
		}
		>
			<div
				id="I2CAnalog1115InputOptions"
				hidden={
					!(values.I2CAnalog1115InputEnabled && getAvailablePeripherals('i2c'))
				}
			>
				<div className="alert alert-info" role="alert">
					The SDA and SCL pins and Speed are configured in <a href="../peripheral-mapping" className="alert-link">Peripheral Mapping</a>
				</div>

			</div>
			<div
				id="I2CAnalog1115Options"
				hidden={
					!values.I2CAnalog1115InputEnabled
				}
			>
				<Row className="mb-3">
					WIP: Enable/Disable Channels

					{/* {values.channelEnabled} */}
				</Row>

				<Row className="mb-3">
					WIP: Channel-to-Analog
					<div className="mb-3">
						LX: Channel A{values.lxChannel}
					</div>
					<div className="mb-3">
						LY: Channel A{values.lyChannel}
					</div>
					<div className="mb-3">
						RX: Channel A{values.rxChannel}
					</div>
					<div className="mb-3">
						RY: Channel A{values.ryChannel}
					</div>
				</Row>

				<Row className="mb-3">
					{/* 
					<FormSelect
						label="Left Stick X-axis Channel"
						name="lxChannel"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.lxChannel}
						error={errors.lxChannel}
						isInvalid={Boolean(errors.lxChannel)}
						onChange={handleChange}
					>
						{[0, 1, 2, 3].map((i) => (
							<option key={`analog1115Pins-lx-option-${i}`} value={i}>
								Channel A{i}
							</option>
						))}
					</FormSelect>

					<FormSelect
						label="Left Stick Y-axis Channel"
						name="lyChannel"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.lyChannel}
						error={errors.lyChannel}
						isInvalid={Boolean(errors.lyChannel)}
						onChange={handleChange}
					>
						{[0, 1, 2, 3].map((i) => (
							<option key={`analog1115Pins-ly-option-${i}`} value={i}>
								Channel A{i}
							</option>
						))}
					</FormSelect>
					<FormSelect
						label="Right Stick X-axis Channel"
						name="rxChannel"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.rxChannel}
						error={errors.rxChannel}
						isInvalid={Boolean(errors.rxChannel)}
						onChange={handleChange}
					>
						{[0, 1, 2, 3].map((i) => (
							<option key={`analog1115Pins-rx-option-${i}`} value={i}>
								Channel A{i}
							</option>
						))}
					</FormSelect>
					<FormSelect
						label="Right Stick Y-axis Channel"
						name="ryChannel"
						className="form-select-sm"
						groupClassName="col-sm-2 mb-3"
						value={values.ryChannel}
						error={errors.ryChannel}
						isInvalid={Boolean(errors.ryChannel)}
						onChange={handleChange}					>
						{[0, 1, 2, 3].map((i) => (
							<option key={`analog1115Pins-ry-option-${i}`} value={i}>
								Channel A{i}
							</option>
						))}
					</FormSelect> */}
				</Row>

				<Row className="mb-3">
					WIP: By-Analog Sticks deadzones
					<Row>
						LStick Deadzone Enable: {values.leftStickDeadzoneEnable}
						<br />
						{values.leftStickDeadzone}
						<br />
						LStick Deadzone Enable: {values.rightStickDeadzoneEnable}
						<br />
						{values.rightStickDeadzone}
					</Row>
				</Row>

				<Row className="mb-3">
					WIP: By-Channel deadzones
				</Row>

			</div>
			{getAvailablePeripherals('i2c') ? (
				<FormCheck
					label={t('Common:switch-enabled')}
					type="switch"
					id="I2CAnalog1115InputButton"
					reverse
					isInvalid={false}
					checked={
						Boolean(values.I2CAnalog1115InputEnabled) &&
						getAvailablePeripherals('i2c')
					}
					onChange={(e) => {
						handleCheckbox('I2CAnalog1115InputEnabled');
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

export default I2CAnalog1115;
