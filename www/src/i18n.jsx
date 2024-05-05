import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import LanguageDetector from 'i18next-browser-languagedetector';
import en from './Locales/en/Index';
import zhCN from './Locales/zh-CN/Index';
import ptBR from './Locales/pt-BR/Index';
import deDE from './Locales/de-DE/Index';

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
			order: ['localStorage', 'navigator'],
		},
		resources: { en, 'pt-BR': ptBR, 'zh-CN': zhCN, 'de-DE': deDE },
	});

export default i18n;
