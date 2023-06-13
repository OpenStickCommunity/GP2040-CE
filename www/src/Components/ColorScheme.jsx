import React, { useContext } from "react";
import { Dropdown } from "react-bootstrap";
import { AppContext } from "../Contexts/AppContext";
import SunIcon from "../Icons/Sun";
import MoonStarsIcon from "../Icons/MoonStars";
import CircleHalfIcon from "../Icons/CircleHalf";

const dropdownOptions = [
	{ scheme: "light", icon: SunIcon, label: "Light" },
	{ scheme: "dark", icon: MoonStarsIcon, label: "Dark" },
	{ scheme: "auto", icon: CircleHalfIcon, label: "Auto" },
];

const setTheme = function (theme) {
	const rootElement = document.documentElement;
	const prefersDarkMode = window.matchMedia(
		"(prefers-color-scheme: dark)"
	).matches;

	if (theme === "auto") {
		rootElement.setAttribute(
			"data-bs-theme",
			prefersDarkMode ? "dark" : "light"
		);
	} else {
		rootElement.setAttribute("data-bs-theme", theme);
	}
};

const ColorScheme = () => {
	const { savedColorScheme, setSavedColorScheme } = useContext(AppContext);

	setTheme(savedColorScheme);

	const setThemeAndState = (newColorScheme) => {
		setTheme(newColorScheme);
		setSavedColorScheme(newColorScheme);
	};

	return (
		<Dropdown>
			<Dropdown.Toggle variant="secondary" style={{ marginRight: "7px" }}>
				<MoonStarsIcon />
			</Dropdown.Toggle>

			<Dropdown.Menu>
				{dropdownOptions.map((option) => (
					<Dropdown.Item
						key={option.theme}
						as={"button"}
						className={savedColorScheme === option.scheme ? "active" : ""}
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
