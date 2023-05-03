#include "configs/webconfig.h"
#include "configs/base64.h"

#include "storagemanager.h"
#include "configmanager.h"
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
#include "httpd.h"
#include "lwip/def.h"
#include "lwip/mem.h"

#include "bitmaps.h"

#define PATH_CGI_ACTION "/cgi/action"

#define API_RESET_SETTINGS "/api/resetSettings"
#define API_GET_DISPLAY_OPTIONS "/api/getDisplayOptions"
#define API_SET_DISPLAY_OPTIONS "/api/setDisplayOptions"
#define API_SET_PREVIEW_DISPLAY_OPTIONS "/api/setPreviewDisplayOptions"
#define API_GET_GAMEPAD_OPTIONS "/api/getGamepadOptions"
#define API_SET_GAMEPAD_OPTIONS "/api/setGamepadOptions"
#define API_GET_LED_OPTIONS "/api/getLedOptions"
#define API_SET_LED_OPTIONS "/api/setLedOptions"
#define API_GET_PIN_MAPPINGS "/api/getPinMappings"
#define API_SET_PIN_MAPPINGS "/api/setPinMappings"
#define API_GET_KEY_MAPPINGS "/api/getKeyMappings"
#define API_SET_KEY_MAPPINGS "/api/setKeyMappings"
#define API_GET_ADDON_OPTIONS "/api/getAddonsOptions"
#define API_SET_ADDON_OPTIONS "/api/setAddonsOptions"
#define API_SET_PS4_OPTIONS "/api/setPS4Options"
#define API_GET_SPLASH_IMAGE "/api/getSplashImage"
#define API_SET_SPLASH_IMAGE "/api/setSplashImage"
#define API_GET_FIRMWARE_VERSION "/api/getFirmwareVersion"
#define API_GET_MEMORY_REPORT "/api/getMemoryReport"
#if !defined(NDEBUG)
#define API_POST_ECHO "/api/echo"
#endif
#define API_REBOOT "/api/reboot"

#define LWIP_HTTPD_POST_MAX_PAYLOAD_LEN 4096

using namespace std;

extern struct fsdata_file file__index_html[];

const static vector<string> spaPaths = { "/display-config", "/led-config", "/pin-mapping", "/keyboard-mapping", "/settings", "/reset-settings", "/add-ons" };
const static vector<string> excludePaths = { "/css", "/images", "/js", "/static" };
const static uint32_t rebootDelayMs = 500;
static string http_post_uri;
static char http_post_payload[LWIP_HTTPD_POST_MAX_PAYLOAD_LEN];
static uint16_t http_post_payload_len = 0;
static absolute_time_t rebootDelayTimeout = nil_time;
static System::BootMode rebootMode = System::BootMode::DEFAULT;

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
	vector<char> raw;
	for (int i = 0; i < http_post_payload_len; i++)
		raw.push_back(http_post_payload[i]);

	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	deserializeJson(doc, raw);
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

std::string serialize_json(DynamicJsonDocument &doc)
{
	string data;
	serializeJson(doc, data);
	return data;
}

