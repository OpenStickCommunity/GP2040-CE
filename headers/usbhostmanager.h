#ifndef _USBHOSTMANAGER_H_
#define _USBHOSTMANAGER_H_

#include "usbaddon.h"
#include <vector>

#include "pio_usb.h"

// Missing TinyUSB call
bool tuh_hid_get_report(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, void* report, uint16_t len);

class USBHostManager {
public:
	USBHostManager(USBHostManager const&) = delete;
	void operator=(USBHostManager const&)  = delete;
    static USBHostManager& getInstance() {// Thread-safe storage ensures cross-thread talk
		static USBHostManager instance; // Guaranteed to be destroyed. // Instantiated on first use.
		return instance;
	}
    void init(uint8_t);
    void pushAddon(USBAddon *); // If anything needs to update in the gpconfig driver
    void processCore0(); // do we separate Core0 and Core1?
    void processCore1();
    void hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len);
    void hid_umount_cb(uint8_t daddr, uint8_t instance);
    void hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
    void hid_set_report_complete_cb(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
    void hid_get_report_complete_cb(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
private:
    USBHostManager() {}
    std::vector<USBAddon*> addons;
    usb_device_t *usb_device;
};

#endif