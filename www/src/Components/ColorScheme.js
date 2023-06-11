import React, { useContext } from 'react';
import { Dropdown } from 'react-bootstrap';
import { AppContext } from '../Contexts/AppContext';


const dropdownOptions = [
	{ scheme: 'light', icon: 'bi-sun-fill', label: 'Light' },
	{ scheme: 'dark', icon: 'bi-moon-stars-fill', label: 'Dark' },
	{ scheme: 'auto', icon: 'bi-circle-half', label: 'Auto' },
];

const setTheme = function (theme) {
	const rootElement = document.documentElement;
	const prefersDarkMode = window.matchMedia('(prefers-color-scheme: dark)').matches;

	if (theme === 'auto') {
		rootElement.setAttribute('data-bs-theme', prefersDarkMode ? 'dark' : 'light');
	} else {
		rootElement.setAttribute('data-bs-theme', theme);
	}
}

const ColorScheme = () => {
	const { savedColorScheme, setSavedColorScheme } = useContext(AppContext);

	setTheme(savedColorScheme);

	const setThemeAndState = (newColorScheme) => {
		setTheme(newColorScheme);
		setSavedColorScheme(newColorScheme);
	}

	return (
		<Dropdown>
			<Dropdown.Toggle variant="secondary" style={{ marginRight: "7px" }}>
				<i class="bi-moon-stars-fill"></i>
			</Dropdown.Toggle>

			<Dropdown.Menu>
				{dropdownOptions.map((option) => (
					<Dropdown.Item
						key={option.theme}
						className={`dropdown-item ${savedColorScheme === option.scheme ? 'active' : ''}`}
						onClick={() => setThemeAndState(option.scheme)}
					>
						<i className={option.icon}></i> {option.label}
					</Dropdown.Item>
				))}
			</Dropdown.Menu>
		</Dropdown>
	);
};

export default ColorScheme;