std::string setDisplayOptions(BoardOptions& boardOptions)
{
	DynamicJsonDocument doc = get_post_data();
	boardOptions.hasI2CDisplay         = doc["enabled"];
	boardOptions.i2cSDAPin             = doc["sdaPin"] == -1 ? 0xFF : doc["sdaPin"];
	boardOptions.i2cSCLPin             = doc["sclPin"] == -1 ? 0xFF : doc["sclPin"];
	boardOptions.displayI2CAddress     = doc["i2cAddress"];
	boardOptions.i2cBlock              = doc["i2cBlock"];
	boardOptions.i2cSpeed              = doc["i2cSpeed"];
	boardOptions.displayFlip           = doc["flipDisplay"];
	boardOptions.displayInvert         = doc["invertDisplay"];
	boardOptions.buttonLayout      	   = doc["buttonLayout"];
	boardOptions.buttonLayoutRight     = doc["buttonLayoutRight"];
	boardOptions.splashMode      	   = doc["splashMode"];
	boardOptions.splashChoice          = doc["splashChoice"];
	boardOptions.splashDuration        = doc["splashDuration"];
	boardOptions.displaySaverTimeout   = doc["displaySaverTimeout"];

	boardOptions.buttonLayoutCustomOptions.params.layout 		 	       = doc["buttonLayoutCustomOptions"]["params"]["layout"];
	boardOptions.buttonLayoutCustomOptions.params.startX 		 	       = doc["buttonLayoutCustomOptions"]["params"]["startX"];
	boardOptions.buttonLayoutCustomOptions.params.startY 		 	       = doc["buttonLayoutCustomOptions"]["params"]["startY"];
	boardOptions.buttonLayoutCustomOptions.params.buttonRadius      	   = doc["buttonLayoutCustomOptions"]["params"]["buttonRadius"];
	boardOptions.buttonLayoutCustomOptions.params.buttonPadding     	   = doc["buttonLayoutCustomOptions"]["params"]["buttonPadding"];

	boardOptions.buttonLayoutCustomOptions.paramsRight.layoutRight 	   	   = doc["buttonLayoutCustomOptions"]["paramsRight"]["layout"];
	boardOptions.buttonLayoutCustomOptions.paramsRight.startX 		 	   = doc["buttonLayoutCustomOptions"]["paramsRight"]["startX"];
	boardOptions.buttonLayoutCustomOptions.paramsRight.startY 		 	   = doc["buttonLayoutCustomOptions"]["paramsRight"]["startY"];
	boardOptions.buttonLayoutCustomOptions.paramsRight.buttonRadius  	   = doc["buttonLayoutCustomOptions"]["paramsRight"]["buttonRadius"];
	boardOptions.buttonLayoutCustomOptions.paramsRight.buttonPadding       = doc["buttonLayoutCustomOptions"]["paramsRight"]["buttonPadding"];

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
	doc["enabled"]       	 = boardOptions.hasI2CDisplay ? 1 : 0;
	doc["sdaPin"]        	 = boardOptions.i2cSDAPin == 0xFF ? -1 : boardOptions.i2cSDAPin;
	doc["sclPin"]        	 = boardOptions.i2cSCLPin == 0xFF ? -1 : boardOptions.i2cSCLPin;
	doc["i2cAddress"]    	 = boardOptions.displayI2CAddress;
	doc["i2cBlock"]      	 = boardOptions.i2cBlock;
	doc["i2cSpeed"]      	 = boardOptions.i2cSpeed;
	doc["flipDisplay"]   	 = boardOptions.displayFlip ? 1 : 0;
	doc["invertDisplay"] 	 = boardOptions.displayInvert ? 1 : 0;
	doc["buttonLayout"]  	 = boardOptions.buttonLayout;
	doc["buttonLayoutRight"] = boardOptions.buttonLayoutRight;
	doc["splashMode"]  	     = boardOptions.splashMode;
	doc["splashChoice"]      = boardOptions.splashChoice;
	doc["splashDuration"]    = boardOptions.splashDuration;
	doc["displaySaverTimeout"] = boardOptions.displaySaverTimeout;

	doc["buttonLayoutCustomOptions"]["params"]["layout"] 		 	 = boardOptions.buttonLayoutCustomOptions.params.layout;
	doc["buttonLayoutCustomOptions"]["params"]["startX"] 		 	 = boardOptions.buttonLayoutCustomOptions.params.startX;
	doc["buttonLayoutCustomOptions"]["params"]["startY"] 		 	 = boardOptions.buttonLayoutCustomOptions.params.startY;
	doc["buttonLayoutCustomOptions"]["params"]["buttonRadius"]  	 = boardOptions.buttonLayoutCustomOptions.params.buttonRadius;
	doc["buttonLayoutCustomOptions"]["params"]["buttonPadding"] 	 = boardOptions.buttonLayoutCustomOptions.params.buttonPadding;

	doc["buttonLayoutCustomOptions"]["paramsRight"]["layout"] 		 = boardOptions.buttonLayoutCustomOptions.paramsRight.layoutRight;
	doc["buttonLayoutCustomOptions"]["paramsRight"]["startX"] 		 = boardOptions.buttonLayoutCustomOptions.paramsRight.startX;
	doc["buttonLayoutCustomOptions"]["paramsRight"]["startY"] 		 = boardOptions.buttonLayoutCustomOptions.paramsRight.startY;
	doc["buttonLayoutCustomOptions"]["paramsRight"]["buttonRadius"]  = boardOptions.buttonLayoutCustomOptions.paramsRight.buttonRadius;
	doc["buttonLayoutCustomOptions"]["paramsRight"]["buttonPadding"] = boardOptions.buttonLayoutCustomOptions.paramsRight.buttonPadding;

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

std::string setSplashImage() // Expects 16 chunked requests because
{							 // it can't handle all the payload at once
	DynamicJsonDocument doc = get_post_data();
	std::string decoded;
	std::string base64String = doc["splashImage"];
	Base64::Decode(base64String, decoded);
	memcpy(splashImageTemp.data, decoded.data(), decoded.length());
	ConfigManager::getInstance().setSplashImage(splashImageTemp);

	return serialize_json(doc);
}

std::string setGamepadOptions()
{
	DynamicJsonDocument doc = get_post_data();
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	gamepad->options.dpadMode  = doc["dpadMode"];
	gamepad->options.inputMode = doc["inputMode"];
	gamepad->options.socdMode  = doc["socdMode"];

	gamepad->options.hotkeyF1Up.action    = doc["hotkeyF1"][0]["action"];
	gamepad->options.hotkeyF1Down.action  = doc["hotkeyF1"][1]["action"];
	gamepad->options.hotkeyF1Left.action  = doc["hotkeyF1"][2]["action"];
	gamepad->options.hotkeyF1Right.action = doc["hotkeyF1"][3]["action"];
	gamepad->options.hotkeyF2Up.action    = doc["hotkeyF2"][0]["action"];
	gamepad->options.hotkeyF2Down.action  = doc["hotkeyF2"][1]["action"];
	gamepad->options.hotkeyF2Left.action  = doc["hotkeyF2"][2]["action"];
	gamepad->options.hotkeyF2Right.action = doc["hotkeyF2"][3]["action"];
	ConfigManager::getInstance().setGamepadOptions(gamepad);
	return serialize_json(doc);
}

std::string getGamepadOptions()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	GamepadOptions options = GamepadStore.getGamepadOptions();
	doc["dpadMode"]  = options.dpadMode;
	doc["inputMode"] = options.inputMode;
	doc["socdMode"]  = options.socdMode;

	auto hotkeyF1 = doc.createNestedArray("hotkeyF1");
	auto f1Up = hotkeyF1.createNestedObject();
	f1Up["action"] = options.hotkeyF1Up.action;
	f1Up["mask"]   = options.hotkeyF1Up.dpadMask;
	auto f1Down = hotkeyF1.createNestedObject();
	f1Down["action"] = options.hotkeyF1Down.action;
	f1Down["mask"]   = options.hotkeyF1Down.dpadMask;
	auto f1Left = hotkeyF1.createNestedObject();
	f1Left["action"] = options.hotkeyF1Left.action;
	f1Left["mask"]   = options.hotkeyF1Left.dpadMask;
	auto f1Right = hotkeyF1.createNestedObject();
	f1Right["action"] = options.hotkeyF1Right.action;
	f1Right["mask"]   = options.hotkeyF1Right.dpadMask;

	auto hotkeyF2 = doc.createNestedArray("hotkeyF2");
	auto f2Up = hotkeyF2.createNestedObject();
	f2Up["action"] = options.hotkeyF2Up.action;
	f2Up["mask"]   = options.hotkeyF2Up.dpadMask;
	auto f2Down = hotkeyF2.createNestedObject();
	f2Down["action"] = options.hotkeyF2Down.action;
	f2Down["mask"]   = options.hotkeyF2Down.dpadMask;
	auto f2Left = hotkeyF2.createNestedObject();
	f2Left["action"] = options.hotkeyF2Left.action;
	f2Left["mask"]   = options.hotkeyF2Left.dpadMask;
	auto f2Right = hotkeyF2.createNestedObject();
	f2Right["action"] = options.hotkeyF2Right.action;
	f2Right["mask"]   = options.hotkeyF2Right.dpadMask;
	return serialize_json(doc);
}

