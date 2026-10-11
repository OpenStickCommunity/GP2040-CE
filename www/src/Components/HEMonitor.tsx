import { useEffect, useRef, useState } from 'react';
import { Alert, Button } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';
import invert from 'lodash/invert';

import WebApi from '../Services/WebApi';
import { BUTTON_ACTIONS } from '../Data/Pins';

import './HECalibration.scss';

const POLL_INTERVAL_MS = 80;
const TRAVEL_MAX = 1000;

type MonitorChannel = {
	id: number;
	travel: number;
	active: boolean;
	rapidTrigger: boolean;
	actuationPoint: number;
};

type MonitorStatus = {
	monitoring: boolean;
	activeProfile: number;
	channels: MonitorChannel[];
	error?: string;
};

type Props = {
	muxChannels: number;
	actionForChannel: (channel: number) => number;
};

const actionLabel = (actionId: number) => {
	const label = invert(BUTTON_ACTIONS)[actionId];
	return label ? label.replace('BUTTON_PRESS_', '') : `#${actionId}`;
};

// Live test view. Shows how deep each switch is pressed and whether it is
// currently sending its action, so a user can sanity check a calibration -- and
// see rapid trigger working -- without running the full wizard.
const HEMonitor = ({ muxChannels, actionForChannel }: Props) => {
	const { t } = useTranslation('');
	const [running, setRunning] = useState(false);
	const [status, setStatus] = useState<MonitorStatus | null>(null);
	const [error, setError] = useState('');
	const timerId = useRef<ReturnType<typeof setTimeout> | undefined>(undefined);
	// Set false to stop the self-scheduling poll loop. A ref rather than state so
	// the running loop sees the change immediately.
	const pollingRef = useRef(false);

	const stopPolling = () => {
		pollingRef.current = false;
		if (timerId.current) {
			clearTimeout(timerId.current);
			timerId.current = undefined;
		}
	};

	// Leaving the device in monitor mode would keep it suppressing input, so make
	// sure it is stopped if the user navigates away.
	useEffect(
		() => () => {
			stopPolling();
			WebApi.stopHEMonitor().catch(() => {});
		},
		[],
	);

	const start = async () => {
		setError('');
		try {
			const result = await WebApi.startHEMonitor();
			if (result?.error) {
				setError(result.error);
				return;
			}
			setRunning(true);
			stopPolling();
			pollingRef.current = true;

			// Poll sequentially rather than on a fixed interval. The device serves one
			// HTTP request at a time, so a setInterval that fires faster than the board
			// can answer stacks overlapping requests -- and a collided response comes
			// back truncated, which surfaces as a JSON parse error. Waiting for each
			// response before scheduling the next keeps at most one in flight.
			const loop = async () => {
				if (!pollingRef.current) return;
				try {
					const data: MonitorStatus = await WebApi.getHEMonitorStatus();
					if (!pollingRef.current) return;
					if (data?.error) {
						setError(data.error);
						stopPolling();
						return;
					}
					setStatus(data);
				} catch (e) {
					setError(String(e));
					stopPolling();
					return;
				}
				if (pollingRef.current) {
					timerId.current = setTimeout(loop, POLL_INTERVAL_MS);
				}
			};
			loop();
		} catch (e) {
			setError(String(e));
		}
	};

	const stop = async () => {
		stopPolling();
		setRunning(false);
		setStatus(null);
		try {
			await WebApi.stopHEMonitor();
		} catch {
			// best effort; the firmware also times out on its own
		}
	};

	const channelLocation = (channel: number) =>
		muxChannels > 1
			? `M${Math.floor(channel / muxChannels)}:C${channel % muxChannels}`
			: `ADC ${channel}`;

	return (
		<div className="mt-4">
			<h1>{t('HETrigger:monitor-header')}</h1>
			<div className="text-muted mb-2">{t('HETrigger:monitor-intro')}</div>

			{error && <Alert variant="danger">{error}</Alert>}

			<div className="d-flex gap-2 mb-3">
				<Button type="button" onClick={running ? stop : start}>
					{running ? t('HETrigger:monitor-stop') : t('HETrigger:monitor-start')}
				</Button>
			</div>

			{running && (
				<>
					<div className="he-monitor-legend mb-2">
						<span className="he-legend-swatch he-legend-active" />
						{t('HETrigger:monitor-legend-active')}
						<span className="he-legend-swatch he-legend-idle" />
						{t('HETrigger:monitor-legend-idle')}
						<span className="he-legend-rt">⚡</span>
						{t('HETrigger:monitor-legend-rt')}
					</div>

					<div className="he-monitor-grid">
						{(status?.channels || []).map((channel) => {
							const percent = Math.max(
								0,
								Math.min(100, (channel.travel / TRAVEL_MAX) * 100),
							);
							return (
								<div
									key={`monitor-${channel.id}`}
									className={`he-monitor-tile ${channel.active ? 'active' : ''}`}
								>
									<div className="he-monitor-head">
										<span className="he-monitor-name">
											{actionLabel(actionForChannel(channel.id))}
										</span>
										{channel.rapidTrigger && (
											<span
												className="he-monitor-rt"
												title={t('HETrigger:monitor-legend-rt')}
											>
												⚡
											</span>
										)}
									</div>

									{/* Depth bar. The marker is the actuation point, so it is
									    obvious whether the switch crossed it. */}
									<div className="he-monitor-bar">
										<div
											className={`he-monitor-fill ${channel.active ? 'active' : ''}`}
											style={{ height: `${percent}%` }}
										/>
										<div
											className="he-monitor-actuation"
											style={{ bottom: `${channel.actuationPoint}%` }}
										/>
									</div>

									<div className="he-monitor-meta">
										{channelLocation(channel.id)}
									</div>
									<div className="he-monitor-meta he-monitor-depth">
										{Math.round(percent)}%
									</div>
								</div>
							);
						})}
					</div>

					{status && status.channels.length === 0 && (
						<Alert variant="warning">
							{t('HETrigger:monitor-none-assigned')}
						</Alert>
					)}
				</>
			)}
		</div>
	);
};

export default HEMonitor;
