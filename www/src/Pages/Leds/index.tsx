import { useEffect, useState } from 'react';

import LightCoordsSection from './LightCoordsSection';
import AnimationSection from './AnimationSection';
import useLedStore from '../../Store/useLedStore';
import { Alert, Form } from 'react-bootstrap';

export default function Leds() {
	const { fetchLedOptions, loading, initialized } = useLedStore();
	const hasLights = useLedStore((state) => state.Lights.length > 0);
	const [advancedMode, setAdvancedMode] = useState(false);

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
			<Form.Check
				type="switch"
				label="Advanced mode"
				checked={advancedMode}
				disabled={!hasLights}
				onChange={(e) => setAdvancedMode(e.target.checked)}
			/>
			<hr />
			{!advancedMode ? (
				<AnimationSection />
			) : (
				<>
					<Alert variant="warning">
						Changing advanced configuration options can break your LED setup.
						Proceed with caution.
					</Alert>
					<AnimationSection advanced />
					<LightCoordsSection />
				</>
			)}
		</>
	);
}
