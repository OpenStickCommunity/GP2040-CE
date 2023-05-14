#include "configs/webconfig.h"
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

const static vector<string> spaPaths = { "/display-config", "/led-config", "/pin-mapping", "/keyboard-mapping", "/settings", "/reset-settings", "/add-ons", "/custom-theme" };
const static vector<string> excludePaths = { "/css", "/images", "/js", "/static" };
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
	return doc[key0].containsKey(key1);
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T>
static void __attribute__((noinline)) docToValue(T& value, const DynamicJsonDocument& doc, const char* key)
{
	if (doc.containsKey(key))
	{
		value = doc[key];
	}
}

// Don't inline this function, we do not want to consume stack space in the calling function
static void __attribute__((noinline)) docToPinLegacy(uint8_t& pin, const DynamicJsonDocument& doc, const char* key)
{
	if (doc.containsKey(key))
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

	const ConfigLegacy::BoardOptions& boardOptions = Storage::getInstance().getBoardOptions();
	addPinIfValid(boardOptions.pinDpadUp);
	addPinIfValid(boardOptions.pinDpadDown);
	addPinIfValid(boardOptions.pinDpadLeft);
	addPinIfValid(boardOptions.pinDpadRight);
	addPinIfValid(boardOptions.pinButtonB1);
	addPinIfValid(boardOptions.pinButtonB2);
	addPinIfValid(boardOptions.pinButtonB3);
	addPinIfValid(boardOptions.pinButtonB4);
	addPinIfValid(boardOptions.pinButtonL1);
	addPinIfValid(boardOptions.pinButtonR1);
	addPinIfValid(boardOptions.pinButtonL2);
	addPinIfValid(boardOptions.pinButtonR2);
	addPinIfValid(boardOptions.pinButtonS1);
	addPinIfValid(boardOptions.pinButtonS2);
	addPinIfValid(boardOptions.pinButtonL3);
	addPinIfValid(boardOptions.pinButtonR3);
	addPinIfValid(boardOptions.pinButtonA1);
	addPinIfValid(boardOptions.pinButtonA2);
	addPinIfValid(boardOptions.i2cSDAPin);
	addPinIfValid(boardOptions.i2cSCLPin);

	const ConfigLegacy::AddonOptions& addonOptions = Storage::getInstance().getLegacyAddonOptions();
	addPinIfValid(addonOptions.analogAdcPinX);
	addPinIfValid(addonOptions.analogAdcPinY);
	addPinIfValid(addonOptions.buzzerPin);
}

std::string serialize_json(JsonDocument &doc)
{
	string data;
	serializeJson(doc, data);
	return data;
}

std::string setDisplayOptions(ConfigLegacy::BoardOptions& boardOptions)
{
	DynamicJsonDocument doc = get_post_data();
	readDoc(boardOptions.hasI2CDisplay, doc, "enabled");
	docToPinLegacy(boardOptions.i2cSDAPin, doc, "sdaPin");
	docToPinLegacy(boardOptions.i2cSCLPin, doc, "sclPin");
	readDoc(boardOptions.displayI2CAddress, doc, "i2cAddress");
	readDoc(boardOptions.i2cBlock, doc, "i2cBlock");
	readDoc(boardOptions.i2cSpeed, doc, "i2cSpeed");
	readDoc(boardOptions.displayFlip, doc, "flipDisplay");
	readDoc(boardOptions.displayInvert, doc, "invertDisplay");
	readDoc(boardOptions.buttonLayout, doc, "buttonLayout");
	readDoc(boardOptions.buttonLayoutRight, doc, "buttonLayoutRight");
	readDoc(boardOptions.splashMode, doc, "splashMode");
	readDoc(boardOptions.splashChoice, doc, "splashChoice");
	readDoc(boardOptions.splashDuration, doc, "splashDuration");
	readDoc(boardOptions.displaySaverTimeout, doc, "displaySaverTimeout");

	readDoc(boardOptions.buttonLayoutCustomOptions.params.layout, doc, "buttonLayoutCustomOptions", "params", "layout");
	readDoc(boardOptions.buttonLayoutCustomOptions.params.startX, doc, "buttonLayoutCustomOptions", "params", "startX");
	readDoc(boardOptions.buttonLayoutCustomOptions.params.startY, doc, "buttonLayoutCustomOptions", "params", "startY");
	readDoc(boardOptions.buttonLayoutCustomOptions.params.buttonRadius, doc, "buttonLayoutCustomOptions", "params", "buttonRadius");
	readDoc(boardOptions.buttonLayoutCustomOptions.params.buttonPadding, doc, "buttonLayoutCustomOptions", "params", "buttonPadding");

	readDoc(boardOptions.buttonLayoutCustomOptions.paramsRight.layoutRight, doc, "buttonLayoutCustomOptions", "paramsRight", "layout");
	readDoc(boardOptions.buttonLayoutCustomOptions.paramsRight.startX, doc, "buttonLayoutCustomOptions", "paramsRight", "startX");
	readDoc(boardOptions.buttonLayoutCustomOptions.paramsRight.startY, doc, "buttonLayoutCustomOptions", "paramsRight", "startY");
	readDoc(boardOptions.buttonLayoutCustomOptions.paramsRight.buttonRadius, doc, "buttonLayoutCustomOptions", "paramsRight", "buttonRadius");
	readDoc(boardOptions.buttonLayoutCustomOptions.paramsRight.buttonPadding, doc, "buttonLayoutCustomOptions", "paramsRight", "buttonPadding");

	return serialize_json(doc);
}

