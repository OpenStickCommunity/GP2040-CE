# BTStack Integration

This is an extremely WIP integration of the BTStack Bluetooth framework.
If you use this as is, be ready to mess with Bluetooth in your settings a lot, as this frequently refuses to connect.

## Todo:
- BLE transport handoff to BR/EDR (will improve the time it takes for the device to appear when connecting.)
- Integrate Async framework around main core loop, and run BTStack on the main core if Bluetooth is enabled.
- Make the XInput mappings sane (I just winged them as a proof of concept.)
- Add support for modes other than XInput/HID
- Manually set the flash memory offset, ensure that BTStack and the config aren't corrupting each other.
- Investigate why reconnecting consistently causes driver failures.
- Optimize report packing
- Don't run send loop if we aren't connected

## HERE BE DRAGONS

If you want to debug the actual bluetooth connection on Windows via Wireshark. Use a Bluetooth USB dongle. Internal chipset bluetooth can't be inspected by Wireshark on Windows.
Wireshark capture USBPcap, with a filter along the lines of:
(hci_usb || bthci_acl || btl2cap || btrfcomm || btspp || hci_h1 || hci_h4 || hci_mon || bluetooth || sm) && (!bthci_evt.bd_addr || bthci_evt.bd_addr == <BTStack Pico MAC address>)
Some of these are redundant, but for me stuff would infrequently not appear unless the lowest protocol was in the filter set.

Device Manager properties for the HID device can also offer info.
What I've seen: 
- address space conflict: HID descriptor bit widths are wrong
- Extra End collection: HID descriptor collection scopes are wrong


If you are messing with this, be warned that Windows is going to FIGHT you.
Sometimes your device will start failing to connect after making a change, and then refuse to connect even after reverting it. (Windows is probably caching bad data)
If this happens:
1) disconnect device if "failed"
2) turn off bluetooth
3) physically disconnect Bluetooth dongle
4) reconnect, turn on, try again

If it STILL doesn't work:
1) turn off bluetooth
2) go to device manager
3) show view->show hidden devices
4) right click uninstall ALL bluetooth devices and drivers
5) restart your computer


USEFUL STUFF

Pico W SDK Notes:
https://github.com/raspberrypi/pico-sdk/releases/tag/1.5.0

Standalone PICO example: (is LE mode)
https://github.com/raspberrypi/pico-examples/tree/master/pico_w/bt/standalone

Classic BT PICO example:
https://github.com/MrGreensWorkshop/RasPiPicoSDK_BT_Classic_SPP

Keyboard Example:
https://github.com/bluekitchen/btstack/blob/master/example/hid_keyboard_demo.c

Gamepad Example: (very out of date)
https://github.com/bluekitchen/btstack/blob/1d20facba516eaeba1c5de014b16b0aeb8e8ae6d/example/hid_gamepad_example.c

HID spec:
https://www.usb.org/sites/default/files/hid1_11.pdf

HID descriptor spec:
https://usb.org/sites/default/files/hut1_4.pdf

HID descriptor parser:
https://eleccelerator.com/usbdescreqparser/

Bluetooth SDP/ATT/etc MAGIC numbers spec:
https://btprodspecificationrefs.blob.core.windows.net/assigned-numbers/Assigned%20Number%20Types/Assigned_Numbers.pdf

The closest thing to an explanation for some of this:
https://www.amd.e-technik.uni-rostock.de/ma/gol/lectures/wirlec/bluetooth_info/sdp.html