import React from 'react';
import styles from './download.module.css';

const configData = {
	'Microcontroller Boards': [
		{
			name: 'Raspberry Pi Pico',
			pinout: '/#/wiring?id=raspberry-pi-pico',
			website: 'https://www.raspberrypi.com/products/raspberry-pi-pico/',
			desc: 'The Raspberry Pi Pico is a powerful, low-cost board based on the Raspberry Pi RP2040 microcontroller. This build is the reference implementation for GP2040-CE.',
		},
		{
			name: 'Raspberry Pi Pico W',
			pinout: '/#/wiring?id=raspberry-pi-pico',
			website: 'https://www.raspberrypi.com/products/raspberry-pi-pico/',
			desc: 'The Raspberry Pi Pico W is a powerful, low-cost board based on the Raspberry Pi RP2040 microcontroller.',
		},
		{
			name: 'SparkFun Pro Micro',
			pinout: '/#/wiring?id=sparkfun-pro-micro-rp2040',
			website:
				'https://learn.sparkfun.com/tutorials/pro-micro-rp2040-hookup-guide',
			desc: 'An RP2040 board in the Pro Micro form factor. This build is a drop-in replacement for the <a href="https://github.com/MickGyver/DaemonBite-Arcade-Encoder">Daemonbite Arcade Encoder</a>.',
		},
		{
			name: 'Waveshare RP2040-Zero',
			pinout: '/#/wiring?id=waveshare-rp2040-zero',
			website: 'https://www.waveshare.com/wiki/RP2040-Zero',
			desc: 'The [analog stick emulation](add-ons/analog) <a href="https://www.waveshare.com/rp2040-zero.htm">Waveshare RP2040-Zero</a> is a small form factor board with castellated pins and USB-C, making this a popular choice for custom built PCBs without the need for SMT assembly.',
		},
		{
			name: 'Adafruit KB2040',
			pinout: '/#/wiring?id=adafruit-kb2040',
			website: 'https://learn.adafruit.com/adafruit-kb2040',
			desc: 'Another RP2040 board in the Pro Micro form factor, with 2 additional pins for USB data. This build is configured for DIY gamepad mods.',
		},
	],
};

const renderDownloadBox = ({ name, pinout, desc, website }) => (
	<div key={`download-${name}`} className={styles.box}>
		<h3>{name}</h3>
		<p>{desc}</p>
		<p>{pinout}</p>
		<p>{website}</p>
	</div>
);

const Container = ({ children }) => <div>{children}</div>;

export const Microcontrollers = () => (
	<Container>
		{configData['Microcontroller Boards'].map(renderDownloadBox)}
	</Container>
);
