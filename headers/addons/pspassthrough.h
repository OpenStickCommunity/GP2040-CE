#ifndef _PSPassthrough_H
#define _PSPassthrough_H

#include "usbaddon.h"

#include "ps4_driver.h"

#ifndef PSPASSTHROUGH_ENABLED
#define PSPASSTHROUGH_ENABLED 0
#endif

#ifndef PSPASSTHROUGH_PIN_DPLUS
#define PSPASSTHROUGH_PIN_DPLUS -1
#endif

#ifndef PSPASSTHROUGH_PIN_5V
#define PSPASSTHROUGH_PIN_5V -1
#endif

// KeyboardHost Module Name
#define PSPassthroughName "PSPassthrough"

class PSPassthroughAddon : public USBAddon {
public:
	virtual bool available();
	virtual void setup();       // PSPassthrough Setup
	virtual void process();     // PSPassthrough Process
	virtual void preprocess() {}
	virtual std::string name() { return PSPassthroughName; }
// USB Add-on Features
	virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len);
	virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {}
	virtual void unmount(uint8_t dev_addr);
	virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
	virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
	virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {}
	virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {}
private:
	uint8_t ps_dev_addr;
	uint8_t ps_instance;
	int8_t nonce_page;
	PS4State passthrough_state;
	int8_t send_nonce_part;
	uint8_t report_buffer[64];
	bool awaiting_cb;
};

#endif  // _PSPassthrough_H_