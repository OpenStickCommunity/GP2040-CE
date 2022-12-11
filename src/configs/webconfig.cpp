#include "configs/webconfig.h"

#include "storagemanager.h"
#include "configmanager.h"

#include <cstring>
#include <string>
#include <vector>

// HTTPD Includes
#include <ArduinoJson.h>
#include "rndis/rndis.h"
#include "httpd/fs.h"
#include "httpd/fscustom.h"
#include "httpd/fsdata.h"
#include "httpd/httpd.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "rndis/rndis.h"

#include "bitmaps.h"

#define PATH_CGI_ACTION "/cgi/action"

#define API_RESET_SETTINGS "/api/resetSettings"
#define API_GET_DISPLAY_OPTIONS "/api/getDisplayOptions"
#define API_SET_DISPLAY_OPTIONS "/api/setDisplayOptions"
#define API_GET_GAMEPAD_OPTIONS "/api/getGamepadOptions"
#define API_SET_GAMEPAD_OPTIONS "/api/setGamepadOptions"
#define API_GET_LED_OPTIONS "/api/getLedOptions"
#define API_SET_LED_OPTIONS "/api/setLedOptions"
#define API_GET_PIN_MAPPINGS "/api/getPinMappings"
#define API_SET_PIN_MAPPINGS "/api/setPinMappings"
#define API_GET_ADDON_OPTIONS "/api/getAddonsOptions"
#define API_SET_ADDON_OPTIONS "/api/setAddonsOptions"
#define API_GET_SPLASH_IMAGE "/api/getSplashImage"
#define API_SET_SPLASH_IMAGE "/api/setSplashImage"
#define API_GET_FIRMWARE_VERSION "/api/getFirmwareVersion"

#define LWIP_HTTPD_POST_MAX_URI_LEN 128
#define LWIP_HTTPD_POST_MAX_PAYLOAD_LEN 2048

using namespace std;

extern struct fsdata_file file__index_html[];

const static vector<string> spaPaths = { "/display-config", "/led-config", "/pin-mapping", "/settings", "/reset-settings", "/add-ons" };
const static vector<string> excludePaths = { "/css", "/images", "/js", "/static" };
static char *http_post_uri;
static char http_post_payload[LWIP_HTTPD_POST_MAX_PAYLOAD_LEN];
static uint16_t http_post_payload_len = 0;
static bool is_post = false;

void WebConfig::setup() {
	rndis_init();
}

void WebConfig::loop() {
	// rndis http server requires inline functions (non-class)
	rndis_task();
}

// **** WEB SERVER Overrides and Special Functionality ****
int set_file_data(struct fs_file *file, string data)
{
	static string returnData;

	returnData = data;
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

	if (!uri || (uri[0] == '\0') || memcmp(uri, "/api", 4))
		return ERR_ARG;

	http_post_uri = (char *)uri;
	is_post = true;
	return ERR_OK;
}