std::string setLedOptions()
{
	DynamicJsonDocument doc = get_post_data();
	LEDOptions ledOptions = Storage::getInstance().getLEDOptions();
	ledOptions.useUserDefinedLEDs = true;
	ledOptions.dataPin            = doc["dataPin"];
	ledOptions.ledFormat          = doc["ledFormat"];
	ledOptions.ledLayout          = doc["ledLayout"];
	ledOptions.ledsPerButton      = doc["ledsPerButton"];
	ledOptions.brightnessMaximum  = doc["brightnessMaximum"];
	ledOptions.brightnessSteps    = doc["brightnessSteps"];
	ledOptions.indexUp            = (doc["ledButtonMap"]["Up"]    == nullptr) ? -1 : doc["ledButtonMap"]["Up"];
	ledOptions.indexDown          = (doc["ledButtonMap"]["Down"]  == nullptr) ? -1 : doc["ledButtonMap"]["Down"];
	ledOptions.indexLeft          = (doc["ledButtonMap"]["Left"]  == nullptr) ? -1 : doc["ledButtonMap"]["Left"];
	ledOptions.indexRight         = (doc["ledButtonMap"]["Right"] == nullptr) ? -1 : doc["ledButtonMap"]["Right"];
	ledOptions.indexB1            = (doc["ledButtonMap"]["B1"]    == nullptr) ? -1 : doc["ledButtonMap"]["B1"];
	ledOptions.indexB2            = (doc["ledButtonMap"]["B2"]    == nullptr) ? -1 : doc["ledButtonMap"]["B2"];
	ledOptions.indexB3            = (doc["ledButtonMap"]["B3"]    == nullptr) ? -1 : doc["ledButtonMap"]["B3"];
	ledOptions.indexB4            = (doc["ledButtonMap"]["B4"]    == nullptr) ? -1 : doc["ledButtonMap"]["B4"];
	ledOptions.indexL1            = (doc["ledButtonMap"]["L1"]    == nullptr) ? -1 : doc["ledButtonMap"]["L1"];
	ledOptions.indexR1            = (doc["ledButtonMap"]["R1"]    == nullptr) ? -1 : doc["ledButtonMap"]["R1"];
	ledOptions.indexL2            = (doc["ledButtonMap"]["L2"]    == nullptr) ? -1 : doc["ledButtonMap"]["L2"];
	ledOptions.indexR2            = (doc["ledButtonMap"]["R2"]    == nullptr) ? -1 : doc["ledButtonMap"]["R2"];
	ledOptions.indexS1            = (doc["ledButtonMap"]["S1"]    == nullptr) ? -1 : doc["ledButtonMap"]["S1"];
	ledOptions.indexS2            = (doc["ledButtonMap"]["S2"]    == nullptr) ? -1 : doc["ledButtonMap"]["S2"];
	ledOptions.indexL3            = (doc["ledButtonMap"]["L3"]    == nullptr) ? -1 : doc["ledButtonMap"]["L3"];
	ledOptions.indexR3            = (doc["ledButtonMap"]["R3"]    == nullptr) ? -1 : doc["ledButtonMap"]["R3"];
	ledOptions.indexA1            = (doc["ledButtonMap"]["A1"]    == nullptr) ? -1 : doc["ledButtonMap"]["A1"];
	ledOptions.indexA2            = (doc["ledButtonMap"]["A2"]    == nullptr) ? -1 : doc["ledButtonMap"]["A2"];
	ConfigManager::getInstance().setLedOptions(ledOptions);
	return serialize_json(doc);
}

