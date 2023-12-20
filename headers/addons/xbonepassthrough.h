#ifndef _XBOnePassthrough_H
#define _XBOnePassthrough_H

#include "usbaddon.h"

#include "xgip_protocol.h"

#ifndef XBONEPASSTHROUGH_ENABLED
#define XBONEPASSTHROUGH_ENABLED 0
#endif

// KeyboardHost Module Name
#define XBOnePassthroughName "XBOnePassthrough"

class XBOnePassthroughAddon : public USBAddon {
public:
	virtual bool available();
	virtual void setup();       // XBOnePassthrough Setup
	virtual void process();     // XBOnePassthrough Process
	virtual void preprocess() {}
	virtual std::string name() { return XBOnePassthroughName; }
// USB Add-on Features
	virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {}
	virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype);
	virtual void unmount(uint8_t dev_addr);
	virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
	virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
	virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
	virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {}

	void queue_host_report(void* report, uint16_t len);
private:
	uint8_t xbone_dev_addr;
	uint8_t xbone_instance;

	bool dongle_ready;

	XGIPProtocol incomingXGIP;
	XGIPProtocol outgoingXGIP;
};

#endif  // _PSPassthrough_H_