// LWIP callback on HTTP POST to for receiving payload
err_t httpd_post_receive_data(void *connection, struct pbuf *p)
{
	LWIP_UNUSED_ARG(connection);

	int count;
	uint32_t http_post_payload_full_flag = 0;

	// Cache the received data to http_post_payload
	http_post_payload_len = 0;
	memset(http_post_payload, 0, LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	while (p != NULL)
	{
		if (http_post_payload_len + p->len <= LWIP_HTTPD_POST_MAX_PAYLOAD_LEN)
		{
			MEMCPY(http_post_payload + http_post_payload_len, p->payload, p->len);
			http_post_payload_len += p->len;
		}
		else // Buffer overflow Set overflow flag
		{
			http_post_payload_full_flag = 1;
			break;
		}

		p = p->next;
	}

	// Need to release memory here or will leak
	pbuf_free(p);

	// If the buffer overflows, error out
	if (http_post_payload_full_flag)
		return ERR_BUF;

	return ERR_OK;
}

// LWIP callback to set the HTTP POST response_uri, which can then be looked up via the fs_custom callbacks
void httpd_post_finished(void *connection, char *response_uri, uint16_t response_uri_len)
{
	LWIP_UNUSED_ARG(connection);
	LWIP_UNUSED_ARG(response_uri);
	LWIP_UNUSED_ARG(response_uri_len);

	response_uri = http_post_uri;
}

std::string serialize_json(DynamicJsonDocument &doc)
{
	string data;
	serializeJson(doc, data);
	return data;
}

std::string setDisplayOptions()
{
	DynamicJsonDocument doc = get_post_data();
	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
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
	ConfigManager::getInstance().setBoardOptions(boardOptions);
	return serialize_json(doc);
}

std::string getDisplayOptions() // Manually set Document Attributes for the display
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
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

	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	auto usedPins = doc.createNestedArray("usedPins");
	usedPins.add(gamepad->mapDpadUp->pin);
	usedPins.add(gamepad->mapDpadDown->pin);
	usedPins.add(gamepad->mapDpadLeft->pin);
	usedPins.add(gamepad->mapDpadRight->pin);
	usedPins.add(gamepad->mapButtonB1->pin);
	usedPins.add(gamepad->mapButtonB2->pin);
	usedPins.add(gamepad->mapButtonB3->pin);
	usedPins.add(gamepad->mapButtonB4->pin);
	usedPins.add(gamepad->mapButtonL1->pin);
	usedPins.add(gamepad->mapButtonR1->pin);
	usedPins.add(gamepad->mapButtonL2->pin);
	usedPins.add(gamepad->mapButtonR2->pin);
	usedPins.add(gamepad->mapButtonS1->pin);
	usedPins.add(gamepad->mapButtonS2->pin);
	usedPins.add(gamepad->mapButtonL3->pin);
	usedPins.add(gamepad->mapButtonR3->pin);
	usedPins.add(gamepad->mapButtonA1->pin);
	usedPins.add(gamepad->mapButtonA2->pin);

	return serialize_json(doc);
}

SplashImage splashImageTemp; // For splash image upload

std::string getSplashImage()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN * 10); // TODO: Figoure out correct length
	SplashImage splashImage = Storage::getInstance().getSplashImage();
	JsonArray splashImageArray = doc.createNestedArray("splashImage");
	copyArray(splashImage.data, splashImageArray);

	return serialize_json(doc);
}

std::string setSplashImage() // Expects 16 chunked requests because
{							 // it can't handle all the payload at once
	DynamicJsonDocument doc = get_post_data();
	int index = doc["index"];
	
	// Clean temp array, just in case
	if (index == 0) {
		for (int i = 0; i < 1024; i++) {
			splashImageTemp.data[i] = 0;
		}
	}

	JsonArray array = doc["splashImage"].as<JsonArray>();
	for (int i = 0; i < 64; i++) {
		splashImageTemp.data[(64 * index) + i] = array[i];
	}
	
	// Persist data, all data bits should be set
	if (index == 15) {
		ConfigManager::getInstance().setSplashImage(splashImageTemp);
	}

	return serialize_json(doc);
}

std::string setGamepadOptions()
{
	DynamicJsonDocument doc = get_post_data();
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	gamepad->options.dpadMode  = doc["dpadMode"];
	gamepad->options.inputMode = doc["inputMode"];
	gamepad->options.socdMode  = doc["socdMode"];
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
	LEDOptions ledOptions = Storage::getInstance().getLEDOptions();
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

	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	auto usedPins = doc.createNestedArray("usedPins");
	usedPins.add(gamepad->mapDpadUp->pin);
	usedPins.add(gamepad->mapDpadDown->pin);
	usedPins.add(gamepad->mapDpadLeft->pin);
	usedPins.add(gamepad->mapDpadRight->pin);
	usedPins.add(gamepad->mapButtonB1->pin);
	usedPins.add(gamepad->mapButtonB2->pin);
	usedPins.add(gamepad->mapButtonB3->pin);
	usedPins.add(gamepad->mapButtonB4->pin);
	usedPins.add(gamepad->mapButtonL1->pin);
	usedPins.add(gamepad->mapButtonR1->pin);
	usedPins.add(gamepad->mapButtonL2->pin);
	usedPins.add(gamepad->mapButtonR2->pin);
	usedPins.add(gamepad->mapButtonS1->pin);
	usedPins.add(gamepad->mapButtonS2->pin);
	usedPins.add(gamepad->mapButtonL3->pin);
	usedPins.add(gamepad->mapButtonR3->pin);
	usedPins.add(gamepad->mapButtonA1->pin);
	usedPins.add(gamepad->mapButtonA2->pin);

	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
	if (boardOptions.i2cSDAPin != -1)
		usedPins.add(boardOptions.i2cSDAPin);
	if (boardOptions.i2cSCLPin != -1)
		usedPins.add(boardOptions.i2cSCLPin);

	return serialize_json(doc);
}

