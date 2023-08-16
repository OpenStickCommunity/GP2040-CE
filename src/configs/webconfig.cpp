#include "configs/webconfig.h"
#include "config.pb.h"
#include "configs/base64.h"

#include "storagemanager.h"
#include "configmanager.h"
#include "AnimationStorage.hpp"
#include "system.h"
#include "config_utils.h"

#include <cstring>
#include <string>
#include <vector>
#include <memory>

#include <pico/types.h>

// HTTPD Includes
#include <ArduinoJson.h>
#include "rndis.h"
#include "fs.h"
#include "fscustom.h"
#include "fsdata.h"
#include "lwip/apps/httpd.h"
#include "lwip/def.h"
#include "lwip/mem.h"

#include "bitmaps.h"

#define PATH_CGI_ACTION "/cgi/action"

#define LWIP_HTTPD_POST_MAX_PAYLOAD_LEN (1024 * 8)

using namespace std;

extern struct fsdata_file file__index_html[];

const static char* spaPaths[] = { "/display-config", "/led-config", "/pin-mapping", "/keyboard-mapping", "/settings", "/reset-settings", "/add-ons", "/custom-theme" };
const static char* excludePaths[] = { "/css", "/images", "/js", "/static" };
const static uint32_t rebootDelayMs = 500;
static string http_post_uri;
static char http_post_payload[LWIP_HTTPD_POST_MAX_PAYLOAD_LEN];
static uint16_t http_post_payload_len = 0;
static absolute_time_t rebootDelayTimeout = nil_time;
static System::BootMode rebootMode = System::BootMode::DEFAULT;

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K>
static void __attribute__((noinline)) readDoc(T& var, const DynamicJsonDocument& doc, const K& key)
{
	var = doc[key];
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K0, typename K1>
static void __attribute__((noinline)) readDoc(T& var, const DynamicJsonDocument& doc, const K0& key0, const K1& key1)
{
	var = doc[key0][key1];
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K0, typename K1, typename K2>
static void __attribute__((noinline)) readDoc(T& var, const DynamicJsonDocument& doc, const K0& key0, const K1& key1, const K2& key2)
{
	var = doc[key0][key1][key2];
}

// Don't inline this function, we do not want to consume stack space in the calling function
static bool __attribute__((noinline)) hasValue(const DynamicJsonDocument& doc, const char* key0, const char* key1)
{
	return doc[key0][key1] != nullptr;
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T>
static void __attribute__((noinline)) docToValue(T& value, const DynamicJsonDocument& doc, const char* key)
{
	if (doc[key] != nullptr)
	{
		value = doc[key];
	}
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T>
static void __attribute__((noinline)) docToValue(T& value, const DynamicJsonDocument& doc, const char* key0, const char* key1)
{
	if (doc[key0][key1] != nullptr)
	{
		value = doc[key0][key1];
	}
}

// Don't inline this function, we do not want to consume stack space in the calling function
static void __attribute__((noinline)) docToPinLegacy(uint8_t& pin, const DynamicJsonDocument& doc, const char* key)
{
	if (doc[key] != nullptr)
	{
		pin = doc[key] < 0 ? 0xff : doc[key];
	}
}

// Don't inline this function, we do not want to consume stack space in the calling function
static void __attribute__((noinline)) docToPin(int32_t& pin, const DynamicJsonDocument& doc, const char* key)
{
	if (doc.containsKey(key))
	{
		pin = doc[key];
	}
	if (!isValidPin(pin))
	{
		pin = -1;
	}
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K>
static void __attribute__((noinline)) writeDoc(DynamicJsonDocument& doc, const K& key, const T& var)
{
	doc[key] = var;
}

// Don't inline this function, we do not want to consume stack space in the calling function
// Web-config frontend compatibility workaround
template <typename K>
static void __attribute__((noinline)) writeDoc(DynamicJsonDocument& doc, const K& key, const bool& var)
{
	doc[key] = var ? 1 : 0;
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K0, typename K1>
static void __attribute__((noinline)) writeDoc(DynamicJsonDocument& doc, const K0& key0, const K1& key1, const T& var)
{
	doc[key0][key1] = var;
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K0, typename K1, typename K2>
static void __attribute__((noinline)) writeDoc(DynamicJsonDocument& doc, const K0& key0, const K1& key1, const K2& key2, const T& var)
{
	doc[key0][key1][key2] = var;
}

static int32_t cleanPin(int32_t pin) { return isValidPin(pin) ? pin : -1; }

void WebConfig::setup() {
	rndis_init();
}

void WebConfig::loop() {
	// rndis http server requires inline functions (non-class)
	rndis_task();

	if (!is_nil_time(rebootDelayTimeout) && time_reached(rebootDelayTimeout)) {
		System::reboot(rebootMode);
	}
}

enum class HttpStatusCode
{
	_200,
	_400,
	_500,
};

struct DataAndStatusCode
{
	DataAndStatusCode(string&& data, HttpStatusCode statusCode) :
		data(std::move(data)),
		statusCode(statusCode)
	{}

	string data;
	HttpStatusCode statusCode;
};

// **** WEB SERVER Overrides and Special Functionality ****
int set_file_data(fs_file* file, const DataAndStatusCode& dataAndStatusCode)
{
	static string returnData;

	const char* statusCodeStr = "";
	switch (dataAndStatusCode.statusCode)
	{
		case HttpStatusCode::_200: statusCodeStr = "200 OK"; break;
		case HttpStatusCode::_400: statusCodeStr = "400 Bad Request"; break;
		case HttpStatusCode::_500: statusCodeStr = "500 Internal Server Error"; break;
	}

	returnData.clear();
	returnData.append("HTTP/1.0 ");
	returnData.append(statusCodeStr);
	returnData.append("\r\n");
	returnData.append(
		"Server: GP2040-CE " GP2040VERSION "\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: "
	);
	returnData.append(std::to_string(dataAndStatusCode.data.length()));
	returnData.append("\r\n\r\n");
	returnData.append(dataAndStatusCode.data);

	file->data = returnData.c_str();
	file->len = returnData.size();
	file->index = file->len;
	file->http_header_included = file->http_header_included;
	file->pextension = NULL;

	return 1;
}

int set_file_data(fs_file *file, string&& data)
{
	return set_file_data(file, DataAndStatusCode(std::move(data), HttpStatusCode::_200));
}

DynamicJsonDocument get_post_data()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	deserializeJson(doc, http_post_payload, http_post_payload_len);
	return doc;
}

void save_hotkey(HotkeyEntry* hotkey, const DynamicJsonDocument& doc, const string hotkey_key)
{
	readDoc(hotkey->auxMask, doc, hotkey_key, "auxMask");
	uint32_t buttonsMask = doc[hotkey_key]["buttonsMask"];
	uint32_t dpadMask = 0;
	if (buttonsMask & GAMEPAD_MASK_DU) {
		dpadMask |= GAMEPAD_MASK_UP;
	}
	if (buttonsMask & GAMEPAD_MASK_DD) {
		dpadMask |= GAMEPAD_MASK_DOWN;
	}
	if (buttonsMask & GAMEPAD_MASK_DL) {
		dpadMask |= GAMEPAD_MASK_LEFT;
	}
	if (buttonsMask & GAMEPAD_MASK_DR) {
		dpadMask |= GAMEPAD_MASK_RIGHT;
	}
	buttonsMask &= ~(GAMEPAD_MASK_DU | GAMEPAD_MASK_DD | GAMEPAD_MASK_DL | GAMEPAD_MASK_DR);
	hotkey->dpadMask = dpadMask;
	hotkey->buttonsMask = buttonsMask;
	readDoc(hotkey->action, doc, hotkey_key, "action");
}

void load_hotkey(const HotkeyEntry* hotkey, DynamicJsonDocument& doc, const string hotkey_key)
{
	writeDoc(doc, hotkey_key, "auxMask", hotkey->auxMask);
	uint32_t buttonsMask = hotkey->buttonsMask;
	if (hotkey->dpadMask & GAMEPAD_MASK_UP) {
		buttonsMask |= GAMEPAD_MASK_DU;
	}
	if (hotkey->dpadMask & GAMEPAD_MASK_DOWN) {
		buttonsMask |= GAMEPAD_MASK_DD;
	}
	if (hotkey->dpadMask & GAMEPAD_MASK_LEFT) {
		buttonsMask |= GAMEPAD_MASK_DL;
	}
	if (hotkey->dpadMask & GAMEPAD_MASK_RIGHT) {
		buttonsMask |= GAMEPAD_MASK_DR;
	}
	writeDoc(doc, hotkey_key, "buttonsMask", buttonsMask);
	writeDoc(doc, hotkey_key, "action", hotkey->action);
}

// LWIP callback on HTTP POST to validate the URI
err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       uint16_t http_request_len, int content_len, char *response_uri,
                       uint16_t response_uri_len, uint8_t *post_auto_wnd)
{
	LWIP_UNUSED_ARG(http_request);
	LWIP_UNUSED_ARG(http_request_len);
	LWIP_UNUSED_ARG(content_len);
	LWIP_UNUSED_ARG(response_uri);
	LWIP_UNUSED_ARG(response_uri_len);
	LWIP_UNUSED_ARG(post_auto_wnd);

	if (!uri || strncmp(uri, "/api", 4) != 0) {
		return ERR_ARG;
	}

	http_post_uri = uri;
	http_post_payload_len = 0;
	memset(http_post_payload, 0, LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	return ERR_OK;
}

// LWIP callback on HTTP POST to for receiving payload
err_t httpd_post_receive_data(void *connection, struct pbuf *p)
{
	LWIP_UNUSED_ARG(connection);

	// Cache the received data to http_post_payload
	while (p != NULL)
	{
		if (http_post_payload_len + p->len <= LWIP_HTTPD_POST_MAX_PAYLOAD_LEN)
		{
			MEMCPY(http_post_payload + http_post_payload_len, p->payload, p->len);
			http_post_payload_len += p->len;
		}
		else // Buffer overflow
		{
			http_post_payload_len = 0xffff;
			break;
		}

		p = p->next;
	}

	// Need to release memory here or will leak
	pbuf_free(p);

	// If the buffer overflows, error out
	if (http_post_payload_len == 0xffff) {
		return ERR_BUF;
	}

	return ERR_OK;
}

// LWIP callback to set the HTTP POST response_uri, which can then be looked up via the fs_custom callbacks
void httpd_post_finished(void *connection, char *response_uri, uint16_t response_uri_len)
{
	LWIP_UNUSED_ARG(connection);

	if (http_post_payload_len != 0xffff) {
		strncpy(response_uri, http_post_uri.c_str(), response_uri_len);
		response_uri[response_uri_len - 1] = '\0';
	}
}

void addUsedPinsArray(DynamicJsonDocument& doc)
{
	auto usedPins = doc.createNestedArray("usedPins");

	const auto addPinIfValid = [&](int pin)
	{
		if (pin >= 0 && pin < NUM_BANK0_GPIOS)
		{
			usedPins.add(pin);
		}
	};

	const PinMappings& pinMappings = Storage::getInstance().getPinMappings();
	addPinIfValid(pinMappings.pinDpadUp);
	addPinIfValid(pinMappings.pinDpadDown);
	addPinIfValid(pinMappings.pinDpadLeft);
	addPinIfValid(pinMappings.pinDpadRight);
	addPinIfValid(pinMappings.pinButtonB1);
	addPinIfValid(pinMappings.pinButtonB2);
	addPinIfValid(pinMappings.pinButtonB3);
	addPinIfValid(pinMappings.pinButtonB4);
	addPinIfValid(pinMappings.pinButtonL1);
	addPinIfValid(pinMappings.pinButtonR1);
	addPinIfValid(pinMappings.pinButtonL2);
	addPinIfValid(pinMappings.pinButtonR2);
	addPinIfValid(pinMappings.pinButtonS1);
	addPinIfValid(pinMappings.pinButtonS2);
	addPinIfValid(pinMappings.pinButtonL3);
	addPinIfValid(pinMappings.pinButtonR3);
	addPinIfValid(pinMappings.pinButtonA1);
	addPinIfValid(pinMappings.pinButtonA2);
	addPinIfValid(pinMappings.pinButtonFn);

	// TODO: Exclude non-button pins from validation for now, fix this when validation reworked
	// addPinIfValid(boardOptions.i2cSDAPin);
	// addPinIfValid(boardOptions.i2cSCLPin);

	// TODO: Used Pins logic does not work in add-ons, fix this
	// const AnalogOptions& analogOptions = Storage::getInstance().getAddonOptions().analogOptions;
	// addPinIfValid(analogOptions.analogAdc1PinX);
	// addPinIfValid(analogOptions.analogAdc1PinY);
	// addPinIfValid(analogOptions.analogAdc2PinX);
	// addPinIfValid(analogOptions.analogAdc2PinY);

	// TODO: Exclude non-button pins from validation for now, fix this when validation reworked
	// addPinIfValid(addonOptions.buzzerPin);
}

std::string serialize_json(JsonDocument &doc)
{
	string data;
	serializeJson(doc, data);
	return data;
}

std::string getUsedPins()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	addUsedPinsArray(doc);
	return serialize_json(doc);
}

std::string setDisplayOptions(DisplayOptions& displayOptions)
{
	DynamicJsonDocument doc = get_post_data();
	readDoc(displayOptions.enabled, doc, "enabled");
	docToPin(displayOptions.i2cSDAPin, doc, "sdaPin");
	docToPin(displayOptions.i2cSCLPin, doc, "sclPin");
	readDoc(displayOptions.i2cAddress, doc, "i2cAddress");
	readDoc(displayOptions.i2cBlock, doc, "i2cBlock");
	readDoc(displayOptions.i2cSpeed, doc, "i2cSpeed");
	readDoc(displayOptions.flip, doc, "flipDisplay");
	readDoc(displayOptions.invert, doc, "invertDisplay");
	readDoc(displayOptions.buttonLayout, doc, "buttonLayout");
	readDoc(displayOptions.buttonLayoutRight, doc, "buttonLayoutRight");
	readDoc(displayOptions.splashMode, doc, "splashMode");
	readDoc(displayOptions.splashChoice, doc, "splashChoice");
	readDoc(displayOptions.splashDuration, doc, "splashDuration");
	readDoc(displayOptions.displaySaverTimeout, doc, "displaySaverTimeout");

	readDoc(displayOptions.buttonLayoutCustomOptions.paramsLeft.layout, doc, "buttonLayoutCustomOptions", "params", "layout");
	readDoc(displayOptions.buttonLayoutCustomOptions.paramsLeft.common.startX, doc, "buttonLayoutCustomOptions", "params", "startX");
	readDoc(displayOptions.buttonLayoutCustomOptions.paramsLeft.common.startY, doc, "buttonLayoutCustomOptions", "params", "startY");
	readDoc(displayOptions.buttonLayoutCustomOptions.paramsLeft.common.buttonRadius, doc, "buttonLayoutCustomOptions", "params", "buttonRadius");
	readDoc(displayOptions.buttonLayoutCustomOptions.paramsLeft.common.buttonPadding, doc, "buttonLayoutCustomOptions", "params", "buttonPadding");

	readDoc(displayOptions.buttonLayoutCustomOptions.paramsRight.layout, doc, "buttonLayoutCustomOptions", "paramsRight", "layout");
	readDoc(displayOptions.buttonLayoutCustomOptions.paramsRight.common.startX, doc, "buttonLayoutCustomOptions", "paramsRight", "startX");
	readDoc(displayOptions.buttonLayoutCustomOptions.paramsRight.common.startY, doc, "buttonLayoutCustomOptions", "paramsRight", "startY");
	readDoc(displayOptions.buttonLayoutCustomOptions.paramsRight.common.buttonRadius, doc, "buttonLayoutCustomOptions", "paramsRight", "buttonRadius");
	readDoc(displayOptions.buttonLayoutCustomOptions.paramsRight.common.buttonPadding, doc, "buttonLayoutCustomOptions", "paramsRight", "buttonPadding");

	return serialize_json(doc);
}

std::string setDisplayOptions()
{
	std::string response = setDisplayOptions(Storage::getInstance().getDisplayOptions());
	Storage::getInstance().save();
	return response;
}

std::string setPreviewDisplayOptions()
{
	return setDisplayOptions(Storage::getInstance().getPreviewDisplayOptions());
}

std::string getDisplayOptions() // Manually set Document Attributes for the display
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	const DisplayOptions& displayOptions = Storage::getInstance().getDisplayOptions();
	writeDoc(doc, "enabled", displayOptions.enabled ? 1 : 0);
	writeDoc(doc, "sdaPin", cleanPin(displayOptions.i2cSDAPin));
	writeDoc(doc, "sclPin", cleanPin(displayOptions.i2cSCLPin));
	writeDoc(doc, "i2cAddress", displayOptions.i2cAddress);
	writeDoc(doc, "i2cBlock", displayOptions.i2cBlock);
	writeDoc(doc, "i2cSpeed", displayOptions.i2cSpeed);
	writeDoc(doc, "flipDisplay", displayOptions.flip ? 1 : 0);
	writeDoc(doc, "invertDisplay", displayOptions.invert ? 1 : 0);
	writeDoc(doc, "buttonLayout", displayOptions.buttonLayout);
	writeDoc(doc, "buttonLayoutRight", displayOptions.buttonLayoutRight);
	writeDoc(doc, "splashMode", displayOptions.splashMode);
	writeDoc(doc, "splashChoice", displayOptions.splashChoice);
	writeDoc(doc, "splashDuration", displayOptions.splashDuration);
	writeDoc(doc, "displaySaverTimeout", displayOptions.displaySaverTimeout);

	writeDoc(doc, "buttonLayoutCustomOptions", "params", "layout", displayOptions.buttonLayoutCustomOptions.paramsLeft.layout);
	writeDoc(doc, "buttonLayoutCustomOptions", "params", "startX", displayOptions.buttonLayoutCustomOptions.paramsLeft.common.startX);
	writeDoc(doc, "buttonLayoutCustomOptions", "params", "startY", displayOptions.buttonLayoutCustomOptions.paramsLeft.common.startY);
	writeDoc(doc, "buttonLayoutCustomOptions", "params", "buttonRadius", displayOptions.buttonLayoutCustomOptions.paramsLeft.common.buttonRadius);
	writeDoc(doc, "buttonLayoutCustomOptions", "params", "buttonPadding", displayOptions.buttonLayoutCustomOptions.paramsLeft.common.buttonPadding);

	writeDoc(doc, "buttonLayoutCustomOptions", "paramsRight", "layout", displayOptions.buttonLayoutCustomOptions.paramsRight.layout);
	writeDoc(doc, "buttonLayoutCustomOptions", "paramsRight", "startX", displayOptions.buttonLayoutCustomOptions.paramsRight.common.startX);
	writeDoc(doc, "buttonLayoutCustomOptions", "paramsRight", "startY", displayOptions.buttonLayoutCustomOptions.paramsRight.common.startY);
	writeDoc(doc, "buttonLayoutCustomOptions", "paramsRight", "buttonRadius", displayOptions.buttonLayoutCustomOptions.paramsRight.common.buttonRadius);
	writeDoc(doc, "buttonLayoutCustomOptions", "paramsRight", "buttonPadding", displayOptions.buttonLayoutCustomOptions.paramsRight.common.buttonPadding);

	return serialize_json(doc);
}

std::string getSplashImage()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN * 10); // TODO: Figoure out correct length

	JsonArray splashImageArray = doc.createNestedArray("splashImage");
	const DisplayOptions& displayOptions = Storage::getInstance().getDisplayOptions();
	std::vector<char> temp(sizeof(displayOptions.splashImage.bytes), '\0');
	memcpy(temp.data(), displayOptions.splashImage.bytes, displayOptions.splashImage.size);
	copyArray(reinterpret_cast<const uint8_t*>(temp.data()), temp.size(), splashImageArray);

	return serialize_json(doc);
}

std::string setSplashImage()
{
	DynamicJsonDocument doc = get_post_data();

	DisplayOptions& displayOptions = Storage::getInstance().getDisplayOptions();

	std::string decoded;
	std::string base64String = doc["splashImage"];
	Base64::Decode(base64String, decoded);
	const size_t length = std::min(decoded.length(), sizeof(displayOptions.splashImage.bytes));

	memcpy(displayOptions.splashImage.bytes, decoded.data(), length);
	displayOptions.splashImage.size = length;

	Storage::getInstance().save();

	return serialize_json(doc);
}

std::string setProfileOptions()
{
	DynamicJsonDocument doc = get_post_data();

	ProfileOptions& profileOptions = Storage::getInstance().getProfileOptions();
	JsonObject options = doc.as<JsonObject>();
	JsonArray alts = options["alternativePinMappings"];
	int altsIndex = 0;
	for (JsonObject alt : alts) {
		profileOptions.alternativePinMappings[altsIndex].pinButtonB1 = alt["B1"].as<int>();
		profileOptions.alternativePinMappings[altsIndex].pinButtonB2 = alt["B2"].as<int>();
		profileOptions.alternativePinMappings[altsIndex].pinButtonB3 = alt["B3"].as<int>();
		profileOptions.alternativePinMappings[altsIndex].pinButtonB4 = alt["B4"].as<int>();
		profileOptions.alternativePinMappings[altsIndex].pinButtonL1 = alt["L1"].as<int>();
		profileOptions.alternativePinMappings[altsIndex].pinButtonR1 = alt["R1"].as<int>();
		profileOptions.alternativePinMappings[altsIndex].pinButtonL2 = alt["L2"].as<int>();
		profileOptions.alternativePinMappings[altsIndex].pinButtonR2 = alt["R2"].as<int>();
		profileOptions.alternativePinMappings[altsIndex].pinDpadUp = alt["Up"].as<int>();
		profileOptions.alternativePinMappings[altsIndex].pinDpadDown = alt["Down"].as<int>();
		profileOptions.alternativePinMappings[altsIndex].pinDpadLeft = alt["Left"].as<int>();
		profileOptions.alternativePinMappings[altsIndex].pinDpadRight = alt["Right"].as<int>();
		profileOptions.alternativePinMappings_count = ++altsIndex;
		if (altsIndex > 2) break;
	}

	Storage::getInstance().save();
	return serialize_json(doc);
}

std::string getProfileOptions()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);

	ProfileOptions& profileOptions = Storage::getInstance().getProfileOptions();
	JsonArray alts = doc.createNestedArray("alternativePinMappings");
	for (int i = 0; i < profileOptions.alternativePinMappings_count; i++) {
		JsonObject altMappings = alts.createNestedObject();
		altMappings["B1"] = profileOptions.alternativePinMappings[i].pinButtonB1;
		altMappings["B2"] = profileOptions.alternativePinMappings[i].pinButtonB2;
		altMappings["B3"] = profileOptions.alternativePinMappings[i].pinButtonB3;
		altMappings["B4"] = profileOptions.alternativePinMappings[i].pinButtonB4;
		altMappings["L1"] = profileOptions.alternativePinMappings[i].pinButtonL1;
		altMappings["R1"] = profileOptions.alternativePinMappings[i].pinButtonR1;
		altMappings["L2"] = profileOptions.alternativePinMappings[i].pinButtonL2;
		altMappings["R2"] = profileOptions.alternativePinMappings[i].pinButtonR2;
		altMappings["Up"] = profileOptions.alternativePinMappings[i].pinDpadUp;
		altMappings["Down"] = profileOptions.alternativePinMappings[i].pinDpadDown;
		altMappings["Left"] = profileOptions.alternativePinMappings[i].pinDpadLeft;
		altMappings["Right"] = profileOptions.alternativePinMappings[i].pinDpadRight;
	}

	return serialize_json(doc);
}

