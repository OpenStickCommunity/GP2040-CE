# FAQ: PS4/PS5 Compatibility

## How do I get my GP2040-CE unit to work on a Playstation 4 or Playstation 5?

For Playstation 4 compatibility, you will need to use either the PS4 Mode or the PS Passthrough add-on.

For Playstation 5 compatibility, you will need to disable the PS4 Mode add-on and enable the PS Passthrough add-on. You will also need to ensure that under the `Settings` section you have chosen `PS4` as your input mode and picked if you want the GP2040-CE unit to function as a `controller` or as a `fightstick`. 

## What is PS4 Mode?

PS4 mode is an add-on that grants GP2040-CE compatibility with the Playstation 4 console. It was built using the efforts of another open source firmware project named [Passing Link](https://github.com/passinglink/passinglink). 

The team behind Passing Link are in no way affiliated with the GP2040-CE project, however the work and information they've made available was greatly appreciated so we could bring this highly-requested feature to our users.

For more information, refer to [PS4 Mode](add-ons/ps4-mode) in the `Web Configurator - Add-ons` section of the documentation.

## What is PS Passthrough?

PS Passthrough is an add-on that grants GP2040-CE compatibility with the Playstation 4 or Playstation 5 console by using another controller or dongle to answer authentication

Depending on the controller or dongle used, you may be limited to only Playstation 4 support. 

For more information, refer to [PS Passthrough](add-ons/ps-passthrough) in the `Web Configurator - Add-ons` section of the documention.

## Why does my controller stop responding after a while on PS4 or PS5?

PlayStation 4 and PlayStation 5 use an authentication mechanism to ensure only authorized controllers are used on the console. If a device doesn't implement this authentication mechanism the controller is subject to an 8-minute timeout.

Using PS4 mode on a PC does not result in any timeout behavior, but note that XInput is the suggested mode on PC for best compatibility.

## What is the 8-Minute Timeout?

The 8-minute timeout works like this:

1. Plug your controller into the PS4/5
2. Press the PS button to initialize the controller
3. Play for roughly 8 minutes
4. Console rejects the controller due to failed authentication
5. User unplugs their non-functional controller
6. Go back to step 1 and repeat

## How do I avoid the 8-Minute Timeout?

GP2040-CE allows you to upload the files required to authenticate your device via the [PS4 Mode add-on](add-ons/ps4-mode), which effectively removes this timeout issue. 

If you're using an OLED display, the input mode will change from `PS4` to `PS4:AS` to indicate your device has successfully authenticated with the PS4/PS5 console.

## How do I get the necessary keys and files for the PS4 Mode add-on?

The GP2040-CE project will not provide ***any*** files or information related to acquiring these keys and files.

!> **Do not ask via any of communication channels (e.g. social media, direct messages, Github Issues, GP2040-CE) as this will result in a permanent blacklist/ban.**

## What controller or dongle do I use for PS Passthrough?

This depends on whether you are aiming to avoid the 8 minute timeout on PS4 or are looking to get your controller working on PS5.

### Playstation 4

For avoiding the 8 minute timeout on PS4, any PS4 controller or converter should work unless the device is not a valid and accepted as a proper controller on its own with a PS4.

### Playstation 5

For PS5 compatibility, there are a number of devices that the PS5 recognizes as a "categorized" controller (e.g. arcade stick, racing wheel, flight sim joystick, etc.) where the developer of the game has supported the use of "categorized" controllers. Any USB device that works as a categorized PS4 controller (arcade stick/racing wheel/flight sim joystick) or a USB dongle that allows a controller to authenticate should work with GP2040-CE. Note that this functionality is not present in all games, but is common in fighting games and racing simulators.

Some devices that do work for PS5 include, but are not limited to;

- [MagicBoots FPS Adapter Joysick Converter for PS4 v1.1](https://www.mayflash.com/product/MAGPS4.html)
- [N5 for PXN Game Steering Wheel](https://pxn-game.com/products/n5-for-pxn-game-steering-wheel)
- PCBs from Licensed PS4 Arcade Sticks

Because your experience and purchased products may vary, GP2040-CE will not provide specific recommendations beyond these known working devices.