std::string setPinMappings()
{
	DynamicJsonDocument doc = get_post_data();

	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
	boardOptions.hasBoardOptions = true;
	boardOptions.pinDpadUp    = doc["Up"];
	boardOptions.pinDpadDown  = doc["Down"];
	boardOptions.pinDpadLeft  = doc["Left"];
	boardOptions.pinDpadRight = doc["Right"];
	boardOptions.pinButtonB1  = doc["B1"];
	boardOptions.pinButtonB2  = doc["B2"];
	boardOptions.pinButtonB3  = doc["B3"];
	boardOptions.pinButtonB4  = doc["B4"];
	boardOptions.pinButtonL1  = doc["L1"];
	boardOptions.pinButtonR1  = doc["R1"];
	boardOptions.pinButtonL2  = doc["L2"];
	boardOptions.pinButtonR2  = doc["R2"];
	boardOptions.pinButtonS1  = doc["S1"];
	boardOptions.pinButtonS2  = doc["S2"];
	boardOptions.pinButtonL3  = doc["L3"];
	boardOptions.pinButtonR3  = doc["R3"];
	boardOptions.pinButtonA1  = doc["A1"];
	boardOptions.pinButtonA2  = doc["A2"];

	Storage::getInstance().setBoardOptions(boardOptions);

	return serialize_json(doc);
}

std::string getPinMappings()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	doc["Up"]    = gamepad->mapDpadUp->pin;
	doc["Down"]  = gamepad->mapDpadDown->pin;
	doc["Left"]  = gamepad->mapDpadLeft->pin;
	doc["Right"] = gamepad->mapDpadRight->pin;
	doc["B1"]    = gamepad->mapButtonB1->pin;
	doc["B2"]    = gamepad->mapButtonB2->pin;
	doc["B3"]    = gamepad->mapButtonB3->pin;
	doc["B4"]    = gamepad->mapButtonB4->pin;
	doc["L1"]    = gamepad->mapButtonL1->pin;
	doc["R1"]    = gamepad->mapButtonR1->pin;
	doc["L2"]    = gamepad->mapButtonL2->pin;
	doc["R2"]    = gamepad->mapButtonR2->pin;
	doc["S1"]    = gamepad->mapButtonS1->pin;
	doc["S2"]    = gamepad->mapButtonS2->pin;
	doc["L3"]    = gamepad->mapButtonL3->pin;
	doc["R3"]    = gamepad->mapButtonR3->pin;
	doc["A1"]    = gamepad->mapButtonA1->pin;
	doc["A2"]    = gamepad->mapButtonA2->pin;

	return serialize_json(doc);
}

