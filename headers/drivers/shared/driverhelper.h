#ifndef _DRIVER_HELPER_H_
#define _DRIVER_HELPER_H_

static uint16_t * getStringDescriptor(const char * value)
{
	static uint8_t descriptorStringBuffer[64]; // Max 64 bytes
	uint8_t charCount;

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
	return (uint16_t*)&descriptorStringBuffer;
}

#endif // _DRIVER_HELPER_H_