std::string setGamepadOptions()
{
	DynamicJsonDocument doc = get_post_data();

	GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();
	readDoc(gamepadOptions.dpadMode, doc, "dpadMode");
	readDoc(gamepadOptions.inputMode, doc, "inputMode");
	readDoc(gamepadOptions.socdMode, doc, "socdMode");
	readDoc(gamepadOptions.switchTpShareForDs4, doc, "switchTpShareForDs4");
	readDoc(gamepadOptions.lockHotkeys, doc, "lockHotkeys");
	readDoc(gamepadOptions.fourWayMode, doc, "fourWayMode");
	readDoc(gamepadOptions.profileNumber, doc, "profileNumber");
	readDoc(gamepadOptions.ps4ControllerType, doc, "ps4ControllerType");

	HotkeyOptions& hotkeyOptions = Storage::getInstance().getHotkeyOptions();
	save_hotkey(&hotkeyOptions.hotkey01, doc, "hotkey01");
	save_hotkey(&hotkeyOptions.hotkey02, doc, "hotkey02");
	save_hotkey(&hotkeyOptions.hotkey03, doc, "hotkey03");
	save_hotkey(&hotkeyOptions.hotkey04, doc, "hotkey04");
	save_hotkey(&hotkeyOptions.hotkey05, doc, "hotkey05");
	save_hotkey(&hotkeyOptions.hotkey06, doc, "hotkey06");
	save_hotkey(&hotkeyOptions.hotkey07, doc, "hotkey07");
	save_hotkey(&hotkeyOptions.hotkey08, doc, "hotkey08");
	save_hotkey(&hotkeyOptions.hotkey09, doc, "hotkey09");
	save_hotkey(&hotkeyOptions.hotkey10, doc, "hotkey10");
	save_hotkey(&hotkeyOptions.hotkey11, doc, "hotkey11");
	save_hotkey(&hotkeyOptions.hotkey12, doc, "hotkey12");

	ForcedSetupOptions& forcedSetupOptions = Storage::getInstance().getForcedSetupOptions();
	readDoc(forcedSetupOptions.mode, doc, "forcedSetupMode");

	Storage::getInstance().save();

	return serialize_json(doc);
}