std::string setAddonOptions()
{
	DynamicJsonDocument doc = get_post_data();

	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
	boardOptions.hasBoardOptions    = true;
	boardOptions.pinButtonTurbo    	= doc["turboPin"] == -1 ? 0xFF : doc["turboPin"];
	boardOptions.pinTurboLED        = doc["turboPinLED"] == -1 ? 0xFF : doc["turboPinLED"];
	boardOptions.pinSliderLS  		= doc["sliderLSPin"] == -1 ? 0xFF : doc["sliderLSPin"];
	boardOptions.pinSliderRS  		= doc["sliderRSPin"] == -1 ? 0xFF : doc["sliderRSPin"];
	boardOptions.turboShotCount 	= doc["turboShotCount"];
	boardOptions.pinButtonReverse  	= doc["reversePin"] == -1 ? 0xFF : doc["reversePin"];
	boardOptions.pinReverseLED  	= doc["reversePinLED"] == -1 ? 0xFF : doc["reversePinLED"];
	boardOptions.reverseActionUp  	= doc["reverseActionUp"] == -1 ? 0xFF : doc["reverseActionUp"];
	boardOptions.reverseActionDown  = doc["reverseActionDown"] == -1 ? 0xFF : doc["reverseActionDown"];
	boardOptions.reverseActionLeft  = doc["reverseActionLeft"] == -1 ? 0xFF : doc["reverseActionLeft"];
	boardOptions.reverseActionRight = doc["reverseActionRight"] == -1 ? 0xFF : doc["reverseActionRight"];
	boardOptions.i2cAnalog1219SDAPin = doc["i2cAnalog1219SDAPin"] == -1 ? 0xFF : doc["i2cAnalog1219SDAPin"];
	boardOptions.i2cAnalog1219SCLPin = doc["i2cAnalog1219SCLPin"] == -1 ? 0xFF : doc["i2cAnalog1219SCLPin"];
	boardOptions.i2cAnalog1219Block = doc["i2cAnalog1219Block"];
	boardOptions.i2cAnalog1219Speed = doc["i2cAnalog1219Speed"];
	boardOptions.i2cAnalog1219Address = doc["i2cAnalog1219Address"];
	boardOptions.pinDualDirDown 	= doc["dualDirDownPin"] == -1 ? 0xFF : doc["dualDirDownPin"];
	boardOptions.pinDualDirUp 		= doc["dualDirUpPin"] == -1 ? 0xFF : doc["dualDirUpPin"];
	boardOptions.pinDualDirLeft 	= doc["dualDirLeftPin"] == -1 ? 0xFF : doc["dualDirLeftPin"];
	boardOptions.pinDualDirRight 	= doc["dualDirRightPin"] == -1 ? 0xFF : doc["dualDirRightPin"];
	boardOptions.dualDirDpadMode    = doc["dualDirDpadMode"];
	boardOptions.dualDirCombineMode = doc["dualDirCombineMode"];

	Storage::getInstance().setBoardOptions(boardOptions);

	return serialize_json(doc);
}

std::string getAddonOptions()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
	doc["turboPin"] = boardOptions.pinButtonTurbo == 0xFF ? -1 : boardOptions.pinButtonTurbo;
	doc["turboPinLED"] = boardOptions.pinTurboLED == 0xFF ? -1 : boardOptions.pinTurboLED;
	doc["sliderLSPin"] = boardOptions.pinSliderLS == 0xFF ? -1 : boardOptions.pinSliderLS;
	doc["sliderRSPin"] = boardOptions.pinSliderRS == 0xFF ? -1 : boardOptions.pinSliderRS;
	doc["turboShotCount"] = boardOptions.turboShotCount;
	doc["reversePin"] = boardOptions.pinButtonReverse == 0xFF ? -1 : boardOptions.pinButtonReverse;
	doc["reversePinLED"] = boardOptions.pinReverseLED == 0xFF ? -1 : boardOptions.pinReverseLED;
	doc["reverseActionUp"] = boardOptions.reverseActionUp == 0xFF ? -1 : boardOptions.reverseActionUp;
	doc["reverseActionDown"] = boardOptions.reverseActionDown == 0xFF ? -1 : boardOptions.reverseActionDown;
	doc["reverseActionLeft"] = boardOptions.reverseActionLeft == 0xFF ? -1 : boardOptions.reverseActionLeft;
	doc["reverseActionRight"] = boardOptions.reverseActionRight == 0xFF ? -1 : boardOptions.reverseActionRight;
	doc["i2cAnalog1219SDAPin"] = boardOptions.i2cAnalog1219SDAPin == 0xFF ? -1 : boardOptions.i2cAnalog1219SDAPin;
	doc["i2cAnalog1219SCLPin"] = boardOptions.i2cAnalog1219SCLPin == 0xFF ? -1 : boardOptions.i2cAnalog1219SCLPin;
	doc["i2cAnalog1219Block"] = boardOptions.i2cAnalog1219Block;
	doc["i2cAnalog1219Speed"] = boardOptions.i2cAnalog1219Speed;
	doc["i2cAnalog1219Address"] = boardOptions.i2cAnalog1219Address;
	doc["dualDirDownPin"] = boardOptions.pinDualDirDown == 0xFF ? -1 : boardOptions.pinDualDirDown;
	doc["dualDirUpPin"] = boardOptions.pinDualDirUp == 0xFF ? -1 : boardOptions.pinDualDirUp;
	doc["dualDirLeftPin"] = boardOptions.pinDualDirLeft == 0xFF ? -1 : boardOptions.pinDualDirLeft;
	doc["dualDirRightPin"] = boardOptions.pinDualDirRight == 0xFF ? -1 : boardOptions.pinDualDirRight;
	doc["dualDirDpadMode"] = boardOptions.dualDirDpadMode;
	doc["dualDirCombineMode"] = boardOptions.dualDirCombineMode;

	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	auto usedPins = doc.createNestedArray("usedPins");
	usedPins.add(gamepad->mapDpadUp->pin);
	usedPins.add(gamepad->mapDpadDown->pin);
	usedPins.add(gamepad->mapDpadLeft->pin);
	usedPins.add(gamepad->mapDpadRight->pin);
	usedPins.add(gamepad->mapButtonB1->pin);
	usedPins.add(gamepad->mapButtonB2->pin);
	usedPins.add(gamepad->mapButtonB3->pin);
	usedPins.add(gamepad->mapButtonB4->pin);
	usedPins.add(gamepad->mapButtonL1->pin);
	usedPins.add(gamepad->mapButtonR1->pin);
	usedPins.add(gamepad->mapButtonL2->pin);
	usedPins.add(gamepad->mapButtonR2->pin);
	usedPins.add(gamepad->mapButtonS1->pin);
	usedPins.add(gamepad->mapButtonS2->pin);
	usedPins.add(gamepad->mapButtonL3->pin);
	usedPins.add(gamepad->mapButtonR3->pin);
	usedPins.add(gamepad->mapButtonA1->pin);
	usedPins.add(gamepad->mapButtonA2->pin);

	return serialize_json(doc);
}

