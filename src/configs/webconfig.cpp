#include "configs/webconfig.h"
#include "configs/base64.h"

#include "storagemanager.h"
#include "configmanager.h"
#include "AnimationStorage.hpp"
#include "system.h"

#include <cstring>
#include <string>
#include <vector>

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

#define LWIP_HTTPD_POST_MAX_PAYLOAD_LEN 4096

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
static void __attribute__((noinline)) docToPin(uint8_t& pin, const DynamicJsonDocument& doc, const char* key)
{
	if (doc.containsKey(key))
	{
		pin = doc[key] < 0 ? 0xff : doc[key];
	}
}

// Don't inline this function, we do not want to consume stack space in the calling function
template <typename T, typename K>
static void __attribute__((noinline)) writeDoc(DynamicJsonDocument& doc, const K& key, const T& var)
{
	doc[key] = var;
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

// **** WEB SERVER Overrides and Special Functionality ****
int set_file_data(struct fs_file *file, string data)
{
	static string returnData;
	returnData.clear();
	returnData.append(
		"HTTP/1.0 200 OK\r\n"
		"Server: GP2040-CE " GP2040VERSION "\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: "
	);
	returnData.append(std::to_string(data.length()));
	returnData.append("\r\n\r\n");
	returnData.append(data);

	file->data = returnData.c_str();
	file->len = returnData.size();
	file->index = file->len;
	file->http_header_included = file->http_header_included;
	file->pextension = NULL;

	return 1;
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

	const BoardOptions& boardOptions = Storage::getInstance().getBoardOptions();
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

	const AddonOptions& addonOptions = Storage::getInstance().getAddonOptions();
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

std::string setDisplayOptions(BoardOptions& boardOptions)
{
	DynamicJsonDocument doc = get_post_data();
	readDoc(boardOptions.hasI2CDisplay, doc, "enabled");
	docToPin(boardOptions.i2cSDAPin, doc, "sdaPin");
	docToPin(boardOptions.i2cSCLPin, doc, "sclPin");
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
	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
	std::string response = setDisplayOptions(boardOptions);
	ConfigManager::getInstance().setBoardOptions(boardOptions);
	return response;
}

std::string setPreviewDisplayOptions()
{
	BoardOptions boardOptions = Storage::getInstance().getPreviewBoardOptions();
	std::string response = setDisplayOptions(boardOptions);
	ConfigManager::getInstance().setPreviewBoardOptions(boardOptions);
	return response;
}

std::string getDisplayOptions() // Manually set Document Attributes for the display
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	const BoardOptions& boardOptions = Storage::getInstance().getBoardOptions();
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

SplashImage splashImageTemp; // For splash image upload

std::string getSplashImage()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN * 10); // TODO: Figoure out correct length
	const SplashImage& splashImage = Storage::getInstance().getSplashImage();
	JsonArray splashImageArray = doc.createNestedArray("splashImage");
	copyArray(splashImage.data, splashImageArray);

	return serialize_json(doc);
}

std::string setSplashImage()
{
	DynamicJsonDocument doc = get_post_data();
	std::string decoded;
	std::string base64String = doc["splashImage"];
	Base64::Decode(base64String, decoded);
	memcpy(splashImageTemp.data, decoded.data(), std::min(decoded.length(), sizeof(splashImageTemp.data)));
	splashImageTemp.checksum = CHECKSUM_MAGIC;
	ConfigManager::getInstance().setSplashImage(splashImageTemp);

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
	GamepadOptions options = GamepadStore.getGamepadOptions();

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

	LEDOptions ledOptions = Storage::getInstance().getLEDOptions();
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
	const LEDOptions& ledOptions = Storage::getInstance().getLEDOptions();
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

	// BoardOptions uses 0xff to denote unassigned pins
	const auto convertPin = [&] (const char* key) -> uint8_t
	{
		int pin = 0;
		readDoc(pin, doc, key);
		return pin >= 0 && pin < NUM_BANK0_GPIOS ? pin : 0xff;
	};

	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
	boardOptions.hasBoardOptions = true;
	boardOptions.pinDpadUp    = convertPin("Up");
	boardOptions.pinDpadDown  = convertPin("Down");
	boardOptions.pinDpadLeft  = convertPin("Left");
	boardOptions.pinDpadRight = convertPin("Right");
	boardOptions.pinButtonB1  = convertPin("B1");
	boardOptions.pinButtonB2  = convertPin("B2");
	boardOptions.pinButtonB3  = convertPin("B3");
	boardOptions.pinButtonB4  = convertPin("B4");
	boardOptions.pinButtonL1  = convertPin("L1");
	boardOptions.pinButtonR1  = convertPin("R1");
	boardOptions.pinButtonL2  = convertPin("L2");
	boardOptions.pinButtonR2  = convertPin("R2");
	boardOptions.pinButtonS1  = convertPin("S1");
	boardOptions.pinButtonS2  = convertPin("S2");
	boardOptions.pinButtonL3  = convertPin("L3");
	boardOptions.pinButtonR3  = convertPin("R3");
	boardOptions.pinButtonA1  = convertPin("A1");
	boardOptions.pinButtonA2  = convertPin("A2");

	Storage::getInstance().setBoardOptions(boardOptions);

	return serialize_json(doc);
}

