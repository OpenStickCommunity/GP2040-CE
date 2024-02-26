#ifndef _PS4AUTHUSB_H_
#define _PS4AUTHUSB_H_

#include "drivers/shared/gpauthdriver.h"
#include "drivers/shared/ps4data.h"
#include "usblistener.h"

class PS4AuthUSB : public GPAuthDriver, public USBListener {
public:
    PS4AuthUSB() { authType = InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB; }
    virtual void initialize();
    virtual bool available();
    virtual void process();
    virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len);
	virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype){}
	virtual void unmount(uint8_t dev_addr);
	virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len){}
	virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len){}
	virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
	virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
private:
	uint8_t ps_dev_addr;
	uint8_t ps_instance;
	int8_t nonce_page;
	PS4State passthrough_state;
	int8_t send_nonce_part;
	uint8_t report_buffer[64];
	bool awaiting_cb;
};

#endif
