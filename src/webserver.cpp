#include <stdio.h>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "httpd/fs.h"
#include "httpd/httpd.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "httpd/fscustom.h"
#include "httpd/fsdata.h"
#include "rndis/rndis.h"
#include "storage.h"
#include "webserver.h"
#include "GamepadStorage.h"

#define PATH_CGI_ACTION "/cgi/action"

#define METHOD_GET_ECHO_PARAMS  "echoParams"
#define METHOD_GET_PIN_MAPPINGS "getPinMappings"
#define METHOD_SET_PIN_MAPPINGS "setPinMappings"

using namespace std;

extern struct fsdata_file file__index_html[];

static vector<string> spaPaths = { "/pin-mapping" };
static vector<string> excludePaths = { "/css", "/images", "/js", "/static" };
static map<string, string> cgiParams;
static Gamepad *gamepad;

// Generic CGI method for capturing query params
static const char *cgi_action(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	cgiParams.clear();

	for (int i = 0; i < iNumParams; i++)
		cgiParams.emplace(pcParam[i], pcValue[i]);

	return PATH_CGI_ACTION;
}

static const tCGI cgi_handlers[] =
{
	{
		PATH_CGI_ACTION,
		cgi_action
	},
};

void webserver(Gamepad *instance)
{
	gamepad = instance;
	rndis_init();
	http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
	while (1)
	{
		rndis_task();
	}
}

/*************************
 * Helper methods
 *************************/

string url_decode(const string& value)
{
	string result;
	result.reserve(value.size());

	for (std::size_t i = 0; i < value.size(); ++i)
	{
		auto ch = value[i];
		if (ch == '%' && (i + 2) < value.size())
		{
			auto hex = value.substr(i + 1, 2);
			auto dec = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
			result.push_back(dec);
			i += 2;
		}
		else if (ch == '+')
		{
			result.push_back(' ');
		}
		else
		{
			result.push_back(ch);
		}
	}

	return result;
}

vector<string> split_string(const string &s, char delim)
{
	vector<string> result;
	stringstream ss(s);
	string item;

	while (getline(ss, item, delim))
		result.push_back(item);

	return result;
}

void set_file_data(struct fs_file *file, const char *data, size_t size)
{
	memset(file, 0, sizeof(struct fs_file));
	file->data = (const char *)data;
	file->len = size;
	file->index = file->len;
	file->http_header_included = 0;
}

string get_param_value(string name)
{
	for (auto& param : cgiParams)
	{
		if (!name.compare(param.first))
			return url_decode(param.second);
	}

	return {};
}

map<string, string> get_param_data(string name)
{
	static string delim = "|";

	map<string, string> data;
	string rawValue = get_param_value(name);
	if (!rawValue.empty())
	{
		int start = 0U;
		int end = (int)rawValue.find(delim);
		while (end != -1)
		{
			vector<string> split = split_string(rawValue.substr(start, end - start), ':');
			data.emplace(split[0], split[1]);
			start = end + (int)delim.length();
			end = (int)rawValue.find(delim, start);
		}

		vector<string> split = split_string(rawValue.substr(start, end - start), ':');
		data.emplace(split[0], split[1]);
	}

	return data;
}

string to_json(string method, map<string, string> props)
{
	string json = "{ \"method\": \"" + method + "\"";

	int count = props.size();
	if (count > 0)
	{
		for (auto &nv : props)
			json = json + string(", \"") + nv.first + "\": \"" + nv.second + "\"";
	}

	json += " }";

	return json;
}

/*************************
 * API methods
 *************************/

