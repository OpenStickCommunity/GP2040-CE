#include "GamepadDescriptors.h"

// This is never used
static uint16_t getConfigurationDescriptor(const uint8_t *buffer, InputMode mode)
{
	switch (mode)
	{
		case INPUT_MODE_XINPUT:
			buffer = xinput_configuration_descriptor;
			return sizeof(xinput_configuration_descriptor);

		case INPUT_MODE_SWITCH:
			buffer = switch_configuration_descriptor;
			return sizeof(switch_configuration_descriptor);

		case INPUT_MODE_KEYBOARD:
			buffer = keyboard_configuration_descriptor;
			return sizeof(keyboard_configuration_descriptor);

		case INPUT_MODE_PS4:
			buffer = ps4_configuration_descriptor;
			return sizeof(ps4_configuration_descriptor);

		case INPUT_MODE_NEOGEO:
			buffer = neogeo_configuration_descriptor;
			return sizeof(neogeo_configuration_descriptor);

		case INPUT_MODE_MDMINI:
			buffer = mdmini_configuration_descriptor;
			return sizeof(mdmini_configuration_descriptor);

		case INPUT_MODE_PCEMINI:
			buffer = pcengine_configuration_descriptor;
			return sizeof(pcengine_configuration_descriptor);

		case INPUT_MODE_EGRET:
			buffer = egret_configuration_descriptor;
			return sizeof(egret_configuration_descriptor);

		case INPUT_MODE_ASTRO:
			buffer = astro_configuration_descriptor;
			return sizeof(astro_configuration_descriptor);

		case INPUT_MODE_PSCLASSIC:
			buffer = psclassic_configuration_descriptor;
			return sizeof(psclassic_configuration_descriptor);

		case INPUT_MODE_XBOXORIGINAL:
			buffer = xboxoriginal_configuration_descriptor;
			return sizeof(xboxoriginal_configuration_descriptor);

		case INPUT_MODE_XBONE:
			buffer = xbone_configuration_descriptor;
			return sizeof(xbone_configuration_descriptor);


		default:
			buffer = hid_configuration_descriptor;
			return sizeof(hid_configuration_descriptor);
	}
}

static uint16_t getDeviceDescriptor(const uint8_t *buffer, InputMode mode)
{
	switch (mode)
	{
		case INPUT_MODE_XINPUT:
			buffer = xinput_device_descriptor;
			return sizeof(xinput_device_descriptor);

		case INPUT_MODE_SWITCH:
			buffer = switch_device_descriptor;
			return sizeof(switch_device_descriptor);

		case INPUT_MODE_KEYBOARD:
			buffer = keyboard_device_descriptor;
			return sizeof(keyboard_device_descriptor);

		case INPUT_MODE_PS4:
			buffer = ps4_device_descriptor;
			return sizeof(ps4_device_descriptor);

		case INPUT_MODE_NEOGEO:
			buffer = neogeo_device_descriptor;
			return sizeof(neogeo_device_descriptor);

		case INPUT_MODE_MDMINI:
			buffer = mdmini_device_descriptor;
			return sizeof(mdmini_device_descriptor);

		case INPUT_MODE_PCEMINI:
			buffer = pcengine_device_descriptor;
			return sizeof(pcengine_device_descriptor);

		case INPUT_MODE_EGRET:
			buffer = egret_device_descriptor;
			return sizeof(egret_device_descriptor);

		case INPUT_MODE_ASTRO:
			buffer = astro_device_descriptor;
			return sizeof(astro_device_descriptor);

		case INPUT_MODE_PSCLASSIC:
			buffer = psclassic_device_descriptor;
			return sizeof(psclassic_device_descriptor);

		case INPUT_MODE_XBOXORIGINAL:
			buffer = xboxoriginal_device_descriptor;
			return sizeof(xboxoriginal_device_descriptor);

		default:
			buffer = hid_device_descriptor;
			return sizeof(hid_device_descriptor);
	}
}

static uint16_t getHIDDescriptor(const uint8_t *buffer, InputMode mode)
{
	switch (mode)
	{
		case INPUT_MODE_SWITCH:
			buffer = switch_hid_descriptor;
			return sizeof(switch_hid_descriptor);

		case INPUT_MODE_KEYBOARD:
			buffer = keyboard_hid_descriptor;
			return sizeof(keyboard_hid_descriptor);

		case INPUT_MODE_NEOGEO:
			buffer = neogeo_hid_descriptor;
			return sizeof(neogeo_hid_descriptor);

		case INPUT_MODE_MDMINI:
			buffer = mdmini_hid_descriptor;
			return sizeof(mdmini_hid_descriptor);

		case INPUT_MODE_PCEMINI:
			buffer = pcengine_hid_descriptor;
			return sizeof(pcengine_hid_descriptor);

		case INPUT_MODE_EGRET:
			buffer = egret_hid_descriptor;
			return sizeof(egret_hid_descriptor);

		case INPUT_MODE_ASTRO:
			buffer = astro_hid_descriptor;
			return sizeof(astro_hid_descriptor);

		case INPUT_MODE_PSCLASSIC:
			buffer = psclassic_hid_descriptor;
			return sizeof(psclassic_hid_descriptor);

		default:
			buffer = hid_hid_descriptor;
			return sizeof(hid_hid_descriptor);
	}
}