std::string getGamepadOptions()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);

	GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();
	writeDoc(doc, "dpadMode", gamepadOptions.dpadMode);
	writeDoc(doc, "inputMode", gamepadOptions.inputMode);
	writeDoc(doc, "socdMode", gamepadOptions.socdMode);
	writeDoc(doc, "switchTpShareForDs4", gamepadOptions.switchTpShareForDs4 ? 1 : 0);
	writeDoc(doc, "lockHotkeys", gamepadOptions.lockHotkeys ? 1 : 0);
	writeDoc(doc, "fourWayMode", gamepadOptions.fourWayMode ? 1 : 0);
	writeDoc(doc, "profileNumber", gamepadOptions.profileNumber);
	writeDoc(doc, "ps4ControllerType", gamepadOptions.ps4ControllerType);

	const PinMappings& pinMappings = Storage::getInstance().getPinMappings();
	writeDoc(doc, "fnButtonPin", pinMappings.pinButtonFn);

	HotkeyOptions& hotkeyOptions = Storage::getInstance().getHotkeyOptions();
	load_hotkey(&hotkeyOptions.hotkey01, doc, "hotkey01");
	load_hotkey(&hotkeyOptions.hotkey02, doc, "hotkey02");
	load_hotkey(&hotkeyOptions.hotkey03, doc, "hotkey03");
	load_hotkey(&hotkeyOptions.hotkey04, doc, "hotkey04");
	load_hotkey(&hotkeyOptions.hotkey05, doc, "hotkey05");
	load_hotkey(&hotkeyOptions.hotkey06, doc, "hotkey06");
	load_hotkey(&hotkeyOptions.hotkey07, doc, "hotkey07");
	load_hotkey(&hotkeyOptions.hotkey08, doc, "hotkey08");
	load_hotkey(&hotkeyOptions.hotkey09, doc, "hotkey09");
	load_hotkey(&hotkeyOptions.hotkey10, doc, "hotkey10");
	load_hotkey(&hotkeyOptions.hotkey11, doc, "hotkey11");
	load_hotkey(&hotkeyOptions.hotkey12, doc, "hotkey12");

	ForcedSetupOptions& forcedSetupOptions = Storage::getInstance().getForcedSetupOptions();
	writeDoc(doc, "forcedSetupMode", forcedSetupOptions.mode);
	return serialize_json(doc);
}

std::string setLedOptions()
{
	DynamicJsonDocument doc = get_post_data();

	const auto readIndex = [&](int32_t& var, const char* key0, const char* key1)
	{
		var = -1;
		if (hasValue(doc, key0, key1))
		{
			readDoc(var, doc, key0, key1);
		}
	};

	LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	docToPin(ledOptions.dataPin, doc, "dataPin");
	readDoc(ledOptions.ledFormat, doc, "ledFormat");
	readDoc(ledOptions.ledLayout, doc, "ledLayout");
	readDoc(ledOptions.ledsPerButton, doc, "ledsPerButton");
	readDoc(ledOptions.brightnessMaximum, doc, "brightnessMaximum");
	readDoc(ledOptions.brightnessSteps, doc, "brightnessSteps");
	readIndex(ledOptions.indexUp, "ledButtonMap", "Up");
	readIndex(ledOptions.indexDown, "ledButtonMap", "Down");
	readIndex(ledOptions.indexLeft, "ledButtonMap", "Left");
	readIndex(ledOptions.indexRight, "ledButtonMap", "Right");
	readIndex(ledOptions.indexB1, "ledButtonMap", "B1");
	readIndex(ledOptions.indexB2, "ledButtonMap", "B2");
	readIndex(ledOptions.indexB3, "ledButtonMap", "B3");
	readIndex(ledOptions.indexB4, "ledButtonMap", "B4");
	readIndex(ledOptions.indexL1, "ledButtonMap", "L1");
	readIndex(ledOptions.indexR1, "ledButtonMap", "R1");
	readIndex(ledOptions.indexL2, "ledButtonMap", "L2");
	readIndex(ledOptions.indexR2, "ledButtonMap", "R2");
	readIndex(ledOptions.indexS1, "ledButtonMap", "S1");
	readIndex(ledOptions.indexS2, "ledButtonMap", "S2");
	readIndex(ledOptions.indexL3, "ledButtonMap", "L3");
	readIndex(ledOptions.indexR3, "ledButtonMap", "R3");
	readIndex(ledOptions.indexA1, "ledButtonMap", "A1");
	readIndex(ledOptions.indexA2, "ledButtonMap", "A2");
	readDoc(ledOptions.pledType, doc, "pledType");
	readDoc(ledOptions.pledPin1, doc, "pledPin1");
	readDoc(ledOptions.pledPin2, doc, "pledPin2");
	readDoc(ledOptions.pledPin3, doc, "pledPin3");
	readDoc(ledOptions.pledPin4, doc, "pledPin4");
	readDoc(ledOptions.pledColor, doc, "pledColor");

	Storage::getInstance().save();
	return serialize_json(doc);
}

