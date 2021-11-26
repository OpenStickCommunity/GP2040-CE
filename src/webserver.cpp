/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include <stdio.h>
#include <vector>
#include <map>
#include <string>
#include <string.h>

#include <ArduinoJson.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "FlashPROM.h"
#include "httpd/fs.h"
#include "httpd/fscustom.h"
#include "httpd/fsdata.h"
#include "httpd/httpd.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "rndis/rndis.h"

#include "gamepad.h"
#include "storage.h"
#include "leds.h"
#include "GamepadStorage.h"

#define PATH_CGI_ACTION "/cgi/action"

#define API_RESET_SETTINGS "/api/resetSettings"
#define API_GET_GAMEPAD_OPTIONS "/api/getGamepadOptions"
#define API_SET_GAMEPAD_OPTIONS "/api/setGamepadOptions"
#define API_GET_LED_OPTIONS "/api/getLedOptions"
#define API_SET_LED_OPTIONS "/api/setLedOptions"
#define API_GET_PIN_MAPPINGS "/api/getPinMappings"
#define API_SET_PIN_MAPPINGS "/api/setPinMappings"

#define LWIP_HTTPD_POST_MAX_URI_LEN 128
#define LWIP_HTTPD_POST_MAX_PAYLOAD_LEN 2048

using namespace std;

extern struct fsdata_file file__index_html[];
extern Gamepad gamepad;

const static vector<string> spaPaths = { "/led-config", "/pin-mapping", "/settings", "/reset-settings" };
const static vector<string> excludePaths = { "/css", "/images", "/js", "/static" };
static char *http_post_uri;
static char http_post_payload[LWIP_HTTPD_POST_MAX_PAYLOAD_LEN];
static uint16_t http_post_payload_len = 0;
static bool is_post = false;

/*************************
 * Helper methods
 *************************/

DynamicJsonDocument get_post_data()
{
	vector<char> raw;
	for (int i = 0; i < http_post_payload_len; i++)
		raw.push_back(http_post_payload[i]);

	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	deserializeJson(doc, raw);
	return doc;
}

inline string serialize_json(DynamicJsonDocument &doc)
{
	string data;
	serializeJson(doc, data);
	return data;
}

int set_file_data(struct fs_file *file, string data)
{
	file->data = data.c_str();
	file->len = data.size();
	file->index = file->len;
	file->is_custom_file = 1;
	file->http_header_included = 0;
	file->pextension = NULL;

	return 1;
}

/*************************
 * API methods
 *************************/

string resetSettings()
{
	EEPROM.reset();
	watchdog_reboot(0, SRAM_END, 2000);
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);
	doc["success"] = true;
	return serialize_json(doc);
}

string getGamepadOptions()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);

	GamepadOptions options = GamepadStore.getGamepadOptions();
	doc["dpadMode"]  = options.dpadMode;
	doc["inputMode"] = options.inputMode;
	doc["socdMode"]  = options.socdMode;

	return serialize_json(doc);
}

string setGamepadOptions()
{
	DynamicJsonDocument doc = get_post_data();

	gamepad.options.dpadMode  = doc["dpadMode"];
	gamepad.options.inputMode = doc["inputMode"];
	gamepad.options.socdMode  = doc["socdMode"];
	gamepad.save();

	return serialize_json(doc);
}