std::string setDisplayOptions()
{
	ConfigLegacy::BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
	std::string response = setDisplayOptions(boardOptions);
	ConfigManager::getInstance().setBoardOptions(boardOptions);
	return response;
}

std::string setPreviewDisplayOptions()
{
	ConfigLegacy::BoardOptions boardOptions = Storage::getInstance().getPreviewBoardOptions();
	std::string response = setDisplayOptions(boardOptions);
	ConfigManager::getInstance().setPreviewBoardOptions(boardOptions);
	return response;
}

std::string getDisplayOptions() // Manually set Document Attributes for the display
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	const ConfigLegacy::BoardOptions& boardOptions = Storage::getInstance().getBoardOptions();
	writeDoc(doc, "enabled", boardOptions.hasI2CDisplay ? 1 : 0);
	writeDoc(doc, "sdaPin", boardOptions.i2cSDAPin == 0xFF ? -1 : boardOptions.i2cSDAPin);
	writeDoc(doc, "sclPin", boardOptions.i2cSCLPin == 0xFF ? -1 : boardOptions.i2cSCLPin);
	writeDoc(doc, "i2cAddress", boardOptions.displayI2CAddress);
	writeDoc(doc, "i2cBlock", boardOptions.i2cBlock);
	writeDoc(doc, "i2cSpeed", boardOptions.i2cSpeed);
	writeDoc(doc, "flipDisplay", boardOptions.displayFlip ? 1 : 0);
	writeDoc(doc, "invertDisplay", boardOptions.displayInvert ? 1 : 0);
	writeDoc(doc, "buttonLayout", boardOptions.buttonLayout);
	writeDoc(doc, "buttonLayoutRight", boardOptions.buttonLayoutRight);
	writeDoc(doc, "splashMode", boardOptions.splashMode);
	writeDoc(doc, "splashChoice", boardOptions.splashChoice);
	writeDoc(doc, "splashDuration", boardOptions.splashDuration);
	writeDoc(doc, "displaySaverTimeout", boardOptions.displaySaverTimeout);

	writeDoc(doc, "buttonLayoutCustomOptions", "params", "layout", boardOptions.buttonLayoutCustomOptions.params.layout);
	writeDoc(doc, "buttonLayoutCustomOptions", "params", "startX", boardOptions.buttonLayoutCustomOptions.params.startX);
	writeDoc(doc, "buttonLayoutCustomOptions", "params", "startY", boardOptions.buttonLayoutCustomOptions.params.startY);
	writeDoc(doc, "buttonLayoutCustomOptions", "params", "buttonRadius", boardOptions.buttonLayoutCustomOptions.params.buttonRadius);
	writeDoc(doc, "buttonLayoutCustomOptions", "params", "buttonPadding", boardOptions.buttonLayoutCustomOptions.params.buttonPadding);

	writeDoc(doc, "buttonLayoutCustomOptions", "paramsRight", "layout", boardOptions.buttonLayoutCustomOptions.paramsRight.layoutRight);
	writeDoc(doc, "buttonLayoutCustomOptions", "paramsRight", "startX", boardOptions.buttonLayoutCustomOptions.paramsRight.startX);
	writeDoc(doc, "buttonLayoutCustomOptions", "paramsRight", "startY", boardOptions.buttonLayoutCustomOptions.paramsRight.startY);
	writeDoc(doc, "buttonLayoutCustomOptions", "paramsRight", "buttonRadius", boardOptions.buttonLayoutCustomOptions.paramsRight.buttonRadius);
	writeDoc(doc, "buttonLayoutCustomOptions", "paramsRight", "buttonPadding", boardOptions.buttonLayoutCustomOptions.paramsRight.buttonPadding);

	addUsedPinsArray(doc);

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

