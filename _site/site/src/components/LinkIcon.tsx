import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import React from 'react';
import {
	faFileDownload,
	faGamepad,
	faGlobe,
	faX,
} from '@fortawesome/free-solid-svg-icons';
import { icon, text } from '@fortawesome/fontawesome-svg-core';

const icons = {
	Download: faFileDownload,
	Pinout: faGamepad,
	Website: faGlobe,
} as const;

type LinkIconTypes = {
	link: string;
	text: keyof typeof icons;
};

const LinkIcon = ({ link, text }: LinkIconTypes) => {
	if(link === null) {
		return null;
	}	else{ 
		return(
			<li>
				<a href={link}>
					<FontAwesomeIcon icon={icons[text]} />
					{text}
				</a>
			</li>
		)
	}
}

export default LinkIcon;