string getLedOptions()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);

	doc["dataPin"]           = ledModule.ledOptions.dataPin;
	doc["ledFormat"]         = ledModule.ledOptions.ledFormat;
	doc["ledLayout"]         = ledModule.ledOptions.ledLayout;
	doc["ledsPerButton"]     = ledModule.ledOptions.ledsPerButton;
	doc["brightnessMaximum"] = ledModule.ledOptions.brightnessMaximum;
	doc["brightnessSteps"]   = ledModule.ledOptions.brightnessSteps;

	auto ledButtonMap = doc.createNestedObject("ledButtonMap");

	if (ledModule.ledOptions.indexUp == -1)    ledButtonMap["Up"]    = nullptr;  else ledButtonMap["Up"]    = ledModule.ledOptions.indexUp;
	if (ledModule.ledOptions.indexDown == -1)  ledButtonMap["Down"]  = nullptr;  else ledButtonMap["Down"]  = ledModule.ledOptions.indexDown;
	if (ledModule.ledOptions.indexLeft == -1)  ledButtonMap["Left"]  = nullptr;  else ledButtonMap["Left"]  = ledModule.ledOptions.indexLeft;
	if (ledModule.ledOptions.indexRight == -1) ledButtonMap["Right"] = nullptr;  else ledButtonMap["Right"] = ledModule.ledOptions.indexRight;
	if (ledModule.ledOptions.indexB1 == -1)    ledButtonMap["B1"]    = nullptr;  else ledButtonMap["B1"]    = ledModule.ledOptions.indexB1;
	if (ledModule.ledOptions.indexB2 == -1)    ledButtonMap["B2"]    = nullptr;  else ledButtonMap["B2"]    = ledModule.ledOptions.indexB2;
	if (ledModule.ledOptions.indexB3 == -1)    ledButtonMap["B3"]    = nullptr;  else ledButtonMap["B3"]    = ledModule.ledOptions.indexB3;
	if (ledModule.ledOptions.indexB4 == -1)    ledButtonMap["B4"]    = nullptr;  else ledButtonMap["B4"]    = ledModule.ledOptions.indexB4;
	if (ledModule.ledOptions.indexL1 == -1)    ledButtonMap["L1"]    = nullptr;  else ledButtonMap["L1"]    = ledModule.ledOptions.indexL1;
	if (ledModule.ledOptions.indexR1 == -1)    ledButtonMap["R1"]    = nullptr;  else ledButtonMap["R1"]    = ledModule.ledOptions.indexR1;
	if (ledModule.ledOptions.indexL2 == -1)    ledButtonMap["L2"]    = nullptr;  else ledButtonMap["L2"]    = ledModule.ledOptions.indexL2;
	if (ledModule.ledOptions.indexR2 == -1)    ledButtonMap["R2"]    = nullptr;  else ledButtonMap["R2"]    = ledModule.ledOptions.indexR2;
	if (ledModule.ledOptions.indexS1 == -1)    ledButtonMap["S1"]    = nullptr;  else ledButtonMap["S1"]    = ledModule.ledOptions.indexS1;
	if (ledModule.ledOptions.indexS2 == -1)    ledButtonMap["S2"]    = nullptr;  else ledButtonMap["S2"]    = ledModule.ledOptions.indexS2;
	if (ledModule.ledOptions.indexL3 == -1)    ledButtonMap["L3"]    = nullptr;  else ledButtonMap["L3"]    = ledModule.ledOptions.indexL3;
	if (ledModule.ledOptions.indexR3 == -1)    ledButtonMap["R3"]    = nullptr;  else ledButtonMap["R3"]    = ledModule.ledOptions.indexR3;
	if (ledModule.ledOptions.indexA1 == -1)    ledButtonMap["A1"]    = nullptr;  else ledButtonMap["A1"]    = ledModule.ledOptions.indexA1;
	if (ledModule.ledOptions.indexA2 == -1)    ledButtonMap["A2"]    = nullptr;  else ledButtonMap["A2"]    = ledModule.ledOptions.indexA2;

	auto usedPins = doc.createNestedArray("usedPins");
	usedPins.add(gamepad.mapDpadUp->pin);
	usedPins.add(gamepad.mapDpadDown->pin);
	usedPins.add(gamepad.mapDpadLeft->pin);
	usedPins.add(gamepad.mapDpadRight->pin);
	usedPins.add(gamepad.mapButtonB1->pin);
	usedPins.add(gamepad.mapButtonB2->pin);
	usedPins.add(gamepad.mapButtonB3->pin);
	usedPins.add(gamepad.mapButtonB4->pin);
	usedPins.add(gamepad.mapButtonL1->pin);
	usedPins.add(gamepad.mapButtonR1->pin);
	usedPins.add(gamepad.mapButtonL2->pin);
	usedPins.add(gamepad.mapButtonR2->pin);
	usedPins.add(gamepad.mapButtonS1->pin);
	usedPins.add(gamepad.mapButtonS2->pin);
	usedPins.add(gamepad.mapButtonL3->pin);
	usedPins.add(gamepad.mapButtonR3->pin);
	usedPins.add(gamepad.mapButtonA1->pin);
	usedPins.add(gamepad.mapButtonA2->pin);

	return serialize_json(doc);
}

