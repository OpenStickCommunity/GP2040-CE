#ifndef _DRIVER_HELPER_H_
#define _DRIVER_HELPER_H_

static uint16_t * getStringDescriptor(const char * value, uint8_t index)
{
	static uint16_t descriptorStringBuffer[128]; // Max 256 bytes, 127 unicode characters
	size_t charCount;
	if ( index == 0 ) // language always has a character count of 1
		charCount = 1;
	else {
		charCount = strlen(value);
		if (charCount > 127)
			charCount = 127;
	}
	// Fill descriptionStringBuffer[1] .. [32]
	for (uint8_t i = 0; i < charCount; i++)
		descriptorStringBuffer[i + 1] = value[i];

	// first byte (descriptionStringBuffer[0]) is length (including header), second byte is string type
	descriptorStringBuffer[0] = (0x03 << 8) | (2 * (uint8_t)charCount + 2);

	// Cast temp buffer to final result
	return descriptorStringBuffer;
}

#endif // _DRIVER_HELPER_H_