std::string getLedOptions()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	const LEDOptions& ledOptions = Storage::getInstance().getLEDOptions();
	doc["dataPin"]           = ledOptions.dataPin;
	doc["ledFormat"]         = ledOptions.ledFormat;
	doc["ledLayout"]         = ledOptions.ledLayout;
	doc["ledsPerButton"]     = ledOptions.ledsPerButton;
	doc["brightnessMaximum"] = ledOptions.brightnessMaximum;
	doc["brightnessSteps"]   = ledOptions.brightnessSteps;

	auto ledButtonMap = doc.createNestedObject("ledButtonMap");
	if (ledOptions.indexUp == -1)    ledButtonMap["Up"]    = nullptr;  else ledButtonMap["Up"]    = ledOptions.indexUp;
	if (ledOptions.indexDown == -1)  ledButtonMap["Down"]  = nullptr;  else ledButtonMap["Down"]  = ledOptions.indexDown;
	if (ledOptions.indexLeft == -1)  ledButtonMap["Left"]  = nullptr;  else ledButtonMap["Left"]  = ledOptions.indexLeft;
	if (ledOptions.indexRight == -1) ledButtonMap["Right"] = nullptr;  else ledButtonMap["Right"] = ledOptions.indexRight;
	if (ledOptions.indexB1 == -1)    ledButtonMap["B1"]    = nullptr;  else ledButtonMap["B1"]    = ledOptions.indexB1;
	if (ledOptions.indexB2 == -1)    ledButtonMap["B2"]    = nullptr;  else ledButtonMap["B2"]    = ledOptions.indexB2;
	if (ledOptions.indexB3 == -1)    ledButtonMap["B3"]    = nullptr;  else ledButtonMap["B3"]    = ledOptions.indexB3;
	if (ledOptions.indexB4 == -1)    ledButtonMap["B4"]    = nullptr;  else ledButtonMap["B4"]    = ledOptions.indexB4;
	if (ledOptions.indexL1 == -1)    ledButtonMap["L1"]    = nullptr;  else ledButtonMap["L1"]    = ledOptions.indexL1;
	if (ledOptions.indexR1 == -1)    ledButtonMap["R1"]    = nullptr;  else ledButtonMap["R1"]    = ledOptions.indexR1;
	if (ledOptions.indexL2 == -1)    ledButtonMap["L2"]    = nullptr;  else ledButtonMap["L2"]    = ledOptions.indexL2;
	if (ledOptions.indexR2 == -1)    ledButtonMap["R2"]    = nullptr;  else ledButtonMap["R2"]    = ledOptions.indexR2;
	if (ledOptions.indexS1 == -1)    ledButtonMap["S1"]    = nullptr;  else ledButtonMap["S1"]    = ledOptions.indexS1;
	if (ledOptions.indexS2 == -1)    ledButtonMap["S2"]    = nullptr;  else ledButtonMap["S2"]    = ledOptions.indexS2;
	if (ledOptions.indexL3 == -1)    ledButtonMap["L3"]    = nullptr;  else ledButtonMap["L3"]    = ledOptions.indexL3;
	if (ledOptions.indexR3 == -1)    ledButtonMap["R3"]    = nullptr;  else ledButtonMap["R3"]    = ledOptions.indexR3;
	if (ledOptions.indexA1 == -1)    ledButtonMap["A1"]    = nullptr;  else ledButtonMap["A1"]    = ledOptions.indexA1;
	if (ledOptions.indexA2 == -1)    ledButtonMap["A2"]    = nullptr;  else ledButtonMap["A2"]    = ledOptions.indexA2;

	addUsedPinsArray(doc);

	return serialize_json(doc);
}

std::string setPinMappings()
{
	DynamicJsonDocument doc = get_post_data();

	// BoardOptions uses 0xff to denote unassigned pins
	const auto convertPin = [] (int pin) -> uint8_t { return pin >= 0 && pin < NUM_BANK0_GPIOS ? pin : 0xff; };

	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
	boardOptions.hasBoardOptions = true;
	boardOptions.pinDpadUp    = convertPin(doc["Up"]);
	boardOptions.pinDpadDown  = convertPin(doc["Down"]);
	boardOptions.pinDpadLeft  = convertPin(doc["Left"]);
	boardOptions.pinDpadRight = convertPin(doc["Right"]);
	boardOptions.pinButtonB1  = convertPin(doc["B1"]);
	boardOptions.pinButtonB2  = convertPin(doc["B2"]);
	boardOptions.pinButtonB3  = convertPin(doc["B3"]);
	boardOptions.pinButtonB4  = convertPin(doc["B4"]);
	boardOptions.pinButtonL1  = convertPin(doc["L1"]);
	boardOptions.pinButtonR1  = convertPin(doc["R1"]);
	boardOptions.pinButtonL2  = convertPin(doc["L2"]);
	boardOptions.pinButtonR2  = convertPin(doc["R2"]);
	boardOptions.pinButtonS1  = convertPin(doc["S1"]);
	boardOptions.pinButtonS2  = convertPin(doc["S2"]);
	boardOptions.pinButtonL3  = convertPin(doc["L3"]);
	boardOptions.pinButtonR3  = convertPin(doc["R3"]);
	boardOptions.pinButtonA1  = convertPin(doc["A1"]);
	boardOptions.pinButtonA2  = convertPin(doc["A2"]);

	Storage::getInstance().setBoardOptions(boardOptions);

	return serialize_json(doc);
}

std::string getPinMappings()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);

	// Webconfig uses -1 to denote unassigned pins
	const auto convertPin = [] (uint8_t pin) -> int { return pin < NUM_BANK0_GPIOS ? pin : -1; };

	const BoardOptions& boardOptions = Storage::getInstance().getBoardOptions();
	doc["Up"]    = convertPin(boardOptions.pinDpadUp);
	doc["Down"]  = convertPin(boardOptions.pinDpadDown);
	doc["Left"]  = convertPin(boardOptions.pinDpadLeft);
	doc["Right"] = convertPin(boardOptions.pinDpadRight);
	doc["B1"]    = convertPin(boardOptions.pinButtonB1);
	doc["B2"]    = convertPin(boardOptions.pinButtonB2);
	doc["B3"]    = convertPin(boardOptions.pinButtonB3);
	doc["B4"]    = convertPin(boardOptions.pinButtonB4);
	doc["L1"]    = convertPin(boardOptions.pinButtonL1);
	doc["R1"]    = convertPin(boardOptions.pinButtonR1);
	doc["L2"]    = convertPin(boardOptions.pinButtonL2);
	doc["R2"]    = convertPin(boardOptions.pinButtonR2);
	doc["S1"]    = convertPin(boardOptions.pinButtonS1);
	doc["S2"]    = convertPin(boardOptions.pinButtonS2);
	doc["L3"]    = convertPin(boardOptions.pinButtonL3);
	doc["R3"]    = convertPin(boardOptions.pinButtonR3);
	doc["A1"]    = convertPin(boardOptions.pinButtonA1);
	doc["A2"]    = convertPin(boardOptions.pinButtonA2);

	return serialize_json(doc);
}