string setLedOptions()
{
	DynamicJsonDocument doc = get_post_data();

	ledModule.ledOptions.useUserDefinedLEDs = true;
	ledModule.ledOptions.dataPin            = doc["dataPin"];
	ledModule.ledOptions.ledFormat          = doc["ledFormat"];
	ledModule.ledOptions.ledLayout          = doc["ledLayout"];
	ledModule.ledOptions.ledsPerButton      = doc["ledsPerButton"];
	ledModule.ledOptions.brightnessMaximum  = doc["brightnessMaximum"];
	ledModule.ledOptions.brightnessSteps    = doc["brightnessSteps"];
	ledModule.ledOptions.indexUp            = (doc["ledButtonMap"]["Up"]    == nullptr) ? -1 : doc["ledButtonMap"]["Up"];
	ledModule.ledOptions.indexDown          = (doc["ledButtonMap"]["Down"]  == nullptr) ? -1 : doc["ledButtonMap"]["Down"];
	ledModule.ledOptions.indexLeft          = (doc["ledButtonMap"]["Left"]  == nullptr) ? -1 : doc["ledButtonMap"]["Left"];
	ledModule.ledOptions.indexRight         = (doc["ledButtonMap"]["Right"] == nullptr) ? -1 : doc["ledButtonMap"]["Right"];
	ledModule.ledOptions.indexB1            = (doc["ledButtonMap"]["B1"]    == nullptr) ? -1 : doc["ledButtonMap"]["B1"];
	ledModule.ledOptions.indexB2            = (doc["ledButtonMap"]["B2"]    == nullptr) ? -1 : doc["ledButtonMap"]["B2"];
	ledModule.ledOptions.indexB3            = (doc["ledButtonMap"]["B3"]    == nullptr) ? -1 : doc["ledButtonMap"]["B3"];
	ledModule.ledOptions.indexB4            = (doc["ledButtonMap"]["B4"]    == nullptr) ? -1 : doc["ledButtonMap"]["B4"];
	ledModule.ledOptions.indexL1            = (doc["ledButtonMap"]["L1"]    == nullptr) ? -1 : doc["ledButtonMap"]["L1"];
	ledModule.ledOptions.indexR1            = (doc["ledButtonMap"]["R1"]    == nullptr) ? -1 : doc["ledButtonMap"]["R1"];
	ledModule.ledOptions.indexL2            = (doc["ledButtonMap"]["L2"]    == nullptr) ? -1 : doc["ledButtonMap"]["L2"];
	ledModule.ledOptions.indexR2            = (doc["ledButtonMap"]["R2"]    == nullptr) ? -1 : doc["ledButtonMap"]["R2"];
	ledModule.ledOptions.indexS1            = (doc["ledButtonMap"]["S1"]    == nullptr) ? -1 : doc["ledButtonMap"]["S1"];
	ledModule.ledOptions.indexS2            = (doc["ledButtonMap"]["S2"]    == nullptr) ? -1 : doc["ledButtonMap"]["S2"];
	ledModule.ledOptions.indexL3            = (doc["ledButtonMap"]["L3"]    == nullptr) ? -1 : doc["ledButtonMap"]["L3"];
	ledModule.ledOptions.indexR3            = (doc["ledButtonMap"]["R3"]    == nullptr) ? -1 : doc["ledButtonMap"]["R3"];
	ledModule.ledOptions.indexA1            = (doc["ledButtonMap"]["A1"]    == nullptr) ? -1 : doc["ledButtonMap"]["A1"];
	ledModule.ledOptions.indexA2            = (doc["ledButtonMap"]["A2"]    == nullptr) ? -1 : doc["ledButtonMap"]["A2"];

	setLEDOptions(ledModule.ledOptions);
	GamepadStore.save();
	ledModule.configureLEDs();

	return serialize_json(doc);
}

string getPinMappings()
{
	DynamicJsonDocument doc(LWIP_HTTPD_POST_MAX_PAYLOAD_LEN);

	doc["Up"]    = gamepad.mapDpadUp->pin;
	doc["Down"]  = gamepad.mapDpadDown->pin;
	doc["Left"]  = gamepad.mapDpadLeft->pin;
	doc["Right"] = gamepad.mapDpadRight->pin;
	doc["B1"]    = gamepad.mapButtonB1->pin;
	doc["B2"]    = gamepad.mapButtonB2->pin;
	doc["B3"]    = gamepad.mapButtonB3->pin;
	doc["B4"]    = gamepad.mapButtonB4->pin;
	doc["L1"]    = gamepad.mapButtonL1->pin;
	doc["R1"]    = gamepad.mapButtonR1->pin;
	doc["L2"]    = gamepad.mapButtonL2->pin;
	doc["R2"]    = gamepad.mapButtonR2->pin;
	doc["S1"]    = gamepad.mapButtonS1->pin;
	doc["S2"]    = gamepad.mapButtonS2->pin;
	doc["L3"]    = gamepad.mapButtonL3->pin;
	doc["R3"]    = gamepad.mapButtonR3->pin;
	doc["A1"]    = gamepad.mapButtonA1->pin;
	doc["A2"]    = gamepad.mapButtonA2->pin;

	return serialize_json(doc);
}

