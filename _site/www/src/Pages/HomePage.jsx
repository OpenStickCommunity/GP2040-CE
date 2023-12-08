import React, { useEffect, useState, useContext } from 'react';
import { AppContext } from '../Contexts/AppContext';
import axios from 'axios';
import orderBy from 'lodash/orderBy';
import { useTranslation } from 'react-i18next';

import Section from '../Components/Section';

import WebApi from '../Services/WebApi';

const percentage = (x, y) => ((x / y) * 100).toFixed(2);
const toKB = (x) => parseFloat((x / 1024).toFixed(2));

export default function HomePage() {
	const [latestVersion, setLatestVersion] = useState('');
	const [latestDownloadUrl, setLatestDownloadUrl] = useState('');
	const [currentVersion, setCurrentVersion] = useState(
		import.meta.env.VITE_CURRENT_VERSION,
	);
	const [boardConfigProperties, setBoardConfigProperties] = useState({});
	const [memoryReport, setMemoryReport] = useState(null);

	const { t } = useTranslation('');

	const { setLoading } = useContext(AppContext);

	useEffect(() => {
		WebApi.getFirmwareVersion(setLoading)
			.then(({ version, boardConfigLabel, boardConfigFileName, boardConfig }) => {
				setCurrentVersion(version);
				setBoardConfigProperties({ label: boardConfigLabel, fileName: boardConfigFileName});
				axios.get('https://api.github.com/repos/OpenStickCommunity/GP2040-CE/releases/latest')
					.then((response) => {
						const latestTag = response.data.tag_name;
						setLatestDownloadUrl(
							response.data?.assets?.find(({ name }) => {
								return name?.substring(name.lastIndexOf('_') + 1)
									?.replace('.uf2', '')
									?.toLowerCase() === boardConfig.toLowerCase()
							})?.browser_download_url || `https://github.com/OpenStickCommunity/GP2040-CE/releases/tag/${latestTag}`
						);
					})
					.catch(console.error)
			})
			.catch(console.error);

		WebApi.getMemoryReport(setLoading)
			.then((response) => {
				const unit = 1024;
				const { totalFlash, usedFlash, staticAllocs, totalHeap, usedHeap } =
					response;
				setMemoryReport({
					totalFlash: toKB(totalFlash),
					usedFlash: toKB(usedFlash),
					staticAllocs: toKB(staticAllocs),
					totalHeap: toKB(totalHeap),
					usedHeap: toKB(usedHeap),
					percentageFlash: percentage(usedFlash, totalFlash),
					percentageHeap: percentage(usedHeap, totalHeap),
				});
			})
			.catch(console.error);
	}, []);

	return (
		<div>
			<h1>{t('HomePage:header-text')}</h1>
			<p>{t('HomePage:sub-header-text')}</p>
			<Section title={t('HomePage:system-stats-header-text')}>
				<div>
					<div>
						<strong>{t('HomePage:version-text')}</strong>
					</div>
					<div>{`${boardConfigProperties.label} (${boardConfigProperties.fileName}.uf2)`}</div>
					<div>{t('HomePage:current-text', { version: currentVersion })}</div>
					<div>{t('HomePage:latest-text', { version: latestVersion })}</div>
					{latestVersion && currentVersion !== latestVersion && (
						<div className="mt-3 mb-3">
							<a
								target="_blank"
								rel="noreferrer"
								href={latestDownloadUrl}
								className="btn btn-primary"
							>
								{t('HomePage:get-update-text')}
							</a>
						</div>
					)}
					{memoryReport && (
						<div>
							<strong>{t('HomePage:memory-header-text')}</strong>
							<div>
								{t('HomePage:memory-flash-text')}: {memoryReport.usedFlash} /{' '}
								{memoryReport.totalFlash} ({memoryReport.percentageFlash}%)
							</div>
							<div>
								{t('HomePage:memory-heap-text')}: {memoryReport.usedHeap} /{' '}
								{memoryReport.totalHeap} ({memoryReport.percentageHeap}%)
							</div>
							<div>
								{t('HomePage:memory-static-allocations-text')}:{' '}
								{memoryReport.staticAllocs}
							</div>
						</div>
					)}
				</div>
			</Section>
		</div>
	);
}