std::string getLedOptions()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	const LEDOptions& ledOptions = Storage::getInstance().getLedOptions();
	writeDoc(doc, "dataPin", cleanPin(ledOptions.dataPin));
	writeDoc(doc, "ledFormat", ledOptions.ledFormat);
	writeDoc(doc, "ledLayout", ledOptions.ledLayout);
	writeDoc(doc, "ledsPerButton", ledOptions.ledsPerButton);
	writeDoc(doc, "brightnessMaximum", ledOptions.brightnessMaximum);
	writeDoc(doc, "brightnessSteps", ledOptions.brightnessSteps);

	const auto writeIndex = [&](const char* key0, const char* key1, int var)
	{
		if (var < 0)
		{
			writeDoc(doc, key0, key1, nullptr);
		}
		else
		{
			writeDoc(doc, key0, key1, var);
		}
	};
	writeIndex("ledButtonMap", "Up", ledOptions.indexUp);
	writeIndex("ledButtonMap", "Down", ledOptions.indexDown);
	writeIndex("ledButtonMap", "Left", ledOptions.indexLeft);
	writeIndex("ledButtonMap", "Right", ledOptions.indexRight);
	writeIndex("ledButtonMap", "B1", ledOptions.indexB1);
	writeIndex("ledButtonMap", "B2", ledOptions.indexB2);
	writeIndex("ledButtonMap", "B3", ledOptions.indexB3);
	writeIndex("ledButtonMap", "B4", ledOptions.indexB4);
	writeIndex("ledButtonMap", "L1", ledOptions.indexL1);
	writeIndex("ledButtonMap", "R1", ledOptions.indexR1);
	writeIndex("ledButtonMap", "L2", ledOptions.indexL2);
	writeIndex("ledButtonMap", "R2", ledOptions.indexR2);
	writeIndex("ledButtonMap", "S1", ledOptions.indexS1);
	writeIndex("ledButtonMap", "S2", ledOptions.indexS2);
	writeIndex("ledButtonMap", "L3", ledOptions.indexL3);
	writeIndex("ledButtonMap", "R3", ledOptions.indexR3);
	writeIndex("ledButtonMap", "A1", ledOptions.indexA1);
	writeIndex("ledButtonMap", "A2", ledOptions.indexA2);
	writeDoc(doc, "pledType", ledOptions.pledType);
	writeDoc(doc, "pledPin1", ledOptions.pledPin1);
	writeDoc(doc, "pledPin2", ledOptions.pledPin2);
	writeDoc(doc, "pledPin3", ledOptions.pledPin3);
	writeDoc(doc, "pledPin4", ledOptions.pledPin4);
	writeDoc(doc, "pledColor", ((RGB)ledOptions.pledColor).value(LED_FORMAT_RGB));

	return serialize_json(doc);
}

std::string setCustomTheme()
{
	DynamicJsonDocument doc = get_post_data();

	AnimationOptions options = AnimationStation::options;

	const auto readDocDefaultToZero = [&](const char* key0, const char* key1) -> uint32_t
	{
		uint32_t result = 0;
		if (hasValue(doc, key0, key1))
		{
			readDoc(result, doc, key0, key1);
		}
		return result;
	};

	readDoc(options.hasCustomTheme, doc, "enabled");
	options.customThemeUp 			= readDocDefaultToZero("Up", "u");
	options.customThemeDown 		= readDocDefaultToZero("Down", "u");
	options.customThemeLeft			= readDocDefaultToZero("Left", "u");
	options.customThemeRight		= readDocDefaultToZero("Right", "u");
	options.customThemeB1			= readDocDefaultToZero("B1", "u");
	options.customThemeB2			= readDocDefaultToZero("B2", "u");
	options.customThemeB3			= readDocDefaultToZero("B3", "u");
	options.customThemeB4			= readDocDefaultToZero("B4", "u");
	options.customThemeL1			= readDocDefaultToZero("L1", "u");
	options.customThemeR1			= readDocDefaultToZero("R1", "u");
	options.customThemeL2			= readDocDefaultToZero("L2", "u");
	options.customThemeR2			= readDocDefaultToZero("R2", "u");
	options.customThemeS1			= readDocDefaultToZero("S1", "u");
	options.customThemeS2			= readDocDefaultToZero("S2", "u");
	options.customThemeL3			= readDocDefaultToZero("L3", "u");
	options.customThemeR3			= readDocDefaultToZero("R3", "u");
	options.customThemeA1			= readDocDefaultToZero("A1", "u");
	options.customThemeA2			= readDocDefaultToZero("A2", "u");
	options.customThemeUpPressed	= readDocDefaultToZero("Up", "d");
	options.customThemeDownPressed	= readDocDefaultToZero("Down", "d");
	options.customThemeLeftPressed	= readDocDefaultToZero("Left", "d");
	options.customThemeRightPressed	= readDocDefaultToZero("Right", "d");
	options.customThemeB1Pressed	= readDocDefaultToZero("B1", "d");
	options.customThemeB2Pressed	= readDocDefaultToZero("B2", "d");
	options.customThemeB3Pressed	= readDocDefaultToZero("B3", "d");
	options.customThemeB4Pressed	= readDocDefaultToZero("B4", "d");
	options.customThemeL1Pressed	= readDocDefaultToZero("L1", "d");
	options.customThemeR1Pressed	= readDocDefaultToZero("R1", "d");
	options.customThemeL2Pressed	= readDocDefaultToZero("L2", "d");
	options.customThemeR2Pressed	= readDocDefaultToZero("R2", "d");
	options.customThemeS1Pressed	= readDocDefaultToZero("S1", "d");
	options.customThemeS2Pressed	= readDocDefaultToZero("S2", "d");
	options.customThemeL3Pressed	= readDocDefaultToZero("L3", "d");
	options.customThemeR3Pressed	= readDocDefaultToZero("R3", "d");
	options.customThemeA1Pressed	= readDocDefaultToZero("A1", "d");
	options.customThemeA2Pressed	= readDocDefaultToZero("A2", "d");

	AnimationStation::SetOptions(options);
	AnimationStore.save();

	return serialize_json(doc);
}

std::string getCustomTheme()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	const AnimationOptions& options = AnimationStation::options;

	writeDoc(doc, "enabled", options.hasCustomTheme);
	writeDoc(doc, "Up", "u", options.customThemeUp);
	writeDoc(doc, "Up", "d", options.customThemeUpPressed);
	writeDoc(doc, "Down", "u", options.customThemeDown);
	writeDoc(doc, "Down", "d", options.customThemeDownPressed);
	writeDoc(doc, "Left", "u", options.customThemeLeft);
	writeDoc(doc, "Left", "d", options.customThemeLeftPressed);
	writeDoc(doc, "Right", "u", options.customThemeRight);
	writeDoc(doc, "Right", "d", options.customThemeRightPressed);
	writeDoc(doc, "B1", "u", options.customThemeB1);
	writeDoc(doc, "B1", "d", options.customThemeB1Pressed);
	writeDoc(doc, "B2", "u", options.customThemeB2);
	writeDoc(doc, "B2", "d", options.customThemeB2Pressed);
	writeDoc(doc, "B3", "u", options.customThemeB3);
	writeDoc(doc, "B3", "d", options.customThemeB3Pressed);
	writeDoc(doc, "B4", "u", options.customThemeB4);
	writeDoc(doc, "B4", "d", options.customThemeB4Pressed);
	writeDoc(doc, "L1", "u", options.customThemeL1);
	writeDoc(doc, "L1", "d", options.customThemeL1Pressed);
	writeDoc(doc, "R1", "u", options.customThemeR1);
	writeDoc(doc, "R1", "d", options.customThemeR1Pressed);
	writeDoc(doc, "L2", "u", options.customThemeL2);
	writeDoc(doc, "L2", "d", options.customThemeL2Pressed);
	writeDoc(doc, "R2", "u", options.customThemeR2);
	writeDoc(doc, "R2", "d", options.customThemeR2Pressed);
	writeDoc(doc, "S1", "u", options.customThemeS1);
	writeDoc(doc, "S1", "d", options.customThemeS1Pressed);
	writeDoc(doc, "S2", "u", options.customThemeS2);
	writeDoc(doc, "S2", "d", options.customThemeS2Pressed);
	writeDoc(doc, "A1", "u", options.customThemeA1);
	writeDoc(doc, "A1", "d", options.customThemeA1Pressed);
	writeDoc(doc, "A2", "u", options.customThemeA2);
	writeDoc(doc, "A2", "d", options.customThemeA2Pressed);
	writeDoc(doc, "L3", "u", options.customThemeL3);
	writeDoc(doc, "L3", "d", options.customThemeL3Pressed);
	writeDoc(doc, "R3", "u", options.customThemeR3);
	writeDoc(doc, "R3", "d", options.customThemeR3Pressed);

	return serialize_json(doc);
}

std::string setPinMappings()
{
	DynamicJsonDocument doc = get_post_data();

	// PinMappings uses -1 to denote unassigned pins
	const auto convertPin = [&] (const char* key) -> int32_t
	{
		int pin = 0;
		readDoc(pin, doc, key);
		return isValidPin(pin) ? pin : -1;
	};

	PinMappings& pinMappings = Storage::getInstance().getPinMappings();
	pinMappings.pinDpadUp    = convertPin("Up");
	pinMappings.pinDpadDown  = convertPin("Down");
	pinMappings.pinDpadLeft  = convertPin("Left");
	pinMappings.pinDpadRight = convertPin("Right");
	pinMappings.pinButtonB1  = convertPin("B1");
	pinMappings.pinButtonB2  = convertPin("B2");
	pinMappings.pinButtonB3  = convertPin("B3");
	pinMappings.pinButtonB4  = convertPin("B4");
	pinMappings.pinButtonL1  = convertPin("L1");
	pinMappings.pinButtonR1  = convertPin("R1");
	pinMappings.pinButtonL2  = convertPin("L2");
	pinMappings.pinButtonR2  = convertPin("R2");
	pinMappings.pinButtonS1  = convertPin("S1");
	pinMappings.pinButtonS2  = convertPin("S2");
	pinMappings.pinButtonL3  = convertPin("L3");
	pinMappings.pinButtonR3  = convertPin("R3");
	pinMappings.pinButtonA1  = convertPin("A1");
	pinMappings.pinButtonA2  = convertPin("A2");
	pinMappings.pinButtonFn  = convertPin("Fn");

	Storage::getInstance().save();

	return serialize_json(doc);
}

