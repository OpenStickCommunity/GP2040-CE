/**
 * Creating a sidebar enables you to:
 - create an ordered group of docs
 - render a sidebar for each doc of that group
 - provide next/previous navigation

 The sidebars can be generated from the filesystem, or explicitly defined here.

 Create as many sidebars as you want.
 */

// @ts-check

/** @type {import('@docusaurus/plugin-content-docs').SidebarsConfig} */
const sidebars = {
  // By default, Docusaurus generates a sidebar from the docs folder structure
  // tutorialSidebar: [{type: 'autogenerated', dirName: '.'}],

  // But you can create a sidebar manually
  sidebar: [
    {
      type: 'doc',
      id: 'home',
    },
    {
      type: 'doc',
      id: 'download'
    },
    {
      type: 'category',
      label: 'General',
      items: [ 
        'introduction',
        'installation',
        'usage',
        'hotkeys',
        'rgb-leds',
        'getting-help-support'
      ]
    },
    {
      type: 'category',
      label: 'FAQ',
      items: [ 
        'faq/faq',
        'faq/faq-ps4-ps5-compatibility',
      ]
    },
    {
      type: 'category',
      label: 'Web Configurator',
      items: [ 
        'web-configurator',
        {
          type: 'category',
          label: 'Add-Ons',
          items: [ 
            {type: 'autogenerated', dirName: 'add-ons'},
          ]
        },
      ]
    },
    {
      type: 'category',
      label: 'Controller Building',
      items: [ 
        'controller-build/wiring',
      ]
    },
    {
      type: 'category',
      label: 'Developing GP2040-CE',
      items: [ 
        'development/development',
      ]
    },
    {
      type: 'category',
      label: 'References',
      items: [ 
        'contribution-guide',
        'templates',
      ]
    },
  ],
};

module.exports = sidebars;