std::string setKeyMappings()
{
	DynamicJsonDocument doc = get_post_data();

	Gamepad* gamepad = Storage::getInstance().GetGamepad();
	gamepad->options.keyDpadUp    = doc["Up"];
	gamepad->options.keyDpadDown  = doc["Down"];
	gamepad->options.keyDpadLeft  = doc["Left"];
	gamepad->options.keyDpadRight = doc["Right"];
	gamepad->options.keyButtonB1  = doc["B1"];
	gamepad->options.keyButtonB2  = doc["B2"];
	gamepad->options.keyButtonB3  = doc["B3"];
	gamepad->options.keyButtonB4  = doc["B4"];
	gamepad->options.keyButtonL1  = doc["L1"];
	gamepad->options.keyButtonR1  = doc["R1"];
	gamepad->options.keyButtonL2  = doc["L2"];
	gamepad->options.keyButtonR2  = doc["R2"];
	gamepad->options.keyButtonS1  = doc["S1"];
	gamepad->options.keyButtonS2  = doc["S2"];
	gamepad->options.keyButtonL3  = doc["L3"];
	gamepad->options.keyButtonR3  = doc["R3"];
	gamepad->options.keyButtonA1  = doc["A1"];
	gamepad->options.keyButtonA2  = doc["A2"];

	gamepad->save();

	return serialize_json(doc);
}

std::string getKeyMappings()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);

	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	doc["Up"]    = gamepad->options.keyDpadUp;
	doc["Down"]  = gamepad->options.keyDpadDown;
	doc["Left"]  = gamepad->options.keyDpadLeft;
	doc["Right"] = gamepad->options.keyDpadRight;
	doc["B1"]    = gamepad->options.keyButtonB1;
	doc["B2"]    = gamepad->options.keyButtonB2;
	doc["B3"]    = gamepad->options.keyButtonB3;
	doc["B4"]    = gamepad->options.keyButtonB4;
	doc["L1"]    = gamepad->options.keyButtonL1;
	doc["R1"]    = gamepad->options.keyButtonR1;
	doc["L2"]    = gamepad->options.keyButtonL2;
	doc["R2"]    = gamepad->options.keyButtonR2;
	doc["S1"]    = gamepad->options.keyButtonS1;
	doc["S2"]    = gamepad->options.keyButtonS2;
	doc["L3"]    = gamepad->options.keyButtonL3;
	doc["R3"]    = gamepad->options.keyButtonR3;
	doc["A1"]    = gamepad->options.keyButtonA1;
	doc["A2"]    = gamepad->options.keyButtonA2;

	return serialize_json(doc);
}

std::string setAddonOptions()
{
	DynamicJsonDocument doc = get_post_data();

	AddonOptions addonOptions = Storage::getInstance().getAddonOptions();
	addonOptions.pinButtonTurbo    	= doc["turboPin"] == -1 ? 0xFF : doc["turboPin"];
	addonOptions.pinTurboLED        = doc["turboPinLED"] == -1 ? 0xFF : doc["turboPinLED"];
	addonOptions.pinSliderLS  		= doc["sliderLSPin"] == -1 ? 0xFF : doc["sliderLSPin"];
	addonOptions.pinSliderRS  		= doc["sliderRSPin"] == -1 ? 0xFF : doc["sliderRSPin"];
	addonOptions.pinSliderSOCDOne  		= doc["sliderSOCDPinOne"] == -1 ? 0xFF : doc["sliderSOCDPinOne"];
	addonOptions.pinSliderSOCDTwo    = doc["sliderSOCDPinTwo"] == -1 ? 0xFF : doc["sliderSOCDPinTwo"];
	addonOptions.turboShotCount 	= doc["turboShotCount"];
	addonOptions.pinButtonReverse  	= doc["reversePin"] == -1 ? 0xFF : doc["reversePin"];
	addonOptions.pinReverseLED  	= doc["reversePinLED"] == -1 ? 0xFF : doc["reversePinLED"];
	addonOptions.reverseActionUp  	= doc["reverseActionUp"] == -1 ? 0xFF : doc["reverseActionUp"];
	addonOptions.reverseActionDown  = doc["reverseActionDown"] == -1 ? 0xFF : doc["reverseActionDown"];
	addonOptions.reverseActionLeft  = doc["reverseActionLeft"] == -1 ? 0xFF : doc["reverseActionLeft"];
	addonOptions.reverseActionRight = doc["reverseActionRight"] == -1 ? 0xFF : doc["reverseActionRight"];
	addonOptions.i2cAnalog1219SDAPin = doc["i2cAnalog1219SDAPin"] == -1 ? 0xFF : doc["i2cAnalog1219SDAPin"];
	addonOptions.i2cAnalog1219SCLPin = doc["i2cAnalog1219SCLPin"] == -1 ? 0xFF : doc["i2cAnalog1219SCLPin"];
	addonOptions.i2cAnalog1219Block = doc["i2cAnalog1219Block"];
	addonOptions.i2cAnalog1219Speed = doc["i2cAnalog1219Speed"];
	addonOptions.i2cAnalog1219Address = doc["i2cAnalog1219Address"];
	addonOptions.onBoardLedMode = doc["onBoardLedMode"];
	addonOptions.pinDualDirDown 	= doc["dualDirDownPin"] == -1 ? 0xFF : doc["dualDirDownPin"];
	addonOptions.pinDualDirUp 		= doc["dualDirUpPin"] == -1 ? 0xFF : doc["dualDirUpPin"];
	addonOptions.pinDualDirLeft 	= doc["dualDirLeftPin"] == -1 ? 0xFF : doc["dualDirLeftPin"];
	addonOptions.pinDualDirRight 	= doc["dualDirRightPin"] == -1 ? 0xFF : doc["dualDirRightPin"];
	addonOptions.dualDirDpadMode    = doc["dualDirDpadMode"];
	addonOptions.dualDirCombineMode = doc["dualDirCombineMode"];
	addonOptions.analogAdcPinX = doc["analogAdcPinX"] == -1 ? 0xFF : doc["analogAdcPinX"];
	addonOptions.analogAdcPinY = doc["analogAdcPinY"] == -1 ? 0xFF : doc["analogAdcPinY"];
	addonOptions.bootselButtonMap = doc["bootselButtonMap"];
	addonOptions.buzzerPin        = doc["buzzerPin"] == -1 ? 0xFF : doc["buzzerPin"];
	addonOptions.buzzerVolume     = doc["buzzerVolume"];
	addonOptions.extraButtonPin        = doc["extraButtonPin"] == -1 ? 0xFF : doc["extraButtonPin"];
	addonOptions.extraButtonMap = doc["extraButtonMap"];
	addonOptions.playerNumber     = doc["playerNumber"];
	addonOptions.shmupMode     = doc["shmupMode"];
	addonOptions.shmupMixMode     = doc["shmupMixMode"];
	addonOptions.shmupAlwaysOn1     = doc["shmupAlwaysOn1"];
	addonOptions.shmupAlwaysOn2     = doc["shmupAlwaysOn2"];
	addonOptions.shmupAlwaysOn3     = doc["shmupAlwaysOn3"];
	addonOptions.shmupAlwaysOn4     = doc["shmupAlwaysOn4"];
	addonOptions.pinShmupBtn1     = doc["pinShmupBtn1"] == -1 ? 0xFF : doc["pinShmupBtn1"];
	addonOptions.pinShmupBtn2     = doc["pinShmupBtn2"] == -1 ? 0xFF : doc["pinShmupBtn2"];
	addonOptions.pinShmupBtn3     = doc["pinShmupBtn3"] == -1 ? 0xFF : doc["pinShmupBtn3"];
	addonOptions.pinShmupBtn4     = doc["pinShmupBtn4"] == -1 ? 0xFF : doc["pinShmupBtn4"];
	addonOptions.shmupBtnMask1     = doc["shmupBtnMask1"];
	addonOptions.shmupBtnMask2     = doc["shmupBtnMask2"];
	addonOptions.shmupBtnMask3     = doc["shmupBtnMask3"];
	addonOptions.shmupBtnMask4     = doc["shmupBtnMask4"];
	addonOptions.pinShmupDial     = doc["pinShmupDial"] == -1 ? 0xFF : doc["pinShmupDial"];
	addonOptions.sliderSOCDModeOne     = doc["sliderSOCDModeOne"];
	addonOptions.sliderSOCDModeTwo     = doc["sliderSOCDModeTwo"];
	addonOptions.sliderSOCDModeDefault     = doc["sliderSOCDModeDefault"];
	addonOptions.AnalogInputEnabled = doc["AnalogInputEnabled"];
	addonOptions.BoardLedAddonEnabled = doc["BoardLedAddonEnabled"];
	addonOptions.BuzzerSpeakerAddonEnabled = doc["BuzzerSpeakerAddonEnabled"];
	addonOptions.BootselButtonAddonEnabled = doc["BootselButtonAddonEnabled"];
	addonOptions.DualDirectionalInputEnabled = doc["DualDirectionalInputEnabled"];
	addonOptions.ExtraButtonAddonEnabled = doc["ExtraButtonAddonEnabled"];
	addonOptions.I2CAnalog1219InputEnabled = doc["I2CAnalog1219InputEnabled"];
	addonOptions.JSliderInputEnabled = doc["JSliderInputEnabled"];
	addonOptions.SliderSOCDInputEnabled = doc["SliderSOCDInputEnabled"];
	addonOptions.PlayerNumAddonEnabled = doc["PlayerNumAddonEnabled"];
	addonOptions.PS4ModeAddonEnabled = doc["PS4ModeAddonEnabled"];
	addonOptions.ReverseInputEnabled = doc["ReverseInputEnabled"];
	addonOptions.TurboInputEnabled = doc["TurboInputEnabled"];

	Storage::getInstance().setAddonOptions(addonOptions);

	return serialize_json(doc);
}