std::string getPinMappings()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);

	// Webconfig uses -1 to denote unassigned pins
	const auto convertPin = [] (uint8_t pin) -> int { return pin < NUM_BANK0_GPIOS ? pin : -1; };

	const BoardOptions& boardOptions = Storage::getInstance().getBoardOptions();
	writeDoc(doc, "Up", convertPin(boardOptions.pinDpadUp));
	writeDoc(doc, "Down", convertPin(boardOptions.pinDpadDown));
	writeDoc(doc, "Left", convertPin(boardOptions.pinDpadLeft));
	writeDoc(doc, "Right", convertPin(boardOptions.pinDpadRight));
	writeDoc(doc, "B1", convertPin(boardOptions.pinButtonB1));
	writeDoc(doc, "B2", convertPin(boardOptions.pinButtonB2));
	writeDoc(doc, "B3", convertPin(boardOptions.pinButtonB3));
	writeDoc(doc, "B4", convertPin(boardOptions.pinButtonB4));
	writeDoc(doc, "L1", convertPin(boardOptions.pinButtonL1));
	writeDoc(doc, "R1", convertPin(boardOptions.pinButtonR1));
	writeDoc(doc, "L2", convertPin(boardOptions.pinButtonL2));
	writeDoc(doc, "R2", convertPin(boardOptions.pinButtonR2));
	writeDoc(doc, "S1", convertPin(boardOptions.pinButtonS1));
	writeDoc(doc, "S2", convertPin(boardOptions.pinButtonS2));
	writeDoc(doc, "L3", convertPin(boardOptions.pinButtonL3));
	writeDoc(doc, "R3", convertPin(boardOptions.pinButtonR3));
	writeDoc(doc, "A1", convertPin(boardOptions.pinButtonA1));
	writeDoc(doc, "A2", convertPin(boardOptions.pinButtonA2));

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

	AddonOptions addonOptions = Storage::getInstance().getAddonOptions();
	docToPin(addonOptions.pinButtonTurbo, doc, "turboPin");
	docToPin(addonOptions.pinTurboLED, doc, "turboPinLED");
	docToPin(addonOptions.pinSliderLS, doc, "sliderLSPin");
	docToPin(addonOptions.pinSliderRS, doc, "sliderRSPin");
	docToPin(addonOptions.pinSliderSOCDOne, doc, "sliderSOCDPinOne");
	docToPin(addonOptions.pinSliderSOCDTwo, doc, "sliderSOCDPinTwo");
	docToValue(addonOptions.turboShotCount, doc, "turboShotCount");
	docToPin(addonOptions.pinButtonReverse, doc, "reversePin");
	docToPin(addonOptions.pinReverseLED, doc, "reversePinLED");
	docToPin(addonOptions.reverseActionUp, doc, "reverseActionUp");
	docToPin(addonOptions.reverseActionDown, doc, "reverseActionDown");
	docToPin(addonOptions.reverseActionLeft, doc, "reverseActionLeft");
	docToPin(addonOptions.reverseActionRight, doc, "reverseActionRight");
	docToPin(addonOptions.i2cAnalog1219SDAPin, doc, "i2cAnalog1219SDAPin");
	docToPin(addonOptions.i2cAnalog1219SCLPin, doc, "i2cAnalog1219SCLPin");
	docToValue(addonOptions.i2cAnalog1219Block, doc, "i2cAnalog1219Block");
	docToValue(addonOptions.i2cAnalog1219Speed, doc, "i2cAnalog1219Speed");
	docToValue(addonOptions.i2cAnalog1219Address, doc, "i2cAnalog1219Address");
	docToValue(addonOptions.onBoardLedMode, doc, "onBoardLedMode");
	docToPin(addonOptions.pinDualDirDown, doc, "dualDirDownPin");
	docToPin(addonOptions.pinDualDirUp, doc, "dualDirUpPin");
	docToPin(addonOptions.pinDualDirLeft, doc, "dualDirLeftPin");
	docToPin(addonOptions.pinDualDirRight, doc, "dualDirRightPin");
	docToValue(addonOptions.dualDirDpadMode, doc, "dualDirDpadMode");
	docToValue(addonOptions.dualDirCombineMode, doc, "dualDirCombineMode");
	docToPin(addonOptions.analogAdcPinX, doc, "analogAdcPinX");
	docToPin(addonOptions.analogAdcPinY, doc, "analogAdcPinY");
	docToValue(addonOptions.bootselButtonMap, doc, "bootselButtonMap");
	docToPin(addonOptions.buzzerPin, doc, "buzzerPin");
	docToValue(addonOptions.buzzerVolume, doc, "buzzerVolume");
	docToPin(addonOptions.extraButtonPin, doc, "extraButtonPin");
	docToValue(addonOptions.extraButtonMap, doc, "extraButtonMap");
	docToValue(addonOptions.playerNumber, doc, "playerNumber");
	docToValue(addonOptions.shmupMode, doc, "shmupMode");
	docToValue(addonOptions.shmupMixMode, doc, "shmupMixMode");
	docToValue(addonOptions.shmupAlwaysOn1, doc, "shmupAlwaysOn1");
	docToValue(addonOptions.shmupAlwaysOn2, doc, "shmupAlwaysOn2");
	docToValue(addonOptions.shmupAlwaysOn3, doc, "shmupAlwaysOn3");
	docToValue(addonOptions.shmupAlwaysOn4, doc, "shmupAlwaysOn4");
	docToPin(addonOptions.pinShmupBtn1, doc, "pinShmupBtn1");
	docToPin(addonOptions.pinShmupBtn2, doc, "pinShmupBtn2");
	docToPin(addonOptions.pinShmupBtn3, doc, "pinShmupBtn3");
	docToPin(addonOptions.pinShmupBtn4, doc, "pinShmupBtn4");
	docToValue(addonOptions.shmupBtnMask1, doc, "shmupBtnMask1");
	docToValue(addonOptions.shmupBtnMask2, doc, "shmupBtnMask2");
	docToValue(addonOptions.shmupBtnMask3, doc, "shmupBtnMask3");
	docToValue(addonOptions.shmupBtnMask4, doc, "shmupBtnMask4");
	docToPin(addonOptions.pinShmupDial, doc, "pinShmupDial");
	docToValue(addonOptions.sliderSOCDModeOne, doc, "sliderSOCDModeOne");
	docToValue(addonOptions.sliderSOCDModeTwo, doc, "sliderSOCDModeTwo");
	docToValue(addonOptions.sliderSOCDModeDefault, doc, "sliderSOCDModeDefault");
	docToValue(addonOptions.AnalogInputEnabled, doc, "AnalogInputEnabled");
	docToValue(addonOptions.BoardLedAddonEnabled, doc, "BoardLedAddonEnabled");
	docToValue(addonOptions.BuzzerSpeakerAddonEnabled, doc, "BuzzerSpeakerAddonEnabled");
	docToValue(addonOptions.BootselButtonAddonEnabled, doc, "BootselButtonAddonEnabled");
	docToValue(addonOptions.DualDirectionalInputEnabled, doc, "DualDirectionalInputEnabled");
	docToValue(addonOptions.ExtraButtonAddonEnabled, doc, "ExtraButtonAddonEnabled");
	docToValue(addonOptions.I2CAnalog1219InputEnabled, doc, "I2CAnalog1219InputEnabled");
	docToValue(addonOptions.JSliderInputEnabled, doc, "JSliderInputEnabled");
	docToValue(addonOptions.SliderSOCDInputEnabled, doc, "SliderSOCDInputEnabled");
	docToValue(addonOptions.PlayerNumAddonEnabled, doc, "PlayerNumAddonEnabled");
	docToValue(addonOptions.PS4ModeAddonEnabled, doc, "PS4ModeAddonEnabled");
	docToValue(addonOptions.ReverseInputEnabled, doc, "ReverseInputEnabled");
	docToValue(addonOptions.TurboInputEnabled, doc, "TurboInputEnabled");

	Storage::getInstance().setAddonOptions(addonOptions);

	return serialize_json(doc);
}

