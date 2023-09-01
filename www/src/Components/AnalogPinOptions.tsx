import React from 'react';
import { useTranslation } from 'react-i18next';
import { ANALOG_PINS } from '../Data/Buttons';

const AnalogPinOptions = () => {
	const { t } = useTranslation();
	return (
		<>
			<option value={-1}>
				{t('AddonsConfig:analog-available-pins-option-not-set')}
			</option>
			{ANALOG_PINS.map((i) => (
				<option key={`analogPins-option-${i}`} value={i}>
					{i}
				</option>
			))}
		</>
	);
};

export default AnalogPinOptions;
