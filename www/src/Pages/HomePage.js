import React, { useEffect, useState } from 'react';
import axios from 'axios';
import { orderBy } from 'lodash';

import Section from '../Components/Section';

import WebApi from '../Services/WebApi';

export default function HomePage() {
	const [latestVersion, setLatestVersion] = useState('');
	const [latestTag, setLatestTag] = useState('');
	const [currentVersion, setCurrentVersion] = useState(process.env.REACT_APP_CURRENT_VERSION);

	useEffect(() => {
		WebApi.getFirmwareVersion().then(response => {
			setCurrentVersion(response.version);
		})
		.catch(console.error);
		
		axios.get('https://api.github.com/repos/OpenStickFoundation/GP2040-CE/releases')
			.then((response) => {
				const sortedData = orderBy(response.data, 'published_at', 'desc');
				setLatestVersion(sortedData[0].name);
				setLatestTag(sortedData[0].tag_name);
			})
			.catch(console.error);
	}, [setLatestVersion, setLatestTag]);

	return (
		<div>
			<h1>Welcome to the GP2040-CE Web Configurator!</h1>
			<p>Please select a menu option to proceed.</p>
			<Section title="Firmware Version">
				<div className="card-body">
					<div className="card-text">Current Version: { currentVersion }</div>
					<div className="card-text">Latest Version: { latestVersion }</div>
					{(latestVersion && currentVersion !== latestVersion) ?
						<div className="mt-3">
							<a
								target="_blank"
								rel="noreferrer"
								href={`https://github.com/OpenStickFoundation/GP2040-CE/releases/tag/${latestTag}`}
								className="btn btn-primary"
							>
								Get Latest Version
							</a>
						</div>
					: null}
				</div>
			</Section>
		</div>
	);
}