std::string setGamepadOptions()
{
	DynamicJsonDocument doc = get_post_data();
	Gamepad * gamepad = Storage::getInstance().GetGamepad();

	readDoc(gamepad->options.dpadMode, doc, "dpadMode");
	readDoc(gamepad->options.inputMode, doc, "inputMode");
	readDoc(gamepad->options.socdMode, doc, "socdMode");

	readDoc(gamepad->options.hotkeyF1Up.action, doc, "hotkeyF1", 0, "action");
	readDoc(gamepad->options.hotkeyF1Down.action, doc, "hotkeyF1", 1, "action");
	readDoc(gamepad->options.hotkeyF1Left.action, doc, "hotkeyF1", 2, "action");
	readDoc(gamepad->options.hotkeyF1Right.action, doc, "hotkeyF1", 3, "action");

	readDoc(gamepad->options.hotkeyF2Up.action, doc, "hotkeyF2", 0, "action");
	readDoc(gamepad->options.hotkeyF2Down.action, doc, "hotkeyF2", 1, "action");
	readDoc(gamepad->options.hotkeyF2Left.action, doc, "hotkeyF2", 2, "action");
	readDoc(gamepad->options.hotkeyF2Right.action, doc, "hotkeyF2", 3, "action");

	ConfigManager::getInstance().setGamepadOptions(gamepad);
	return serialize_json(doc);
}

std::string getGamepadOptions()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	const ConfigLegacy::GamepadOptions& options = GamepadStore.getGamepadOptions();

	writeDoc(doc, "dpadMode", options.dpadMode);
	writeDoc(doc, "inputMode", options.inputMode);
	writeDoc(doc, "socdMode", options.socdMode);

	writeDoc(doc, "hotkeyF1", 0, "action", options.hotkeyF1Up.action);
	writeDoc(doc, "hotkeyF1", 0, "mask", options.hotkeyF1Up.dpadMask);
	writeDoc(doc, "hotkeyF1", 1, "action", options.hotkeyF1Down.action);
	writeDoc(doc, "hotkeyF1", 1, "mask", options.hotkeyF1Down.dpadMask);
	writeDoc(doc, "hotkeyF1", 2, "action", options.hotkeyF1Left.action);
	writeDoc(doc, "hotkeyF1", 2, "mask", options.hotkeyF1Left.dpadMask);
	writeDoc(doc, "hotkeyF1", 3, "action", options.hotkeyF1Right.action);
	writeDoc(doc, "hotkeyF1", 3, "mask", options.hotkeyF1Right.dpadMask);

	writeDoc(doc, "hotkeyF2", 0, "action", options.hotkeyF2Up.action);
	writeDoc(doc, "hotkeyF2", 0, "mask", options.hotkeyF2Up.dpadMask);
	writeDoc(doc, "hotkeyF2", 1, "action", options.hotkeyF2Down.action);
	writeDoc(doc, "hotkeyF2", 1, "mask", options.hotkeyF2Down.dpadMask);
	writeDoc(doc, "hotkeyF2", 2, "action", options.hotkeyF2Left.action);
	writeDoc(doc, "hotkeyF2", 2, "mask", options.hotkeyF2Left.dpadMask);
	writeDoc(doc, "hotkeyF2", 3, "action", options.hotkeyF2Right.action);
	writeDoc(doc, "hotkeyF2", 3, "mask", options.hotkeyF2Right.dpadMask);

	return serialize_json(doc);
}

