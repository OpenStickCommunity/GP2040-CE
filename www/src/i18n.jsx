import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import LanguageDetector from 'i18next-browser-languagedetector';
import en from './Locales/en/Index';
import enGB from './Locales/en-GB/Index';
import nl from './Locales/nl/Index';

i18n
	.use(LanguageDetector)
	.use(initReactI18next)
	.init({
		debug: true,
		fallbackLng: 'en',
		interpolation: {
			escapeValue: true,
		},
		detection: {
			order: ['localStorage', 'navigator']
		},
		resources: {en, 'en-GB': enGB, nl}
	});

export default i18n;
