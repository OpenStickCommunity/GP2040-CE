# GP2040-CE Web Configurator

Simple web application for gamepad configuration.

## Requirements

* NodeJS and NPM to build the React app

## Development

### Mocked board

Run `npm run dev`. This will start up the React app and an Express instance for mock data during development, allowing testing of the configurator without loading it onto the MCU, which is a SLOW process.

The mock data Express server is running at http://localhost:8080.

### Connected board

Run `npm run dev-board`. This will start up the React app and try connect to the board running the
[web-configurator](https://gp2040-ce.info/#/web-configurator).

### API Endpoints

When adding a new API endpoint to the GP2040-CE Configurator:

> NOTE: All endpoints should be under the `/api` path

* Add the endpoint to `src/webserver.cpp`.
  * Add a define at the top: `#define API_GET_NEW_ENDPOINT "/api/getNewEndpoint"`
    * Use the naming convention `API_[GET/SET]_{ENDPOINT_PATH}` for the define
    * Use the naming convention `/api/{[get/set]EndpointPath}` for the path
  * Create the backing method with the same name as the API path: `string getNewEndpoint()`
  * Add handling code in `fs_open_custom` for the API path
* Add a mock data endpoint to `src/server/app.js`
* Add the client-side API function to `www/src/Services/WebApi.js`.
* Add the endpoint to the Postman collection at `www/server/docs/GP2040-CE.postman_collection.json`

### Files

Use JPG and PNG files for images, SVG file support requires modification to the lwIP library that hasn't been completed.

## Building

If you just want to rebuild the React app in production mode for some reason, you can run `npm run build` from the `www` folder.

The `makedatafs.js` script is used to build the React application and regenerate the embedded data in `lib/httpd/fsdata.c`. The `makefsdata` tool that performs the conversion doesn't set the correct `#include` lines for our use. This script will fix this issue.

Precompiled binaries of `makedatafs` for Windows, Linux and macOS are included in the `tools` folder.

## References

Original example:

* <https://forums.raspberrypi.com/viewtopic.php?t=306888>
* <https://github.com/maxnet/pico-webserver>

Convert text to bytes:

* <https://onlineasciitools.com/convert-ascii-to-bytes>
* <https://onlineasciitools.com/convert-bytes-to-ascii>

Create image map:

* <https://www.image-map.net/>
