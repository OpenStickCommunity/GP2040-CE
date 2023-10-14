import React, { useEffect } from 'react';
import styles from './download.module.css';
import LinkIcon from './LinkIcon';
import boards from '../config/boards';
import { useGitHubInfo } from '../hooks/useGitHubInfo';

const createDownloadLink = ({ releaseVersion, configName }) =>
	releaseVersion
		? `https://github.com/OpenStickCommunity/GP2040-CE/releases/download/${releaseVersion}/GP2040-CE_${releaseVersion.substring(
				1
		  )}_${configName}.uf2`
		: '';

const RenderDownloadBox = ({
	name,
	configName,
	pinout,
	desc,
	image,
	website,
}) => {
	const releaseVersion = useGitHubInfo((state) => state.releaseVersion);
	return (
		<div key={`download-${name}`} className={styles.box}>
			<img className={styles.boxImage} src={image} />
			<div className={styles.boxInfo}>
				<h3 className={styles.boxTitle}>{name}</h3>
				<div>
					<ul className={styles.downloadLinks}>
						<LinkIcon
							link={createDownloadLink({ releaseVersion, configName })}
							text={'Download'}
						/>
						<LinkIcon link={pinout} text={'Pinout'} />
						<LinkIcon link={website} text={'Website'} />
					</ul>
				</div>
				<div className="download-desc">{desc()}</div>
			</div>
		</div>
	);
};

const Container = ({ children }) => {
	const getInfo = useGitHubInfo((state) => state.getInfo);
	useEffect(() => {
		getInfo();
	}, []);
	return <div className={styles.row}>{children}</div>;
};

export const Microcontrollers = () => (
	<Container>
		{boards['Microcontroller Boards'].map(RenderDownloadBox)}
	</Container>
);

export const OfficialDevices = () => (
	<Container>
		{boards['Community Devices']
			.filter((config) => config.category === 'official')
			.map(RenderDownloadBox)}
	</Container>
);

export const OpenSourceDevices = () => (
	<Container>
		{boards['Community Devices']
			.filter((config) => config.category === 'open')
			.map(RenderDownloadBox)}
	</Container>
);

export const ClosedSourceDevices = () => (
	<Container>
		{boards['Community Devices']
			.filter((config) => config.category === 'closed')
			.map(RenderDownloadBox)}
	</Container>
);

export const LegacyDevices = () => (
	<Container>
		{boards['Community Devices']
			.filter((config) => config.category === 'legacy')
			.map(RenderDownloadBox)}
	</Container>
);