std::string getFirmwareVersion()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	doc["version"] = GP2040VERSION;

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

int fs_open_custom(struct fs_file *file, const char *name)
{
	if (is_post)
	{
		if (!memcmp(http_post_uri, API_SET_DISPLAY_OPTIONS, sizeof(API_SET_DISPLAY_OPTIONS)))
			return set_file_data(file, setDisplayOptions());
		if (!memcmp(http_post_uri, API_SET_GAMEPAD_OPTIONS, sizeof(API_SET_GAMEPAD_OPTIONS)))
			return set_file_data(file, setGamepadOptions());
		if (!memcmp(http_post_uri, API_SET_LED_OPTIONS, sizeof(API_SET_LED_OPTIONS)))
			return set_file_data(file, setLedOptions());
		if (!memcmp(http_post_uri, API_SET_PIN_MAPPINGS, sizeof(API_SET_PIN_MAPPINGS)))
			return set_file_data(file, setPinMappings());
		if (!memcmp(http_post_uri, API_SET_ADDON_OPTIONS, sizeof(API_SET_ADDON_OPTIONS)))
			return set_file_data(file, setAddonOptions());
		if (!memcmp(http_post_uri, API_SET_SPLASH_IMAGE, sizeof(API_SET_SPLASH_IMAGE)))
			return set_file_data(file, setSplashImage());
	}
	else
	{
		if (!memcmp(name, API_GET_DISPLAY_OPTIONS, sizeof(API_GET_DISPLAY_OPTIONS)))
			return set_file_data(file, getDisplayOptions());
		if (!memcmp(name, API_GET_GAMEPAD_OPTIONS, sizeof(API_GET_GAMEPAD_OPTIONS)))
			return set_file_data(file, getGamepadOptions());
		if (!memcmp(name, API_GET_LED_OPTIONS, sizeof(API_GET_LED_OPTIONS)))
			return set_file_data(file, getLedOptions());
		if (!memcmp(name, API_GET_PIN_MAPPINGS, sizeof(API_GET_PIN_MAPPINGS)))
			return set_file_data(file, getPinMappings());
		if (!memcmp(name, API_GET_ADDON_OPTIONS, sizeof(API_GET_ADDON_OPTIONS)))
			return set_file_data(file, getAddonOptions());
		if (!memcmp(name, API_RESET_SETTINGS, sizeof(API_RESET_SETTINGS)))
			return set_file_data(file, resetSettings());
		if (!memcmp(name, API_GET_SPLASH_IMAGE, sizeof(API_GET_SPLASH_IMAGE)))
			return set_file_data(file, getSplashImage());
		if (!memcmp(name, API_GET_FIRMWARE_VERSION, sizeof(API_GET_FIRMWARE_VERSION)))
			return set_file_data(file, getFirmwareVersion());
	}

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

	is_post = false;
}
