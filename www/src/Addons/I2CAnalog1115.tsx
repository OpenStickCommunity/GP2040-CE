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
	channel0InnerDeadzone: 5,
	channel1InnerDeadzone: 5,
	channel2InnerDeadzone: 5,
	channel3InnerDeadzone: 5,
	channel0OuterDeadzone: 95,
	channel1OuterDeadzone: 95,
	channel2OuterDeadzone: 95,
	channel3OuterDeadzone: 95,
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
					</FormSelect>
				</Row>

				<Row className="mb-3">
					WIP: By-Analog Sticks deadzones
					<Row>
						{/* LStick Deadzone Enable: {values.leftStickDeadzoneEnable} */}
						<FormCheck
							label="Left Stick Deadzone Enable"
							type="switch"
							id="ADS1115LeftStickDeadzoneEnable"
							className="col-sm-3 ms-3"
							isInvalid={false}
							checked={Boolean(values.leftStickDeadzoneEnable)}
							onChange={(e) => {
								handleCheckbox('leftStickDeadzoneEnable');
								handleChange(e);
							}}
						/>

						{/* {values.leftStickDeadzone} */}
						<FormControl
							type="number"
							label="Left Stick Deadzone"
							name="leftStickDeadzone"
							className="form-control-sm"
							groupClassName="col-sm-3 mb-3"
							value={values.leftStickDeadzone}
							error={errors.leftStickDeadzone}
							isInvalid={Boolean(errors.leftStickDeadzone)}
							onChange={handleChange}
							min={0}
							max={100}
						/>
					</Row>

					<Row>
						{/* RStick Deadzone Enable: {values.rightStickDeadzoneEnable} */}
						<FormCheck
							label="Right Stick Deadzone Enable"
							type="switch"
							id="ADS1115RightStickDeadzoneEnable"
							className="col-sm-3 ms-3"
							isInvalid={false}
							checked={Boolean(values.rightStickDeadzoneEnable)}
							onChange={(e) => {
								handleCheckbox('rightStickDeadzoneEnable');
								handleChange(e);
							}}
						/>
						{/* {values.rightStickDeadzone} */}
						<FormControl
							type="number"
							label="Right Stick Deadzone"
							name="rightStickDeadzone"
							className="form-control-sm"
							groupClassName="col-sm-3 mb-3"
							value={values.rightStickDeadzone}
							error={errors.rightStickDeadzone}
							isInvalid={Boolean(errors.rightStickDeadzone)}
							onChange={handleChange}
							min={0}
							max={100}
						/>
					</Row>
				</Row>

				<Row className="mb-3">
					WIP: By-Channel deadzones
					<Row>
						<br />
						<FormControl
							type="number"
							label="Channel A0 Inner Deadzone"
							name="channel0InnerDeadzone"
							className="form-control-sm"
							groupClassName="col-sm-3 mb-3"
							value={values.channel0InnerDeadzone}
							error={errors.channel0InnerDeadzone}
							isInvalid={Boolean(errors.channel0InnerDeadzone)}
							onChange={handleChange}
							min={0}
							max={100}
						/>
						<br />
						<FormControl
							type="number"
							label="Channel A1 Inner Deadzone"
							name="channel1InnerDeadzone"
							className="form-control-sm"
							groupClassName="col-sm-3 mb-3"
							value={values.channel1InnerDeadzone}
							error={errors.channel1InnerDeadzone}
							isInvalid={Boolean(errors.channel1InnerDeadzone)}
							onChange={handleChange}
							min={0}
							max={100}
						/>
						<br />
						<FormControl
							type="number"
							label="Channel A2 Inner Deadzone"
							name="channel2InnerDeadzone"
							className="form-control-sm"
							groupClassName="col-sm-3 mb-3"
							value={values.channel2InnerDeadzone}
							error={errors.channel2InnerDeadzone}
							isInvalid={Boolean(errors.channel2InnerDeadzone)}
							onChange={handleChange}
							min={0}
							max={100}
						/>
						<br />
						<FormControl
							type="number"
							label="Channel A3 Inner Deadzone"
							name="channel3InnerDeadzone"
							className="form-control-sm"
							groupClassName="col-sm-3 mb-3"
							value={values.channel3InnerDeadzone}
							error={errors.channel3InnerDeadzone}
							isInvalid={Boolean(errors.channel3InnerDeadzone)}
							onChange={handleChange}
							min={0}
							max={100}
						/>
					</Row>

					<Row>
						<br />
						<FormControl
							type="number"
							label="Channel A0 Outer Deadzone"
							name="channel0OuterDeadzone"
							className="form-control-sm"
							groupClassName="col-sm-3 mb-3"
							value={values.channel0OuterDeadzone}
							error={errors.channel0OuterDeadzone}
							isInvalid={Boolean(errors.channel0OuterDeadzone)}
							onChange={handleChange}
							min={0}
							max={100}
						/>
						<br />
						<FormControl
							type="number"
							label="Channel A1 Outer Deadzone"
							name="channel1OuterDeadzone"
							className="form-control-sm"
							groupClassName="col-sm-3 mb-3"
							value={values.channel1OuterDeadzone}
							error={errors.channel1OuterDeadzone}
							isInvalid={Boolean(errors.channel1OuterDeadzone)}
							onChange={handleChange}
							min={0}
							max={100}
						/>
						<br />
						<FormControl
							type="number"
							label="Channel A2 Outer Deadzone"
							name="channel2OuterDeadzone"
							className="form-control-sm"
							groupClassName="col-sm-3 mb-3"
							value={values.channel2OuterDeadzone}
							error={errors.channel2OuterDeadzone}
							isInvalid={Boolean(errors.channel2OuterDeadzone)}
							onChange={handleChange}
							min={0}
							max={100}
						/>
						<br />
						<FormControl
							type="number"
							label="Channel A3 Outer Deadzone"
							name="channel3OuterDeadzone"
							className="form-control-sm"
							groupClassName="col-sm-3 mb-3"
							value={values.channel3OuterDeadzone}
							error={errors.channel3OuterDeadzone}
							isInvalid={Boolean(errors.channel3OuterDeadzone)}
							onChange={handleChange}
							min={0}
							max={100}
						/>
					</Row>
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
