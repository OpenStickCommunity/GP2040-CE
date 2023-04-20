#include "config_utils.h"

#include "enums.pb.h"

#include "BoardConfig.h"
#include "helper.h"
#include "addons/analog.h"
#include "addons/board_led.h"
#include "addons/bootsel_button.h"
#include "addons/buzzerspeaker.h"
#include "addons/dualdirectional.h"
#include "addons/extra_button.h"
#include "addons/i2canalog1219.h"
#include "addons/i2cdisplay.h"
#include "addons/jslider.h"
#include "addons/neopicoleds.h"
#include "addons/playernum.h"
#include "addons/pleds.h"
#include "addons/reverse.h"
#include "addons/turbo.h"

#include "FlashPROM.h"

#include <ArduinoJson.h>

// Verify that the maximum size of the serialized Config object fits into the allocated flash block
#if defined(Config_size)
	#if Config_size > EEPROM_SIZE_BYTES
		#error "Maximum size of Config exceeds the maximum size allocated for FlashPROM"
	#endif
#else
	#error "Maximum size of Config cannot be determined statically, make sure that you do not use any dynamically sized arrays or strings"
#endif

// -----------------------------------------------------
// Default values
// -----------------------------------------------------

#define PREPROCESSOR_JOIN2(x, y) x ## y
#define PREPROCESSOR_JOIN(x, y) PREPROCESSOR_JOIN2(x, y)

#define INIT_UNSET_PROPERTY(parent, property, value) \
	if (!parent.PREPROCESSOR_JOIN(has_, property)) \
	{ \
		parent.property = value; \
		parent.PREPROCESSOR_JOIN(has_, property) = true; \
	} \

#define INIT_UNSET_PROPERTY_STR(parent, property, str) \
	if (!parent.PREPROCESSOR_JOIN(has_, property)) \
	{ \
		strncpy(parent.property, str, sizeof(parent.property)); \
		parent.property[sizeof(parent.property) - 1] = '\0'; \
		parent.PREPROCESSOR_JOIN(has_, property) = true; \
	} \

