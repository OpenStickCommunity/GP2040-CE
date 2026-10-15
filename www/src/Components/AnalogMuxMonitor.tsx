import { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Button, ProgressBar } from 'react-bootstrap';

import WebApi from '../Services/WebApi';

type MuxRaw = {
	error?: string;
	z?: number;
	s?: number[];
	values?: number[];
};

const ADC_MAX = 4095;
const ADC_REFERENCE_VOLTAGE = 3.3;

// Live raw 12-bit reading of every 74HC4051 channel, using the saved settings
const AnalogMuxMonitor = () => {
	const { t } = useTranslation();
	const [running, setRunning] = useState(false);
	const [data, setData] = useState<MuxRaw>({});

	useEffect(() => {
		if (!running) return;
		let cancelled = false;
		const poll = async () => {
			const result = await WebApi.getAnalogMuxRaw();
			if (!cancelled) setData(result);
		};
		poll();
		const timer = setInterval(poll, 300);
		return () => {
			cancelled = true;
			clearInterval(timer);
		};
	}, [running]);

	return (
		<div className="mb-3">
			<h5>{t('AddonsConfig:analog-monitor-title')}</h5>
			<div className="alert alert-secondary" role="alert">
				{t('AddonsConfig:analog-monitor-description')}
			</div>
			<Button
				variant={running ? 'danger' : 'success'}
				size="sm"
				className="mb-3"
				onClick={() => setRunning(!running)}
			>
				{running
					? t('AddonsConfig:analog-monitor-stop')
					: t('AddonsConfig:analog-monitor-start')}
			</Button>
			{data.s && data.z !== undefined && (
				<div className="mb-2 small">
					{`S0 / S1 / S2 / Z: GP${data.s[0]} / GP${data.s[1]} / GP${data.s[2]} / GP${data.z}`}
				</div>
			)}
			{data.error && (
				<div className="alert alert-warning" role="alert">
					{data.error}
				</div>
			)}
			{data.values &&
				data.values.map((value, channel) => (
					<div key={`analogMuxMonitor-${channel}`} className="mb-1">
						<div className="small">
							{`Y${channel}: ${value}  (${((value / ADC_MAX) * ADC_REFERENCE_VOLTAGE).toFixed(2)} V)`}
						</div>
						<ProgressBar now={value} max={ADC_MAX} />
					</div>
				))}
		</div>
	);
};

export default AnalogMuxMonitor;
