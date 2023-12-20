#ifndef _USBAddon_H_
#define _USBAddon_H_

#include "gpaddon.h"

#include <string>

class USBAddon : public GPAddon
{
public:
	virtual ~USBAddon() { }
	virtual bool available() = 0;
	virtual void setup() = 0;
	virtual void process() = 0;
	virtual void preprocess() = 0;
	virtual std::string name() = 0;
	virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) = 0;
	virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) = 0;
	virtual void unmount(uint8_t dev_addr) = 0;
	virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) = 0;
	virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) = 0;
	virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) = 0;
	virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) = 0;
};

#endif