void ConfigUtils::initUnsetPropertiesWithDefaults(Config& config)
{
	INIT_UNSET_PROPERTY_STR(config, boardVersion, GP2040VERSION);

	// gamepadOptions
	INIT_UNSET_PROPERTY(config.gamepadOptions, inputMode, InputMode_Proto_INPUT_MODE_XINPUT);
	INIT_UNSET_PROPERTY(config.gamepadOptions, dpadMode, DpadMode_Proto_DPAD_MODE_DIGITAL);
#ifdef DEFAULT_SOCD_MODE
	INIT_UNSET_PROPERTY(config.gamepadOptions, socdMode, static_cast<SOCDMode_Proto>(DEFAULT_SOCD_MODE));
#else
	INIT_UNSET_PROPERTY(config.gamepadOptions, socdMode, SOCDMode_Proto_SOCD_MODE_NEUTRAL);
#endif
	INIT_UNSET_PROPERTY(config.gamepadOptions, invertXAxis, false);
	INIT_UNSET_PROPERTY(config.gamepadOptions, invertYAxis, false);

	// pinMappings
	INIT_UNSET_PROPERTY(config.pinMappings, pinDpadUp, PIN_DPAD_UP);
	INIT_UNSET_PROPERTY(config.pinMappings, pinDpadDown, PIN_DPAD_DOWN);
	INIT_UNSET_PROPERTY(config.pinMappings, pinDpadLeft, PIN_DPAD_LEFT);
	INIT_UNSET_PROPERTY(config.pinMappings, pinDpadRight, PIN_DPAD_RIGHT);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonB1, PIN_BUTTON_B1);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonB2, PIN_BUTTON_B2);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonB3, PIN_BUTTON_B3);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonB4, PIN_BUTTON_B4);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonL1, PIN_BUTTON_L1);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonR1, PIN_BUTTON_R1);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonL2, PIN_BUTTON_L2);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonR2, PIN_BUTTON_R2);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonS1, PIN_BUTTON_S1);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonS2, PIN_BUTTON_S2);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonL3, PIN_BUTTON_L3);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonR3, PIN_BUTTON_R3);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonA1, PIN_BUTTON_A1);
	INIT_UNSET_PROPERTY(config.pinMappings, pinButtonA2, PIN_BUTTON_A2);

	// displayOptions
	INIT_UNSET_PROPERTY(config.displayOptions, enabled, !!HAS_I2C_DISPLAY);
	INIT_UNSET_PROPERTY(config.displayOptions, i2cBlock, (I2C_BLOCK == i2c0) ? 0 : 1);
	INIT_UNSET_PROPERTY(config.displayOptions, i2cSDAPin, I2C_SDA_PIN);
	INIT_UNSET_PROPERTY(config.displayOptions, i2cSCLPin, I2C_SCL_PIN);
	INIT_UNSET_PROPERTY(config.displayOptions, i2cAddress, DISPLAY_I2C_ADDR);
	INIT_UNSET_PROPERTY(config.displayOptions, i2cSpeed, I2C_SPEED);
	INIT_UNSET_PROPERTY(config.displayOptions, buttonLayout, BUTTON_LAYOUT);
	INIT_UNSET_PROPERTY(config.displayOptions, buttonLayoutRight, BUTTON_LAYOUT_RIGHT);

	ButtonLayoutParamsLeft& paramsLeft = config.displayOptions.buttonLayoutCustomOptions.paramsLeft;
	INIT_UNSET_PROPERTY(paramsLeft, layout, BUTTON_LAYOUT);
	INIT_UNSET_PROPERTY(paramsLeft.common, startX, 8);
	INIT_UNSET_PROPERTY(paramsLeft.common, startY, 28);
	INIT_UNSET_PROPERTY(paramsLeft.common, buttonRadius, 8);
	INIT_UNSET_PROPERTY(paramsLeft.common, buttonPadding, 2);

	ButtonLayoutParamsRight& paramsRight = config.displayOptions.buttonLayoutCustomOptions.paramsRight;
	INIT_UNSET_PROPERTY(paramsRight, layout, BUTTON_LAYOUT_RIGHT);
	INIT_UNSET_PROPERTY(paramsRight.common, startX, 8);
	INIT_UNSET_PROPERTY(paramsRight.common, startY, 28);
	INIT_UNSET_PROPERTY(paramsRight.common, buttonRadius, 8);
	INIT_UNSET_PROPERTY(paramsRight.common, buttonPadding, 2);

	INIT_UNSET_PROPERTY(config.displayOptions, splashMode, SPLASH_MODE);
	INIT_UNSET_PROPERTY(config.displayOptions, splashChoice, SPLASH_CHOICE);
	INIT_UNSET_PROPERTY(config.displayOptions, splashDuration, SPLASH_DURATION);
	INIT_UNSET_PROPERTY_STR(config.displayOptions, splashImage, "");
	INIT_UNSET_PROPERTY(config.displayOptions, size, DISPLAY_SIZE);
	INIT_UNSET_PROPERTY(config.displayOptions, flip, DISPLAY_FLIP);
	INIT_UNSET_PROPERTY(config.displayOptions, invert, !!DISPLAY_INVERT);
	INIT_UNSET_PROPERTY(config.displayOptions, displaySaverTimeout, DISPLAY_SAVER_TIMEOUT);

	// ledOptions
	INIT_UNSET_PROPERTY(config.ledOptions, dataPin, BOARD_LEDS_PIN);
	INIT_UNSET_PROPERTY(config.ledOptions, ledFormat, static_cast<LEDFormat_Proto>(LED_FORMAT));
	INIT_UNSET_PROPERTY(config.ledOptions, ledLayout, BUTTON_LAYOUT);
	INIT_UNSET_PROPERTY(config.ledOptions, ledsPerButton, LEDS_PER_PIXEL);
	INIT_UNSET_PROPERTY(config.ledOptions, brightnessMaximum, LED_BRIGHTNESS_MAXIMUM);
	INIT_UNSET_PROPERTY(config.ledOptions, brightnessSteps, LED_BRIGHTNESS_STEPS);

	INIT_UNSET_PROPERTY(config.ledOptions, indexUp, LEDS_DPAD_UP);
	INIT_UNSET_PROPERTY(config.ledOptions, indexDown, LEDS_DPAD_DOWN);
	INIT_UNSET_PROPERTY(config.ledOptions, indexLeft, LEDS_DPAD_LEFT);
	INIT_UNSET_PROPERTY(config.ledOptions, indexRight, LEDS_DPAD_RIGHT);
	INIT_UNSET_PROPERTY(config.ledOptions, indexB1, LEDS_BUTTON_B1);
	INIT_UNSET_PROPERTY(config.ledOptions, indexB2, LEDS_BUTTON_B2);
	INIT_UNSET_PROPERTY(config.ledOptions, indexB3, LEDS_BUTTON_B3);
	INIT_UNSET_PROPERTY(config.ledOptions, indexB4, LEDS_BUTTON_B4);
	INIT_UNSET_PROPERTY(config.ledOptions, indexL1, LEDS_BUTTON_L1);
	INIT_UNSET_PROPERTY(config.ledOptions, indexR1, LEDS_BUTTON_R1);
	INIT_UNSET_PROPERTY(config.ledOptions, indexL2, LEDS_BUTTON_L2);
	INIT_UNSET_PROPERTY(config.ledOptions, indexR2, LEDS_BUTTON_R2);
	INIT_UNSET_PROPERTY(config.ledOptions, indexS1, LEDS_BUTTON_S1);
	INIT_UNSET_PROPERTY(config.ledOptions, indexS2, LEDS_BUTTON_S2);
	INIT_UNSET_PROPERTY(config.ledOptions, indexL3, LEDS_BUTTON_L3);
	INIT_UNSET_PROPERTY(config.ledOptions, indexR3, LEDS_BUTTON_R3);
	INIT_UNSET_PROPERTY(config.ledOptions, indexA1, LEDS_BUTTON_A1);
	INIT_UNSET_PROPERTY(config.ledOptions, indexA2, LEDS_BUTTON_A2);

	// animationOptions
	INIT_UNSET_PROPERTY(config.animationOptions, baseAnimationIndex, LEDS_BASE_ANIMATION_INDEX);
	INIT_UNSET_PROPERTY(config.animationOptions, brightness, LEDS_BRIGHTNESS);
	INIT_UNSET_PROPERTY(config.animationOptions, staticColorIndex, LEDS_STATIC_COLOR_INDEX);
	INIT_UNSET_PROPERTY(config.animationOptions, buttonColorIndex, LEDS_BUTTON_COLOR_INDEX);
	INIT_UNSET_PROPERTY(config.animationOptions, chaseCycleTime, LEDS_CHASE_CYCLE_TIME);
	INIT_UNSET_PROPERTY(config.animationOptions, rainbowCycleTime, LEDS_RAINBOW_CYCLE_TIME);
	INIT_UNSET_PROPERTY(config.animationOptions, themeIndex, LEDS_THEME_INDEX);

	// addonOptions.bootselButtonOptions
	INIT_UNSET_PROPERTY(config.addonOptions.bootselButtonOptions, enabled, !!BOOTSEL_BUTTON_ENABLED);
	INIT_UNSET_PROPERTY(config.addonOptions.bootselButtonOptions, buttonMap, BOOTSEL_BUTTON_MASK);

	// addonOptions.onBoardLedOptions
	INIT_UNSET_PROPERTY(config.addonOptions.onBoardLedOptions, mode, BOARD_LED_TYPE);

	// addonOptions.analogOptions
	INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, enabled, !!ANALOG_INPUT_ENABLED);
	INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, analogAdcPinX, ANALOG_ADC_VRX);
	INIT_UNSET_PROPERTY(config.addonOptions.analogOptions, analogAdcPinY, ANALOG_ADC_VRY);

	// addonOptions.turboOptions
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, enabled, !!TURBO_ENABLED);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, buttonPin, PIN_BUTTON_TURBO);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, ledPin, TURBO_LED_PIN);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shotCount, DEFAULT_SHOT_PER_SEC);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupDialPin, PIN_SHMUP_DIAL);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupModeEnabled, !!TURBO_SHMUP_MODE);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupAlwaysOn1, SHMUP_ALWAYS_ON1);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupAlwaysOn2, SHMUP_ALWAYS_ON2);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupAlwaysOn3, SHMUP_ALWAYS_ON3);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupAlwaysOn4, SHMUP_ALWAYS_ON4);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtn1Pin, PIN_SHMUP_BUTTON1);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtn2Pin, PIN_SHMUP_BUTTON2);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtn3Pin, PIN_SHMUP_BUTTON3);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtn4Pin, PIN_SHMUP_BUTTON4);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtnMask1, SHMUP_BUTTON1);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtnMask2, SHMUP_BUTTON2);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtnMask3, SHMUP_BUTTON3);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupBtnMask4, SHMUP_BUTTON4);
	INIT_UNSET_PROPERTY(config.addonOptions.turboOptions, shmupMixMode, SHMUP_MIX_MODE);

	// addonOptions.sliderOptions
	INIT_UNSET_PROPERTY(config.addonOptions.sliderOptions, enabled, !!JSLIDER_ENABLED);
	INIT_UNSET_PROPERTY(config.addonOptions.sliderOptions, pinLS, PIN_SLIDER_LS);
	INIT_UNSET_PROPERTY(config.addonOptions.sliderOptions, pinRS, PIN_SLIDER_RS);

	// addonOptions.reverseOptions
	INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, enabled, !!REVERSE_ENABLED);
	INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, buttonPin, PIN_REVERSE);
	INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, ledPin, REVERSE_LED_PIN);
	INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, actionUp, REVERSE_UP_DEFAULT);
	INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, actionDown, REVERSE_DOWN_DEFAULT);
	INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, actionLeft, REVERSE_LEFT_DEFAULT);
	INIT_UNSET_PROPERTY(config.addonOptions.reverseOptions, actionRight, REVERSE_RIGHT_DEFAULT);

	// addonOptions.analogADS1219Options
	INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, enabled, !!I2C_ANALOG1219_ENABLED);
	INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, i2cBlock, (I2C_ANALOG1219_BLOCK == i2c0) ? 0 : 1)
	INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, i2cSDAPin, I2C_ANALOG1219_SDA_PIN);
	INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, i2cSCLPin, I2C_ANALOG1219_SCL_PIN);
	INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, i2cAddress, I2C_ANALOG1219_ADDRESS);
	INIT_UNSET_PROPERTY(config.addonOptions.analogADS1219Options, i2cSpeed, I2C_ANALOG1219_SPEED);

	// addonOptions.dualDirectionalOptions
	INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, enabled, !!DUAL_DIRECTIONAL_ENABLED);
	INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, upPin, PIN_DUAL_DIRECTIONAL_UP);
	INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, downPin, PIN_DUAL_DIRECTIONAL_DOWN)
	INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, leftPin, PIN_DUAL_DIRECTIONAL_LEFT);
	INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, rightPin, PIN_DUAL_DIRECTIONAL_RIGHT);
	INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, dpadMode, static_cast<DpadMode_Proto>(DUAL_DIRECTIONAL_STICK_MODE));
	INIT_UNSET_PROPERTY(config.addonOptions.dualDirectionalOptions, combineMode, DUAL_DIRECTIONAL_COMBINE_MODE);

	// addonOptions.buzzerOptions
	INIT_UNSET_PROPERTY(config.addonOptions.buzzerOptions, enabled, !!BUZZER_ENABLED);
	INIT_UNSET_PROPERTY(config.addonOptions.buzzerOptions, pin, BUZZER_PIN);
	INIT_UNSET_PROPERTY(config.addonOptions.buzzerOptions, volume, BUZZER_VOLUME);

	// addonOptions.extraOptions
	INIT_UNSET_PROPERTY(config.addonOptions.extraOptions, enabled, !!EXTRA_BUTTON_ENABLED);
	INIT_UNSET_PROPERTY(config.addonOptions.extraOptions, pin, EXTRA_BUTTON_PIN);
	INIT_UNSET_PROPERTY(config.addonOptions.extraOptions, buttonMap, EXTRA_BUTTON_MASK);

	// addonOptions.playerNumberOptions
	INIT_UNSET_PROPERTY(config.addonOptions.playerNumberOptions, enabled, !!PLAYERNUM_ADDON_ENABLED);
	INIT_UNSET_PROPERTY(config.addonOptions.playerNumberOptions, number, PLAYER_NUMBER);
}

