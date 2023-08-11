const configCategories = [
	'Microcontroller Boards',
	'Open Source Devices',
	'Closed Source Devices',
	'Legacy Devices'
];

const configData = {
	AlpacaOwO: {
		name: 'Alpaca-OwO',
		category: configCategories[1],
		website: 'https://github.com/Project-Alpaca/Alpaca-OwO',
		desc: 'Arcade controller I/O platform designed by <a target="_blank" href="https://github.com/dogtopus">dogtopus</a>.'
	},
	BentoBox: {
		name: 'ScrubTier BentoBox',
		category: configCategories[2],
		website: 'https://github.com/OpenStickCommunity/GP2040-CE/tree/RP2040-arcade-board/configs/BentoBox',
		desc: ''
	},
	DURAL: {
		name: 'BatiKING DURAL',
		category: configCategories[3],
		website: 'https://github.com/BatiKING/DURAL_DIY',
		desc: ''
	},
	FightboardV3: {
		name: 'thnikk Fightboard V3',
		category: configCategories[3],
		website: 'https://docs.thnikk.moe/models/fightboard/v3.html',
		desc: ''
	},
	FightboardV3Mirrored: {
		name: 'thnikk Fightboard V3 (Mirrored)',
		category: configCategories[3],
		website: 'https://docs.thnikk.moe/models/fightboard/v3.html',
		desc: ''
	},
	FlatboxRev4: {
		name: 'Flatbox Rev 4',
		category: configCategories[1],
		website: 'https://github.com/jfedor2/flatbox/tree/master/hardware-rev4',
		desc: 'Stickless PCB designed by <a target="_blank" href="https://github.com/jfedor2">jfedor2</a> using an embedded RP2040 chip.'
	},
	FlatboxRev5: {
		name: 'Flatbox Rev 5',
		category: configCategories[1],
		website: 'https://github.com/jfedor2/flatbox/tree/master/hardware-rev5',
		desc: 'Stickless PCB designed by <a target="_blank" href="https://github.com/jfedor2">jfedor2</a> using the Waveshare RP2040-Zero.'
	},
	FlatboxRev5RGB: {
		name: 'Flatbox Rev 5 RGB',
		category: configCategories[1],
		website: 'https://github.com/OpenStickCommunity/Hardware/tree/main/Flatbox%20Rev%205%20RGB',
		desc: 'Stickless PCB designed by <a target="_blank" href="https://github.com/jfedor2">jfedor2</a> and <a target="_blank" href="https://github.com/TheTrainGoes">TheTrain</a> using the Waveshare RP2040-Zero.'
	},
	KB2040: {
		name: 'Adafruit KB2040',
		category: configCategories[0],
		pinout: '/#/wiring?id=adafruit-kb2040',
		website: 'https://learn.adafruit.com/adafruit-kb2040',
		desc: 'Another RP2040 board in the Pro Micro form factor, with 2 additional pins for USB data. This build is configured for DIY gamepad mods.'
	},
	MavercadeKeebfighter: {
		name: 'Mavercade Keebfighter',
		category: configCategories[2],
		website: 'https://github.com/OpenStickCommunity/GP2040-CE/tree/RP2040-arcade-board/configs/MavercadeKeebfighter',
		desc: ''
	},
	Pico: {
		name: 'Raspberry Pi Pico',
		category: configCategories[0],
		pos: 1,
		pinout: '/#/wiring?id=raspberry-pi-pico',
		website: 'https://www.raspberrypi.com/products/raspberry-pi-pico/',
		desc: 'The Raspberry Pi Pico is a powerful, low-cost board based on the Raspberry Pi RP2040 microcontroller. This build is the reference implementation for GP2040-CE.'
	},
	PicoAnn: {
		name: 'PicoAnn',
		category: configCategories[3],
		website: '',
		desc: ''
	},
	PicoFightingBoard: {
		name: 'Pico Fighting Board',
		category: configCategories[1],
		website: 'https://github.com/FeralAI/PicoFightingBoard',
		desc: 'Arcade encoder board designed by <a target="_blank" href="https://github.com/FeralAI">FeralAI</a> using a Raspberry Pi Pico or pin-equivalent RP2040 board.'
	},
	RP2040AdvancedBreakoutBoard: {
		name: 'RP2040 Advanced Breakout',
		category: configCategories[1],
		website: 'https://github.com/OpenStickCommunity/Hardware/tree/main/RP2040%20Advanced%20Breakout%20Board',
		desc: 'Arcade encoder board designed by <a target="_blank" href="https://github.com/TheTrainGoes">TheTrain</a> using an embedded RP2040, and is the official board of the GP2040-CE project.'
	},
	RP2040MiniBreakoutBoard: {
		name: 'RP2040 Mini Breakout',
		category: configCategories[1],
		website: 'https://github.com/OpenStickCommunity/Hardware/tree/main/RP2040%20Mini%20Breakout%20Board',
		desc: 'A reduced-footprint spin off of the RP2040 Advanced Breakout Board designed by <a target="_blank" href="https://github.com/TheTrainGoes">TheTrain</a>.'
	},
	ReflexEncoder: {
		name: 'Reflex Arcade Encoder',
		category: configCategories[1],
		website: 'https://github.com/misteraddons/ReflexFightingBoard',
		desc: 'Arcade encoder board designed and sold by <a target="_blank" href="https://github.com/misteraddons">MiSTerAddons</a> using an embedded RP2040 chip.'
	},
	SparkFunProMicro: {
		name: 'SparkFun Pro Micro',
		category: configCategories[0],
		pos: 2,
		pinout: '/#/wiring?id=sparkfun-pro-micro-rp2040',
		website: 'https://learn.sparkfun.com/tutorials/pro-micro-rp2040-hookup-guide',
		desc: 'An RP2040 board in the Pro Micro form factor. This build is a drop-in replacement for the <a target="_blank" href="https://github.com/MickGyver/DaemonBite-Arcade-Encoder">Daemonbite Arcade Encoder</a>.'
	},
	Stress: {
		name: 'Stress',
		category: configCategories[1],
		website: 'https://github.com/GroooveBob/Stress',
		desc: 'A small and portable PCB-as-a-controller designed by <a target="_blank" href="https://github.com/GroooveBob">GroooveBob</a> using the Waveshare RP-2040-Zero.'
	},
	WaveshareZero: {
		name: 'Waveshare RP2040-Zero',
		category: configCategories[0],
		pos: 3,
		pinout: '/#/wiring?id=waveshare-rp2040-zero',
		website: 'https://www.waveshare.com/wiki/RP2040-Zero',
		desc: 'The <a target="_blank" href="https://www.waveshare.com/rp2040-zero.htm">Waveshare RP2040-Zero</a> is a small form factor board with castellated pins and USB-C, making this a popular choice for custom built PCBs without the need for SMT assembly.'
	},
};

