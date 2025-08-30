import { AppContext } from '../Contexts/AppContext';
import { useContext, useEffect, useState } from 'react';
import { Trans, useTranslation } from 'react-i18next';
import { FormCheck, Row, FormLabel } from 'react-bootstrap';
import { NavLink } from 'react-router-dom';
import * as yup from 'yup';

import Section from '../Components/Section';
import FormControl from '../Components/FormControl';
import FormSelect from '../Components/FormSelect';
import boards from '../Data/Boards.json';
import { SPI_BLOCKS } from '../Data/Peripherals';
import WebApi from '../Services/WebApi';

export const analog1256Scheme = {
	Analog1256Enabled: yup.number().label('Analog1256 Input Enabled'),
	analog1256Block: yup
		.number()
		.label('Analog1256 Block')
		.validateSelectionWhenValue('Analog1256Enabled', SPI_BLOCKS),
	analog1256CsPin: yup.number().label('Analog1256 CS Pin'),
	analog1256DrdyPin: yup
		.number()
		.label('Analog1256 DRDY Pin')
		.validatePinWhenValue('Analog1256Enabled'),
};

export const analog1256State = {
	Analog1256Enabled: 0,
	analog1256Block: 0,
	analog1256CsPin: -1,
	analog1256DrdyPin: -1,
	analog1256AnalogMax: 3.3,
	analog1256EnableTriggers: false,
};

const Analog1256 = ({ values, errors, handleChange, handleCheckbox }) => {
	const {
		getAvailablePeripherals,
		getSelectedPeripheral,
		setLoading,
		usedPins,
	} = useContext(AppContext);
	const [csPins, setCsPins] = useState([]);

	const { t } = useTranslation();

	const handlePeripheralChange = (e) => {
		let device = getSelectedPeripheral('spi', e.target.value);
		handleChange(e);
	};

	const getAvailableCsPins = async (spiBlock: number) => {
		const csPins = [];

		const peripheralOptions = await WebApi.getPeripheralOptions(setLoading);
		if (
			peripheralOptions.peripheral[`spi${spiBlock}`] &&
			peripheralOptions.peripheral[`spi${spiBlock}`].cs > -1
		)
			csPins.push({
				pin: peripheralOptions.peripheral[`spi${spiBlock}`].cs,
				hwcs: true,
			});

		const availablePins = [
			...Array(boards[import.meta.env.VITE_GP2040_BOARD].maxPin + 1).keys(),
		].filter((p) => (usedPins || []).indexOf(p) === -1); // Filter out used pins

		csPins.push(...availablePins.map((pin) => ({ pin, hwcs: false })));

		return csPins;
	};

	useEffect(() => {
		async function fetchData() {
			const csPins = await getAvailableCsPins(values.analog1256Block);
			setCsPins(csPins);
		}

		fetchData();
	}, [usedPins]);

	return (
		<Section title={t('AddonsConfig:analog1256-header-text')}>
			<div
				id="Analog1256InputOptions"
				hidden={!(values.Analog1256Enabled && getAvailablePeripherals('spi'))}
			>
				<Row className="mb-3">
					{getAvailablePeripherals('spi') ? (
						<FormSelect
							label={t('AddonsConfig:analog1256-block-label')}
							name="analog1256Block"
							className="form-select-sm"
							groupClassName="col-sm-3 mb-3"
							value={values.analog1256Block}
							error={errors.analog1256Block}
							isInvalid={errors.analog1256Block}
							onChange={handlePeripheralChange}
						>
							{getAvailablePeripherals('spi').map((o, i) => (
								<option key={`spiBlock-option-${i}`} value={o.value}>
									{o.label}
								</option>
							))}
						</FormSelect>
					) : (
						''
					)}
					<FormSelect
						label={t('AddonsConfig:analog1256-cs-pin')}
						name="analog1256CsPin"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analog1256CsPin}
						error={errors.analog1256CsPin}
						isInvalid={errors.analog1256CsPin}
						onChange={handleChange}
					>
						{csPins.map((p, i) => (
							<option key={`cs-pin-select-${i}`} value={p.pin}>
								{p.pin}
								{p.hwcs ? ' (HW)' : ''}
							</option>
						))}
					</FormSelect>
					<FormControl
						label={t('AddonsConfig:analog1256-drdy-pin')}
						name="analog1256DrdyPin"
						className="form-control-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analog1256DrdyPin}
						error={errors.analog1256DrdyPin}
						isInvalid={errors.analog1256DrdyPin}
						onChange={handleChange}
						maxLength={2}
					/>
					<FormSelect
						label={t('AddonsConfig:analog1256-analog-max')}
						name="analog1256AnalogMax"
						className="form-select-sm"
						groupClassName="col-sm-3 mb-3"
						value={values.analog1256AnalogMax}
						error={errors.analog1256AnalogMax}
						isInvalid={errors.analog1256AnalogMax}
						onChange={handleChange}
					>
						<option value="3.3">{'3.3v'}</option>
						<option value="5.0">{'5v'}</option>
					</FormSelect>
				</Row>
				<Row>
					<FormCheck
						label={t('AddonsConfig:analog1256-enable-triggers')}
						type="switch"
						id="analog1256EnableTriggers"
						className="col-sm-3 ms-2"
						isInvalid={false}
						checked={Boolean(values.analog1256EnableTriggers)}
						onChange={(e) => {
							handleCheckbox('analog1256EnableTriggers', values);
							handleChange(e);
						}}
					/>
				</Row>
			</div>
			{getAvailablePeripherals('spi') ? (
				<FormCheck
					label={t('Common:switch-enabled')}
					type="switch"
					id="Analog1256Button"
					reverse
					isInvalid={false}
					checked={
						Boolean(values.Analog1256Enabled) && getAvailablePeripherals('spi')
					}
					onChange={(e) => {
						handleCheckbox('Analog1256Enabled', values);
						handleChange(e);
					}}
				/>
			) : (
				<FormLabel>
					<Trans
						ns="PeripheralMapping"
						i18nKey="peripheral-toggle-unavailable"
						values={{ name: 'SPI' }}
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

export default Analog1256;