std::string setPS4Options()
{
	DynamicJsonDocument doc = get_post_data();
	PS4Options * ps4Options = Storage::getInstance().getPS4Options();
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
	const AddonOptions& addonOptions = Storage::getInstance().getAddonOptions();
	writeDoc(doc, "turboPin", addonOptions.pinButtonTurbo == 0xFF ? -1 : addonOptions.pinButtonTurbo);
	writeDoc(doc, "turboPinLED", addonOptions.pinTurboLED == 0xFF ? -1 : addonOptions.pinTurboLED);
	writeDoc(doc, "sliderLSPin", addonOptions.pinSliderLS == 0xFF ? -1 : addonOptions.pinSliderLS);
	writeDoc(doc, "sliderRSPin", addonOptions.pinSliderRS == 0xFF ? -1 : addonOptions.pinSliderRS);
	writeDoc(doc, "sliderSOCDPinOne", addonOptions.pinSliderSOCDOne == 0xFF ? -1 : addonOptions.pinSliderSOCDOne);
	writeDoc(doc, "sliderSOCDPinTwo", addonOptions.pinSliderSOCDTwo == 0xFF ? -1 : addonOptions.pinSliderSOCDTwo);
	writeDoc(doc, "turboShotCount", addonOptions.turboShotCount);
	writeDoc(doc, "reversePin", addonOptions.pinButtonReverse == 0xFF ? -1 : addonOptions.pinButtonReverse);
	writeDoc(doc, "reversePinLED", addonOptions.pinReverseLED == 0xFF ? -1 : addonOptions.pinReverseLED);
	writeDoc(doc, "reverseActionUp", addonOptions.reverseActionUp == 0xFF ? -1 : addonOptions.reverseActionUp);
	writeDoc(doc, "reverseActionDown", addonOptions.reverseActionDown == 0xFF ? -1 : addonOptions.reverseActionDown);
	writeDoc(doc, "reverseActionLeft", addonOptions.reverseActionLeft == 0xFF ? -1 : addonOptions.reverseActionLeft);
	writeDoc(doc, "reverseActionRight", addonOptions.reverseActionRight == 0xFF ? -1 : addonOptions.reverseActionRight);
	writeDoc(doc, "i2cAnalog1219SDAPin", addonOptions.i2cAnalog1219SDAPin == 0xFF ? -1 : addonOptions.i2cAnalog1219SDAPin);
	writeDoc(doc, "i2cAnalog1219SCLPin", addonOptions.i2cAnalog1219SCLPin == 0xFF ? -1 : addonOptions.i2cAnalog1219SCLPin);
	writeDoc(doc, "i2cAnalog1219Block", addonOptions.i2cAnalog1219Block);
	writeDoc(doc, "i2cAnalog1219Speed", addonOptions.i2cAnalog1219Speed);
	writeDoc(doc, "i2cAnalog1219Address", addonOptions.i2cAnalog1219Address);
	writeDoc(doc, "onBoardLedMode", addonOptions.onBoardLedMode);
	writeDoc(doc, "dualDirDownPin", addonOptions.pinDualDirDown == 0xFF ? -1 : addonOptions.pinDualDirDown);
	writeDoc(doc, "dualDirUpPin", addonOptions.pinDualDirUp == 0xFF ? -1 : addonOptions.pinDualDirUp);
	writeDoc(doc, "dualDirLeftPin", addonOptions.pinDualDirLeft == 0xFF ? -1 : addonOptions.pinDualDirLeft);
	writeDoc(doc, "dualDirRightPin", addonOptions.pinDualDirRight == 0xFF ? -1 : addonOptions.pinDualDirRight);
	writeDoc(doc, "dualDirDpadMode", addonOptions.dualDirDpadMode);
	writeDoc(doc, "dualDirCombineMode", addonOptions.dualDirCombineMode);
	writeDoc(doc, "analogAdcPinX", addonOptions.analogAdcPinX == 0xFF ? -1 : addonOptions.analogAdcPinX);
	writeDoc(doc, "analogAdcPinY", addonOptions.analogAdcPinY == 0xFF ? -1 : addonOptions.analogAdcPinY);
	writeDoc(doc, "bootselButtonMap", addonOptions.bootselButtonMap);
	writeDoc(doc, "buzzerPin", addonOptions.buzzerPin == 0xFF ? -1 : addonOptions.buzzerPin);
	writeDoc(doc, "buzzerVolume", addonOptions.buzzerVolume);
	writeDoc(doc, "extraButtonPin", addonOptions.extraButtonPin == 0xFF ? -1 : addonOptions.extraButtonPin);
	writeDoc(doc, "extraButtonMap", addonOptions.extraButtonMap);
	writeDoc(doc, "playerNumber", addonOptions.playerNumber);
	writeDoc(doc, "shmupMode", addonOptions.shmupMode);
	writeDoc(doc, "shmupMixMode", addonOptions.shmupMixMode);
	writeDoc(doc, "shmupAlwaysOn1", addonOptions.shmupAlwaysOn1);
	writeDoc(doc, "shmupAlwaysOn2", addonOptions.shmupAlwaysOn2);
	writeDoc(doc, "shmupAlwaysOn3", addonOptions.shmupAlwaysOn3);
	writeDoc(doc, "shmupAlwaysOn4", addonOptions.shmupAlwaysOn4);
	writeDoc(doc, "pinShmupBtn1", addonOptions.pinShmupBtn1 == 0xFF ? -1 : addonOptions.pinShmupBtn1);
	writeDoc(doc, "pinShmupBtn2", addonOptions.pinShmupBtn2 == 0xFF ? -1 : addonOptions.pinShmupBtn2);
	writeDoc(doc, "pinShmupBtn3", addonOptions.pinShmupBtn3 == 0xFF ? -1 : addonOptions.pinShmupBtn3);
	writeDoc(doc, "pinShmupBtn4", addonOptions.pinShmupBtn4 == 0xFF ? -1 : addonOptions.pinShmupBtn4);
	writeDoc(doc, "shmupBtnMask1", addonOptions.shmupBtnMask1);
	writeDoc(doc, "shmupBtnMask2", addonOptions.shmupBtnMask2);
	writeDoc(doc, "shmupBtnMask3", addonOptions.shmupBtnMask3);
	writeDoc(doc, "shmupBtnMask4", addonOptions.shmupBtnMask4);
	writeDoc(doc, "pinShmupDial", addonOptions.pinShmupDial == 0xFF ? -1 : addonOptions.pinShmupDial);
	writeDoc(doc, "sliderSOCDModeOne", addonOptions.sliderSOCDModeOne);
	writeDoc(doc, "sliderSOCDModeTwo", addonOptions.sliderSOCDModeTwo);
	writeDoc(doc, "sliderSOCDModeDefault", addonOptions.sliderSOCDModeDefault);
	writeDoc(doc, "AnalogInputEnabled", addonOptions.AnalogInputEnabled);
	writeDoc(doc, "BoardLedAddonEnabled", addonOptions.BoardLedAddonEnabled);
	writeDoc(doc, "BuzzerSpeakerAddonEnabled", addonOptions.BuzzerSpeakerAddonEnabled);
	writeDoc(doc, "BootselButtonAddonEnabled", addonOptions.BootselButtonAddonEnabled);
	writeDoc(doc, "DualDirectionalInputEnabled", addonOptions.DualDirectionalInputEnabled);
	writeDoc(doc, "ExtraButtonAddonEnabled", addonOptions.ExtraButtonAddonEnabled);
	writeDoc(doc, "I2CAnalog1219InputEnabled", addonOptions.I2CAnalog1219InputEnabled);
	writeDoc(doc, "JSliderInputEnabled", addonOptions.JSliderInputEnabled);
	writeDoc(doc, "SliderSOCDInputEnabled", addonOptions.SliderSOCDInputEnabled);
	writeDoc(doc, "PlayerNumAddonEnabled", addonOptions.PlayerNumAddonEnabled);
	writeDoc(doc, "PS4ModeAddonEnabled", addonOptions.PS4ModeAddonEnabled);
	writeDoc(doc, "ReverseInputEnabled", addonOptions.ReverseInputEnabled);
	writeDoc(doc, "TurboInputEnabled", addonOptions.TurboInputEnabled);

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
#if !defined(NDEBUG)
	{ "/api/echo", echo },
#endif
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
