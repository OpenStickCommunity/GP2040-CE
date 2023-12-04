#ifndef _XBOnePassthrough_H
#define _XBOnePassthrough_H

#include "usbaddon.h"

//#include "ps4_driver.h"

#ifndef XBONEPASSTHROUGH_ENABLED
#define XBONEPASSTHROUGH_ENABLED 1
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
	virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);

	void send_host_report(void* report, uint16_t len);
private:
	void send_xbone_ack(uint8_t dev_addr, uint8_t instance, uint8_t sequence, uint16_t received, uint16_t total_size);

	uint8_t xbone_dev_addr;
	uint8_t xbone_instance;

	uint8_t xbone_ep_in;
	uint8_t xbone_ep_out;

	// We have to keep track of packet chunks if chunked is set
	uint16_t descriptor_size;
	uint16_t packet_chunk_received;

	bool awaiting_cb;
};

#endif  // _PSPassthrough_H_