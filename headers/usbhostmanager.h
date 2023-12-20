#ifndef _USBHOSTMANAGER_H_
#define _USBHOSTMANAGER_H_

#include "usbaddon.h"
#include <vector>

#include "pio_usb.h"

#include "host/usbh_pvt.h"

// USB Host manager decides on TinyUSB Host driver
usbh_class_driver_t const* usbh_app_driver_get_cb(uint8_t *driver_count);

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
    void setDataPin(uint8_t); // start USB host (change CPU, setup PIO PICO usb pin)
    void start();
    void pushAddon(USBAddon *); // If anything needs to update in the gpconfig driver
    void process();
    void hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len);
    void hid_umount_cb(uint8_t daddr, uint8_t instance);
    void hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
    void hid_set_report_complete_cb(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
    void hid_get_report_complete_cb(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
    void xinput_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype);
    void xinput_umount_cb(uint8_t dev_addr);
    void xinput_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
    void xinput_report_sent_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
    
private:
    USBHostManager() : tuh_ready(false), core0Ready(false), core1Ready(false) {

    }
    std::vector<USBAddon*> addons;
    usb_device_t *usb_device;
    uint8_t dataPin;
    bool tuh_ready;
    bool core0Ready;
    bool core1Ready;
};

#endif