std::string getPinMappings()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);

	const PinMappings& pinMappings = Storage::getInstance().getPinMappings();
	writeDoc(doc, "Up", cleanPin(pinMappings.pinDpadUp));
	writeDoc(doc, "Down", cleanPin(pinMappings.pinDpadDown));
	writeDoc(doc, "Left", cleanPin(pinMappings.pinDpadLeft));
	writeDoc(doc, "Right", cleanPin(pinMappings.pinDpadRight));
	writeDoc(doc, "B1", cleanPin(pinMappings.pinButtonB1));
	writeDoc(doc, "B2", cleanPin(pinMappings.pinButtonB2));
	writeDoc(doc, "B3", cleanPin(pinMappings.pinButtonB3));
	writeDoc(doc, "B4", cleanPin(pinMappings.pinButtonB4));
	writeDoc(doc, "L1", cleanPin(pinMappings.pinButtonL1));
	writeDoc(doc, "R1", cleanPin(pinMappings.pinButtonR1));
	writeDoc(doc, "L2", cleanPin(pinMappings.pinButtonL2));
	writeDoc(doc, "R2", cleanPin(pinMappings.pinButtonR2));
	writeDoc(doc, "S1", cleanPin(pinMappings.pinButtonS1));
	writeDoc(doc, "S2", cleanPin(pinMappings.pinButtonS2));
	writeDoc(doc, "L3", cleanPin(pinMappings.pinButtonL3));
	writeDoc(doc, "R3", cleanPin(pinMappings.pinButtonR3));
	writeDoc(doc, "A1", cleanPin(pinMappings.pinButtonA1));
	writeDoc(doc, "A2", cleanPin(pinMappings.pinButtonA2));
	writeDoc(doc, "Fn", cleanPin(pinMappings.pinButtonFn));

	return serialize_json(doc);
}

std::string setKeyMappings()
{
	DynamicJsonDocument doc = get_post_data();

	KeyboardMapping& keyboardMapping = Storage::getInstance().getKeyboardMapping();

	readDoc(keyboardMapping.keyDpadUp, doc, "Up");
	readDoc(keyboardMapping.keyDpadDown, doc, "Down");
	readDoc(keyboardMapping.keyDpadLeft, doc, "Left");
	readDoc(keyboardMapping.keyDpadRight, doc, "Right");
	readDoc(keyboardMapping.keyButtonB1, doc, "B1");
	readDoc(keyboardMapping.keyButtonB2, doc, "B2");
	readDoc(keyboardMapping.keyButtonB3, doc, "B3");
	readDoc(keyboardMapping.keyButtonB4, doc, "B4");
	readDoc(keyboardMapping.keyButtonL1, doc, "L1");
	readDoc(keyboardMapping.keyButtonR1, doc, "R1");
	readDoc(keyboardMapping.keyButtonL2, doc, "L2");
	readDoc(keyboardMapping.keyButtonR2, doc, "R2");
	readDoc(keyboardMapping.keyButtonS1, doc, "S1");
	readDoc(keyboardMapping.keyButtonS2, doc, "S2");
	readDoc(keyboardMapping.keyButtonL3, doc, "L3");
	readDoc(keyboardMapping.keyButtonR3, doc, "R3");
	readDoc(keyboardMapping.keyButtonA1, doc, "A1");
	readDoc(keyboardMapping.keyButtonA2, doc, "A2");

	Storage::getInstance().save();

	return serialize_json(doc);
}

std::string getKeyMappings()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	const KeyboardMapping& keyboardMapping = Storage::getInstance().getKeyboardMapping();

	writeDoc(doc, "Up", keyboardMapping.keyDpadUp);
	writeDoc(doc, "Down", keyboardMapping.keyDpadDown);
	writeDoc(doc, "Left", keyboardMapping.keyDpadLeft);
	writeDoc(doc, "Right", keyboardMapping.keyDpadRight);
	writeDoc(doc, "B1", keyboardMapping.keyButtonB1);
	writeDoc(doc, "B2", keyboardMapping.keyButtonB2);
	writeDoc(doc, "B3", keyboardMapping.keyButtonB3);
	writeDoc(doc, "B4", keyboardMapping.keyButtonB4);
	writeDoc(doc, "L1", keyboardMapping.keyButtonL1);
	writeDoc(doc, "R1", keyboardMapping.keyButtonR1);
	writeDoc(doc, "L2", keyboardMapping.keyButtonL2);
	writeDoc(doc, "R2", keyboardMapping.keyButtonR2);
	writeDoc(doc, "S1", keyboardMapping.keyButtonS1);
	writeDoc(doc, "S2", keyboardMapping.keyButtonS2);
	writeDoc(doc, "L3", keyboardMapping.keyButtonL3);
	writeDoc(doc, "R3", keyboardMapping.keyButtonR3);
	writeDoc(doc, "A1", keyboardMapping.keyButtonA1);
	writeDoc(doc, "A2", keyboardMapping.keyButtonA2);

	return serialize_json(doc);
}