window.$docsify = {
	name: 'GP2040-CE',
	repo: 'https://github.com/OpenStickCommunity/GP2040-CE',
	homepage: 'https://raw.githubusercontent.com/OpenStickCommunity/GP2040-CE/main/README.md',
	logo: 'assets/images/gp2040-ce-logo.png',
	loadSidebar: '_sidebar.md',
	auto2top: true,
	subMaxLevel: 3,
	themeable: {
		readyTransition : true, // default
		responsiveTables: true  // default
	},
	vueGlobalOptions: {
		directives: {
			VScrollThreshold,
			VueStickyElement,
		},
		data() {
			return {
				labelData: {
					"GP2040": { "name": "GP2040", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "B1", "B2": "B2", "B3": "B3", "B4": "B4", "L1": "L1", "R1": "R1", "L2": "L2", "R2": "R2", "S1": "S1", "S2": "S2", "L3": "L3", "R3": "R3", "A1": "A1", "A2": "A2" },
					"XInput": { "name": "XInput", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "A", "B2": "B", "B3": "X", "B4": "Y", "L1": "LB", "R1": "RB", "L2": "LT", "R2": "RT", "S1": "Back", "S2": "Start", "L3": "LS", "R3": "RS", "A1": "Guide", "A2": "(A2)" },
					"DirectInput": { "name": "DirectInput", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "2", "B2": "3", "B3": "1", "B4": "4", "L1": "5", "R1": "6", "L2": "7", "R2": "8", "S1": "9", "S2": "10", "L3": "11", "R3": "12", "A1": "13", "A2": "14" },
					"Nintendo Switch": { "name": "Nintendo Switch", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "B", "B2": "A", "B3": "Y", "B4": "X", "L1": "L", "R1": "R", "L2": "ZL", "R2": "ZR", "S1": "Minus", "S2": "Plus", "L3": "LS", "R3": "RS", "A1": "Home", "A2": "Capture" },
					"PS4":    { "name": "PS4", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "Cross", "B2": "Circle", "B3": "Square", "B4": "Triangle", "L1": "L1", "R1": "R1", "L2": "L2", "R2": "R2", "S1": "Share", "S2": "Options", "L3": "L3", "R3": "R3", "A1": "PS", "A2": "Touchpad" },
					"PS3":    { "name": "PS3", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "Cross", "B2": "Circle", "B3": "Square", "B4": "Triangle", "L1": "L1", "R1": "R1", "L2": "L2", "R2": "R2", "S1": "Select", "S2": "Start", "L3": "L3", "R3": "R3", "A1": "PS", "A2": "(A2)" },
					"Arcade": { "name": "Arcade", "Up": "Up", "Down": "Down", "Left": "Left", "Right": "Right", "B1": "K1", "B2": "K2", "B3": "P1", "B4": "P2", "L1": "P4", "R1": "P3", "L2": "K4", "R2": "K3", "S1": "Select", "S2": "Start", "L3": "LS", "R3": "RS", "A1": "Home", "A2": "(A2)" },
				},
				selectedLabels: localStorage.getItem('selectedLabels') || 'GP2040',
			};
		}
	},
	vueComponents: {
		'download-box': {
			props: ['release'],
			template: `
				<div class="download-box">
					<a class="download-img" :href="release.url">
						<img :src="'/assets/boards/' + release.key + '.jpg'">
					</a>
					<div class="download-info">
						<div class="download-title">{{ release.name }}</div>
						<ul class="download-links">
							<li>
								<a :href="release.url">
									<i class="fa-solid fa-file-arrow-down fa-2xl"></i>
									Download
								</a>
							</li>
							<li v-if="release.pinout">
								<a :href="release.pinout" target="_blank">
									<i class="fa-solid fa-gamepad fa-2xl"></i>
									Pinout
								</a>
							</li>
							<li v-if="release.website">
								<a :href="release.website" target="_blank">
									<i class="fa-solid fa-globe fa-2xl"></i>
									Website
								</a>
							</li>
						</ul>
						<div v-if="release.desc" class="download-desc" v-html="release.desc">
						</div>
					</div>
				</div>
			`,
		},

		'download-category': {
			props: ['category'],
			template: `
				<div class="download-category">
					<h3>{{ category }}</h3>
					<div v-if="category === 'Legacy Devices'">
						<p>GP2040-CE builds for these devices have been deprecated due to one or more of the following reasons:</p>
						<ul>
							<li>The manufacturer already provides a custom build of GP2040-CE</li>
							<li>The device is no longer available for purchase</li>
							<li>The operation of the device running an updated GP2040-CE build cannot be verified by the development team</li>
						</ul>
						<p>These builds remain available, however <strong>legacy devices are not supported by the GP2040-CE team</strong>.</p>
						<p><strong>Please ensure you have a backup of the original firmware and your settings before updating your device!</strong></p>
					</div>
					<div class="download-category-list">
						<download-box
							v-for="release in releases[category]"
							:key="release.title"
							:release="release">
						</download-box>
					</div>
				</div>
			`,
			data() {
				return {
					releases: this.$root.releases,
				};
			},
		},

		'download-page': {
			template: `
			<div v-if="fetched">
				<version-name></version-name>
				<download-category
				v-for="category of configCategories"
				:key="category"
				:category="category">
				</download-category>
				<div>
					<h3>Flash Nuke</h3>
					<p>
						Use the <a href="https://raw.githubusercontent.com/OpenStickCommunity/GP2040-CE/main/docs/downloads/flash_nuke.uf2">flash_nuke.uf2</a>
						file to completely erase the flash of the RP2040 board. <strong>THIS WILL ERASE ALL SAVED SETTINGS!</strong>
						Use this prior to flashing GP2040-CE on your device if you want to start from a clean slate, or as a troubleshooting step before to reporting an issue.
					</p>
				</div>
				<release-notes></release-notes>
			</div>
			`,
			data() {
				return {
					configCategories,
					configData,
					fetched: false,
					releaseNotes: this.$root.releaseNotes,
					releaseVersion: this.$root.releaseVersion,
					releases: this.$root.releases,
				};
			},
			created() {
				fetch("https://api.github.com/repos/OpenStickCommunity/GP2040-CE/releases/latest", { method: "GET", cache: "force-cache" })
					.then(response => response.json())
					.then(data => {
						const releaseInfo = data;

						this.$root.releaseNotes = releaseInfo.body;
						this.$root.releaseVersion = releaseInfo.name;
						if (window.location.hostname === 'localhost') {
							this.$root.releases = Object.keys(this.configData)
							.map(key => {
								return {
									key,
									...this.configData[key],
									url: '',
									img: `/assets/boards/${key}.jpg`,
								};
							})
							.sort((a, b) => {
								if (a.name < b.name) return -1;
								if (a.name > b.name) return 1;
								return 0;
							})
							.sort((a, b) => (a.pos === undefined ? Number.MAX_VALUE : a.pos) - (b.pos === undefined ? Number.MAX_VALUE : b.pos))
							.reduce((p, r) => {
								let values = { ...p };
								if (!values[r.category])
									values[r.category] = [];

								values[r.category].push(r);
								return values;
							}, { });
						}
						else {
							this.$root.releases = releaseInfo.assets
								.filter(a => a.name.startsWith('GP2040-CE_'))
								.filter(a => {
									const key = a.name.substring(a.name.lastIndexOf('_') + 1).replace('.uf2', '');
									return Object.keys(this.configData).indexOf(key) > -1;
								})
								.map(a => {
									const key = a.name.substring(a.name.lastIndexOf('_') + 1).replace('.uf2', '');
									return {
										key,
										...this.configData[key],
										url: a.browser_download_url,
										img: `/assets/boards/${key}.jpg`,
									};
								})
								.sort((a, b) => {
									if (a.name < b.name) return -1;
									if (a.name > b.name) return 1;
									return 0;
								})
								.sort((a, b) => (a.pos === undefined ? Number.MAX_VALUE : a.pos) - (b.pos === undefined ? Number.MAX_VALUE : b.pos))
								.reduce((p, r) => {
									let values = { ...p };
									if (!values[r.category])
										values[r.category] = [];

									values[r.category].push(r);
									return values;
								}, { });
						}


						console.log(this.$root.releases);

						this.fetched = true;
					});
			},
		},

		'label-selector': {
			template: `
				<VueStickyElement visibleOnDirection="disabled" class="label-select" stuckClass="sticky-label-select">
					<div class="form-dropdown">
						<select name="labelSelect" v-model="selectedLabels" v-on:change="setSelectedLabels">
							<option v-for="item in labelData" :key="item.name" :value="item.name">{{ item.name }}</option>
						</select>
					</div>
				</VueStickyElement>
			`,
			data() {
				return {
					labelData: this.$root.labelData,
					selectedLabels: this.$root.selectedLabels,
				};
			},
			methods: {
				setSelectedLabels($event) {
					localStorage.setItem('selectedLabels', $event.target.value);
					this.$root.selectedLabels = $event.target.value;
				},
			},
		},

		'hotkey': {
			props: ['buttons'],
			template: `
				<strong>
					<code class="hotkey">{{ buttons.reduce((prev, next) => prev ? prev + " + " + labelData[state.selectedLabels][next] : labelData[state.selectedLabels][next] , "") }}</code>
				</strong>
			`,
			data() {
				return {
					state: this.$root.$data,
					labelData: this.$root.labelData,
				};
			},
		},

		'version-name': {
			template: `<h2>GP2040-CE <strong v-text="version" v-if="version"></strong></h2>`,
			data() {
				return {
					version: this.$root.releaseVersion,
				};
			},
		},

		'release-notes': {
			template: `
				<div v-html="body"></div>
			`,
			data() {
				marked.use({
					renderer: {
						heading(text, level) {
							return `<h3>${text}</h3>`;
						}
					}
				})
				return {
					body: marked.parse(this.$root.releaseNotes ? "---\r\n## Release Notes\r\n" + this.$root.releaseNotes : ""),
				}
			},
		},

	}
};
