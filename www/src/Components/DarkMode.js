import React, { useState, useEffect } from 'react';

const storedTheme = localStorage.getItem('theme')

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
		<div className="dropdown">
			<button className="btn btn-secondary dropdown-toggle" type="button" id="dropdownMenuButton1" data-bs-toggle="dropdown" aria-expanded="false">
				<i class="bi-moon-stars-fill"></i>
			</button>

			<ul className="dropdown-menu">
				<li>
					<a
						className={`dropdown-item ${theme === 'light' ? 'active' : ''}`}
						href="#"
						onClick={() => setThemeAndState('light')}
					>
						<i className="bi-sun-fill"></i> Light
					</a>
				</li>
				<li>
					<a
						className={`dropdown-item ${theme === 'dark' ? 'active' : ''}`}
						href="#"
						onClick={() => setThemeAndState('dark')}
					>
						<i className="bi-moon-stars-fill"></i> Dark
					</a>
				</li>
				<li>
					<a
						className={`dropdown-item ${theme === 'auto' ? 'active' : ''}`}
						href="#"
						onClick={() => setThemeAndState('auto')}
					>
						<i className="bi-circle-half"></i> Auto
					</a>
				</li>
			</ul>
		</div>
	);
};

export default DarkMode;