std::string setAddonOptions()
{
	DynamicJsonDocument doc = get_post_data();

    AnalogOptions& analogOptions = Storage::getInstance().getAddonOptions().analogOptions;
	docToPin(analogOptions.analogAdc1PinX, doc, "analogAdc1PinX");
	docToPin(analogOptions.analogAdc1PinY, doc, "analogAdc1PinY");
	docToValue(analogOptions.analogAdc1Mode, doc, "analogAdc1Mode");
	docToValue(analogOptions.analogAdc1Invert, doc, "analogAdc1Invert");
	docToPin(analogOptions.analogAdc2PinX, doc, "analogAdc2PinX");
	docToPin(analogOptions.analogAdc2PinY, doc, "analogAdc2PinY");
	docToValue(analogOptions.analogAdc2Mode, doc, "analogAdc2Mode");
	docToValue(analogOptions.analogAdc2Invert, doc, "analogAdc2Invert");
	docToValue(analogOptions.forced_circularity, doc, "forced_circularity");
	docToValue(analogOptions.analog_deadzone, doc, "analog_deadzone");
	docToValue(analogOptions.auto_calibrate, doc, "auto_calibrate");
	docToValue(analogOptions.enabled, doc, "AnalogInputEnabled");

    BootselButtonOptions& bootselButtonOptions = Storage::getInstance().getAddonOptions().bootselButtonOptions;
	docToValue(bootselButtonOptions.buttonMap, doc, "bootselButtonMap");
	docToValue(bootselButtonOptions.enabled, doc, "BootselButtonAddonEnabled");

	BuzzerOptions& buzzerOptions = Storage::getInstance().getAddonOptions().buzzerOptions;
	docToPin(buzzerOptions.pin, doc, "buzzerPin");
	docToValue(buzzerOptions.volume, doc, "buzzerVolume");
	docToValue(buzzerOptions.enabled, doc, "BuzzerSpeakerAddonEnabled");

    DualDirectionalOptions& dualDirectionalOptions = Storage::getInstance().getAddonOptions().dualDirectionalOptions;
	docToPin(dualDirectionalOptions.downPin, doc, "dualDirDownPin");
	docToPin(dualDirectionalOptions.upPin, doc, "dualDirUpPin");
	docToPin(dualDirectionalOptions.leftPin, doc, "dualDirLeftPin");
	docToPin(dualDirectionalOptions.rightPin, doc, "dualDirRightPin");
	docToValue(dualDirectionalOptions.dpadMode, doc, "dualDirDpadMode");
	docToValue(dualDirectionalOptions.combineMode, doc, "dualDirCombineMode");
	docToValue(dualDirectionalOptions.fourWayMode, doc, "dualDirFourWayMode");
	docToValue(dualDirectionalOptions.enabled, doc, "DualDirectionalInputEnabled");

		TiltOptions& tiltOptions = Storage::getInstance().getAddonOptions().tiltOptions;
	docToPin(tiltOptions.tilt1Pin, doc, "tilt1Pin");
	docToPin(tiltOptions.tilt2Pin, doc, "tilt2Pin");
	docToPin(tiltOptions.tiltLeftAnalogUpPin, doc, "tiltLeftAnalogUpPin");
	docToPin(tiltOptions.tiltLeftAnalogDownPin, doc, "tiltLeftAnalogDownPin");
	docToPin(tiltOptions.tiltLeftAnalogLeftPin, doc, "tiltLeftAnalogLeftPin");
	docToPin(tiltOptions.tiltLeftAnalogRightPin, doc, "tiltLeftAnalogRightPin");
	docToPin(tiltOptions.tiltRightAnalogUpPin, doc, "tiltRightAnalogUpPin");
	docToPin(tiltOptions.tiltRightAnalogDownPin, doc, "tiltRightAnalogDownPin");
	docToPin(tiltOptions.tiltRightAnalogLeftPin, doc, "tiltRightAnalogLeftPin");
	docToPin(tiltOptions.tiltRightAnalogRightPin, doc, "tiltRightAnalogRightPin");
	docToValue(tiltOptions.tiltSOCDMode, doc, "tiltSOCDMode");
	docToValue(tiltOptions.enabled, doc, "TiltInputEnabled");

    ExtraButtonOptions& extraButtonOptions = Storage::getInstance().getAddonOptions().extraButtonOptions;
	docToPin(extraButtonOptions.pin, doc, "extraButtonPin");
	docToValue(extraButtonOptions.buttonMap, doc, "extraButtonMap");
	docToValue(extraButtonOptions.enabled, doc, "ExtraButtonAddonEnabled");

    FocusModeOptions& focusModeOptions = Storage::getInstance().getAddonOptions().focusModeOptions;
	docToPin(focusModeOptions.pin, doc, "focusModePin");
	docToValue(focusModeOptions.buttonLockMask, doc, "focusModeButtonLockMask");
	docToValue(focusModeOptions.buttonLockEnabled, doc, "focusModeButtonLockEnabled");
	docToValue(focusModeOptions.oledLockEnabled, doc, "focusModeOledLockEnabled");
	docToValue(focusModeOptions.rgbLockEnabled, doc, "focusModeRgbLockEnabled");
	docToValue(focusModeOptions.enabled, doc, "FocusModeAddonEnabled");

    AnalogADS1219Options& analogADS1219Options = Storage::getInstance().getAddonOptions().analogADS1219Options;
	docToPin(analogADS1219Options.i2cSDAPin, doc, "i2cAnalog1219SDAPin");
	docToPin(analogADS1219Options.i2cSCLPin, doc, "i2cAnalog1219SCLPin");
	docToValue(analogADS1219Options.i2cBlock, doc, "i2cAnalog1219Block");
	docToValue(analogADS1219Options.i2cSpeed, doc, "i2cAnalog1219Speed");
	docToValue(analogADS1219Options.i2cAddress, doc, "i2cAnalog1219Address");
	docToValue(analogADS1219Options.enabled, doc, "I2CAnalog1219InputEnabled");

    SliderOptions& sliderOptions = Storage::getInstance().getAddonOptions().sliderOptions;
	docToPin(sliderOptions.pinSliderOne, doc, "sliderPinOne");
	docToPin(sliderOptions.pinSliderTwo, doc, "sliderPinTwo");
	docToValue(sliderOptions.modeZero, doc, "sliderModeZero");
	docToValue(sliderOptions.modeOne, doc, "sliderModeOne");
	docToValue(sliderOptions.modeTwo, doc, "sliderModeTwo");
	docToValue(sliderOptions.enabled, doc, "JSliderInputEnabled");

    PlayerNumberOptions& playerNumberOptions = Storage::getInstance().getAddonOptions().playerNumberOptions;
	docToValue(playerNumberOptions.number, doc, "playerNumber");
	docToValue(playerNumberOptions.enabled, doc, "PlayerNumAddonEnabled");

	ReverseOptions& reverseOptions = Storage::getInstance().getAddonOptions().reverseOptions;
	docToValue(reverseOptions.enabled, doc, "ReverseInputEnabled");
	docToPin(reverseOptions.buttonPin, doc, "reversePin");
	docToPin(reverseOptions.ledPin, doc, "reversePinLED");
	docToValue(reverseOptions.actionUp, doc, "reverseActionUp");
	docToValue(reverseOptions.actionDown, doc, "reverseActionDown");
	docToValue(reverseOptions.actionLeft, doc, "reverseActionLeft");
	docToValue(reverseOptions.actionRight, doc, "reverseActionRight");

    SOCDSliderOptions& socdSliderOptions = Storage::getInstance().getAddonOptions().socdSliderOptions;
	docToValue(socdSliderOptions.enabled, doc, "SliderSOCDInputEnabled");
	docToPin(socdSliderOptions.pinOne, doc, "sliderSOCDPinOne");
	docToPin(socdSliderOptions.pinTwo, doc, "sliderSOCDPinTwo");
	docToValue(socdSliderOptions.modeOne, doc, "sliderSOCDModeOne");
	docToValue(socdSliderOptions.modeTwo, doc, "sliderSOCDModeTwo");
	docToValue(socdSliderOptions.modeDefault, doc, "sliderSOCDModeDefault");

    OnBoardLedOptions& onBoardLedOptions = Storage::getInstance().getAddonOptions().onBoardLedOptions;
	docToValue(onBoardLedOptions.mode, doc, "onBoardLedMode");
	docToValue(onBoardLedOptions.enabled, doc, "BoardLedAddonEnabled");

    TurboOptions& turboOptions = Storage::getInstance().getAddonOptions().turboOptions;
	docToPin(turboOptions.buttonPin, doc, "turboPin");
	docToPin(turboOptions.ledPin, doc, "turboPinLED");
	docToValue(turboOptions.shotCount, doc, "turboShotCount");
	docToValue(turboOptions.shmupModeEnabled, doc, "shmupMode");
	docToValue(turboOptions.shmupMixMode, doc, "shmupMixMode");
	docToValue(turboOptions.shmupAlwaysOn1, doc, "shmupAlwaysOn1");
	docToValue(turboOptions.shmupAlwaysOn2, doc, "shmupAlwaysOn2");
	docToValue(turboOptions.shmupAlwaysOn3, doc, "shmupAlwaysOn3");
	docToValue(turboOptions.shmupAlwaysOn4, doc, "shmupAlwaysOn4");
	docToPin(turboOptions.shmupBtn1Pin, doc, "pinShmupBtn1");
	docToPin(turboOptions.shmupBtn2Pin, doc, "pinShmupBtn2");
	docToPin(turboOptions.shmupBtn3Pin, doc, "pinShmupBtn3");
	docToPin(turboOptions.shmupBtn4Pin, doc, "pinShmupBtn4");
	docToValue(turboOptions.shmupBtnMask1, doc, "shmupBtnMask1");
	docToValue(turboOptions.shmupBtnMask2, doc, "shmupBtnMask2");
	docToValue(turboOptions.shmupBtnMask3, doc, "shmupBtnMask3");
	docToValue(turboOptions.shmupBtnMask4, doc, "shmupBtnMask4");
	docToPin(turboOptions.shmupDialPin, doc, "pinShmupDial");
	docToValue(turboOptions.enabled, doc, "TurboInputEnabled");

    WiiOptions& wiiOptions = Storage::getInstance().getAddonOptions().wiiOptions;
	docToPin(wiiOptions.i2cSDAPin, doc, "wiiExtensionSDAPin");
	docToPin(wiiOptions.i2cSCLPin, doc, "wiiExtensionSCLPin");
	docToValue(wiiOptions.i2cBlock, doc, "wiiExtensionBlock");
	docToValue(wiiOptions.i2cSpeed, doc, "wiiExtensionSpeed");
	docToValue(wiiOptions.enabled, doc, "WiiExtensionAddonEnabled");

    PS4Options& ps4Options = Storage::getInstance().getAddonOptions().ps4Options;
	docToValue(ps4Options.enabled, doc, "PS4ModeAddonEnabled");

	SNESOptions& snesOptions = Storage::getInstance().getAddonOptions().snesOptions;
	docToValue(snesOptions.enabled, doc, "SNESpadAddonEnabled");
	docToPin(snesOptions.clockPin, doc, "snesPadClockPin");
	docToPin(snesOptions.latchPin, doc, "snesPadLatchPin");
	docToPin(snesOptions.dataPin, doc, "snesPadDataPin");

	KeyboardHostOptions& keyboardHostOptions = Storage::getInstance().getAddonOptions().keyboardHostOptions;
	docToValue(keyboardHostOptions.enabled, doc, "KeyboardHostAddonEnabled");
	docToPin(keyboardHostOptions.pinDplus, doc, "keyboardHostPinDplus");
	docToPin(keyboardHostOptions.pin5V, doc, "keyboardHostPin5V");
	docToValue(keyboardHostOptions.mapping.keyDpadUp, doc, "keyboardHostMap", "Up");
	docToValue(keyboardHostOptions.mapping.keyDpadDown, doc, "keyboardHostMap", "Down");
	docToValue(keyboardHostOptions.mapping.keyDpadLeft, doc, "keyboardHostMap", "Left");
	docToValue(keyboardHostOptions.mapping.keyDpadRight, doc, "keyboardHostMap", "Right");
	docToValue(keyboardHostOptions.mapping.keyButtonB1, doc, "keyboardHostMap", "B1");
	docToValue(keyboardHostOptions.mapping.keyButtonB2, doc, "keyboardHostMap", "B2");
	docToValue(keyboardHostOptions.mapping.keyButtonB3, doc, "keyboardHostMap", "B3");
	docToValue(keyboardHostOptions.mapping.keyButtonB4, doc, "keyboardHostMap", "B4");
	docToValue(keyboardHostOptions.mapping.keyButtonL1, doc, "keyboardHostMap", "L1");
	docToValue(keyboardHostOptions.mapping.keyButtonR1, doc, "keyboardHostMap", "R1");
	docToValue(keyboardHostOptions.mapping.keyButtonL2, doc, "keyboardHostMap", "L2");
	docToValue(keyboardHostOptions.mapping.keyButtonR2, doc, "keyboardHostMap", "R2");
	docToValue(keyboardHostOptions.mapping.keyButtonS1, doc, "keyboardHostMap", "S1");
	docToValue(keyboardHostOptions.mapping.keyButtonS2, doc, "keyboardHostMap", "S2");
	docToValue(keyboardHostOptions.mapping.keyButtonL3, doc, "keyboardHostMap", "L3");
	docToValue(keyboardHostOptions.mapping.keyButtonR3, doc, "keyboardHostMap", "R3");
	docToValue(keyboardHostOptions.mapping.keyButtonA1, doc, "keyboardHostMap", "A1");
	docToValue(keyboardHostOptions.mapping.keyButtonA2, doc, "keyboardHostMap", "A2");

	PSPassthroughOptions& psPassthroughOptions = Storage::getInstance().getAddonOptions().psPassthroughOptions;
	docToValue(psPassthroughOptions.enabled, doc, "PSPassthroughAddonEnabled");
	docToPin(psPassthroughOptions.pinDplus, doc, "psPassthroughPinDplus");
	docToPin(psPassthroughOptions.pin5V, doc, "psPassthroughPin5V");

	Storage::getInstance().save();

	return serialize_json(doc);
}

std::string setPS4Options()
{
	DynamicJsonDocument doc = get_post_data();
	PS4Options& ps4Options = Storage::getInstance().getAddonOptions().ps4Options;
	std::string encoded;
	std::string decoded;
	size_t length;

	const auto readEncoded = [&](const char* key) -> bool
	{
		if (doc.containsKey(key))
		{
			const char* str = nullptr;
			readDoc(str, doc, key);
			encoded = str;
			return true;
		}
		else
		{
			return false;
		}
	};

	// RSA Context
	if ( readEncoded("N") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == sizeof(ps4Options.rsaN.bytes)) ) {
			memcpy(ps4Options.rsaN.bytes, decoded.data(), decoded.length());
			ps4Options.rsaN.size = decoded.length();
		}
	}
	if ( readEncoded("E") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == sizeof(ps4Options.rsaE.bytes)) ) {
			memcpy(ps4Options.rsaE.bytes, decoded.data(), decoded.length());
			ps4Options.rsaE.size = decoded.length();
		}
	}
	if ( readEncoded("P") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == sizeof(ps4Options.rsaP.bytes)) ) {
			memcpy(ps4Options.rsaP.bytes, decoded.data(), decoded.length());
			ps4Options.rsaP.size = decoded.length();
		}
	}
	if ( readEncoded("Q") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == sizeof(ps4Options.rsaQ.bytes)) ) {
			memcpy(ps4Options.rsaQ.bytes, decoded.data(), decoded.length());
			ps4Options.rsaQ.size = decoded.length();
		}
	}
	// Serial & Signature
	if ( readEncoded("serial") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == sizeof(ps4Options.serial.bytes)) ) {
			memcpy(ps4Options.serial.bytes, decoded.data(), decoded.length());
			ps4Options.serial.size = decoded.length();
		}
	}
	if ( readEncoded("signature") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == sizeof(ps4Options.signature.bytes)) ) {
			memcpy(ps4Options.signature.bytes, decoded.data(), decoded.length());
			ps4Options.signature.size = decoded.length();
		}
	}

	// Zap deprecated fields
	if (ps4Options.rsaD.size != 0) ps4Options.rsaD.size = 0;
	if (ps4Options.rsaDP.size != 0) ps4Options.rsaDP.size = 0;
	if (ps4Options.rsaDQ.size != 0) ps4Options.rsaDQ.size = 0;
	if (ps4Options.rsaQP.size != 0) ps4Options.rsaQP.size = 0;
	if (ps4Options.rsaRN.size != 0) ps4Options.rsaRN.size = 0;

	Storage::getInstance().save();

	return "{\"success\":true}";
}

