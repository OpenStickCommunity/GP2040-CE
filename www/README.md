# GP2040 Web Configurator

Simple web application for gamepad configuration.

## Requirements

* PlatformIO with the [Wiz-IO Pico](https://github.com/Wiz-IO/wizio-pico) platform module
* NodeJS and NPM to build the React app
* Python for the React app build script
* Perl for some text replacement done by the React app build script

## Development

The `build-web-py` is used to build the React application and regenerate the embedded version in `lib/httpd/fsdata.c`. The `makefsdata` tool that performs the conversion doesn't set the correct `#include` lines for our use. This script will fix this issue.

The lwIP lib in Wiz-IO Pico PlatformIO platform doesn't include the `makefsdata` source. A precompiled version for Windows and Unix are included in the `tools` folder.

Use JPG and PNG files for images, SVG file support requires modification that I haven't done.

## References

Original example:

* <https://forums.raspberrypi.com/viewtopic.php?t=306888>
* <https://github.com/maxnet/pico-webserver>

WizIO example:

* <https://github.com/Wiz-IO/wizio-pico-examples/tree/main/baremetal/pico-usb-webserver>

Convert text to bytes:

* <https://onlineasciitools.com/convert-ascii-to-bytes>
* <https://onlineasciitools.com/convert-bytes-to-ascii>

Create image map:

* <https://www.image-map.net/>
