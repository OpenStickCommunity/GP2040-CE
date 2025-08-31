import { useEffect } from 'react';

import LightCoordsSection from './LightCoordsSection';
import LedsSections from './LedsSections';
import useLedStore from '../Store/useLedStore';

export default function Leds() {
	const { fetchLedOptions, loading, initialized } = useLedStore();

	useEffect(() => {
		fetchLedOptions();
	}, []);

	if (loading || !initialized) {
		return (
			<div className="d-flex justify-content-center">
				<span className="spinner-border"></span>
			</div>
		);
	}

	return (
		<>
			<LedsSections />
			<LightCoordsSection />
		</>
	);
}