std::string setLedOptions()
{
	DynamicJsonDocument doc = get_post_data();

	const auto readIndex = [&](int& var, const char* key0, const char* key1)
	{
		var = -1;
		if (hasValue(doc, key0, key1))
		{
			readDoc(var, doc, key0, key1);
		}
	};

	ConfigLegacy::LEDOptions ledOptions = Storage::getInstance().getLEDOptions();
	ledOptions.useUserDefinedLEDs = true;
	readDoc(ledOptions.dataPin, doc, "dataPin");
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
	ConfigManager::getInstance().setLedOptions(ledOptions);
	return serialize_json(doc);
}

std::string getLedOptions()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	const ConfigLegacy::LEDOptions& ledOptions = Storage::getInstance().getLEDOptions();
	writeDoc(doc, "dataPin", ledOptions.dataPin);
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

	addUsedPinsArray(doc);

	return serialize_json(doc);
}

std::string setCustomTheme()
{
	DynamicJsonDocument doc = get_post_data();

	AnimationOptions options = AnimationStore.getAnimationOptions();

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
	AnimationOptions options = AnimationStore.getAnimationOptions();

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

	Storage::getInstance().save();

	return serialize_json(doc);
}

std::string getPinMappings()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);

	const PinMappings& pinMappings = Storage::getInstance().getPinMappings();
	writeDoc(doc, "Up", pinMappings.pinDpadUp);
	writeDoc(doc, "Down", pinMappings.pinDpadDown);
	writeDoc(doc, "Left", pinMappings.pinDpadLeft);
	writeDoc(doc, "Right", pinMappings.pinDpadRight);
	writeDoc(doc, "B1", pinMappings.pinButtonB1);
	writeDoc(doc, "B2", pinMappings.pinButtonB2);
	writeDoc(doc, "B3", pinMappings.pinButtonB3);
	writeDoc(doc, "B4", pinMappings.pinButtonB4);
	writeDoc(doc, "L1", pinMappings.pinButtonL1);
	writeDoc(doc, "R1", pinMappings.pinButtonR1);
	writeDoc(doc, "L2", pinMappings.pinButtonL2);
	writeDoc(doc, "R2", pinMappings.pinButtonR2);
	writeDoc(doc, "S1", pinMappings.pinButtonS1);
	writeDoc(doc, "S2", pinMappings.pinButtonS2);
	writeDoc(doc, "L3", pinMappings.pinButtonL3);
	writeDoc(doc, "R3", pinMappings.pinButtonR3);
	writeDoc(doc, "A1", pinMappings.pinButtonA1);
	writeDoc(doc, "A2", pinMappings.pinButtonA2);

	return serialize_json(doc);
}

std::string setKeyMappings()
{
	DynamicJsonDocument doc = get_post_data();
	Gamepad* gamepad = Storage::getInstance().GetGamepad();

	readDoc(gamepad->options.keyDpadUp, doc, "Up");
	readDoc(gamepad->options.keyDpadDown, doc, "Down");
	readDoc(gamepad->options.keyDpadLeft, doc, "Left");
	readDoc(gamepad->options.keyDpadRight, doc, "Right");
	readDoc(gamepad->options.keyButtonB1, doc, "B1");
	readDoc(gamepad->options.keyButtonB2, doc, "B2");
	readDoc(gamepad->options.keyButtonB3, doc, "B3");
	readDoc(gamepad->options.keyButtonB4, doc, "B4");
	readDoc(gamepad->options.keyButtonL1, doc, "L1");
	readDoc(gamepad->options.keyButtonR1, doc, "R1");
	readDoc(gamepad->options.keyButtonL2, doc, "L2");
	readDoc(gamepad->options.keyButtonR2, doc, "R2");
	readDoc(gamepad->options.keyButtonS1, doc, "S1");
	readDoc(gamepad->options.keyButtonS2, doc, "S2");
	readDoc(gamepad->options.keyButtonL3, doc, "L3");
	readDoc(gamepad->options.keyButtonR3, doc, "R3");
	readDoc(gamepad->options.keyButtonA1, doc, "A1");
	readDoc(gamepad->options.keyButtonA2, doc, "A2");

	gamepad->save();
	return serialize_json(doc);
}

