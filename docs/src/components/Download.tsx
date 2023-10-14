import React from 'react';
import styles from './download.module.css';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import { faFileDownload, faGamepad, faGlobe, faX } from '@fortawesome/free-solid-svg-icons';

var releaseURL = "https://api.github.com/repos/OpenStickCommunity/GP2040-CE/releases/latest";
const releaseVersion = "0.7.4";
// let releaseVersion: string = await fetch(releaseURL, { method: "GET", cache: "force-cache" })
// 	.then(response => response.json())
// 	// .then(json => json.html_url)
// 	// .then(url => url.split("/").split(-1));
// console.log(releaseVersion);



const configData = {
	'Microcontroller Boards': [
		{
			name: 'Raspberry Pi Pico',
			configName: 'Pico',
			pinout: "/controller-build/wiring#raspberry-pi-pico",
			website: 'https://www.raspberrypi.com/products/raspberry-pi-pico/',
			image: require('@site/docs/assets/boards/Pico.jpg').default,
			category: null,
			desc: () => (
				<p>
					The Raspberry Pi Pico is a powerful, low-cost board based on the
					Raspberry Pi RP2040 microcontroller. This build is the reference
					implementation for GP2040-CE.
				</p>
			),
		},
		{
			name: 'Raspberry Pi Pico W',
			configName: 'PicoW',
			pinout: "/controller-build/wiring#raspberry-pi-pico",
			website: 'https://www.raspberrypi.com/products/raspberry-pi-pico/',
			image: require('@site/docs/assets/boards/PicoW.jpg').default,
			category: null,
			desc: () => (
				<p>
					The Raspberry Pi Pico W is a powerful, low-cost board based on the
					Raspberry Pi RP2040 microcontroller.
				</p>
			),
		},
		{
			name: 'SparkFun Pro Micro',
			configName: 'SparkFunProMicro',
			pinout: '/controller-build/wiring#sparkfun-pro-micro---rp2040',
			website: 'https://learn.sparkfun.com/tutorials/pro-micro-rp2040-hookup-guide',
			image: require('@site/docs/assets/boards/SparkFunProMicro.jpg').default,
			category: null,
			desc: () => (
				<p>
					An RP2040 board in the Pro Micro form factor. This build is a drop-in
					replacement for the{' '}
					<a href="https://github.com/MickGyver/DaemonBite-Arcade-Encoder">
						Daemonbite Arcade Encoder
					</a>
					.
				</p>
			),
		},
		{
			name: 'Waveshare RP2040-Zero',
			configName: 'WaveshareZero',
			pinout: '/controller-build/wiring#waveshare-rp2040-zero',
			website: 'https://www.waveshare.com/wiki/RP2040-Zero',
			image: require('@site/docs/assets/boards/WaveshareZero.jpg').default,
			category: null,
			desc: () => (
				<p>
					The{' '}
					<a href="https://www.waveshare.com/rp2040-zero.htm">
						Waveshare RP2040-Zero
					</a>{' '}
					is a small form factor board with castellated pins and USB-C, making
					this a popular choice for custom built PCBs without the need for SMT
					assembly.
				</p>
			),
		},
		{
			name: 'Adafruit KB2040',
			configName: 'KB2040',
			pinout: '/controller-build/wiring#adafruit-kb2040',
			website: 'https://learn.adafruit.com/adafruit-kb2040',
			image: require('@site/docs/assets/boards/KB2040.jpg').default,
			category: null,
			desc: () => (
				<p>
					Another RP2040 board in the Pro Micro form factor, with 2 additional
					pins for USB data. This build is configured for DIY gamepad mods.
				</p>
			),
		},
		{
			name: 'Liatris',
			configName: 'Liatris',
			pinout: '',
			website: 'https://splitkb.com/products/liatris',
			image: require('@site/docs/assets/boards/Liatris.jpg').default,
			category: null,
			desc: () => (
				<p>
					Basic pin setup for a stock <a href="https://splitkb.com/products/liatris">Liatris</a>.
				</p>
			),
		},
	],
	'Community Devices': [
		{
			name: 'Alpaca-OwO',
			configName: 'AlpacaOwO',
			pinout: null,
			website: 'https://github.com/Project-Alpaca/Alpaca-OwO/',
			image: require('@site/docs/assets/boards/AlpacaOwO.jpg').default,
			category: "open",
			desc: () => (
				<p>
					Arcade controller I/O platform designed by <a href="https://github.com/dogtopus">dogtopus</a>.
				</p>
			),
		},
		{
			name: 'BatiKING DURAL',
			configName: 'DURAL',
			pinout: null,
			website: 'https://github.com/BatiKING/DURAL_DIY',
			image: require('@site/docs/assets/boards/DURAL.jpg').default,
			category: 'legacy',
			desc: () => (
				<p>
					Configuration for the ScrubTier BentoBox v1 and v2 as well as the GGEZStick Keebfighters.
				</p>
			),
		},
		{
			name: 'Flatbox Rev 4',
			configName: 'FlatboxRev4',
			pinout: null,
			website: 'https://github.com/jfedor2/flatbox/tree/master/hardware-rev4',
			image: require('@site/docs/assets/boards/FlatboxRev4.jpg').default,
			category: "open",
			desc: () => (
				<p>
					Stickless PCB designed by <a href="https://github.com/jfedor2">jfedor2</a> using an embedded RP2040 chip.
				</p>
			),
		},
		{
			name: 'Flatbox Rev 5',
			configName: 'FlatboxRev5',
			pinout: null,
			website: 'https://github.com/jfedor2/flatbox/tree/master/hardware-rev5',
			image: require('@site/docs/assets/boards/FlatboxRev5.jpg').default,
			category: "open",
			desc: () => (
				<p>
					Stickless PCB designed by <a href="https://github.com/jfedor2">jfedor2</a> using the Waveshare RP2040-Zero.
				</p>
			),
		},
		{
			name: 'Flatbox Rev 5 RGB',
			configName: 'FlatboxRev5RGB',
			pinout: null,
			website: 'https://github.com/OpenStickCommunity/Hardware/tree/main/Flatbox%20Rev%205%20RGB',
			image: require('@site/docs/assets/boards/FlatboxRev5RGB.jpg').default,
			category: "open",
			desc: () => (
				<p>
					Stickless PCB designed by <a href="https://github.com/jfedor2">jfedor2</a> and <a href="https://github.com/TheTrainGoes">TheTrain</a> using the Waveshare RP2040-Zero.
				</p>
			),
		},
		{
			name: 'GP2040-CE Keyboard Converter',
			configName: 'KeyboardConverter',
			pinout: null,
			website: 'https://github.com/OpenStickCommunity/Hardware/blob/main/GP2040%20Keyboard%20Converter/Waveshare%20Zero-PCB/README.md',
			image: require('@site/docs/assets/boards/KeyboardConverter.jpg').default,
			category: "",
			desc: () => (
				<p>
					The GP2040-CE Keyboard Converter Case is designed to be a USB Host Device for the use of a keyboard with the GP2040-CE project. The end result is a device that allows you to play on a keyboard, but have it registered just as GP2040-CE controller, whether that is XInput, DInput, PS4, or Switch.
				</p>
			),
		},
		{
			name: 'Mavercade Keebfighter',
			configName: 'MavercadeKeebfighter',
			pinout: null,
			website: 'https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/MavercadeKeebfighter',
			image: require('@site/docs/assets/boards/MavercadeKeebfighter.jpg').default,
			category: "closed",
			desc: () => (
				<p>
					Configuration for the <a href="https://mavercade.com/collections/mavercade-fightsticks">Mavercade Keebfighters</a>. These revisions use a Waveshare RP2040-Zero board.
				</p>
			),
		},
		{
			name: 'PicoAnn',
			configName: 'PicoAnn',
			pinout: null,
			website: '',
			image: require('@site/docs/assets/boards/PicoAnn.jpg').default,
			category: "legacy",
			desc: () => (
				<p>
					This configuration is commonly used with Aliexpress boards. It cannot be guaranteed that this configuration will work for all devices.
				</p>
			),
		},
		{
			name: 'Pico Fighting Board',
			configName: 'PicoFightingBoard',
			pinout: null,
			website: 'https://github.com/FeralAI/PicoFightingBoard',
			image: require('@site/docs/assets/boards/PicoFightingBoard.jpg').default,
			category: "open",
			desc: () => (
				<p>
					Arcade encoder board designed by <a href="https://github.com/FeralAI">FeralAI</a> using a Raspberry Pi Pico or pin-equivalent RP2040 board.
				</p>
			),
		},
		{
			name: 'Reflex CTRL SNES',
			configName: 'ReflexCtrlSNES',
			pinout: null,
			website: 'https://github.com/misteraddons/Reflex-CTRL',
			image: require('@site/docs/assets/boards/ReflexCtrlSNES.jpg').default,
			category: "open",
			desc: () => (
				<p>
					Reflex Board Open source PCB for SNES Controller replacement PCBs designed and sold by <a href="https://github.com/misteraddons">MiSTerAddons</a> using an embedded RP2040 chip.
				</p>
			),
		},
		{
			name: 'ReflexEncoder',
			configName: 'ReflexEncoder',
			pinout: null,
			website: 'https://github.com/misteraddons/ReflexFightingBoard',
			image: require('@site/docs/assets/boards/ReflexEncoder.jpg').default,
			category: "open",
			desc: () => (
				<p>
					Arcade encoder board designed and sold by <a href="https://github.com/misteraddons">MiSTerAddons</a> using an embedded RP2040 chip.
				</p>
			),
		},
		{
			name: 'RP2040 Advanced Breakout',
			configName: 'RP2040AdvancedBreakoutBoard',
			pinout: null,
			website: 'https://github.com/OpenStickCommunity/Hardware/tree/main/RP2040%20Advanced%20Breakout%20Board',
			image: require('@site/docs/assets/boards/RP2040AdvancedBreakoutBoard.jpg').default,
			category: "official",
			desc: () => (
				<p>
					Arcade encoder board designed by <a href="https://github.com/TheTrainGoes">TheTrain</a> using an embedded RP2040, and is the official board of the GP2040-CE project.
				</p>
			),
		},
		{
			name: 'RP2040 Mini Breakout Board',
			configName: 'RP2040MiniBreakoutBoard',
			pinout: null,
			website: 'https://github.com/OpenStickCommunity/Hardware/tree/main/RP2040%20Mini%20Breakout%20Board',
			image: require('@site/docs/assets/boards/RP2040MiniBreakoutBoard.jpg').default,
			category: "open",
			desc: () => (
				<p>
					A reduced-footprint spin off of the RP2040 Advanced Breakout Board designed by <a href="https://github.com/TheTrainGoes">TheTrain</a>.
				</p>
			),
		},
		{
			name: 'ScrubTier BentoBox',
			configName: 'BentoBox',
			pinout: null,
			website: 'https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/BentoBox',
			image: require('@site/docs/assets/boards/BentoBox.jpg').default,
			category: "closed",
			desc: () => (
				<p>
					Configuration for the <a href="https://https://scrubtier.co.uk/">ScrubTier BentoBox v1 and v2 as well as the GGEZStick Keebfighters</a>.
				</p>
			),
		},
		{
			name: 'SGF Flatbox',
			configName: 'SGFDevices',
			pinout: null,
			website: 'https://github.com/sgfdevices/SGFlatbox',
			image: require('@site/docs/assets/boards/SGFDevices.jpg').default,
			category: "open",
			desc: () => (
				<p>
					Configuration for the <a href="https://github.com/sgfdevices/SGFlatbox">SGF Flatbox-based controllers</a>, based on the excellent revisions of the Flatbox design by jfedor2 powered by an RP2040 MCU.
				</p>
			),
		},
		{
			name: 'Stress',
			configName: 'Stress',
			pinout: null,
			website: 'https://github.com/GroooveBob/Stress',
			image: require('@site/docs/assets/boards/Stress.jpg').default,
			category: "open",
			desc: () => (
				<p>
					A small and portable PCB-as-a-controller designed by <a href="https://github.com/GroooveBob">GroooveBob</a> using the Waveshare RP-2040-Zero.
				</p>
			),
		},
		{
			name: 'thnikk Fightboard V3',
			configName: 'FightboardV3',
			pinout: null,
			website: 'https://docs.thnikk.moe/models/fightboard/v3.html',
			image: require('@site/docs/assets/boards/FightboardV3.jpg').default,
			category: "legacy",
			desc: () => (
				<p>
					Configuration for the Fightboard v3
				</p>
			),
		},
		{
			name: 'thnikk Fightboard V3 (Mirrored)',
			configName: 'FightboardV3Mirrored',
			pinout: null,
			website: 'https://docs.thnikk.moe/models/fightboard/v3.html',
			image: require('@site/docs/assets/boards/FightboardV3Mirrored.jpg').default,
			category: "legacy",
			desc: () => (
				<p>
					Configuration for the Fightboard v3 mirrored with directional inputs on the right hand and the action buttons on the left.
				</p>
			),
		},
		{
			name: '',
			configName: '',
			pinout: null,
			website: '',
			image: require('@site/docs/assets/boards/Pico.jpg').default,
			category: "",
			desc: () => (
				<p>

				</p>
			),
		},
	],
};