string getPinMappings()
{
	string result;
	map<string, string> props;

	props.emplace("Up", to_string(gamepad->mapDpadUp->pin));
	props.emplace("Down", to_string(gamepad->mapDpadDown->pin));
	props.emplace("Left", to_string(gamepad->mapDpadLeft->pin));
	props.emplace("Right", to_string(gamepad->mapDpadRight->pin));
	props.emplace("B1", to_string(gamepad->mapButtonB1->pin));
	props.emplace("B2", to_string(gamepad->mapButtonB2->pin));
	props.emplace("B3", to_string(gamepad->mapButtonB3->pin));
	props.emplace("B4", to_string(gamepad->mapButtonB4->pin));
	props.emplace("L1", to_string(gamepad->mapButtonL1->pin));
	props.emplace("R1", to_string(gamepad->mapButtonR1->pin));
	props.emplace("L2", to_string(gamepad->mapButtonL2->pin));
	props.emplace("R2", to_string(gamepad->mapButtonR2->pin));
	props.emplace("S1", to_string(gamepad->mapButtonS1->pin));
	props.emplace("S2", to_string(gamepad->mapButtonS2->pin));
	props.emplace("L3", to_string(gamepad->mapButtonL3->pin));
	props.emplace("R3", to_string(gamepad->mapButtonR3->pin));
	props.emplace("A1", to_string(gamepad->mapButtonA1->pin));
	props.emplace("A2", to_string(gamepad->mapButtonA2->pin));

	return to_json(METHOD_GET_PIN_MAPPINGS, props);
}

string setPinMappings()
{
	// Data format example:
	// A1:28|A2:18|B1:4|B2:5|B3:0|B4:1|Down:11|L1:3|L2:7|L3:17|Left:10|R1:2|R2:6|R3:16|Right:12|S1:8|S2:9|Up:13
	map<string, string> data = get_param_data("mappings");
	BoardOptions options =
	{
		.useUserDefinedPins = true,
		.pinDpadUp    = (uint8_t)stoul(data.find("Up")->second),
		.pinDpadDown  = (uint8_t)stoul(data.find("Down")->second),
		.pinDpadLeft  = (uint8_t)stoul(data.find("Left")->second),
		.pinDpadRight = (uint8_t)stoul(data.find("Right")->second),
		.pinButtonB1  = (uint8_t)stoul(data.find("B1")->second),
		.pinButtonB2  = (uint8_t)stoul(data.find("B2")->second),
		.pinButtonB3  = (uint8_t)stoul(data.find("B3")->second),
		.pinButtonB4  = (uint8_t)stoul(data.find("B4")->second),
		.pinButtonL1  = (uint8_t)stoul(data.find("L1")->second),
		.pinButtonR1  = (uint8_t)stoul(data.find("R1")->second),
		.pinButtonL2  = (uint8_t)stoul(data.find("L2")->second),
		.pinButtonR2  = (uint8_t)stoul(data.find("R2")->second),
		.pinButtonS1  = (uint8_t)stoul(data.find("S1")->second),
		.pinButtonS2  = (uint8_t)stoul(data.find("S2")->second),
		.pinButtonL3  = (uint8_t)stoul(data.find("L3")->second),
		.pinButtonR3  = (uint8_t)stoul(data.find("R3")->second),
		.pinButtonA1  = (uint8_t)stoul(data.find("A1")->second),
		.pinButtonA2  = (uint8_t)stoul(data.find("A2")->second),
	};

	setBoardOptions(options);
	GamepadStore.save();
	return to_json(METHOD_SET_PIN_MAPPINGS, data);
}

/*************************
 * LWIP implementation
 *************************/

int fs_open_custom(struct fs_file *file, const char *name)
{
	if (!memcmp(name, PATH_CGI_ACTION, sizeof(PATH_CGI_ACTION)))
	{
		string method = get_param_value("method");

		if (!method.compare(METHOD_GET_ECHO_PARAMS))
		{
			string json = to_json(METHOD_GET_ECHO_PARAMS, cgiParams);
			set_file_data(file, json.data(), json.size());
			return 1;
		}
		else if (!method.compare(METHOD_GET_PIN_MAPPINGS))
		{
			string json = getPinMappings();
			set_file_data(file, json.data(), json.size());
			return 1;
		}
		else if (!method.compare(METHOD_SET_PIN_MAPPINGS))
		{
			string json = setPinMappings();
			set_file_data(file, json.data(), json.size());
			return 1;
		}
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
}
