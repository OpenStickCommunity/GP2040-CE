// @ts-check
// Note: type annotations allow type checking and IDEs autocompletion

const lightCodeTheme = require('prism-react-renderer').themes.github;
const darkCodeTheme = require('prism-react-renderer').themes.dracula;

/** @type {import('@docusaurus/types').Config} */
const config = {
	title: 'GP2040-CE',
	tagline: 'Community Edition Firmware',
	favicon: 'img/favicon.ico',

	// Set the production url of your site here
	url: 'https://gp2040-ce.info',
	// Set the /<baseUrl>/ pathname under which your site is served
	// For GitHub pages deployment, it is often '/<projectName>/'
	baseUrl: '/',

	// GitHub pages deployment config.
	// If you aren't using GitHub pages, you don't need these.
	organizationName: 'OpenStickCommunity', // Usually your GitHub org/user name.
	projectName: 'GP2040-CE', // Usually your repo name.

	onBrokenLinks: 'throw',
	onBrokenMarkdownLinks: 'warn',

	// Even if you don't use internalization, you can use this field to set useful
	// metadata like html lang. For example, if your site is Chinese, you may want
	// to replace "en" with "zh-Hans".
	i18n: {
		defaultLocale: 'en',
		locales: ['en'],
	},

	presets: [
		[
			'classic',
			/** @type {import('@docusaurus/preset-classic').Options} */
			({
				docs: {
					sidebarPath: require.resolve('./sidebars.js'),
					routeBasePath: '/',
					// Please change this to your repo.
					// Remove this to remove the "edit this page" links.
					// editUrl:
					//   '',
				},
				blog: false,
				theme: {
					customCss: require.resolve('./src/css/custom.css'),
				},
			}),
		],
	],
	themes: [
		[
			'@easyops-cn/docusaurus-search-local',
			{
				hashed: true,
				highlightSearchTermsOnTargetPage: true,
				docsRouteBasePath: '/',
			},
		],
	],
	themeConfig:
		/** @type {import('@docusaurus/preset-classic').ThemeConfig} */
		({
			announcementBar: {
				id: 'new_release',
				content:
					'New Version Available! To get the v0.7.4 update, go to <a href="https://github.com/OpenStickCommunity/GP2040-CE/releases/tag/v0.7.4">GP2040-CE Releases<a/>',
				backgroundColor: '#ec008c',
				textColor: '#FFFFFF',
				isCloseable: true,
			},
			// Replace with your project's social card
			// image: 'img/social-card.jpg',
			navbar: {
				title: 'GP2040-CE | Community Edition Firmware',
				logo: {
					alt: 'GP2040-CE Logo',
					src: 'img/gp2040-ce-logo.svg',
				},
				items: [
					{
						type: 'docSidebar',
						position: 'left',
						sidebarId: 'docSidebar',
						label: 'Docs',
					},
					{
						type: 'docSidebar',
						position: 'left',
						sidebarId: 'webConfigSidebar',
						label: 'Web Configurator',
					},
					{
						type: 'docSidebar',
						position: 'left',
						sidebarId: 'devSidebar',
						label: 'Development',
					},
					{
						type: 'doc',
						position: 'left',
						docId: 'downloads/download-page',
						label: 'Downloads',
					},
					{
						type: 'docsVersionDropdown',
						position: 'right',
						dropdownActiveClassDisabled: true,
					},
					{
						href: 'https://discord.gg/k2pxhke7q8',
						label: 'Discord',
						position: 'right',
					},
					{
						href: 'https://github.com/OpenStickCommunity/GP2040-CE',
						label: 'GitHub',
						position: 'right',
					},
				],
			},
			docs: {
				sidebar: {
					hideable: true,
					autoCollapseCategories: false,
				},
			},
			// footer: {

			// },
			prism: {
				theme: lightCodeTheme,
				darkTheme: darkCodeTheme,
			},
		}),
};

module.exports = config;
