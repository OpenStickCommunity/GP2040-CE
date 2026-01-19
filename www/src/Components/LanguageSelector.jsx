import { useContext, useEffect } from 'react';
import { Dropdown, DropdownButton } from 'react-bootstrap';
import { AppContext } from '../Contexts/AppContext';
import { useTranslation } from 'react-i18next';
import GlobeIcon from '../Icons/Globe';
import UsFlag from '../Icons/Flags/Us';
import zhCNFlag from '../Icons/Flags/zhCN';
import ptBRFlag from '../Icons/Flags/ptBR';
import deDEFlag from '../Icons/Flags/De';
import jaJPFlag from '../Icons/Flags/jaJP';
import krKOFlag from '../Icons/Flags/koKR';
import esMXFlag from '../Icons/Flags/esMX';
import trTRFlag from '../Icons/Flags/trTR';
import frFRFlag from '../Icons/Flags/fr';

const dropdownOptions = [
	{ code: 'en', icon: UsFlag },
	{ code: 'pt-BR', icon: ptBRFlag },
	{ code: 'zh-CN', icon: zhCNFlag },
	{ code: 'de-DE', icon: deDEFlag },
	{ code: 'ja-JP', icon: jaJPFlag },
	{ code: 'ko-KR', icon: krKOFlag },
	{ code: 'es-MX', icon: esMXFlag },
	{ code: 'tr-TR', icon: trTRFlag },
	{ code: 'fr-FR', icon: frFRFlag },
];

const LanguageSelector = () => {
	const { savedLanguage, setSavedLanguage } = useContext(AppContext);
	const { i18n, t } = useTranslation('Components');

	useEffect(() => {
		if (!dropdownOptions.some((o) => o.code === i18n.language)) {
			setSavedLanguage(dropdownOptions[0].code);
		} else {
			setSavedLanguage(i18n.language);
		}
	}, [i18n.language, setSavedLanguage]);

	const setLanguage = (newLanguage) => {
		i18n.changeLanguage(newLanguage);
	};

	const setLanguageAndState = (newLanguage) => {
		setLanguage(newLanguage);
		setSavedLanguage(newLanguage);
	};

	return (
		<DropdownButton variant="secondary" align="end" title={<GlobeIcon />}>
			{dropdownOptions.map((option) => (
				<Dropdown.Item
					key={option.code}
					className={`dropdown-item ${
						savedLanguage === option.code ? 'active' : ''
					}`}
					onClick={() => setLanguageAndState(option.code)}
				>
					<option.icon /> {t(`language-selector.${option.code}`)}
				</Dropdown.Item>
			))}
		</DropdownButton>
	);
};

export default LanguageSelector;