std::string getKeyMappings()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	Gamepad* gamepad = Storage::getInstance().GetGamepad();

	writeDoc(doc, "Up", gamepad->options.keyDpadUp);
	writeDoc(doc, "Down", gamepad->options.keyDpadDown);
	writeDoc(doc, "Left", gamepad->options.keyDpadLeft);
	writeDoc(doc, "Right", gamepad->options.keyDpadRight);
	writeDoc(doc, "B1", gamepad->options.keyButtonB1);
	writeDoc(doc, "B2", gamepad->options.keyButtonB2);
	writeDoc(doc, "B3", gamepad->options.keyButtonB3);
	writeDoc(doc, "B4", gamepad->options.keyButtonB4);
	writeDoc(doc, "L1", gamepad->options.keyButtonL1);
	writeDoc(doc, "R1", gamepad->options.keyButtonR1);
	writeDoc(doc, "L2", gamepad->options.keyButtonL2);
	writeDoc(doc, "R2", gamepad->options.keyButtonR2);
	writeDoc(doc, "S1", gamepad->options.keyButtonS1);
	writeDoc(doc, "S2", gamepad->options.keyButtonS2);
	writeDoc(doc, "L3", gamepad->options.keyButtonL3);
	writeDoc(doc, "R3", gamepad->options.keyButtonR3);
	writeDoc(doc, "A1", gamepad->options.keyButtonA1);
	writeDoc(doc, "A2", gamepad->options.keyButtonA2);

	return serialize_json(doc);
}

std::string setAddonOptions()
{
	DynamicJsonDocument doc = get_post_data();

    AnalogOptions& analogOptions = Storage::getInstance().getAddonOptions().analogOptions;
	docToPin(analogOptions.analogAdcPinX, doc, "analogAdcPinX");
	docToPin(analogOptions.analogAdcPinY, doc, "analogAdcPinY");
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
	docToValue(dualDirectionalOptions.enabled, doc, "DualDirectionalInputEnabled");

    ExtraOptions& extraOptions = Storage::getInstance().getAddonOptions().extraOptions;
	docToPin(extraOptions.pin, doc, "extraButtonPin");
	docToValue(extraOptions.buttonMap, doc, "extraButtonMap");
	docToValue(extraOptions.enabled, doc, "ExtraButtonAddonEnabled");

    AnalogADS1219Options& analogADS1219Options = Storage::getInstance().getAddonOptions().analogADS1219Options;
	docToPin(analogADS1219Options.i2cSDAPin, doc, "i2cAnalog1219SDAPin");
	docToPin(analogADS1219Options.i2cSCLPin, doc, "i2cAnalog1219SCLPin");
	docToValue(analogADS1219Options.i2cBlock, doc, "i2cAnalog1219Block");
	docToValue(analogADS1219Options.i2cSpeed, doc, "i2cAnalog1219Speed");
	docToValue(analogADS1219Options.i2cAddress, doc, "i2cAnalog1219Address");
	docToValue(analogADS1219Options.enabled, doc, "I2CAnalog1219InputEnabled");

    SliderOptions& sliderOptions = Storage::getInstance().getAddonOptions().sliderOptions;
	docToPin(sliderOptions.pinLS, doc, "sliderLSPin");
	docToPin(sliderOptions.pinRS, doc, "sliderRSPin");
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

	ConfigLegacy::AddonOptions addonOptions = Storage::getInstance().getLegacyAddonOptions();
	docToPinLegacy(addonOptions.reverseActionUp, doc, "reverseActionUp");
	docToPinLegacy(addonOptions.reverseActionDown, doc, "reverseActionDown");
	docToPinLegacy(addonOptions.reverseActionLeft, doc, "reverseActionLeft");
	docToPinLegacy(addonOptions.reverseActionRight, doc, "reverseActionRight");
	docToValue(addonOptions.PS4ModeAddonEnabled, doc, "PS4ModeAddonEnabled");

	Storage::getInstance().setLegacyAddonOptions(addonOptions);
	Storage::getInstance().save();

	return serialize_json(doc);
}

