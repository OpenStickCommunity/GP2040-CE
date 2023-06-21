import React, { useContext } from 'react';
import { Dropdown } from 'react-bootstrap';
import { AppContext } from '../Contexts/AppContext';
import { useTranslation } from 'react-i18next';
import GlobeIcon from "../Icons/Globe";
import GbFlag from "../Icons/Flags/Gb";
import NlFlag from "../Icons/Flags/Nl";
import UsFlag from "../Icons/Flags/Us";

const dropdownOptions = [
	{ code: 'en', icon: UsFlag },
	{ code: 'en-GB', icon: GbFlag },
	{ code: 'nl', icon: NlFlag },
];

const LanguageSelector = () => {
	const { savedLanguage, setSavedLanguage } = useContext(AppContext);
	const { i18n, t } = useTranslation('Components');

	// If i18n.language not in dropdownOptions, set it to the first option
	if (!dropdownOptions.some(o => o.code === i18n.language)) {
		setSavedLanguage(dropdownOptions[0].code);
	} else {
		setSavedLanguage(i18n.language);
	}

	const setLanguage = (newLanguage) => {
		i18n.changeLanguage(newLanguage);
	};

	const setLanguageAndState = (newLanguage) => {
		setLanguage(newLanguage);
		setSavedLanguage(newLanguage);
	};

	return (
		<Dropdown>
			<Dropdown.Toggle variant="secondary" style={{ marginRight: "7px" }}>
				<GlobeIcon />
			</Dropdown.Toggle>

			<Dropdown.Menu>
				{dropdownOptions.map((option) => (
					<Dropdown.Item
						key={option.code}
						className={`dropdown-item ${savedLanguage === option.code ? 'active' : ''}`}
						onClick={() => setLanguageAndState(option.code)}
					>
						<option.icon /> {t(`language-selector.${option.code}`)}
					</Dropdown.Item>
				))}
			</Dropdown.Menu>
		</Dropdown>
	);
};

export default LanguageSelector;