std::string setPS4Options()
{
	DynamicJsonDocument doc = get_post_data();
	PS4Options * ps4Options = Storage::getInstance().getPS4Options();
	std::string decoded;
	std::size_t len;

	// RSA Context
	if ( doc.containsKey("N") ) {
		std::string decoded;
		Base64::Decode(doc["N"], decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_n ) ) {
			memcpy(ps4Options->rsa_n, decoded.data(), decoded.length());
		}
	}
	if ( doc.containsKey("E") ) {
		std::string decoded;
		Base64::Decode(doc["E"], decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_e ) ) {
			memcpy(ps4Options->rsa_e, decoded.data(), decoded.length());
		}
	}
	if ( doc.containsKey("D") ) {
		std::string decoded;
		Base64::Decode(doc["D"], decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_d ) ) {
			memcpy(ps4Options->rsa_d, decoded.data(), decoded.length());
		}
	}
	if ( doc.containsKey("P") ) {
		std::string decoded;
		Base64::Decode(doc["P"], decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_p ) ) {
			memcpy(ps4Options->rsa_p, decoded.data(), decoded.length());
		}			
	}
	if ( doc.containsKey("Q") ) {
		std::string decoded;
		Base64::Decode(doc["Q"], decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_q ) ) {
			memcpy(ps4Options->rsa_q, decoded.data(), decoded.length());
		}			
	}
	if ( doc.containsKey("DP") ) {
		std::string decoded;
		Base64::Decode(doc["DP"], decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_dp ) ) {
			memcpy(ps4Options->rsa_dp, decoded.data(), decoded.length());
		}
	}
	if ( doc.containsKey("DQ") ) {
		std::string decoded;
		Base64::Decode(doc["DQ"], decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_dq ) ) {
			memcpy(ps4Options->rsa_dq, decoded.data(), decoded.length());
		}
	}
	if ( doc.containsKey("QP") ) {	
		std::string decoded;
		Base64::Decode(doc["QP"], decoded);	
		if ( decoded.length() == sizeof(ps4Options->rsa_qp ) ) {
			memcpy(ps4Options->rsa_qp, decoded.data(), decoded.length());
		}
	}
	if ( doc.containsKey("RN") ) {
		std::string decoded;
		Base64::Decode(doc["RN"], decoded);
		if ( decoded.length() == sizeof(ps4Options->rsa_rn ) ) {
			memcpy(ps4Options->rsa_rn, decoded.data(), decoded.length());
		}
	}
	// Serial & Signature
	if ( doc.containsKey("serial") ) {
		std::string decoded;
		Base64::Decode(doc["serial"], decoded);
		const char * testDecode = decoded.c_str();
		if ( decoded.length() == sizeof(ps4Options->serial ) ) {
			memcpy(ps4Options->serial, decoded.data(), decoded.length());
		}
	}
	if ( doc.containsKey("signature") ) {
		std::string decoded;
		Base64::Decode(doc["signature"], decoded);
		if ( decoded.length() == sizeof(ps4Options->signature ) ) {
			memcpy(ps4Options->signature, decoded.data(), decoded.length());
		}
	}

	Storage::getInstance().savePS4Options();

	DynamicJsonDocument rdoc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	rdoc["success"] = true;
	return serialize_json(rdoc);
}

