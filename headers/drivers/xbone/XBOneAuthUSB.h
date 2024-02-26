#ifndef _XBONEAUTHUSB_H_
#define _XBONE4AUTHUSB_H_

#include "drivers/shared/gpauthdriver.h"
#include "drivers/shared/xgip_protocol.h"
#include "usblistener.h"

class XBOneAuthUSB : public GPAuthDriver, public USBListener {
public:
    virtual void initialize();
    virtual bool available();
    virtual void process();
    virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len){}
	virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype);
	virtual void unmount(uint8_t dev_addr);
	virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
	virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len){}
	virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len){}
	virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len){}
private:
    void queue_host_report(void* report, uint16_t len);
	uint8_t xbone_dev_addr;
	uint8_t xbone_instance;
	bool dongle_ready;
	XGIPProtocol incomingXGIP;
	XGIPProtocol outgoingXGIP;
};

#endif
