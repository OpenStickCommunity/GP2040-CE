#ifndef _XBONE_DATA_H_
#define _XBONE_DATA_H_

typedef enum {
    auth_idle_state = 0,
	send_auth_console_to_dongle = 1,
	send_auth_dongle_to_console = 2,
	wait_auth_console_to_dongle = 3,
	wait_auth_dongle_to_console = 4,
} XboxOneState;

// Storage manager for board, LED options, and thread-safe settings
class XboxOneData {
public:
	XboxOneData(XboxOneData const&) = delete;
	void operator=(XboxOneData const&)  = delete;
	static XboxOneData& getInstance() // Thread-safe storage ensures cross-thread talk
	{
		static XboxOneData instance;
		return instance;
	}

	void setState(XboxOneState newState) {
		xboneState = newState;
	}

	uint8_t * getAuthBuffer() {
		return authBuffer;
	}

	uint8_t getSequence() {
		return authSequence;
	}

	uint16_t getAuthLen() {
		return authLen;
	}

	uint8_t getAuthType() {
		return authType;
	}

	XboxOneState getState() {
		return xboneState;
	}

	void setAuthData(uint8_t * buf, uint16_t bufLen, uint8_t seq, uint8_t type, XboxOneState newState) {
		// Cannot copy larger than our buffer
		if ( bufLen > 1024) {
			return;
		}
		memcpy(authBuffer, buf, bufLen);
		authLen = bufLen;
		authType = type;
		authSequence = seq;
		xboneState = newState;
	}

	bool getAuthCompleted() {
		return authCompleted;
	}

	void setAuthCompleted(bool completed) {
		authCompleted = completed;
	}

private:
	XboxOneData() {
		xboneState = auth_idle_state;
		authLen = 0;
		authSequence = 0;
		authCompleted = false;
	}

	XboxOneState xboneState;

	// Console-to-Host e.g. Xbox One to MagicBoots
	//  Note: the Xbox One Passthrough can call send_xbone_report() directly but not the other way around
	bool authCompleted;

	// Auth Buffer
	uint8_t authBuffer[1024];
	uint8_t authSequence;
	uint16_t authLen;
	uint8_t authType;
};

#endif // _XBONE_DATA_H_