std::string getAddonOptions()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);

    const AnalogOptions& analogOptions = Storage::getInstance().getAddonOptions().analogOptions;
	writeDoc(doc, "analogAdc1PinX", cleanPin(analogOptions.analogAdc1PinX));
	writeDoc(doc, "analogAdc1PinY", cleanPin(analogOptions.analogAdc1PinY));
	writeDoc(doc, "analogAdc1Mode", analogOptions.analogAdc1Mode);
	writeDoc(doc, "analogAdc1Invert", analogOptions.analogAdc1Invert);
	writeDoc(doc, "analogAdc2PinX", cleanPin(analogOptions.analogAdc2PinX));
	writeDoc(doc, "analogAdc2PinY", cleanPin(analogOptions.analogAdc2PinY));
	writeDoc(doc, "analogAdc2Mode", analogOptions.analogAdc2Mode);
	writeDoc(doc, "analogAdc2Invert", analogOptions.analogAdc2Invert);
	writeDoc(doc, "forced_circularity", analogOptions.forced_circularity);
	writeDoc(doc, "analog_deadzone", analogOptions.analog_deadzone);
	writeDoc(doc, "auto_calibrate", analogOptions.auto_calibrate);
	writeDoc(doc, "AnalogInputEnabled", analogOptions.enabled);

    const BootselButtonOptions& bootselButtonOptions = Storage::getInstance().getAddonOptions().bootselButtonOptions;
	writeDoc(doc, "bootselButtonMap", bootselButtonOptions.buttonMap);
	writeDoc(doc, "BootselButtonAddonEnabled", bootselButtonOptions.enabled);

    const BuzzerOptions& buzzerOptions = Storage::getInstance().getAddonOptions().buzzerOptions;
	writeDoc(doc, "buzzerPin", cleanPin(buzzerOptions.pin));
	writeDoc(doc, "buzzerVolume", buzzerOptions.volume);
	writeDoc(doc, "BuzzerSpeakerAddonEnabled", buzzerOptions.enabled);

    const DualDirectionalOptions& dualDirectionalOptions = Storage::getInstance().getAddonOptions().dualDirectionalOptions;
	writeDoc(doc, "dualDirDownPin", cleanPin(dualDirectionalOptions.downPin));
	writeDoc(doc, "dualDirUpPin", cleanPin(dualDirectionalOptions.upPin));
	writeDoc(doc, "dualDirLeftPin", cleanPin(dualDirectionalOptions.leftPin));
	writeDoc(doc, "dualDirRightPin", cleanPin(dualDirectionalOptions.rightPin));
	writeDoc(doc, "dualDirDpadMode", dualDirectionalOptions.dpadMode);
	writeDoc(doc, "dualDirCombineMode", dualDirectionalOptions.combineMode);
	writeDoc(doc, "dualDirFourWayMode", dualDirectionalOptions.fourWayMode);
	writeDoc(doc, "DualDirectionalInputEnabled", dualDirectionalOptions.enabled);

		const TiltOptions& tiltOptions = Storage::getInstance().getAddonOptions().tiltOptions;
	writeDoc(doc, "tilt1Pin", cleanPin(tiltOptions.tilt1Pin));
	writeDoc(doc, "tilt2Pin", cleanPin(tiltOptions.tilt2Pin));
	writeDoc(doc, "tiltLeftAnalogUpPin", cleanPin(tiltOptions.tiltLeftAnalogUpPin));
	writeDoc(doc, "tiltLeftAnalogDownPin", cleanPin(tiltOptions.tiltLeftAnalogDownPin));
	writeDoc(doc, "tiltLeftAnalogLeftPin", cleanPin(tiltOptions.tiltLeftAnalogLeftPin));
	writeDoc(doc, "tiltLeftAnalogRightPin", cleanPin(tiltOptions.tiltLeftAnalogRightPin));
	writeDoc(doc, "tiltRightAnalogUpPin", cleanPin(tiltOptions.tiltRightAnalogUpPin));
	writeDoc(doc, "tiltRightAnalogDownPin", cleanPin(tiltOptions.tiltRightAnalogDownPin));
	writeDoc(doc, "tiltRightAnalogLeftPin", cleanPin(tiltOptions.tiltRightAnalogLeftPin));
	writeDoc(doc, "tiltRightAnalogRightPin", cleanPin(tiltOptions.tiltRightAnalogRightPin));
	writeDoc(doc, "tiltSOCDMode", tiltOptions.tiltSOCDMode);
	writeDoc(doc, "TiltInputEnabled", tiltOptions.enabled);

    const ExtraButtonOptions& extraButtonOptions = Storage::getInstance().getAddonOptions().extraButtonOptions;
	writeDoc(doc, "extraButtonPin", cleanPin(extraButtonOptions.pin));
	writeDoc(doc, "extraButtonMap", extraButtonOptions.buttonMap);
	writeDoc(doc, "ExtraButtonAddonEnabled", extraButtonOptions.enabled);

    const AnalogADS1219Options& analogADS1219Options = Storage::getInstance().getAddonOptions().analogADS1219Options;
	writeDoc(doc, "i2cAnalog1219SDAPin", cleanPin(analogADS1219Options.i2cSDAPin));
	writeDoc(doc, "i2cAnalog1219SCLPin", cleanPin(analogADS1219Options.i2cSCLPin));
	writeDoc(doc, "i2cAnalog1219Block", analogADS1219Options.i2cBlock);
	writeDoc(doc, "i2cAnalog1219Speed", analogADS1219Options.i2cSpeed);
	writeDoc(doc, "i2cAnalog1219Address", analogADS1219Options.i2cAddress);
	writeDoc(doc, "I2CAnalog1219InputEnabled", analogADS1219Options.enabled);

    const SliderOptions& sliderOptions = Storage::getInstance().getAddonOptions().sliderOptions;
	writeDoc(doc, "sliderPinOne", cleanPin(sliderOptions.pinSliderOne));
	writeDoc(doc, "sliderPinTwo", cleanPin(sliderOptions.pinSliderTwo));
	writeDoc(doc, "sliderModeZero", sliderOptions.modeZero);
	writeDoc(doc, "sliderModeOne", sliderOptions.modeOne);
	writeDoc(doc, "sliderModeTwo", sliderOptions.modeTwo);
	writeDoc(doc, "JSliderInputEnabled", sliderOptions.enabled);

    const PlayerNumberOptions& playerNumberOptions = Storage::getInstance().getAddonOptions().playerNumberOptions;
	writeDoc(doc, "playerNumber", playerNumberOptions.number);
	writeDoc(doc, "PlayerNumAddonEnabled", playerNumberOptions.enabled);

	const ReverseOptions& reverseOptions = Storage::getInstance().getAddonOptions().reverseOptions;
	writeDoc(doc, "reversePin", cleanPin(reverseOptions.buttonPin));
	writeDoc(doc, "reversePinLED", cleanPin(reverseOptions.ledPin));
	writeDoc(doc, "reverseActionUp", reverseOptions.actionUp);
	writeDoc(doc, "reverseActionDown", reverseOptions.actionDown);
	writeDoc(doc, "reverseActionLeft", reverseOptions.actionLeft);
	writeDoc(doc, "reverseActionRight", reverseOptions.actionRight);
	writeDoc(doc, "ReverseInputEnabled", reverseOptions.enabled);

    const SOCDSliderOptions& socdSliderOptions = Storage::getInstance().getAddonOptions().socdSliderOptions;
	writeDoc(doc, "sliderSOCDPinOne", cleanPin(socdSliderOptions.pinOne));
	writeDoc(doc, "sliderSOCDPinTwo", cleanPin(socdSliderOptions.pinTwo));
	writeDoc(doc, "sliderSOCDModeOne", socdSliderOptions.modeOne);
	writeDoc(doc, "sliderSOCDModeTwo", socdSliderOptions.modeTwo);
	writeDoc(doc, "sliderSOCDModeDefault", socdSliderOptions.modeDefault);
	writeDoc(doc, "SliderSOCDInputEnabled", socdSliderOptions.enabled);

    const OnBoardLedOptions& onBoardLedOptions = Storage::getInstance().getAddonOptions().onBoardLedOptions;
	writeDoc(doc, "onBoardLedMode", onBoardLedOptions.mode);
	writeDoc(doc, "BoardLedAddonEnabled", onBoardLedOptions.enabled);

    const TurboOptions& turboOptions = Storage::getInstance().getAddonOptions().turboOptions;
	writeDoc(doc, "turboPin", cleanPin(turboOptions.buttonPin));
	writeDoc(doc, "turboPinLED", cleanPin(turboOptions.ledPin));
	writeDoc(doc, "turboShotCount", turboOptions.shotCount);
	writeDoc(doc, "shmupMode", turboOptions.shmupModeEnabled);
	writeDoc(doc, "shmupMixMode", turboOptions.shmupMixMode);
	writeDoc(doc, "shmupAlwaysOn1", turboOptions.shmupAlwaysOn1);
	writeDoc(doc, "shmupAlwaysOn2", turboOptions.shmupAlwaysOn2);
	writeDoc(doc, "shmupAlwaysOn3", turboOptions.shmupAlwaysOn3);
	writeDoc(doc, "shmupAlwaysOn4", turboOptions.shmupAlwaysOn4);
	writeDoc(doc, "pinShmupBtn1", cleanPin(turboOptions.shmupBtn1Pin));
	writeDoc(doc, "pinShmupBtn2", cleanPin(turboOptions.shmupBtn2Pin));
	writeDoc(doc, "pinShmupBtn3", cleanPin(turboOptions.shmupBtn3Pin));
	writeDoc(doc, "pinShmupBtn4", cleanPin(turboOptions.shmupBtn4Pin));
	writeDoc(doc, "shmupBtnMask1", turboOptions.shmupBtnMask1);
	writeDoc(doc, "shmupBtnMask2", turboOptions.shmupBtnMask2);
	writeDoc(doc, "shmupBtnMask3", turboOptions.shmupBtnMask3);
	writeDoc(doc, "shmupBtnMask4", turboOptions.shmupBtnMask4);
	writeDoc(doc, "pinShmupDial", cleanPin(turboOptions.shmupDialPin));
	writeDoc(doc, "TurboInputEnabled", turboOptions.enabled);

    const WiiOptions& wiiOptions = Storage::getInstance().getAddonOptions().wiiOptions;
	writeDoc(doc, "wiiExtensionSDAPin", cleanPin(wiiOptions.i2cSDAPin));
	writeDoc(doc, "wiiExtensionSCLPin", cleanPin(wiiOptions.i2cSCLPin));
	writeDoc(doc, "wiiExtensionBlock", wiiOptions.i2cBlock);
	writeDoc(doc, "wiiExtensionSpeed", wiiOptions.i2cSpeed);
	writeDoc(doc, "WiiExtensionAddonEnabled", wiiOptions.enabled);

	const PS4Options& ps4Options = Storage::getInstance().getAddonOptions().ps4Options;
	writeDoc(doc, "PS4ModeAddonEnabled", ps4Options.enabled);

	const SNESOptions& snesOptions = Storage::getInstance().getAddonOptions().snesOptions;
	writeDoc(doc, "snesPadClockPin", cleanPin(snesOptions.clockPin));
	writeDoc(doc, "snesPadLatchPin", cleanPin(snesOptions.latchPin));
	writeDoc(doc, "snesPadDataPin", cleanPin(snesOptions.dataPin));
	writeDoc(doc, "SNESpadAddonEnabled", snesOptions.enabled);

	const KeyboardHostOptions& keyboardHostOptions = Storage::getInstance().getAddonOptions().keyboardHostOptions;
	writeDoc(doc, "KeyboardHostAddonEnabled", keyboardHostOptions.enabled);
	writeDoc(doc, "keyboardHostPinDplus", keyboardHostOptions.pinDplus);
	writeDoc(doc, "keyboardHostPin5V", keyboardHostOptions.pin5V);
	writeDoc(doc, "keyboardHostMap", "Up", keyboardHostOptions.mapping.keyDpadUp);
	writeDoc(doc, "keyboardHostMap", "Down", keyboardHostOptions.mapping.keyDpadDown);
	writeDoc(doc, "keyboardHostMap", "Left", keyboardHostOptions.mapping.keyDpadLeft);
	writeDoc(doc, "keyboardHostMap", "Right", keyboardHostOptions.mapping.keyDpadRight);
	writeDoc(doc, "keyboardHostMap", "B1", keyboardHostOptions.mapping.keyButtonB1);
	writeDoc(doc, "keyboardHostMap", "B2", keyboardHostOptions.mapping.keyButtonB2);
	writeDoc(doc, "keyboardHostMap", "B3", keyboardHostOptions.mapping.keyButtonB3);
	writeDoc(doc, "keyboardHostMap", "B4", keyboardHostOptions.mapping.keyButtonB4);
	writeDoc(doc, "keyboardHostMap", "L1", keyboardHostOptions.mapping.keyButtonL1);
	writeDoc(doc, "keyboardHostMap", "R1", keyboardHostOptions.mapping.keyButtonR1);
	writeDoc(doc, "keyboardHostMap", "L2", keyboardHostOptions.mapping.keyButtonL2);
	writeDoc(doc, "keyboardHostMap", "R2", keyboardHostOptions.mapping.keyButtonR2);
	writeDoc(doc, "keyboardHostMap", "S1", keyboardHostOptions.mapping.keyButtonS1);
	writeDoc(doc, "keyboardHostMap", "S2", keyboardHostOptions.mapping.keyButtonS2);
	writeDoc(doc, "keyboardHostMap", "L3", keyboardHostOptions.mapping.keyButtonL3);
	writeDoc(doc, "keyboardHostMap", "R3", keyboardHostOptions.mapping.keyButtonR3);
	writeDoc(doc, "keyboardHostMap", "A1", keyboardHostOptions.mapping.keyButtonA1);
	writeDoc(doc, "keyboardHostMap", "A2", keyboardHostOptions.mapping.keyButtonA2);

	PSPassthroughOptions& psPassthroughOptions = Storage::getInstance().getAddonOptions().psPassthroughOptions;
	writeDoc(doc, "PSPassthroughAddonEnabled", psPassthroughOptions.enabled);
	writeDoc(doc, "psPassthroughPinDplus", psPassthroughOptions.pinDplus);
	writeDoc(doc, "psPassthroughPin5V", psPassthroughOptions.pin5V);

	const FocusModeOptions& focusModeOptions = Storage::getInstance().getAddonOptions().focusModeOptions;
	writeDoc(doc, "focusModePin", cleanPin(focusModeOptions.pin));
	writeDoc(doc, "focusModeButtonLockMask", focusModeOptions.buttonLockMask);
	writeDoc(doc, "focusModeButtonLockEnabled", focusModeOptions.buttonLockEnabled);
	writeDoc(doc, "focusModeOledLockEnabled", focusModeOptions.oledLockEnabled);
	writeDoc(doc, "focusModeRgbLockEnabled", focusModeOptions.rgbLockEnabled);
	writeDoc(doc, "FocusModeAddonEnabled", focusModeOptions.enabled);

	return serialize_json(doc);
}

