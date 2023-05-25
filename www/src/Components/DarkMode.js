import React, { useState, useEffect } from 'react';
import { Dropdown } from 'react-bootstrap';

const storedTheme = localStorage.getItem('theme')
const dropdownOptions = [
	{ theme: 'light', icon: 'bi-sun-fill', label: 'Light' },
	{ theme: 'dark', icon: 'bi-moon-stars-fill', label: 'Dark' },
	{ theme: 'auto', icon: 'bi-circle-half', label: 'Auto' },
];

const getPreferredTheme = () => {
	if (storedTheme) {
		return storedTheme;
	}

	return 'auto';
}

const setTheme = function (theme) {
	if (theme === 'auto' && window.matchMedia('(prefers-color-scheme: dark)').matches) {
		document.documentElement.setAttribute('data-bs-theme', 'dark');
	} else {
		document.documentElement.setAttribute('data-bs-theme', theme);
	}

	localStorage.setItem('theme', theme);
}

setTheme(getPreferredTheme());

const DarkMode = () => {
	const [theme, setThemeState] = useState(getPreferredTheme());

	const setThemeAndState = (newTheme) => {
		setThemeState(newTheme);
		setTheme(newTheme);
	}

	useEffect(() => {
		setThemeState(getPreferredTheme());
	}, []);

	return (
		<Dropdown>
			<Dropdown.Toggle variant="secondary" style={{ marginRight: "7px" }}>
				<i class="bi-moon-stars-fill"></i>
			</Dropdown.Toggle>

			<Dropdown.Menu>
				{dropdownOptions.map((option) => (
					<Dropdown.Item
						key={option.theme}
						className={`dropdown-item ${theme === option.theme ? 'active' : ''}`}
						onClick={() => setThemeAndState(option.theme)}
					>
						<i className={option.icon}></i> {option.label}
					</Dropdown.Item>
				))}
			</Dropdown.Menu>
		</Dropdown>
	);
};

export default DarkMode;