// -----------------------------------------------------
// To JSON
// -----------------------------------------------------

static void writeIndentation(std::string& str, int level)
{
	str.append(static_cast<std::string::size_type>(level), '\t');
}

#define TO_JSON_UENUM(fieldname, submessageType) str.append(std::to_string(s.fieldname));
#define TO_JSON_INT32(fieldname, submessageType) str.append(std::to_string(s.fieldname));
#define TO_JSON_UINT32(fieldname, submessageType) str.append(std::to_string(s.fieldname));
#define TO_JSON_BOOL(fieldname, submessageType) str.append((s.fieldname) ? "true" : "false");
#define TO_JSON_STRING(fieldname, submessageType) str.push_back('"'); str.append(s.fieldname); str.push_back('"');
#define TO_JSON_MESSAGE(fieldname, submessageType) PREPROCESSOR_JOIN(toJSON, submessageType)(str, s.fieldname, indentLevel + 1);

#define TO_JSON_REPEATED_UENUM(fieldname, submessageType) str.append(std::to_string(s.fieldname[i]));
#define TO_JSON_REPEATED_INT32(fieldname, submessageType) str.append(std::to_string(s.fieldname[i]));
#define TO_JSON_REPEATED_UINT32(fieldname, submessageType) str.append(std::to_string(s.fieldname[i]));
#define TO_JSON_REPEATED_BOOL(fieldname, submessageType) str.append((s.fieldname[i]) ? "true" : "false");
#define TO_JSON_REPEATED_STRING(fieldname, submessageType) str.push_back('"'); str.append(s.fieldname[i]); str.push_back('"');
#define TO_JSON_REPEATED_MESSAGE(fieldname, submessageType) PREPROCESSOR_JOIN(toJSON, submessageType)(str, s.fieldname[i], indentLevel + 1);

