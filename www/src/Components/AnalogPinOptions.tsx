import { useContext, useEffect, useState } from 'react';
import { AppContext } from '../Contexts/AppContext';
import { useTranslation } from 'react-i18next';

import useBoardDefinition from '../Store/useBoardDefinitionStore';

// 74HC4051 mux channels Y0-Y7 are stored as 100-107 (they are not GPIOs)
export const ANALOG_MUX_PIN_BASE = 100;
export const ANALOG_MUX_CHANNELS = 8;

export const isAnalogMuxPin = (pin: number) =>
	pin >= ANALOG_MUX_PIN_BASE && pin < ANALOG_MUX_PIN_BASE + ANALOG_MUX_CHANNELS;

type AnalogPinOptionsProps = {
	// also offer the mux channels Y0-Y7 next to the ADC pins
	mux?: boolean;
};

const AnalogPinOptions = ({ mux = false }: AnalogPinOptionsProps) => {
	const { usedPins } = useContext(AppContext);
	const { t } = useTranslation();

	const { boardDefinition, getBoardDefinition } = useBoardDefinition();

	useEffect(() => {
		getBoardDefinition();
	}, []);

	const ANALOG_PINS = boardDefinition.analogPins;

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
			{mux &&
				Array.from({ length: ANALOG_MUX_CHANNELS }, (_, channel) => (
					<option
						key={`analogMuxPins-option-${channel}`}
						value={ANALOG_MUX_PIN_BASE + channel}
					>
						{`Y${channel} (mux)`}
					</option>
				))}
		</>
	);
};

export default AnalogPinOptions;