string setPinMappings()
{
	DynamicJsonDocument doc = get_post_data();

	BoardOptions options;
	options.useUserDefinedPins = true;
	options.pinDpadUp    = doc["Up"];
	options.pinDpadDown  = doc["Down"];
	options.pinDpadLeft  = doc["Left"];
	options.pinDpadRight = doc["Right"];
	options.pinButtonB1  = doc["B1"];
	options.pinButtonB2  = doc["B2"];
	options.pinButtonB3  = doc["B3"];
	options.pinButtonB4  = doc["B4"];
	options.pinButtonL1  = doc["L1"];
	options.pinButtonR1  = doc["R1"];
	options.pinButtonL2  = doc["L2"];
	options.pinButtonR2  = doc["R2"];
	options.pinButtonS1  = doc["S1"];
	options.pinButtonS2  = doc["S2"];
	options.pinButtonL3  = doc["L3"];
	options.pinButtonR3  = doc["R3"];
	options.pinButtonA1  = doc["A1"];
	options.pinButtonA2  = doc["A2"];

	setBoardOptions(options);
	GamepadStore.save();

	gamepad.mapDpadUp->setPin(options.pinDpadUp);
	gamepad.mapDpadDown->setPin(options.pinDpadDown);
	gamepad.mapDpadLeft->setPin(options.pinDpadLeft);
	gamepad.mapDpadRight->setPin(options.pinDpadRight);
	gamepad.mapButtonB1->setPin(options.pinButtonB1);
	gamepad.mapButtonB2->setPin(options.pinButtonB2);
	gamepad.mapButtonB3->setPin(options.pinButtonB3);
	gamepad.mapButtonB4->setPin(options.pinButtonB4);
	gamepad.mapButtonL1->setPin(options.pinButtonL1);
	gamepad.mapButtonR1->setPin(options.pinButtonR1);
	gamepad.mapButtonL2->setPin(options.pinButtonL2);
	gamepad.mapButtonR2->setPin(options.pinButtonR2);
	gamepad.mapButtonS1->setPin(options.pinButtonS1);
	gamepad.mapButtonS2->setPin(options.pinButtonS2);
	gamepad.mapButtonL3->setPin(options.pinButtonL3);
	gamepad.mapButtonR3->setPin(options.pinButtonR3);
	gamepad.mapButtonA1->setPin(options.pinButtonA1);
	gamepad.mapButtonA2->setPin(options.pinButtonA2);

	return serialize_json(doc);
}

/*************************
 * LWIP implementation
 *************************/

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

int fs_open_custom(struct fs_file *file, const char *name)
{
	if (is_post)
	{
		if (!memcmp(http_post_uri, API_SET_GAMEPAD_OPTIONS, sizeof(API_SET_GAMEPAD_OPTIONS)))
			return set_file_data(file, setGamepadOptions());
		if (!memcmp(http_post_uri, API_SET_LED_OPTIONS, sizeof(API_SET_LED_OPTIONS)))
			return set_file_data(file, setLedOptions());
		if (!memcmp(http_post_uri, API_SET_PIN_MAPPINGS, sizeof(API_SET_PIN_MAPPINGS)))
			return set_file_data(file, setPinMappings());
	}
	else
	{
		if (!memcmp(name, API_GET_GAMEPAD_OPTIONS, sizeof(API_GET_GAMEPAD_OPTIONS)))
			return set_file_data(file, getGamepadOptions());
		if (!memcmp(name, API_GET_LED_OPTIONS, sizeof(API_GET_LED_OPTIONS)))
			return set_file_data(file, getLedOptions());
		if (!memcmp(name, API_GET_PIN_MAPPINGS, sizeof(API_GET_PIN_MAPPINGS)))
			return set_file_data(file, getPinMappings());
		if (!memcmp(name, API_RESET_SETTINGS, sizeof(API_RESET_SETTINGS)))
			return set_file_data(file, resetSettings());
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
