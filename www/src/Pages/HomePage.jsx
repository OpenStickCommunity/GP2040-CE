import React, { useEffect, useState } from 'react';
import axios from 'axios';
import orderBy from 'lodash/orderBy';

import Section from '../Components/Section';

import WebApi from '../Services/WebApi';

const percentage = (x, y) => (x / y * 100).toFixed(2)
const toKB = (x) => parseFloat((x / 1024).toFixed(2))

export default function HomePage() {
	const [latestVersion, setLatestVersion] = useState('');
	const [latestTag, setLatestTag] = useState('');
	const [currentVersion, setCurrentVersion] = useState(import.meta.env.VITE_CURRENT_VERSION);
	const [memoryReport, setMemoryReport] = useState(null);

	useEffect(() => {
		WebApi.getFirmwareVersion().then(response => {
			setCurrentVersion(response.version);
		})
		.catch(console.error);

		WebApi.getMemoryReport().then(response => {
			const unit = 1024;
			const {totalFlash, usedFlash, staticAllocs, totalHeap, usedHeap} = response;
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
			<h1>Welcome to the GP2040-CE Web Configurator!</h1>
			<p>Please select a menu option to proceed.</p>
			<Section title="System Stats">
				<div>
					<div><strong>Version</strong></div>
					<div>Current: { currentVersion }</div>
					<div>Latest: { latestVersion }</div>
					{(latestVersion && currentVersion !== latestVersion) &&
						<div className="mt-3 mb-3">
							<a
								target="_blank"
								rel="noreferrer"
								href={`https://github.com/OpenStickCommunity/GP2040-CE/releases/tag/${latestTag}`}
								className="btn btn-primary"
							>Get Latest Version</a>
						</div>
					}
					{memoryReport &&
						<div>
							<strong>Memory (KB)</strong>
							<div>Flash: {memoryReport.usedFlash} / {memoryReport.totalFlash} ({memoryReport.percentageFlash}%)</div>
							<div>Heap: {memoryReport.usedHeap} / {memoryReport.totalHeap} ({memoryReport.percentageHeap}%)</div>
							<div>Static Allocations: {memoryReport.staticAllocs}</div>
						</div>
					}
				</div>
			</Section>
		</div>
	);
}