function CreateLink({ link, text }) {
	var linkIcon;
	switch (text) {
		case 'Download':
			linkIcon = faFileDownload;
			break;
		case 'Pinout':
			linkIcon = faGamepad;
			break;
		case 'Website':
			linkIcon = faGlobe;
			break;
		default:
			linkIcon = faX;
	}

	if (link) {
		return (<li>
			<a href={link}>
				<FontAwesomeIcon icon={linkIcon} />
				{text}
			</a>
		</li>);
	} else {
		return null;
	}
}

function CreateDownloadLink({ configName }) {
	const baseRepoURL = "https://github.com/OpenStickCommunity/GP2040-CE/releases/download/";
	let firmwarePath = "v" + releaseVersion + "/GP2040-CE_" + releaseVersion + "_" + configName + ".uf2";
	let downloadURL = new URL(firmwarePath, baseRepoURL);
	return downloadURL;
}

const renderDownloadBox = ({ name, configName, pinout, desc, image, website }) => (
	<div key={`download-${name}`} className={styles.box}>
		<img className={styles.boxImage} src={image} />
		<div className={styles.boxInfo}>
			<h3 className={styles.boxTitle}>{name}</h3>
			<div>
				<ul className={styles.downloadLinks}>
					<CreateLink
						link={CreateDownloadLink({configName})}
						text={"Download"}
					/>
					<CreateLink
						link={pinout}
						text={"Pinout"}
					/>
					<CreateLink
						link={website}
						text={"Website"}
					/>
				</ul>
			</div>
			<div className='download-desc'>
				{desc()}
			</div>
		</div>
	</div>
);

const Container = ({ children }) => (
	<div className={styles.row}>{children}</div>
);

export const Microcontrollers = () => (
	<Container>
		{configData['Microcontroller Boards'].map(renderDownloadBox)}
	</Container>
);

export const OfficialDevices = () => (
	<Container>
		{configData['Community Devices'].filter(config => config.category === 'official').map(renderDownloadBox)}
	</Container>
);

export const OpenSourceDevices = () => (
	<Container>
		{configData['Community Devices'].filter(config => config.category === 'open').map(renderDownloadBox)}
	</Container>
);

export const ClosedSourceDevices = () => (
	<Container>
		{configData['Community Devices'].filter(config => config.category === 'closed').map(renderDownloadBox)}
	</Container>
);

export const LegacyDevices = () => (
	<Container>
		{configData['Community Devices'].filter(config => config.category === 'legacy').map(renderDownloadBox)}
	</Container>
);