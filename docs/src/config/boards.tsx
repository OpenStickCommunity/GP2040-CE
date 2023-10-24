import React from 'react';

const boards = {
	'Microcontroller Boards': [
		{
			name: 'Raspberry Pi Pico',
			configName: 'Pico',
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
			configName: 'PicoW',
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
			name: 'Adafruit KB2040',
			configName: 'KB2040',
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
		{
			name: 'Liatris',
			configName: 'SparkFunProMicro',
			pinout: '/#/wiring?id=sparkfun-pro-micro-rp2040',
			website:
				'https://splitkb.com/products/liatris',
			image: require('@site/docs/assets/boards/Liatris.jpg').default,
			supported: true,
			desc: () => (
				<p>
					Drop in RP2040 replacement for Pro Micro builds by {' '}
					<a href="SplitKB.com">
						SplitKB.com
					</a>
					.
				</p>
			),
		},
		{
			name: 'SparkFun Pro Micro',
			configName: 'SparkFunProMicro',
			pinout: '/#/wiring?id=sparkfun-pro-micro-rp2040',
			website:
				'https://learn.sparkfun.com/tutorials/pro-micro-rp2040-hookup-guide',
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
			configName: 'WaveshareZero',
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
	],
	'Community Devices': [
		{
			name: 'Alpaca-OwO',
			configName: 'AlpacaOwO',
			pinout: null,
			website: 'https://github.com/Project-Alpaca/Alpaca-OwO/',
			image: require('@site/docs/assets/boards/AlpacaOwO.jpg').default,
			category: 'open',
			desc: () => (
				<p>
					Arcade controller I/O platform designed by{' '}
					<a href="https://github.com/dogtopus">dogtopus</a>.
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
					Configuration for the ScrubTier BentoBox v1 and v2 as well as the
					GGEZStick Keebfighters.
				</p>
			),
		},
		{
			name: 'Flatbox Rev 4',
			configName: 'FlatboxRev4',
			pinout: null,
			website: 'https://github.com/jfedor2/flatbox/tree/master/hardware-rev4',
			image: require('@site/docs/assets/boards/FlatboxRev4.jpg').default,
			category: 'open',
			desc: () => (
				<p>
					Stickless PCB designed by{' '}
					<a href="https://github.com/jfedor2">jfedor2</a> using an embedded
					RP2040 chip.
				</p>
			),
		},
		{
			name: 'Flatbox Rev 5',
			configName: 'FlatboxRev5',
			pinout: null,
			website: 'https://github.com/jfedor2/flatbox/tree/master/hardware-rev5',
			image: require('@site/docs/assets/boards/FlatboxRev5.jpg').default,
			category: 'open',
			desc: () => (
				<p>
					Stickless PCB designed by{' '}
					<a href="https://github.com/jfedor2">jfedor2</a> using the Waveshare
					RP2040-Zero.
				</p>
			),
		},
		{
			name: 'Flatbox Rev 5 Southpaw',
			configName: 'FlatboxRev5Southpaw',
			pinout: null,
			website: 'https://github.com/SkylaHila/flatbox-southpaw',
			image: require('@site/docs/assets/boards/FlatboxRev5Southpaw.jpg').default,
			category: 'open',
			desc: () => (
				<p>
					Mirrored version of the Flatbox Rev 5 by SkylaHila. Based on the Flatbox Rev 5 by 
					<a href="https://github.com/jfedor2">jfedor2</a>.
				</p>
			),
		},
		{
			name: 'Flatbox Rev 5 RGB',
			configName: 'FlatboxRev5RGB',
			pinout: null,
			website:
				'https://github.com/OpenStickCommunity/Hardware/tree/main/Flatbox%20Rev%205%20RGB',
			image: require('@site/docs/assets/boards/FlatboxRev5RGB.jpg').default,
			category: 'open',
			desc: () => (
				<p>
					Stickless PCB designed by{' '}
					<a href="https://github.com/jfedor2">jfedor2</a> and{' '}
					<a href="https://github.com/TheTrainGoes">TheTrain</a> using the
					Waveshare RP2040-Zero.
				</p>
			),
		},
		{
			name: 'GP2040-CE Keyboard Converter',
			configName: 'KeyboardConverter',
			pinout: null,
			website:
				'https://github.com/OpenStickCommunity/Hardware/blob/main/GP2040%20Keyboard%20Converter/Waveshare%20Zero-PCB/README.md',
			image: require('@site/docs/assets/boards/KeyboardConverter.jpg').default,
			category: 'open',
			desc: () => (
				<p>
					The GP2040-CE Keyboard Converter Case is designed to be a USB Host
					Device for the use of a keyboard with the GP2040-CE project.
				</p>
			),
		},
		{
			name: 'Mavercade Keebfighter',
			configName: 'MavercadeKeebfighter',
			pinout: null,
			website:
				'https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/MavercadeKeebfighter',
			image: require('@site/docs/assets/boards/MavercadeKeebfighter.jpg')
				.default,
			category: 'closed',
			desc: () => (
				<p>
					Configuration for the{' '}
					<a href="https://mavercade.com/collections/mavercade-fightsticks">
						Mavercade Keebfighters
					</a>
					. These revisions use a Waveshare RP2040-Zero board.
				</p>
			),
		},		{
			name: 'Open Core0',
			configName: 'OpenCore0',
			pinout: null,
			website:
				'https://github.com/OpenStickCommunity/Hardware/tree/main/Open_Core0',
			image: require('@site/docs/assets/boards/OpenCore0.jpg').default,
			category: 'official',
			desc: () => (
				<p>
					Open source stickless controller designed by {' '}
					<a href="https://github.com/TheTrainGoes">
						TheTrain
					</a>.
				</p>
			),
		},
		{
			name: 'Open Core0 WASD',
			configName: 'OpenCore0WASD',
			pinout: null,
			website:
				'https://github.com/OpenStickCommunity/Hardware/tree/main/Open_Core0',
			image: require('@site/docs/assets/boards/OpenCore0WASD.jpg').default,
			category: 'official',
			desc: () => (
				<p>
					Open source stickless WASD controller designed by {' '} 
					<a href="https://github.com/TheTrainGoes"> 
						TheTrain
					</a>.
				</p>
			),
		},
		{
			name: 'PicoAnn',
			configName: 'PicoAnn',
			pinout: null,
			website: '',
			image: require('@site/docs/assets/boards/PicoAnn.jpg').default,
			category: 'legacy',
			desc: () => (
				<p>
					This configuration is commonly used with Aliexpress boards. It cannot
					be guaranteed that this configuration will work for all devices.
				</p>
			),
		},
		{
			name: 'Pico Fighting Board',
			configName: 'PicoFightingBoard',
			pinout: null,
			website: 'https://github.com/FeralAI/PicoFightingBoard',
			image: require('@site/docs/assets/boards/PicoFightingBoard.jpg').default,
			category: 'open',
			desc: () => (
				<p>
					Arcade encoder board designed by {' '}
					<a href="https://github.com/FeralAI">FeralAI</a> using a Raspberry Pi
					Pico or pin-equivalent RP2040 board.
				</p>
			),
		},
		{
			name: 'Rana Tadpole',
			configName: 'RanaTadpole',
			pinout: null,
			website: 'https://github.com/rana-sylvatica/rana-tadpole',
			image: require('@site/docs/assets/boards/RanaTadpole.jpg').default,
			category: 'open',
			desc: () => (
				<p>
					Pocket-sized digital controller from RanaLabs.
				</p>
			),
		},
		{
			name: 'Reflex Encode v1.2',
			configName: 'ReflexEncodeV1.2',
			pinout: null,
			website: 'https://github.com/misteraddons/ReflexFightingBoard',
			image: require('@site/docs/assets/boards/ReflexEncode_v1.2.jpg').default,
			category: 'open',
			desc: () => (
				<p>
					Arcade encoder board designed and sold by{' '}
					<a href="https://github.com/misteraddons">MiSTerAddons</a> using an
					embedded RP2040 chip.
				</p>
			),
		},
		{
			name: 'Reflex Encode v2.0',
			configName: 'ReflexEncodeV2.0',
			pinout: null,
			website: 'https://github.com/misteraddons/ReflexFightingBoard',
			image: require('@site/docs/assets/boards/ReflexEncode_v2.0.jpg').default,
			category: 'open',
			desc: () => (
				<p>
					Arcade encoder board designed and sold by {' '}
					<a href="https://github.com/misteraddons">MiSTerAddons</a> using an
					embedded RP2040 chip.
				</p>
			),
		},
		{
			name: 'Reflex CTRL SNES',
			configName: 'ReflexCtrlSNES',
			pinout: null,
			website: 'https://github.com/misteraddons/Reflex-CTRL',
			image: require('@site/docs/assets/boards/ReflexCtrlSNES.jpg').default,
			category: 'open',
			desc: () => (
				<p>
					Reflex Board Open source PCB for SNES Controller replacement PCBs
					designed and sold by{' '}
					<a href="https://github.com/misteraddons">MiSTerAddons</a> using an
					embedded RP2040 chip.
				</p>
			),
		},
		{
			name: 'RP2040 Advanced Breakout',
			configName: 'RP2040AdvancedBreakoutBoard',
			pinout: null,
			website:
				'https://github.com/OpenStickCommunity/Hardware/tree/main/RP2040%20Advanced%20Breakout%20Board',
			image: require('@site/docs/assets/boards/RP2040AdvancedBreakoutBoard.jpg')
				.default,
			category: 'official',
			desc: () => (
				<p>
					Arcade encoder board designed by {' '}
					<a href="https://github.com/TheTrainGoes">TheTrain</a> using an
					embedded RP2040, and is the official board of the GP2040-CE project.
				</p>
			),
		},
		{
			name: 'RP2040 Advanced Breakout (USB Passthrough)',
			configName: 'RP2040AdvancedBreakoutBoardUSBPassthrough',
			pinout: null,
			website: 'https://github.com/OpenStickCommunity/Hardware/tree/main/RP2040%20Advanced%20Breakout%20Board%20-%20Passthrough',
			image: require('@site/docs/assets/boards/RP2040AdvancedBreakoutBoardUSBPassthrough.jpg').default,
			category: 'official',
			desc: () => 
				<p>
					Official USB Passthrough Board of the Open Stick project.  Updated version of 
					the RP2040 Advanced Breakout Board with USB passthrough included on the board.
				</p>,
		},
		{
			name: 'RP2040 Mini Breakout Board',
			configName: 'RP2040MiniBreakoutBoard',
			pinout: null,
			website:
				'https://github.com/OpenStickCommunity/Hardware/tree/main/RP2040%20Mini%20Breakout%20Board',
			image: require('@site/docs/assets/boards/RP2040MiniBreakoutBoard.jpg')
				.default,
			category: 'open',
			desc: () => (
				<p>
					A reduced-footprint spin off of the RP2040 Advanced Breakout Board designed by {' '}
					<a href="https://github.com/TheTrainGoes">
						TheTrain
					</a>.
				</p>
			),
		},
		{
			name: 'ScrubTier BentoBox',
			configName: 'BentoBox',
			pinout: null,
			website:
				'https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/BentoBox',
			image: require('@site/docs/assets/boards/BentoBox.jpg').default,
			category: 'closed',
			desc: () => (
				<p>
					Configuration for the{' '}
					<a href="https://https://scrubtier.co.uk/">
						ScrubTier BentoBox v1 and v2 as well as the GGEZStick Keebfighters
					</a>
					.
				</p>
			),
		},
		{
			name: 'SGF Flatbox',
			configName: 'SGFDevices',
			pinout: null,
			website: 'https://github.com/sgfdevices/SGFlatbox',
			image: require('@site/docs/assets/boards/SGFDevices.jpg').default,
			category: 'open',
			desc: () => (
				<p>
					Configuration for the{' '}
					<a href="https://github.com/sgfdevices/SGFlatbox">
						SGF Flatbox-based controllers
					</a>
					, based on the excellent revisions of the Flatbox design by jfedor2
					powered by an RP2040 MCU.
				</p>
			),
		},
		{
			name: 'Stress',
			configName: 'Stress',
			pinout: null,
			website: 'https://github.com/GroooveBob/Stress',
			image: require('@site/docs/assets/boards/Stress.jpg').default,
			category: 'open',
			desc: () => (
				<p>
					A small and portable PCB-as-a-controller designed by{' '}
					<a href="https://github.com/GroooveBob">GroooveBob</a> using the
					Waveshare RP-2040-Zero.
				</p>
			),
		},
		{
			name: 'thnikk Fightboard V3',
			configName: 'FightboardV3',
			pinout: null,
			website: 'https://docs.thnikk.moe/models/fightboard/v3.html',
			image: require('@site/docs/assets/boards/FightboardV3.jpg').default,
			category: 'legacy',
			desc: () => <p>Configuration for the Fightboard v3</p>,
		},
		{
			name: 'thnikk Fightboard V3 (Mirrored)',
			configName: 'FightboardV3Mirrored',
			pinout: null,
			website: 'https://docs.thnikk.moe/models/fightboard/v3.html',
			image: require('@site/docs/assets/boards/FightboardV3Mirrored.jpg')
				.default,
			category: 'legacy',
			desc: () => (
				<p>
					Configuration for the Fightboard v3 mirrored with directional inputs
					on the right hand and the action buttons on the left.
				</p>
			),
		},
	],
};

export default boards;
