#ifndef _USB_DRIVER_H_
#define _USB_DRIVER_H_

#include <stdint.h>

bool get_usb_mounted(void);
bool get_usb_suspended(void);
void request_remote_wakeup(uint32_t duration_ms = 500);
bool is_remote_wakeup_requested(void);

#endif // #ifndef _USB_DRIVER_H_