std::string getAddonOptions()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	const AddonOptions& addonOptions = Storage::getInstance().getAddonOptions();
	doc["turboPin"] = addonOptions.pinButtonTurbo == 0xFF ? -1 : addonOptions.pinButtonTurbo;
	doc["turboPinLED"] = addonOptions.pinTurboLED == 0xFF ? -1 : addonOptions.pinTurboLED;
	doc["sliderLSPin"] = addonOptions.pinSliderLS == 0xFF ? -1 : addonOptions.pinSliderLS;
	doc["sliderRSPin"] = addonOptions.pinSliderRS == 0xFF ? -1 : addonOptions.pinSliderRS;
	doc["sliderSOCDPinOne"] = addonOptions.pinSliderSOCDOne == 0xFF ? -1 : addonOptions.pinSliderSOCDOne;
	doc["sliderSOCDPinTwo"] = addonOptions.pinSliderSOCDTwo == 0xFF ? -1 : addonOptions.pinSliderSOCDTwo;
	doc["turboShotCount"] = addonOptions.turboShotCount;
	doc["reversePin"] = addonOptions.pinButtonReverse == 0xFF ? -1 : addonOptions.pinButtonReverse;
	doc["reversePinLED"] = addonOptions.pinReverseLED == 0xFF ? -1 : addonOptions.pinReverseLED;
	doc["reverseActionUp"] = addonOptions.reverseActionUp == 0xFF ? -1 : addonOptions.reverseActionUp;
	doc["reverseActionDown"] = addonOptions.reverseActionDown == 0xFF ? -1 : addonOptions.reverseActionDown;
	doc["reverseActionLeft"] = addonOptions.reverseActionLeft == 0xFF ? -1 : addonOptions.reverseActionLeft;
	doc["reverseActionRight"] = addonOptions.reverseActionRight == 0xFF ? -1 : addonOptions.reverseActionRight;
	doc["i2cAnalog1219SDAPin"] = addonOptions.i2cAnalog1219SDAPin == 0xFF ? -1 : addonOptions.i2cAnalog1219SDAPin;
	doc["i2cAnalog1219SCLPin"] = addonOptions.i2cAnalog1219SCLPin == 0xFF ? -1 : addonOptions.i2cAnalog1219SCLPin;
	doc["i2cAnalog1219Block"] = addonOptions.i2cAnalog1219Block;
	doc["i2cAnalog1219Speed"] = addonOptions.i2cAnalog1219Speed;
	doc["i2cAnalog1219Address"] = addonOptions.i2cAnalog1219Address;
	doc["onBoardLedMode"] = addonOptions.onBoardLedMode;
	doc["dualDirDownPin"] = addonOptions.pinDualDirDown == 0xFF ? -1 : addonOptions.pinDualDirDown;
	doc["dualDirUpPin"] = addonOptions.pinDualDirUp == 0xFF ? -1 : addonOptions.pinDualDirUp;
	doc["dualDirLeftPin"] = addonOptions.pinDualDirLeft == 0xFF ? -1 : addonOptions.pinDualDirLeft;
	doc["dualDirRightPin"] = addonOptions.pinDualDirRight == 0xFF ? -1 : addonOptions.pinDualDirRight;
	doc["dualDirDpadMode"] = addonOptions.dualDirDpadMode;
	doc["dualDirCombineMode"] = addonOptions.dualDirCombineMode;
	doc["analogAdcPinX"] = addonOptions.analogAdcPinX == 0xFF ? -1 : addonOptions.analogAdcPinX;
	doc["analogAdcPinY"] = addonOptions.analogAdcPinY == 0xFF ? -1 : addonOptions.analogAdcPinY;
	doc["bootselButtonMap"] = addonOptions.bootselButtonMap;
	doc["buzzerPin"] = addonOptions.buzzerPin == 0xFF ? -1 : addonOptions.buzzerPin;
	doc["buzzerVolume"] = addonOptions.buzzerVolume;
	doc["extraButtonPin"] = addonOptions.extraButtonPin == 0xFF ? -1 : addonOptions.extraButtonPin;
	doc["extraButtonMap"] = addonOptions.extraButtonMap;
	doc["playerNumber"] = addonOptions.playerNumber;
	doc["shmupMode"] = addonOptions.shmupMode;
	doc["shmupMixMode"] = addonOptions.shmupMixMode;
	doc["shmupAlwaysOn1"] = addonOptions.shmupAlwaysOn1;
	doc["shmupAlwaysOn2"] = addonOptions.shmupAlwaysOn2;
	doc["shmupAlwaysOn3"] = addonOptions.shmupAlwaysOn3;
	doc["shmupAlwaysOn4"] = addonOptions.shmupAlwaysOn4;
	doc["pinShmupBtn1"] = addonOptions.pinShmupBtn1 == 0xFF ? -1 : addonOptions.pinShmupBtn1;
	doc["pinShmupBtn2"] = addonOptions.pinShmupBtn2 == 0xFF ? -1 : addonOptions.pinShmupBtn2;
	doc["pinShmupBtn3"] = addonOptions.pinShmupBtn3 == 0xFF ? -1 : addonOptions.pinShmupBtn3;
	doc["pinShmupBtn4"] = addonOptions.pinShmupBtn4 == 0xFF ? -1 : addonOptions.pinShmupBtn4;
	doc["shmupBtnMask1"] = addonOptions.shmupBtnMask1;
	doc["shmupBtnMask2"] = addonOptions.shmupBtnMask2;
	doc["shmupBtnMask3"] = addonOptions.shmupBtnMask3;
	doc["shmupBtnMask4"] = addonOptions.shmupBtnMask4;
	doc["pinShmupDial"] = addonOptions.pinShmupDial == 0xFF ? -1 : addonOptions.pinShmupDial;
	doc["sliderSOCDModeOne"] = addonOptions.sliderSOCDModeOne;
	doc["sliderSOCDModeTwo"] = addonOptions.sliderSOCDModeTwo;
	doc["sliderSOCDModeDefault"] = addonOptions.sliderSOCDModeDefault;
	doc["AnalogInputEnabled"] = addonOptions.AnalogInputEnabled;
	doc["BoardLedAddonEnabled"] = addonOptions.BoardLedAddonEnabled;
	doc["BuzzerSpeakerAddonEnabled"] = addonOptions.BuzzerSpeakerAddonEnabled;
	doc["BootselButtonAddonEnabled"] = addonOptions.BootselButtonAddonEnabled;
	doc["DualDirectionalInputEnabled"] = addonOptions.DualDirectionalInputEnabled;
	doc["ExtraButtonAddonEnabled"] = addonOptions.ExtraButtonAddonEnabled;
	doc["I2CAnalog1219InputEnabled"] = addonOptions.I2CAnalog1219InputEnabled;
	doc["JSliderInputEnabled"] = addonOptions.JSliderInputEnabled;
	doc["SliderSOCDInputEnabled"] = addonOptions.SliderSOCDInputEnabled;
	doc["PlayerNumAddonEnabled"] = addonOptions.PlayerNumAddonEnabled;
	doc["PS4ModeAddonEnabled"] = addonOptions.PS4ModeAddonEnabled;
	doc["ReverseInputEnabled"] = addonOptions.ReverseInputEnabled;
	doc["TurboInputEnabled"] = addonOptions.TurboInputEnabled;

	addUsedPinsArray(doc);

	return serialize_json(doc);
}

