import React, { useContext } from 'react';
import { Dropdown } from 'react-bootstrap';
import { useTranslation } from 'react-i18next';

import { AppContext } from '../Contexts/AppContext';
import SunIcon from '../Icons/Sun';
import MoonStarsIcon from '../Icons/MoonStars';
import CircleHalfIcon from '../Icons/CircleHalf';

const dropdownOptions = [
	{ scheme: 'light', icon: SunIcon },
	{ scheme: 'dark', icon: MoonStarsIcon },
	{ scheme: 'auto', icon: CircleHalfIcon },
];

const setTheme = function (theme) {
	const rootElement = document.documentElement;
	const prefersDarkMode = window.matchMedia(
		'(prefers-color-scheme: dark)',
	).matches;

	if (theme === 'auto') {
		rootElement.setAttribute(
			'data-bs-theme',
			prefersDarkMode ? 'dark' : 'light',
		);
	} else {
		rootElement.setAttribute('data-bs-theme', theme);
	}
};

const ColorScheme = () => {
	const { savedColorScheme, setSavedColorScheme } = useContext(AppContext);

	const { t } = useTranslation('');

	const translatedDropdownOptions = dropdownOptions.map((option) => ({
		...option,
		label: t(`Components:color-scheme.${option.scheme}`),
	}));

	setTheme(savedColorScheme);

	const setThemeAndState = (newColorScheme) => {
		setTheme(newColorScheme);
		setSavedColorScheme(newColorScheme);
	};

	return (
		<Dropdown>
			<Dropdown.Toggle variant="secondary" style={{ marginRight: '7px' }}>
				<MoonStarsIcon />
			</Dropdown.Toggle>

			<Dropdown.Menu>
				{translatedDropdownOptions.map((option) => (
					<Dropdown.Item
						key={option.scheme}
						as={'button'}
						className={savedColorScheme === option.scheme ? 'active' : ''}
						onClick={() => setThemeAndState(option.scheme)}
					>
						<option.icon /> {option.label}
					</Dropdown.Item>
				))}
			</Dropdown.Menu>
		</Dropdown>
	);
};

export default ColorScheme;
