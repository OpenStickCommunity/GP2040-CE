import { useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import { ProgressBar } from 'react-bootstrap';

import useSystemStats from '../Store/useSystemStats';
import Section from '../Components/Section';

export default function HomePage() {
	const { t } = useTranslation('');
	const {
		latestVersion,
		latestDownloadUrl,
		currentVersion,
		boardConfigProperties,
		memoryReport,
		getSystemStats,
		loading,
	} = useSystemStats();

	useEffect(() => {
		getSystemStats();
	}, []);

	if (loading) {
		return (
			<div className="d-flex justify-content-center align-items-center loading">
				<div className="spinner-border" role="status">
					<span className="visually-hidden">{t('Common:loading-text')}</span>
				</div>
			</div>
		);
	}
	return (
		<div>
			<h1>{t('HomePage:header-text')}</h1>
			<p>{t('HomePage:sub-header-text')}</p>
			<Section title={t('HomePage:system-stats-header-text')}>
				<div>
					<strong className="system-text">{t('HomePage:version-text')}</strong>
					<div className="system-text">{`${boardConfigProperties.label} (${boardConfigProperties.fileName}.uf2)`}</div>
					<div className="system-text">
						{t('HomePage:current-text', { version: currentVersion })}
					</div>
					<div className="system-text">
						{t('HomePage:latest-text', { version: latestVersion })}
					</div>
					{latestVersion &&
						currentVersion?.split('-').length == 1 &&
						currentVersion !== latestVersion && (
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

					<strong className="system-text">
						{t('HomePage:memory-header-text')}
					</strong>
					<div className="system-text">
						{t('HomePage:memory-flash-text')}: {memoryReport.usedFlash} /{' '}
						{memoryReport.totalFlash} ({memoryReport.percentageFlash}%)
					</div>
					<div className="system-text">
						{t('HomePage:memory-heap-text')}: {memoryReport.usedHeap} /{' '}
						{memoryReport.totalHeap} ({memoryReport.percentageHeap}%)
					</div>
					<div className="system-text">
						{t('HomePage:memory-static-allocations-text')}:{' '}
						{memoryReport.staticAllocs}
					</div>
					<div>
						{t('HomePage:memory-board-text')}: {memoryReport.physicalFlash}
					</div>

					<ProgressBar
						className="my-1 system-text w-50"
						now={memoryReport.percentageFlash}
						label={`${t('HomePage:memory-flash-text')} ${
							memoryReport.percentageFlash
						}%`}
					/>
					<ProgressBar
						className="my-1 system-text w-50"
						now={memoryReport.percentageHeap}
						label={`${t('HomePage:memory-heap-text')} ${
							memoryReport.percentageHeap
						}%`}
					/>
				</div>
			</Section>
		</div>
	);
}
