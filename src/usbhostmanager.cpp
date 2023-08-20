#include "usbhostmanager.h"

#include "pio_usb.h"
#include "tusb.h"
#include "host/usbh_classdriver.h"

void USBHostManager::setDataPin(uint8_t inPin) {
    dataPin = inPin;
}

void USBHostManager::start() {
    if ( !addons.empty() ) {
        pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
        pio_cfg.pin_dp = dataPin;
        pio_cfg.sm_tx = 1; // NeoPico uses PIO0:0, move to state machine 1
        tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);
        tuh_init(BOARD_TUH_RHPORT);
        sleep_us(10); // ensure we are ready
        tuh_ready = true;
    }
}

void USBHostManager::pushAddon(USBAddon * usbAddon) { // If anything needs to update in the gpconfig driver
    addons.push_back(usbAddon);
}

// Host manager should call tuh_task as fast as possible
void USBHostManager::process() {
    if ( tuh_ready ){
        tuh_task();
    }
}

void USBHostManager::hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    for( std::vector<USBAddon*>::iterator it = addons.begin(); it != addons.end(); it++ ){
        (*it)->mount(dev_addr, instance, desc_report, desc_len);
    }
}

void USBHostManager::hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    for( std::vector<USBAddon*>::iterator it = addons.begin(); it != addons.end(); it++ ){
        (*it)->unmount(dev_addr);
    }
}

void USBHostManager::hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    for( std::vector<USBAddon*>::iterator it = addons.begin(); it != addons.end(); it++ ){
        (*it)->report_received(dev_addr, instance, report, len);
    }
}

void USBHostManager::hid_set_report_complete_cb(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    for( std::vector<USBAddon*>::iterator it = addons.begin(); it != addons.end(); it++ ){
        (*it)->set_report_complete(dev_addr, instance, report_id, report_type, len);
    }
}

void USBHostManager::hid_get_report_complete_cb(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    for( std::vector<USBAddon*>::iterator it = addons.begin(); it != addons.end(); it++ ){
        (*it)->get_report_complete(dev_addr, instance, report_id, report_type, len);
    }
}

static uint8_t _intf_num = 0;

// Required helper class for HID_REQ_CONTROL_GET_REPORT addition
uint16_t count_interface_total_len(tusb_desc_interface_t const* desc_itf, uint8_t itf_count, uint16_t max_len)
{
  uint8_t const* p_desc = (uint8_t const*) desc_itf;
  uint16_t len = 0;

  while (itf_count--)
  {
    // Next on interface desc
    len += tu_desc_len(desc_itf);
    p_desc = tu_desc_next(p_desc);

    while (len < max_len)
    {
      // return on IAD regardless of itf count
      if ( tu_desc_type(p_desc) == TUSB_DESC_INTERFACE_ASSOCIATION ) return len;

      if ( (tu_desc_type(p_desc) == TUSB_DESC_INTERFACE) &&
           ((tusb_desc_interface_t const*) p_desc)->bAlternateSetting == 0 )
      {
        break;
      }

      len += tu_desc_len(p_desc);
      p_desc = tu_desc_next(p_desc);
    }
  }

  return len;
}

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
    // Get Interface Number for our HID class
    uint16_t temp_buf[128];
    tusb_desc_configuration_t const* desc_cfg;
    if (XFER_RESULT_SUCCESS == tuh_descriptor_get_configuration_sync(dev_addr, 0, temp_buf, sizeof(temp_buf)))
    {
        tusb_desc_configuration_t const* desc_cfg = (tusb_desc_configuration_t*) temp_buf;
        uint8_t const* desc_end = ((uint8_t const*) desc_cfg) + tu_le16toh(desc_cfg->wTotalLength);
        uint8_t const* p_desc   = tu_desc_next(desc_cfg);

        // parse each interfaces
        while( p_desc < desc_end )
        {
            uint8_t assoc_itf_count = 1;
            // Class will always starts with Interface Association (if any) and then Interface descriptor
            if ( TUSB_DESC_INTERFACE_ASSOCIATION == tu_desc_type(p_desc) )
            {
                tusb_desc_interface_assoc_t const * desc_iad = (tusb_desc_interface_assoc_t const *) p_desc;
                assoc_itf_count = desc_iad->bInterfaceCount;

                p_desc = tu_desc_next(p_desc); // next to Interface
            }

            // must be interface from now
            if( TUSB_DESC_INTERFACE != tu_desc_type(p_desc) ) return;
            tusb_desc_interface_t const* desc_itf = (tusb_desc_interface_t const*) p_desc;

            // only open and listen to HID endpoint IN
            if (desc_itf->bInterfaceClass == TUSB_CLASS_HID)
            {
                _intf_num = desc_itf->bInterfaceNumber;
                break; // we got the interface number
            }
            
            // next Interface or IAD descriptor
            uint16_t const drv_len = count_interface_total_len(desc_itf, assoc_itf_count, (uint16_t) (desc_end-p_desc));
            p_desc += drv_len;
        }
    } // This block can be removed once TinyUSB library incorporates HID_REQ_CONTROL_GET_REPORT callback

    USBHostManager::getInstance().hid_mount_cb(dev_addr, instance, desc_report, desc_len);
    if ( !tuh_hid_receive_report(dev_addr, instance) ) {
        // Error: cannot request report
    }
}

/// Invoked when device is unmounted (bus reset/unplugged)
void tuh_hid_umount_cb(uint8_t daddr, uint8_t instance)
{
    USBHostManager::getInstance().hid_umount_cb(daddr, instance);
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
    if ( len != 0 )
        USBHostManager::getInstance().hid_set_report_complete_cb(dev_addr, instance, report_id, report_type, len);
}


// GET REPORT FEATURE
void tuh_hid_get_report_complete_cb(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    if ( len != 0 )
        USBHostManager::getInstance().hid_get_report_complete_cb(dev_addr, instance, report_id, report_type, len);
}

// Request for HID_REQ_CONTROL_GET_REPORT missing from TinyUSB
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
    .wIndex   = _intf_num, // pulled in from tuh_hid_mount_cb()
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
