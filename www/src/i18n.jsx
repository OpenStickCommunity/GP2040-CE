import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import LanguageDetector from 'i18next-browser-languagedetector';
import en from './Locales/en/Index';
import zhCN from './Locales/zh-CN/Index';
import ptBR from './Locales/pt-BR/Index';
import deDE from './Locales/de-DE/Index';
import jaJP from './Locales/ja-JP/Index';
import koKR from './Locales/ko-KR/Index';
import esMX from './Locales/es-MX/Index';
import trTR from './Locales/tr-TR/Index';
import frFR from './Locales/fr-FR/Index';

i18n
	.use(LanguageDetector)
	.use(initReactI18next)
	.init({
		debug: true,
		fallbackLng: 'en',
		returnEmptyString: false,
		interpolation: {
			escapeValue: true,
		},
		detection: {
			order: ['localStorage', 'navigator'],
		},
		resources: {
			en,
			'pt-BR': ptBR,
			'zh-CN': zhCN,
			'de-DE': deDE,
			'ja-JP': jaJP,
			'ko-KR': koKR,
			'es-MX': esMX,
			'tr-TR': trTR,
			'fr-FR': frFR,			
		},
	});

export default i18n;