#define TO_JSON_REPEATED(ltype, fieldname, submessageType) \
	str.append("["); \
	for (int i = 0; i < s.PREPROCESSOR_JOIN(fieldname, _count); ++i) \
	{ \
		if (i != 0) str.append(",");\
		str.append("\n"); \
		writeIndentation(str, indentLevel + 1); \
		PREPROCESSOR_JOIN(TO_JSON_REPEATED_, ltype)(fieldname, submessageType) \
	} \
	str.append("\n"); \
	writeIndentation(str, indentLevel); \
	str.append("]"); \

#define TO_JSON_REQUIRED(ltype, fieldname, submessageType) PREPROCESSOR_JOIN(TO_JSON_, ltype)(fieldname, submessageType)
#define TO_JSON_OPTIONAL(ltype, fieldname, submessageType) PREPROCESSOR_JOIN(TO_JSON_, ltype)(fieldname, submessageType)
#define TO_JSON_SINGULAR(ltype, fieldname, submessageType) static_assert(false, "not supported");
#define TO_JSON_FIXARRAY(ltype, fieldname, submessageType) static_assert(false, "not supported");
#define TO_JSON_ONEOF(ltype, fieldname, submessageType) static_assert(false, "not supported");

#define TO_JSON_STATIC(htype, ltype, fieldname, submessageType) PREPROCESSOR_JOIN(TO_JSON_, htype)(ltype, fieldname, submessageType)
#define TO_JSON_POINTER(htype, ltype, fieldname, submessageType) static_assert(false, "not supported");
#define TO_JSON_CALLBACK(htype, ltype, fieldname, submessageType) static_assert(false, "not supported");