std::string setPS4Options()
{
	DynamicJsonDocument doc = get_post_data();
	ConfigLegacy::PS4Options * ps4Options = Storage::getInstance().getPS4Options();
	std::string encoded;
	std::string decoded;

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
		Base64::Decode(encoded, decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_n ) ) {
			memcpy(ps4Options->rsa_n, decoded.data(), decoded.length());
		}
	}
	if ( readEncoded("E") ) {
		Base64::Decode(encoded, decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_e ) ) {
			memcpy(ps4Options->rsa_e, decoded.data(), decoded.length());
		}
	}
	if ( readEncoded("D") ) {
		Base64::Decode(encoded, decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_d ) ) {
			memcpy(ps4Options->rsa_d, decoded.data(), decoded.length());
		}
	}
	if ( readEncoded("P") ) {
		Base64::Decode(encoded, decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_p ) ) {
			memcpy(ps4Options->rsa_p, decoded.data(), decoded.length());
		}
	}
	if ( readEncoded("Q") ) {
		Base64::Decode(encoded, decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_q ) ) {
			memcpy(ps4Options->rsa_q, decoded.data(), decoded.length());
		}
	}
	if ( readEncoded("DP") ) {
		Base64::Decode(encoded, decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_dp ) ) {
			memcpy(ps4Options->rsa_dp, decoded.data(), decoded.length());
		}
	}
	if ( readEncoded("DQ") ) {
		Base64::Decode(encoded, decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_dq ) ) {
			memcpy(ps4Options->rsa_dq, decoded.data(), decoded.length());
		}
	}
	if ( readEncoded("QP") ) {
		Base64::Decode(encoded, decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_qp ) ) {
			memcpy(ps4Options->rsa_qp, decoded.data(), decoded.length());
		}
	}
	if ( readEncoded("RN") ) {
		Base64::Decode(encoded, decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_rn ) ) {
			memcpy(ps4Options->rsa_rn, decoded.data(), decoded.length());
		}
	}
	// Serial & Signature
	if ( readEncoded("serial") ) {
		Base64::Decode(encoded, decoded);
		if ( decoded.length() == sizeof(ps4Options->serial ) ) {
			memcpy(ps4Options->serial, decoded.data(), decoded.length());
		}
	}
	if ( readEncoded("signature") ) {
		Base64::Decode(encoded, decoded);
		if ( decoded.length() == sizeof(ps4Options->signature ) ) {
			memcpy(ps4Options->signature, decoded.data(), decoded.length());
		}
	}

	Storage::getInstance().savePS4Options();

	return "{\"success\":true}";
}

