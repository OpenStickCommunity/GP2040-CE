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

	// TODO: Exclude non-button pins from validation for now, fix this when validation reworked
	// addPinIfValid(boardOptions.i2cSDAPin);
	// addPinIfValid(boardOptions.i2cSCLPin);

	const AnalogOptions& analogOptions = Storage::getInstance().getAddonOptions().analogOptions;
	addPinIfValid(analogOptions.analogAdcPinX);
	addPinIfValid(analogOptions.analogAdcPinY);

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
	DisplayOptions& displayOptions = Storage::getInstance().getDisplayOptions();
	std::string response = setDisplayOptions(displayOptions);
	Storage::getInstance().save();
	return response;
}

std::string setPreviewDisplayOptions()
{
	ConfigLegacy::BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
	std::string response = ""; // TODO: Setup preview display
	ConfigManager::getInstance().setPreviewBoardOptions(boardOptions);
	return response;
}

std::string getDisplayOptions() // Manually set Document Attributes for the display
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	const DisplayOptions& displayOptions = Storage::getInstance().getDisplayOptions();
	writeDoc(doc, "enabled", displayOptions.enabled ? 1 : 0);
	writeDoc(doc, "sdaPin", displayOptions.i2cSDAPin == 0xFF ? -1 : displayOptions.i2cSDAPin);
	writeDoc(doc, "sclPin", displayOptions.i2cSCLPin == 0xFF ? -1 : displayOptions.i2cSCLPin);
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