std::string getFirmwareVersion()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	writeDoc(doc, "version", GP2040VERSION);
	return serialize_json(doc);
}

std::string getMemoryReport()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	writeDoc(doc, "totalFlash", System::getTotalFlash());
	writeDoc(doc, "usedFlash", System::getUsedFlash());
	writeDoc(doc, "staticAllocs", System::getStaticAllocs());
	writeDoc(doc, "totalHeap", System::getTotalHeap());
	writeDoc(doc, "usedHeap", System::getUsedHeap());
	return serialize_json(doc);
}

std::string getConfig()
{
	return ConfigUtils::toJSON(Storage::getInstance().getConfig());
}

DataAndStatusCode setConfig()
{
	bool success = false;

	// Store config struct on the heap to avoid stack overflow
	std::unique_ptr<Config> config(new Config);
	*config.get() = Config Config_init_default;
	if (ConfigUtils::fromJSON(*config.get(), http_post_payload, http_post_payload_len))
	{
		Storage::getInstance().getConfig() = *config.get();
		config.reset();
		if (Storage::getInstance().save())
		{
			return DataAndStatusCode(getConfig(), HttpStatusCode::_200);
		}
		else
		{
			return DataAndStatusCode("{ \"error\": \"internal error while saving config\" }", HttpStatusCode::_500);
		}
	}
	else
	{
		return DataAndStatusCode("{ \"error\": \"invalid JSON document\" }", HttpStatusCode::_400);
	}
}

// This should be a storage feature
std::string resetSettings()
{
	Storage::getInstance().ResetSettings();
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	doc["success"] = true;
	return serialize_json(doc);
}

#if !defined(NDEBUG)
std::string echo()
{
	DynamicJsonDocument doc = get_post_data();
	return serialize_json(doc);
}
#endif

std::string reboot()
{
	DynamicJsonDocument doc = get_post_data();
	doc["success"] = true;
	// We need to wait for a bit before we actually reboot to leave the webclient some time to receive the response
	rebootDelayTimeout = make_timeout_time_ms(rebootDelayMs);
	WebConfig::BootModes bootMode = doc["bootMode"];
	switch (bootMode) {
		case WebConfig::BootModes::GAMEPAD:
			rebootMode = System::BootMode::GAMEPAD;
		break;
		case WebConfig::BootModes::WEBCONFIG:
			rebootMode = System::BootMode::WEBCONFIG;
		break;
		case WebConfig::BootModes::BOOTSEL:
			rebootMode = System::BootMode::USB;
		break;
		default:
			rebootMode = System::BootMode::DEFAULT;
	}
	return serialize_json(doc);
}

typedef std::string (*HandlerFuncPtr)();
static const std::pair<const char*, HandlerFuncPtr> handlerFuncs[] =
{
	{ "/api/setDisplayOptions", setDisplayOptions },
	{ "/api/setPreviewDisplayOptions", setPreviewDisplayOptions },
	{ "/api/setGamepadOptions", setGamepadOptions },
	{ "/api/setLedOptions", setLedOptions },
	{ "/api/setCustomTheme", setCustomTheme },
	{ "/api/getCustomTheme", getCustomTheme },
	{ "/api/setPinMappings", setPinMappings },
	{ "/api/setProfileOptions", setProfileOptions },
	{ "/api/setKeyMappings", setKeyMappings },
	{ "/api/setAddonsOptions", setAddonOptions },
	{ "/api/setPS4Options", setPS4Options },
	{ "/api/setSplashImage", setSplashImage },
	{ "/api/reboot", reboot },
	{ "/api/getDisplayOptions", getDisplayOptions },
	{ "/api/getGamepadOptions", getGamepadOptions },
	{ "/api/getLedOptions", getLedOptions },
	{ "/api/getPinMappings", getPinMappings },
	{ "/api/getProfileOptions", getProfileOptions },
	{ "/api/getKeyMappings", getKeyMappings },
	{ "/api/getAddonsOptions", getAddonOptions },
	{ "/api/resetSettings", resetSettings },
	{ "/api/getSplashImage", getSplashImage },
	{ "/api/getFirmwareVersion", getFirmwareVersion },
	{ "/api/getMemoryReport", getMemoryReport },
	{ "/api/getUsedPins", getUsedPins },
	{ "/api/getConfig", getConfig },
#if !defined(NDEBUG)
	{ "/api/echo", echo },
#endif
};

typedef DataAndStatusCode (*HandlerFuncStatusCodePtr)();
static const std::pair<const char*, HandlerFuncStatusCodePtr> handlerFuncsWithStatusCode[] =
{
	{ "/api/setConfig", setConfig },
};

int fs_open_custom(struct fs_file *file, const char *name)
{
	for (const auto& handlerFunc : handlerFuncs)
	{
		if (strcmp(handlerFunc.first, name) == 0)
		{
			return set_file_data(file, handlerFunc.second());
		}
	}

	for (const auto& handlerFunc : handlerFuncsWithStatusCode)
	{
		if (strcmp(handlerFunc.first, name) == 0)
		{
			return set_file_data(file, handlerFunc.second());
		}
	}

	bool isExclude = false;
	for (const char* excludePath : excludePaths)
		if (strcmp(excludePath, name) == 0)
			return 0;

	for (const char* spaPath : spaPaths)
	{
		if (strcmp(spaPath, name) == 0)
		{
			file->data = (const char *)file__index_html[0].data;
			file->len = file__index_html[0].len;
			file->index = file__index_html[0].len;
			file->http_header_included = file__index_html[0].http_header_included;
			file->pextension = NULL;
			file->is_custom_file = 0;
			return 1;
		}
	}

	return 0;
}

void fs_close_custom(struct fs_file *file)
{
	if (file && file->is_custom_file && file->pextension)
	{
		mem_free(file->pextension);
		file->pextension = NULL;
	}
}