#define TO_JSON_FIELD(parenttype, atype, htype, ltype, fieldname, tag) \
	if (!firstField) str.append(",\n"); \
	firstField = false; \
	writeIndentation(str, indentLevel); \
	str.append("\"" #fieldname "\": "); \
	PREPROCESSOR_JOIN(TO_JSON_, atype)(htype, ltype, fieldname, parenttype ## _ ## fieldname ## _MSGTYPE)

#define GEN_TO_JSON_FUNCTION_DECL(structtype) static void toJSON ## structtype(std::string& str, const structtype& s, int indentLevel);

#define GEN_TO_JSON_FUNCTION(structtype) \
	static void toJSON ## structtype(std::string& str, const structtype& s, int indentLevel) \
	{ \
		bool firstField = true; \
		str.append("{\n"); \
		structtype ## _FIELDLIST(TO_JSON_FIELD, structtype) \
		str.push_back('\n'); \
		writeIndentation(str, indentLevel - 1); \
		str.push_back('}'); \
	} \

#if defined(CONFIG_MESSAGES_GP2040)
    CONFIG_MESSAGES_GP2040(GEN_TO_JSON_FUNCTION_DECL)
    CONFIG_MESSAGES_GP2040(GEN_TO_JSON_FUNCTION)
#endif
#if defined(ENUM_MESSAGES_GP2040)
    ENUM_MESSAGES_GP2040(GEN_TO_JSON_FUNCTION_DECL)
    ENUM_MESSAGES_GP2040(GEN_TO_JSON_FUNCTION)
#endif

std::string ConfigUtils::toJSON(const Config& config)
{
	std::string str;
	str.reserve(1024 * 4);
	toJSONConfig(str, config, 1);
	str.push_back('\n');

	return str;
}

// -----------------------------------------------------
// From JSON
// -----------------------------------------------------

#define TEST_VALUE(name, value) if (v == value) return true; 

#define GEN_IS_VALID_ENUM_VALUE_FUNCTION(enumtype) \
	static bool isValid ## enumtype(int v) \
	{ \
		PREPROCESSOR_JOIN(enumtype, _VALUELIST)(TEST_VALUE) \
		return false; \
	}

#if defined(CONFIG_ENUMS_GP2040)
	CONFIG_ENUMS_GP2040(GEN_IS_VALID_ENUM_VALUE_FUNCTION)
#endif
#if defined(ENUMS_ENUMS_GP2040)
	ENUMS_ENUMS_GP2040(GEN_IS_VALID_ENUM_VALUE_FUNCTION)
#endif

#define FROM_JSON_UENUM(fieldname, enumType) \
	if (jsonObject.containsKey(#fieldname)) \
	{ \
		JsonVariantConst value = jsonObject[#fieldname]; \
		if (value.is<int>()) \
		{ \
			const int v = value.as<int>(); \
			if (PREPROCESSOR_JOIN(isValid, PREPROCESSOR_JOIN(enumType, _ENUMTYPE))(v)) \
			{ \
				configStruct.fieldname = static_cast<decltype(configStruct.fieldname)>(v); \
				configStruct.PREPROCESSOR_JOIN(has_, fieldname) = true; \
			} \
			else \
			{ \
				return false; \
			} \
		} \
		else \
		{ \
			return false; \
		} \
	}

#define FROM_JSON_INT32(fieldname, submessageType) \
	if (jsonObject.containsKey(#fieldname)) \
	{ \
		JsonVariantConst value = jsonObject[#fieldname]; \
		if (value.is<int>()) \
		{ \
			configStruct.fieldname = value.as<int>(); \
			configStruct.PREPROCESSOR_JOIN(has_, fieldname) = true; \
		} \
		else \
		{ \
			return false; \
		} \
	}

#define FROM_JSON_UINT32(fieldname, submessageType) \
	if (jsonObject.containsKey(#fieldname)) \
	{ \
		JsonVariantConst value = jsonObject[#fieldname]; \
		if (value.is<unsigned int>()) \
		{ \
			configStruct.fieldname = value.as<unsigned int>(); \
			configStruct.PREPROCESSOR_JOIN(has_, fieldname) = true; \
		} \
		else \
		{ \
			return false; \
		} \
	}

#define FROM_JSON_BOOL(fieldname, submessageType) \
	if (jsonObject.containsKey(#fieldname)) \
	{ \
		JsonVariantConst value = jsonObject[#fieldname]; \
		if (value.is<bool>()) \
		{ \
			configStruct.fieldname = value.as<bool>(); \
			configStruct.PREPROCESSOR_JOIN(has_, fieldname) = true; \
		} \
		else \
		{ \
			return false; \
		} \
	}

#define FROM_JSON_STRING(fieldname, submessageType) \
	if (jsonObject.containsKey(#fieldname)) \
	{ \
		JsonVariantConst value = jsonObject[#fieldname]; \
		if (value.is<const char*>() && strlen(value.as<const char*>()) < sizeof(configStruct.fieldname)) \
		{ \
			strncpy(configStruct.fieldname, value.as<const char*>(), sizeof(configStruct.fieldname)); \
			configStruct.fieldname[sizeof(configStruct.fieldname) - 1] = '\0'; \
			configStruct.PREPROCESSOR_JOIN(has_, fieldname) = true; \
		} \
		else \
		{ \
			return false; \
		} \
	}

#define FROM_JSON_MESSAGE(fieldname, submessageType) \
	if (jsonObject.containsKey(#fieldname)) \
	{ \
		JsonVariantConst value = jsonObject[#fieldname]; \
		if (!value.is<JsonObjectConst>() || !PREPROCESSOR_JOIN(fromJSON, PREPROCESSOR_JOIN(submessageType, _MSGTYPE))(value.as<JsonObjectConst>(), configStruct.fieldname)) \
		{ \
			return false; \
		} \
	}

#define FROM_JSON_REPEATED_UENUM(fieldname, enumType) \
	configStruct.fieldname ## _count = 0; \
	for (size_t index = 0; index < array.size(); ++index) \
	{ \
		if (!array[index].is<int>() || !PREPROCESSOR_JOIN(isValid, PREPROCESSOR_JOIN(enumType, _ENUMTYPE))(array[index].as<int>())) \
		{ \
			return false; \
		} \
		configStruct.fieldname[index] = static_cast<PREPROCESSOR_JOIN(enumType, _ENUMTYPE)>(array[index].as<int>()); \
		++configStruct.fieldname ## _count; \
	}

#define FROM_JSON_REPEATED_INT32(fieldname, submessageType) \
	configStruct.fieldname ## _count = 0; \
	for (size_t index = 0; index < array.size(); ++index) \
	{ \
		if (!array[index].is<int>()) \
		{ \
			return false; \
		} \
		configStruct.fieldname[index] = array[index].as<int>(); \
		++configStruct.fieldname ## _count; \
	}

#define FROM_JSON_REPEATED_UINT32(fieldname, submessageType) \
	configStruct.fieldname ## _count = 0; \
	for (size_t index = 0; index < array.size(); ++index) \
	{ \
		if (!array[index].is<unsigned int>()) \
		{ \
			return false; \
		} \
		configStruct.fieldname[index] = array[index].as<unsigned int>(); \
		++configStruct.fieldname ## _count; \
	}

#define FROM_JSON_REPEATED_BOOL(fieldname, submessageType) \
	configStruct.fieldname ## _count = 0; \
	for (size_t index = 0; index < array.size(); ++index) \
	{ \
		if (!array[index].is<bool>()) \
		{ \
			return false; \
		} \
		configStruct.fieldname[index] = array[index].as<bool>(); \
		++configStruct.fieldname ## _count; \
	}

#define FROM_JSON_REPEATED_STRING(fieldname, submessageType) \
	configStruct.fieldname ## _count = 0; \
	for (size_t index = 0; index < array.size(); ++index) \
	{ \
		if (!array[index].is<const char*>() || strlen(array[index].as<const char*>()) >= sizeof(configStruct.fieldname[index])) \
		{ \
			return false; \
		} \
		strncpy(configStruct.fieldname[index], array[index].as<const char*>(), sizeof(configStruct.fieldname[index])); \
		configStruct.fieldname[index][sizeof(configStruct.fieldname[index]) - 1] = '\0'; \
		++configStruct.fieldname ## _count; \
	}

#define FROM_JSON_REPEATED_MESSAGE(fieldname, submessageType) \
	configStruct.fieldname ## _count = 0; \
	for (size_t index = 0; index < array.size(); ++index) \
	{ \
		if (!array[index].is<JsonObjectConst>() || !PREPROCESSOR_JOIN(fromJSON, PREPROCESSOR_JOIN(submessageType, _MSGTYPE))(array[index].as<JsonObjectConst>(), configStruct.fieldname[index])) \
		{ \
			return false; \
		} \
		++configStruct.fieldname ## _count; \
	}

#define FROM_JSON_REPEATED(ltype, fieldname, submessageType) \
	if (jsonObject.containsKey(#fieldname)) \
	{ \
		JsonVariantConst value = jsonObject[#fieldname]; \
		if (!value.is<JsonArrayConst>()) \
		{ \
			return false; \
		} \
		JsonArrayConst array = value.as<JsonArrayConst>(); \
		if (array.size() > sizeof(configStruct.fieldname) / sizeof(configStruct.fieldname[0])) \
		{ \
			return false; \
		} \
		PREPROCESSOR_JOIN(FROM_JSON_REPEATED_, ltype)(fieldname, submessageType) \
	}

#define FROM_JSON_REQUIRED(ltype, fieldname, submessageType) PREPROCESSOR_JOIN(FROM_JSON_, ltype)(fieldname, submessageType)
#define FROM_JSON_OPTIONAL(ltype, fieldname, submessageType) PREPROCESSOR_JOIN(FROM_JSON_, ltype)(fieldname, submessageType)
#define FROM_JSON_SINGULAR(ltype, fieldname, submessageType) static_assert(false, "not supported");
#define FROM_JSON_FIXARRAY(ltype, fieldname, submessageType) static_assert(false, "not supported");
#define FROM_JSON_ONEOF(ltype, fieldname, submessageType) static_assert(false, "not supported");

#define FROM_JSON_STATIC(htype, ltype, fieldname, submessageType) PREPROCESSOR_JOIN(FROM_JSON_, htype)(ltype, fieldname, submessageType)
#define FROM_JSON_POINTER(htype, ltype, fieldname, submessageType) static_assert(false, "not supported");
#define FROM_JSON_CALLBACK(htype, ltype, fieldname, submessageType) static_assert(false, "not supported");

#define FROM_JSON_FIELD(parenttype, atype, htype, ltype, fieldname, tag) \
	PREPROCESSOR_JOIN(FROM_JSON_, atype)(htype, ltype, fieldname, parenttype ## _ ## fieldname)

#define GEN_FROM_JSON_FUNCTION_DECL(structtype) static bool fromJSON ## structtype(JsonObjectConst jsonObject, structtype& configStruct);

#define GEN_FROM_JSON_FUNCTION(structtype) \
	static bool fromJSON ## structtype(JsonObjectConst jsonObject, structtype& configStruct) \
	{ \
		structtype ## _FIELDLIST(FROM_JSON_FIELD, structtype) \
		return true; \
	}

#if defined(CONFIG_MESSAGES_GP2040)
    CONFIG_MESSAGES_GP2040(GEN_FROM_JSON_FUNCTION_DECL)
    CONFIG_MESSAGES_GP2040(GEN_FROM_JSON_FUNCTION)
#endif
#if defined(ENUM_MESSAGES_GP2040)
    ENUM_MESSAGES_GP2040(GEN_FROM_JSON_FUNCTION_DECL)
    ENUM_MESSAGES_GP2040(GEN_FROM_JSON_FUNCTION)
#endif

// Missing properties are ignored and initialized with default values
// Type mismatches, buffer overruns or illegal enum values cause an error
Config ConfigUtils::fromJSON(const char* data, size_t dataLen, bool& success)
{
	success = false;

	DynamicJsonDocument doc(1024 * 10);
	if (deserializeJson(doc, data, dataLen) != DeserializationError::Ok || !doc.is<JsonObject>())
	{
		return Config_init_default;
	}

	Config config = Config_init_default;
	if (!fromJSONConfig(doc.as<JsonObjectConst>(), config))
	{
		return Config_init_default;
	}

	initUnsetPropertiesWithDefaults(config);

	success = true;
	return config;
}
