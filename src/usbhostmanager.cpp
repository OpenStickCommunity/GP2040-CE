#include "usbhostmanager.h"

#include "tusb.h"
#include "host/usbh_classdriver.h"

void USBHostManager::init() {
}

void USBHostManager::setClock() {
    set_sys_clock_khz(120000, true); // Set Clock to 120MHz to avoid potential USB timing issues

    stdio_init_all();
    printf("Setting speed to 120khz!\r\n");
    // Adjust our clock speed for USB
}

void USBHostManager::pushAddon(USBAddon * usbAddon) { // If anything needs to update in the gpconfig driver
    printf("Pushing add-on %s\r\n", usbAddon->name().c_str());
    addons.push_back(usbAddon);
}

// Core 0 - USB host manager does nothing for now
void USBHostManager::processCore0() {
}

// Core 1 - USB host manager calls TinyUSB Host task
void USBHostManager::processCore1() {
    if ( !addons.empty() ){
        tuh_task();
    }
}

void USBHostManager::hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    printf("USB HID Mounted %i %i\r\n", dev_addr, instance);
    for( std::vector<USBAddon*>::iterator it = addons.begin(); it != addons.end(); it++ ){
        printf("Calling mount on add-on %s\r\n", (*it)->name().c_str());
        (*it)->mount(dev_addr, instance, desc_report, desc_len);
    }
}

void USBHostManager::hid_umount_cb(uint8_t daddr) {
    printf("USB HID unmounted %i\r\n", daddr);
    for( std::vector<USBAddon*>::iterator it = addons.begin(); it != addons.end(); it++ ){
        printf("Calling unmount on add-on %s\r\n", (*it)->name().c_str());
        (*it)->unmount(daddr);
    }
}

void USBHostManager::hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    //printf("Report Received!\r\n");
    for( std::vector<USBAddon*>::iterator it = addons.begin(); it != addons.end(); it++ ){
        //printf("Sending to add-on %s\r\n", (*it)->name());
        (*it)->report_received(dev_addr, instance, report, len);
    }
}

void USBHostManager::hid_set_report_complete_cb(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    printf("Set Report Received!\r\n");
    for( std::vector<USBAddon*>::iterator it = addons.begin(); it != addons.end(); it++ ){
        printf("Sending to add-on %s\r\n", (*it)->name().c_str());
        (*it)->set_report_complete(dev_addr, instance, report_id, report_type, len);
    }
}

void USBHostManager::hid_get_report_complete_cb(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    printf("Get Report Received!\r\n");
    for( std::vector<USBAddon*>::iterator it = addons.begin(); it != addons.end(); it++ ){
        printf("Sending to add-on %s\r\n", (*it)->name().c_str());
        (*it)->get_report_complete(dev_addr, instance, report_id, report_type, len);
    }
}

// TinyUSB Mount Callback
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
    USBHostManager::getInstance().hid_mount_cb(dev_addr, instance, desc_report, desc_len);

    if ( !tuh_hid_receive_report(dev_addr, instance) ) {
    // Error: cannot request report
    }
}

/// Invoked when device is unmounted (bus reset/unplugged)
void tuh_hid_umount_cb(uint8_t daddr)
{
    USBHostManager::getInstance().hid_umount_cb(daddr);
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
    USBHostManager::getInstance().hid_report_received_cb(dev_addr, instance, report, len);

    if ( !tuh_hid_receive_report(dev_addr, instance) ) {
        //Error: cannot request report
    }
}


// On IN/OUT/FEATURE set report callback
void tuh_hid_set_report_complete_cb(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    USBHostManager::getInstance().hid_set_report_complete_cb(dev_addr, instance, report_id, report_type, len);
}


// GET REPORT FEATURE
void tuh_hid_get_report_complete_cb(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    USBHostManager::getInstance().hid_get_report_complete_cb(dev_addr, instance, report_id, report_type, len);
}

// TinyUSB Calls missing from TinyUSB :(
static void get_report_complete(tuh_xfer_t* xfer)
{
  if (tuh_hid_get_report_complete_cb)
  {
    uint8_t const itf_num     = (uint8_t) tu_le16toh(xfer->setup->wIndex);
    uint8_t const instance    = 0;

    uint8_t const report_type = tu_u16_high(xfer->setup->wValue);
    uint8_t const report_id   = tu_u16_low(xfer->setup->wValue);

    tuh_hid_get_report_complete_cb(xfer->daddr, instance, report_id, report_type,
                                   (xfer->result == XFER_RESULT_SUCCESS) ? xfer->setup->wLength : 0);
  }
}

bool tuh_hid_get_report(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, void* report, uint16_t len)
{
  tusb_control_request_t const request =
  {
    .bmRequestType_bit =
    {
      .recipient = TUSB_REQ_RCPT_INTERFACE,
      .type      = TUSB_REQ_TYPE_CLASS,
      .direction = TUSB_DIR_IN
    },
    .bRequest = HID_REQ_CONTROL_GET_REPORT,
    .wValue   = tu_u16(report_type, report_id),
    .wIndex   = 0, //hid_itf->itf_num,
    .wLength  = len
  };

  tuh_xfer_t xfer =
  {
    .daddr       = dev_addr,
    .ep_addr     = 0,
    .setup       = &request,
    .buffer      = (uint8_t*)report,
    .complete_cb = get_report_complete,
    .user_data   = 0
  };

  TU_ASSERT( tuh_control_xfer(&xfer) );
  return true;
}
