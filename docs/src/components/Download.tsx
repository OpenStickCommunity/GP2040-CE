import React from 'react';
import styles from './download.module.css';
import LinkIcon from './LinkIcon';
import boards from '../config/boards';

const rootRepoURL =
	'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/';
const releaseVersion = '0.7.4';
const downloadURL =
	rootRepoURL + 'v' + releaseVersion + '/GP2040-CE_' + releaseVersion + '_';

function CreateDownloadLink({ configName }) {
	if (configName) {
		return downloadURL + configName + '.uf2';
	} else {
		return null;
	}
}

const RenderDownloadBox = ({
	name,
	configName,
	pinout,
	desc,
	image,
	website,
}) => (
	<div key={`download-${name}`} className={styles.box}>
		<img className={styles.boxImage} src={image} />
		<div className={styles.boxInfo}>
			<h3 className={styles.boxTitle}>{name}</h3>
			<div>
				<ul className={styles.downloadLinks}>
					<LinkIcon
						link={CreateDownloadLink({ configName })}
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

const Container = ({ children }) => (
	<div className={styles.row}>{children}</div>
);

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
