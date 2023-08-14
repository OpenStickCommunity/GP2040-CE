import React, { useEffect, useState, useContext } from 'react';
import { AppContext } from '../Contexts/AppContext';
import axios from 'axios';
import orderBy from 'lodash/orderBy';
import { useTranslation } from 'react-i18next';

import Section from '../Components/Section';

import WebApi from '../Services/WebApi';

const percentage = (x, y) => (x / y * 100).toFixed(2)
const toKB = (x) => parseFloat((x / 1024).toFixed(2))
let loading = true;

export default function HomePage() {
	const [latestVersion, setLatestVersion] = useState('');
	const [latestTag, setLatestTag] = useState('');
	const [currentVersion, setCurrentVersion] = useState(import.meta.env.VITE_CURRENT_VERSION);
	const [memoryReport, setMemoryReport] = useState(null);

	const { t } = useTranslation('');

	const { setLoading } = useContext(AppContext);

	useEffect(() => {
		WebApi.getFirmwareVersion(setLoading).then(response => {
			setCurrentVersion(response.version);
		})
			.catch(console.error);

		WebApi.getMemoryReport(setLoading).then(response => {
			const unit = 1024;
			const { totalFlash, usedFlash, staticAllocs, totalHeap, usedHeap } = response;
			setMemoryReport({
				totalFlash: toKB(totalFlash),
				usedFlash: toKB(usedFlash),
				staticAllocs: toKB(staticAllocs),
				totalHeap: toKB(totalHeap),
				usedHeap: toKB(usedHeap),
				percentageFlash: percentage(usedFlash, totalFlash),
				percentageHeap: percentage(usedHeap, totalHeap)
			});
		})
			.catch(console.error);

		axios.get('https://api.github.com/repos/OpenStickCommunity/GP2040-CE/releases')
			.then((response) => {
				// Filter out pre-releases
				response.data = response.data.filter((release) => !release.prerelease);

				const sortedData = orderBy(response.data, 'published_at', 'desc');
				setLatestVersion(sortedData[0].name);
				setLatestTag(sortedData[0].tag_name);
			})
			.catch(console.error);
	}, []);

	return (
		<div>
			<h1>{t('HomePage:header-text')}</h1>
			<p>{t('HomePage:sub-header-text')}</p>
			<Section title={t('HomePage:system-stats-header-text')}>
				<div>
					<div><strong>{t('HomePage:version-text')}</strong></div>
					<div>{t('HomePage:current-text', { version: currentVersion })}</div>
					<div>{t('HomePage:latest-text', { version: latestVersion })}</div>
					{(latestVersion && currentVersion !== latestVersion) &&
						<div className="mt-3 mb-3">
							<a
								target="_blank"
								rel="noreferrer"
								href={`https://github.com/OpenStickCommunity/GP2040-CE/releases/tag/${latestTag}`}
								className="btn btn-primary"
							>{t('HomePage:get-update-text')}</a>
						</div>
					}
					{memoryReport &&
						<div>
							<strong>{t('HomePage:memory-header-text')}</strong>
							<div>{t('HomePage:memory-flash-text')}: {memoryReport.usedFlash} / {memoryReport.totalFlash} ({memoryReport.percentageFlash}%)</div>
							<div>{t('HomePage:memory-heap-text')}: {memoryReport.usedHeap} / {memoryReport.totalHeap} ({memoryReport.percentageHeap}%)</div>
							<div>{t('HomePage:memory-static-allocations-text')}: {memoryReport.staticAllocs}</div>
						</div>
					}
				</div>
			</Section>
		</div>
	);
}
