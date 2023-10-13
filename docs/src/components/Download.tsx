import React from 'react';
import styles from './download.module.css';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import { faFileDownload, faGamepad, faGlobe, faX } from '@fortawesome/free-solid-svg-icons';

const configData = {
	'Microcontroller Boards': [
		{
			name: 'Raspberry Pi Pico',
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_Pico.uf2',
			pinout: '/#/wiring?id=raspberry-pi-pico',
			website: 'https://www.raspberrypi.com/products/raspberry-pi-pico/',
			image: require('@site/docs/assets/boards/Pico.jpg').default,
			supported: true,
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_PicoW.uf2',
			pinout: '/#/wiring?id=raspberry-pi-pico',
			website: 'https://www.raspberrypi.com/products/raspberry-pi-pico/',
			image: require('@site/docs/assets/boards/PicoW.jpg').default,
			supported: true,
			desc: () => (
				<p>
					The Raspberry Pi Pico W is a powerful, low-cost board based on the
					Raspberry Pi RP2040 microcontroller.
				</p>
			),
		},
		{
			name: 'SparkFun Pro Micro',
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_SparkFunProMicro.uf2',
			pinout: '/#/wiring?id=sparkfun-pro-micro-rp2040',
			website: 'https://learn.sparkfun.com/tutorials/pro-micro-rp2040-hookup-guide',
			image: require('@site/docs/assets/boards/SparkFunProMicro.jpg').default,
			supported: true,
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_WaveshareZero.uf2',
			pinout: '/#/wiring?id=waveshare-rp2040-zero',
			website: 'https://www.waveshare.com/wiki/RP2040-Zero',
			image: require('@site/docs/assets/boards/WaveshareZero.jpg').default,
			supported: true,
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_KB2040.uf2',
			pinout: '/#/wiring?id=adafruit-kb2040',
			website: 'https://learn.adafruit.com/adafruit-kb2040',
			image: require('@site/docs/assets/boards/KB2040.jpg').default,
			supported: true,
			desc: () => (
				<p>
					Another RP2040 board in the Pro Micro form factor, with 2 additional
					pins for USB data. This build is configured for DIY gamepad mods.
				</p>
			),
		},
	],
	'Community Devices': [
		{
			name: 'Alpaca-OwO',
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_AlpacaOwO.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_DURAL.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_FlatboxRev4.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_FlatboxRev5.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_FlatboxRev5RGB.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_KeyboardConverter.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_MavercadeKeebfighter.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_PicoAnn.uf2',
			pinout: null,
			website: '',
			image: require('@site/docs/assets/boards/PicoAnn.jpg').default,
			category: "legacy",
			desc: () => (
				<p>
					
				</p>
			),
		},
		{
			name: 'Pico Fighting Board',
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_PicoFightingBoard.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_ReflexCtrlSNES.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_ReflexEncoder.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_RP2040AdvancedBreakoutBoard.uf2',
			pinout: null,
			website: 'https://github.com/OpenStickCommunity/Hardware/tree/main/RP2040%20Advanced%20Breakout%20Board',
			image: require('@site/docs/assets/boards/RP2040AdvancedBreakoutBoard.jpg').default,
			category: "open",
			desc: () => (
				<p>
					Arcade encoder board designed by <a href="https://github.com/TheTrainGoes">TheTrain</a> using an embedded RP2040, and is the official board of the GP2040-CE project.
				</p>
			),
		},
		{
			name: 'RP2040MiniBreakoutBoard',
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_RP2040MiniBreakoutBoard.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_BentoBox.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_SGFDevices.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_Stress.uf2',
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
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_FightboardV3.uf2',
			pinout: null,
			website: 'https://docs.thnikk.moe/models/fightboard/v3.html',
			image: require('@site/docs/assets/boards/FightboardV3.jpg').default,
			category: "legacy",
			desc: () => (
				<p>
					
				</p>
			),
		},
		{
			name: 'thnikk Fightboard V3 (Mirrored)',
			download: 'https://github.com/OpenStickCommunity/GP2040-CE/releases/download/v0.7.4/GP2040-CE_0.7.4_FightboardV3Mirrored.uf2',
			pinout: null,
			website: 'https://docs.thnikk.moe/models/fightboard/v3.html',
			image: require('@site/docs/assets/boards/FightboardV3Mirrored.jpg').default,
			category: "legacy",
			desc: () => (
				<p>
					
				</p>
			),
		},
		{
			name: '',
			download: '',
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

function DownloadLink({ downloadLink, text }) {
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

	if(downloadLink) {
		return (<li>
			<a href={downloadLink}>
				<FontAwesomeIcon icon={linkIcon} />
				{text}
			</a>
		</li>);
	} else {
		return null;
	}
}

const renderDownloadBox = ({ name, download, pinout, desc, image, website }) => (
	<div key={`download-${name}`} className={styles.box}>
		<img className={styles.boxImage} src={image} />
		<div className={styles.boxInfo}>
			<h3 className={styles.boxTitle}>{name}</h3>
			<div>
				<ul className="download-links">
					<DownloadLink 
						downloadLink={download}
						text={"Download"}
					/>
					<DownloadLink 
						downloadLink={pinout}
						text={"Pinout"}
					/>
					<DownloadLink 
						downloadLink={website}
						text={"Website"}
					/>
				</ul>
			</div>
			{desc()}
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