std::string setGamepadOptions()
{
	DynamicJsonDocument doc = get_post_data();
	
	GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();
	readDoc(gamepadOptions.dpadMode, doc, "dpadMode");
	readDoc(gamepadOptions.inputMode, doc, "inputMode");
	readDoc(gamepadOptions.socdMode, doc, "socdMode");

	HotkeyOptions& hotkeyOptions = Storage::getInstance().getHotkeyOptions();
	readDoc(hotkeyOptions.hotkeyF1Up.action, doc, "hotkeyF1", 0, "action");
	readDoc(hotkeyOptions.hotkeyF1Down.action, doc, "hotkeyF1", 1, "action");
	readDoc(hotkeyOptions.hotkeyF1Left.action, doc, "hotkeyF1", 2, "action");
	readDoc(hotkeyOptions.hotkeyF1Right.action, doc, "hotkeyF1", 3, "action");

	readDoc(hotkeyOptions.hotkeyF2Up.action, doc, "hotkeyF2", 0, "action");
	readDoc(hotkeyOptions.hotkeyF2Down.action, doc, "hotkeyF2", 1, "action");
	readDoc(hotkeyOptions.hotkeyF2Left.action, doc, "hotkeyF2", 2, "action");
	readDoc(hotkeyOptions.hotkeyF2Right.action, doc, "hotkeyF2", 3, "action");

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

	HotkeyOptions& hotkeyOptions = Storage::getInstance().getHotkeyOptions();
	writeDoc(doc, "hotkeyF1", 0, "action", hotkeyOptions.hotkeyF1Up.action);
	writeDoc(doc, "hotkeyF1", 0, "mask", hotkeyOptions.hotkeyF1Up.dpadMask);
	writeDoc(doc, "hotkeyF1", 1, "action", hotkeyOptions.hotkeyF1Down.action);
	writeDoc(doc, "hotkeyF1", 1, "mask", hotkeyOptions.hotkeyF1Down.dpadMask);
	writeDoc(doc, "hotkeyF1", 2, "action", hotkeyOptions.hotkeyF1Left.action);
	writeDoc(doc, "hotkeyF1", 2, "mask", hotkeyOptions.hotkeyF1Left.dpadMask);
	writeDoc(doc, "hotkeyF1", 3, "action", hotkeyOptions.hotkeyF1Right.action);
	writeDoc(doc, "hotkeyF1", 3, "mask", hotkeyOptions.hotkeyF1Right.dpadMask);

	writeDoc(doc, "hotkeyF2", 0, "action", hotkeyOptions.hotkeyF2Up.action);
	writeDoc(doc, "hotkeyF2", 0, "mask", hotkeyOptions.hotkeyF2Up.dpadMask);
	writeDoc(doc, "hotkeyF2", 1, "action", hotkeyOptions.hotkeyF2Down.action);
	writeDoc(doc, "hotkeyF2", 1, "mask", hotkeyOptions.hotkeyF2Down.dpadMask);
	writeDoc(doc, "hotkeyF2", 2, "action", hotkeyOptions.hotkeyF2Left.action);
	writeDoc(doc, "hotkeyF2", 2, "mask", hotkeyOptions.hotkeyF2Left.dpadMask);
	writeDoc(doc, "hotkeyF2", 3, "action", hotkeyOptions.hotkeyF2Right.action);
	writeDoc(doc, "hotkeyF2", 3, "mask", hotkeyOptions.hotkeyF2Right.dpadMask);

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
	readDoc(ledOptions.pledType, doc, "pledType");
	readDoc(ledOptions.pledPin1, doc, "pledPin1");
	readDoc(ledOptions.pledPin2, doc, "pledPin2");
	readDoc(ledOptions.pledPin3, doc, "pledPin3");
	readDoc(ledOptions.pledPin4, doc, "pledPin4");
	uint32_t pledColor;
	readDoc(pledColor, doc, "pledColor");
	ledOptions.pledColor = RGB(pledColor);
	Storage::getInstance().setLEDOptions(ledOptions);
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

    PS4Options& ps4Options = Storage::getInstance().getAddonOptions().ps4Options;
	docToValue(ps4Options.enabled, doc, "PS4ModeAddonEnabled");

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
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == ps4Options.rsaN.size) ) {
			memcpy(ps4Options.rsaN.bytes, decoded.data(), decoded.length());
			ps4Options.rsaN.size = decoded.length();
		}
	}
	if ( readEncoded("E") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == ps4Options.rsaE.size) ) {
			memcpy(ps4Options.rsaE.bytes, decoded.data(), decoded.length());
			ps4Options.rsaE.size = decoded.length();
		}
	}
	if ( readEncoded("D") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == ps4Options.rsaD.size) ) {
			memcpy(ps4Options.rsaD.bytes, decoded.data(), decoded.length());
			ps4Options.rsaD.size = decoded.length();
		}
	}
	if ( readEncoded("P") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == ps4Options.rsaP.size) ) {
			memcpy(ps4Options.rsaP.bytes, decoded.data(), decoded.length());
			ps4Options.rsaP.size = decoded.length();
		}
	}
	if ( readEncoded("Q") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == ps4Options.rsaQ.size) ) {
			memcpy(ps4Options.rsaQ.bytes, decoded.data(), decoded.length());
			ps4Options.rsaQ.size = decoded.length();
		}
	}
	if ( readEncoded("DP") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == ps4Options.rsaDP.size) ) {
			memcpy(ps4Options.rsaDP.bytes, decoded.data(), decoded.length());
			ps4Options.rsaDP.size = decoded.length();
		}
	}
	if ( readEncoded("DQ") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == ps4Options.rsaDQ.size) ) {
			memcpy(ps4Options.rsaDQ.bytes, decoded.data(), decoded.length());
			ps4Options.rsaDQ.size = decoded.length();
		}
	}
	if ( readEncoded("QP") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == ps4Options.rsaQP.size) ) {
			memcpy(ps4Options.rsaQP.bytes, decoded.data(), decoded.length());
			ps4Options.rsaQP.size = decoded.length();
		}
	}
	if ( readEncoded("RN") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == ps4Options.rsaRN.size) ) {
			memcpy(ps4Options.rsaRN.bytes, decoded.data(), decoded.length());
			ps4Options.rsaRN.size = decoded.length();
		}
	}
	// Serial & Signature
	if ( readEncoded("serial") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == ps4Options.serial.size) ) {
			memcpy(ps4Options.serial.bytes, decoded.data(), decoded.length());
			ps4Options.serial.size = decoded.length();
		}
	}
	if ( readEncoded("signature") ) {
		if ( Base64::Decode(encoded, decoded) && (decoded.length() == ps4Options.signature.size) ) {
			memcpy(ps4Options.signature.bytes, decoded.data(), decoded.length());
			ps4Options.signature.size = decoded.length();
		}
	}

	Storage::getInstance().save();

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

	const PS4Options& ps4Options = Storage::getInstance().getAddonOptions().ps4Options;
	writeDoc(doc, "PS4ModeAddonEnabled", ps4Options.enabled);

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
	{ "/api/getUsedPins", getUsedPins },
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
