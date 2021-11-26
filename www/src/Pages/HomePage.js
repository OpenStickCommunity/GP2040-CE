import React, { useEffect, useState } from 'react';
import axios from 'axios';
import { orderBy } from 'lodash';

import Section from '../Components/Section';

const currentVersion = process.env.REACT_APP_CURRENT_VERSION;

export default function HomePage() {
	const [latestVersion, setLatestVersion] = useState('');

	useEffect(() => {
		axios.get('https://api.github.com/repos/FeralAI/GP2040/releases')
			.then((response) => {
				const sortedData = orderBy(response.data, 'published_at', 'desc');
				setLatestVersion(sortedData[0].name);
			})
			.catch(console.error);
	}, [setLatestVersion]);

	return (
		<div>
			<h1>Welcome to the GP2040 Web Configurator!</h1>
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
								href={`https://github.com/FeralAI/GP2040/releases/${latestVersion}`}
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