std::string getFirmwareVersion()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	doc["version"] = GP2040VERSION;

	return serialize_json(doc);
}

std::string getMemoryReport()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	doc["totalFlash"] = System::getTotalFlash();
	doc["usedFlash"] = System::getUsedFlash();
	doc["staticAllocs"] = System::getStaticAllocs();
	doc["totalHeap"] = System::getTotalHeap();
	doc["usedHeap"] = System::getUsedHeap();
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

int fs_open_custom(struct fs_file *file, const char *name)
{
	if (strcmp(name, API_SET_DISPLAY_OPTIONS) == 0)
			return set_file_data(file, setDisplayOptions());
	if (strcmp(name, API_SET_PREVIEW_DISPLAY_OPTIONS) == 0)
			return set_file_data(file, setPreviewDisplayOptions());
	if (strcmp(name, API_SET_GAMEPAD_OPTIONS) == 0)
			return set_file_data(file, setGamepadOptions());
	if (strcmp(name, API_SET_LED_OPTIONS) == 0)
			return set_file_data(file, setLedOptions());
	if (strcmp(name, API_SET_PIN_MAPPINGS) == 0)
			return set_file_data(file, setPinMappings());
	if (strcmp(name, API_SET_KEY_MAPPINGS) == 0)
			return set_file_data(file, setKeyMappings());
	if (strcmp(name, API_SET_ADDON_OPTIONS) == 0)
			return set_file_data(file, setAddonOptions());
	if (strcmp(name, API_SET_PS4_OPTIONS) == 0)
			return set_file_data(file, setPS4Options());
	if (strcmp(name, API_SET_SPLASH_IMAGE) == 0)
			return set_file_data(file, setSplashImage());
	if (strcmp(name, API_REBOOT) == 0)
			return set_file_data(file, reboot());
#if !defined(NDEBUG)
	if (strcmp(name, API_POST_ECHO) == 0)
		return set_file_data(file, echo());
#endif
	if (strcmp(name, API_GET_DISPLAY_OPTIONS) == 0)
			return set_file_data(file, getDisplayOptions());
	if (strcmp(name, API_GET_GAMEPAD_OPTIONS) == 0)
			return set_file_data(file, getGamepadOptions());
	if (strcmp(name, API_GET_LED_OPTIONS) == 0)
			return set_file_data(file, getLedOptions());
	if (strcmp(name, API_GET_PIN_MAPPINGS) == 0)
			return set_file_data(file, getPinMappings());
	if (strcmp(name, API_GET_KEY_MAPPINGS) == 0)
			return set_file_data(file, getKeyMappings());
	if (strcmp(name, API_GET_ADDON_OPTIONS) == 0)
			return set_file_data(file, getAddonOptions());
	if (strcmp(name, API_RESET_SETTINGS) == 0)
			return set_file_data(file, resetSettings());
	if (strcmp(name, API_GET_SPLASH_IMAGE) == 0)
			return set_file_data(file, getSplashImage());
	if (strcmp(name, API_GET_FIRMWARE_VERSION) == 0)
			return set_file_data(file, getFirmwareVersion());
	if (strcmp(name, API_GET_MEMORY_REPORT) == 0)
			return set_file_data(file, getMemoryReport());

	bool isExclude = false;
	for (auto &excludePath : excludePaths)
		if (!excludePath.compare(name))
			return 0;

	for (auto &spaPath : spaPaths)
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