std::string getAddonOptions()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);

    const AnalogOptions& analogOptions = Storage::getInstance().getAddonOptions().analogOptions;
	writeDoc(doc, "analogAdcPinX", analogOptions.analogAdcPinX == 0xFF ? -1 : analogOptions.analogAdcPinX);
	writeDoc(doc, "analogAdcPinY", analogOptions.analogAdcPinY == 0xFF ? -1 : analogOptions.analogAdcPinY);
	writeDoc(doc, "AnalogInputEnabled", analogOptions.enabled);

    const BootselButtonOptions& bootselButtonOptions = Storage::getInstance().getAddonOptions().bootselButtonOptions;
	writeDoc(doc, "bootselButtonMap", bootselButtonOptions.buttonMap);
	writeDoc(doc, "BootselButtonAddonEnabled", bootselButtonOptions.enabled);

    const BuzzerOptions& buzzerOptions = Storage::getInstance().getAddonOptions().buzzerOptions;
	writeDoc(doc, "buzzerPin", buzzerOptions.pin == 0xFF ? -1 : buzzerOptions.pin);
	writeDoc(doc, "buzzerVolume", buzzerOptions.volume);
	writeDoc(doc, "BuzzerSpeakerAddonEnabled", buzzerOptions.enabled);

    const DualDirectionalOptions& dualDirectionalOptions = Storage::getInstance().getAddonOptions().dualDirectionalOptions;
	writeDoc(doc, "dualDirDownPin", dualDirectionalOptions.downPin == 0xFF ? -1 : dualDirectionalOptions.downPin);
	writeDoc(doc, "dualDirUpPin", dualDirectionalOptions.upPin == 0xFF ? -1 : dualDirectionalOptions.upPin);
	writeDoc(doc, "dualDirLeftPin", dualDirectionalOptions.leftPin == 0xFF ? -1 : dualDirectionalOptions.leftPin);
	writeDoc(doc, "dualDirRightPin", dualDirectionalOptions.rightPin == 0xFF ? -1 : dualDirectionalOptions.rightPin);
	writeDoc(doc, "dualDirDpadMode", dualDirectionalOptions.dpadMode);
	writeDoc(doc, "dualDirCombineMode", dualDirectionalOptions.combineMode);
	writeDoc(doc, "DualDirectionalInputEnabled", dualDirectionalOptions.enabled);

    const ExtraOptions& extraOptions = Storage::getInstance().getAddonOptions().extraOptions;
	writeDoc(doc, "extraButtonPin", extraOptions.pin == 0xFF ? -1 : extraOptions.pin);
	writeDoc(doc, "extraButtonMap", extraOptions.buttonMap);
	writeDoc(doc, "ExtraButtonAddonEnabled", extraOptions.enabled);

    const AnalogADS1219Options& analogADS1219Options = Storage::getInstance().getAddonOptions().analogADS1219Options;
	writeDoc(doc, "i2cAnalog1219SDAPin", analogADS1219Options.i2cSDAPin == 0xFF ? -1 : analogADS1219Options.i2cSDAPin);
	writeDoc(doc, "i2cAnalog1219SCLPin", analogADS1219Options.i2cSCLPin == 0xFF ? -1 : analogADS1219Options.i2cSCLPin);
	writeDoc(doc, "i2cAnalog1219Block", analogADS1219Options.i2cBlock);
	writeDoc(doc, "i2cAnalog1219Speed", analogADS1219Options.i2cSpeed);
	writeDoc(doc, "i2cAnalog1219Address", analogADS1219Options.i2cAddress);
	writeDoc(doc, "I2CAnalog1219InputEnabled", analogADS1219Options.enabled);

    const SliderOptions& sliderOptions = Storage::getInstance().getAddonOptions().sliderOptions;
	writeDoc(doc, "sliderLSPin", sliderOptions.pinLS == 0xFF ? -1 : sliderOptions.pinLS);
	writeDoc(doc, "sliderRSPin", sliderOptions.pinRS == 0xFF ? -1 : sliderOptions.pinRS);
	writeDoc(doc, "JSliderInputEnabled", sliderOptions.enabled);

    const PlayerNumberOptions& playerNumberOptions = Storage::getInstance().getAddonOptions().playerNumberOptions;
	writeDoc(doc, "playerNumber", playerNumberOptions.number);
	writeDoc(doc, "PlayerNumAddonEnabled", playerNumberOptions.enabled);

	const ReverseOptions& reverseOptions = Storage::getInstance().getAddonOptions().reverseOptions;
	writeDoc(doc, "reversePin", isValidPin(reverseOptions.buttonPin) ? reverseOptions.buttonPin : -1);
	writeDoc(doc, "reversePinLED", isValidPin(reverseOptions.ledPin) ? reverseOptions.ledPin : -1);
	writeDoc(doc, "reverseActionUp", reverseOptions.actionUp);
	writeDoc(doc, "reverseActionDown", reverseOptions.actionDown);
	writeDoc(doc, "reverseActionLeft", reverseOptions.actionLeft);
	writeDoc(doc, "reverseActionRight", reverseOptions.actionRight);
	writeDoc(doc, "ReverseInputEnabled", reverseOptions.enabled);

    const SOCDSliderOptions& socdSliderOptions = Storage::getInstance().getAddonOptions().socdSliderOptions;
	writeDoc(doc, "sliderSOCDPinOne", socdSliderOptions.pinOne == 0xFF ? -1 : socdSliderOptions.pinOne);
	writeDoc(doc, "sliderSOCDPinTwo", socdSliderOptions.pinTwo == 0xFF ? -1 : socdSliderOptions.pinTwo);
	writeDoc(doc, "sliderSOCDModeOne", socdSliderOptions.modeOne);
	writeDoc(doc, "sliderSOCDModeTwo", socdSliderOptions.modeTwo);
	writeDoc(doc, "sliderSOCDModeDefault", socdSliderOptions.modeDefault);
	writeDoc(doc, "SliderSOCDInputEnabled", socdSliderOptions.enabled);

    const OnBoardLedOptions& onBoardLedOptions = Storage::getInstance().getAddonOptions().onBoardLedOptions;
	writeDoc(doc, "onBoardLedMode", onBoardLedOptions.mode);
	writeDoc(doc, "BoardLedAddonEnabled", onBoardLedOptions.enabled);

    const TurboOptions& turboOptions = Storage::getInstance().getAddonOptions().turboOptions;
	writeDoc(doc, "turboPin", turboOptions.buttonPin == 0xFF ? -1 : turboOptions.buttonPin);
	writeDoc(doc, "turboPinLED", turboOptions.ledPin == 0xFF ? -1 : turboOptions.ledPin);
	writeDoc(doc, "turboShotCount", turboOptions.shotCount);
	writeDoc(doc, "shmupMode", turboOptions.shmupModeEnabled);
	writeDoc(doc, "shmupMixMode", turboOptions.shmupMixMode);
	writeDoc(doc, "shmupAlwaysOn1", turboOptions.shmupAlwaysOn1);
	writeDoc(doc, "shmupAlwaysOn2", turboOptions.shmupAlwaysOn2);
	writeDoc(doc, "shmupAlwaysOn3", turboOptions.shmupAlwaysOn3);
	writeDoc(doc, "shmupAlwaysOn4", turboOptions.shmupAlwaysOn4);
	writeDoc(doc, "pinShmupBtn1", turboOptions.shmupBtn1Pin == 0xFF ? -1 : turboOptions.shmupBtn1Pin);
	writeDoc(doc, "pinShmupBtn2", turboOptions.shmupBtn2Pin == 0xFF ? -1 : turboOptions.shmupBtn2Pin);
	writeDoc(doc, "pinShmupBtn3", turboOptions.shmupBtn3Pin == 0xFF ? -1 : turboOptions.shmupBtn3Pin);
	writeDoc(doc, "pinShmupBtn4", turboOptions.shmupBtn4Pin == 0xFF ? -1 : turboOptions.shmupBtn4Pin);
	writeDoc(doc, "shmupBtnMask1", turboOptions.shmupBtnMask1);
	writeDoc(doc, "shmupBtnMask2", turboOptions.shmupBtnMask2);
	writeDoc(doc, "shmupBtnMask3", turboOptions.shmupBtnMask3);
	writeDoc(doc, "shmupBtnMask4", turboOptions.shmupBtnMask4);
	writeDoc(doc, "pinShmupDial", turboOptions.shmupDialPin == 0xFF ? -1 : turboOptions.shmupDialPin);
	writeDoc(doc, "TurboInputEnabled", turboOptions.enabled);

    const WiiOptions& wiiOptions = Storage::getInstance().getAddonOptions().wiiOptions;
	writeDoc(doc, "wiiExtensionSDAPin", wiiOptions.i2cSDAPin == 0xFF ? -1 : wiiOptions.i2cSDAPin);
	writeDoc(doc, "wiiExtensionSCLPin", wiiOptions.i2cSCLPin == 0xFF ? -1 : wiiOptions.i2cSCLPin);
	writeDoc(doc, "wiiExtensionBlock", wiiOptions.i2cBlock);
	writeDoc(doc, "wiiExtensionSpeed", wiiOptions.i2cSpeed);
	writeDoc(doc, "WiiExtensionAddonEnabled", wiiOptions.enabled);

	const ConfigLegacy::AddonOptions& addonOptions = Storage::getInstance().getLegacyAddonOptions();
	writeDoc(doc, "PS4ModeAddonEnabled", addonOptions.PS4ModeAddonEnabled);

	addUsedPinsArray(doc);

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
	if (ConfigUtils::fromJSON(*config.get(), http_post_payload, http_post_payload_len))
	{
		Storage::getInstance().getConfig() = *config.get();
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
	{ "/api/setKeyMappings", setKeyMappings },
	{ "/api/setAddonsOptions", setAddonOptions },
	{ "/api/setPS4Options", setPS4Options },
	{ "/api/setSplashImage", setSplashImage },
	{ "/api/reboot", reboot },
	{ "/api/getDisplayOptions", getDisplayOptions },
	{ "/api/getGamepadOptions", getGamepadOptions },
	{ "/api/getLedOptions", getLedOptions },
	{ "/api/getPinMappings", getPinMappings },
	{ "/api/getKeyMappings", getKeyMappings },
	{ "/api/getAddonsOptions", getAddonOptions },
	{ "/api/resetSettings", resetSettings },
	{ "/api/getSplashImage", getSplashImage },
	{ "/api/getFirmwareVersion", getFirmwareVersion },
	{ "/api/getMemoryReport", getMemoryReport },
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
	for (const auto &excludePath : excludePaths)
		if (!excludePath.compare(name))
			return 0;

	for (const auto &spaPath : spaPaths)
	{
		if (!spaPath.compare(name))
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
