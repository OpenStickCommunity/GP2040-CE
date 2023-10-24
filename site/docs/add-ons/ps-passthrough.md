---
title: PS Passthrough
tags:
- PS4
- PS5
pagination_next: null
pagination_prev: null
---

import PinOption from './usb-host-add-on/_pin-options.md'
import ExampleWiringDiagram from './usb-host-add-on/_example-wiring-diagram.md'

# PS Passthrough

Purpose: Enabling this add-on will allow you to use a licensed 3rd party device to authenticate off of.

![GP2040-CE Configurator - PS Passthrough](../assets/images/gpc-add-ons-ps-passthrough.png)

<PinOption />

## Hardware Requirements

 This add-on requires that you have something like the ![USB Passthrough Board](https://github.com/OpenStickCommunity/Hardware/tree/main/USB%20Passthrough%20Board) or a board with a USB passthrough port on it already.  

 ### Example Wiring Diagram

<ExampleWiringDiagram />

## Note
 
 If you have passthrough enabled you can turn off the above `PS4 Mode` add-on as the two will not work together.  Please also ensure that under the `Settings` section you have chosen PS4 mode and picked if you want the GP2040-CE unit to function as a controller or as a fightstick.