static uint16_t getHIDReport(const uint8_t *buffer, InputMode mode)
{
	switch (mode)
	{
		case INPUT_MODE_SWITCH:
			buffer = switch_report_descriptor;
			return sizeof(switch_report_descriptor);

		case INPUT_MODE_KEYBOARD:
			buffer = keyboard_report_descriptor;
			return sizeof(keyboard_report_descriptor);

		case INPUT_MODE_NEOGEO:
			buffer = neogeo_report_descriptor;
			return sizeof(neogeo_report_descriptor);

		case INPUT_MODE_MDMINI:
			buffer = mdmini_report_descriptor;
			return sizeof(mdmini_report_descriptor);

		case INPUT_MODE_PCEMINI:
			buffer = pcengine_report_descriptor;
			return sizeof(pcengine_report_descriptor);

		case INPUT_MODE_EGRET:
			buffer = egret_report_descriptor;
			return sizeof(egret_report_descriptor);

		case INPUT_MODE_ASTRO:
			buffer = astro_report_descriptor;
			return sizeof(astro_report_descriptor);

		case INPUT_MODE_PSCLASSIC:
			buffer = psclassic_report_descriptor;
			return sizeof(psclassic_report_descriptor);

		default:
			buffer = hid_report_descriptor;
			return sizeof(hid_report_descriptor);
	}
}

static uint16_t getStringDescriptor(const uint16_t *buffer, InputMode mode, uint8_t index)
{
	static uint8_t descriptorStringBuffer[64]; // Max 64 bytes

	const char *value;
	uint16_t size;
	uint8_t charCount;

	switch (mode)
	{
		case INPUT_MODE_XINPUT:
			value = (const char *)xinput_string_descriptors[index];
			size = sizeof(xinput_string_descriptors[index]);
			break;

		case INPUT_MODE_SWITCH:
			value = (const char *)switch_string_descriptors[index];
			size = sizeof(switch_string_descriptors[index]);
			break;

		case INPUT_MODE_KEYBOARD:
			value = (const char *)keyboard_string_descriptors[index];
			size = sizeof(keyboard_string_descriptors[index]);
			break;

		case INPUT_MODE_PS4:
			value = (const char *)ps4_string_descriptors[index];
			size = sizeof(ps4_string_descriptors[index]);
			break;

		case INPUT_MODE_NEOGEO:
			value = (const char *)neogeo_string_descriptors[index];
			size = sizeof(neogeo_string_descriptors[index]);
			break;

		case INPUT_MODE_MDMINI:
			value = (const char *)mdmini_string_descriptors[index];
			size = sizeof(mdmini_string_descriptors[index]);
			break;

		case INPUT_MODE_PCEMINI:
			value = (const char *)pcengine_string_descriptors[index];
			size = sizeof(pcengine_string_descriptors[index]);
			break;

		case INPUT_MODE_EGRET:
			value = (const char *)egret_string_descriptors[index];
			size = sizeof(egret_string_descriptors[index]);
			break;

		case INPUT_MODE_ASTRO:
			value = (const char *)astro_string_descriptors[index];
			size = sizeof(astro_string_descriptors[index]);
			break;

		case INPUT_MODE_PSCLASSIC:
			value = (const char *)psclassic_string_descriptors[index];
			size = sizeof(psclassic_string_descriptors[index]);
			break;

		case INPUT_MODE_XBOXORIGINAL:
			value = (const char *)xboxoriginal_string_descriptors[index];
			size = sizeof(xboxoriginal_string_descriptors[index]);
			break;

		default:
			value = (const char *)hid_string_descriptors[index];
			size = sizeof(hid_string_descriptors[index]);
			break;
	}

	// Cap at max char
	charCount = strlen(value);
	if (charCount > 31)
		charCount = 31;

	for (uint8_t i = 0; i < charCount; i++)
		descriptorStringBuffer[1 + i] = value[i];


	// first byte is length (including header), second byte is string type
	descriptorStringBuffer[0] = (2 * charCount + 2);
	descriptorStringBuffer[1] = 0x03;

	// Cast temp buffer to final result
	buffer = (uint16_t *)descriptorStringBuffer;

	return size;
}
