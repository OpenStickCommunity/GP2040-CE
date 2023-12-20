/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 OpenStickCommunity (gp2040-ce.info)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */

#ifndef _TUSB_XINPUT_HOST_H_
#define _TUSB_XINPUT_HOST_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------+
// Class Driver Configuration
//--------------------------------------------------------------------+

// TODO Highspeed interrupt can be up to 512 bytes
#ifndef CFG_TUH_XINPUT_EPIN_BUFSIZE
#define CFG_TUH_XINPUT_EPIN_BUFSIZE 64
#endif

#ifndef CFG_TUH_XINPUT_EPOUT_BUFSIZE
#define CFG_TUH_XINPUT_EPOUT_BUFSIZE 64
#endif

//--------------------------------------------------------------------+
// Interface API
//--------------------------------------------------------------------+

// Get the number of XINPUT instances
uint8_t tuh_xinput_instance_count(uint8_t dev_addr);

// Check if XINPUT instance is mounted
bool tuh_xinput_mounted(uint8_t dev_addr, uint8_t instance);

//--------------------------------------------------------------------+
// Interrupt Endpoint API
//--------------------------------------------------------------------+

// Try to receive next report on Interrupt Endpoint. Immediately return
// - true If succeeded, tuh_xinput_report_received_cb() callback will be invoked when report is available
// - false if failed to queue the transfer e.g endpoint is busy
bool tuh_xinput_receive_report(uint8_t dev_addr, uint8_t instance);

//--------------------------------------------------------------------+
// Callbacks (Weak is optional)
//--------------------------------------------------------------------+

// Invoked when device with XINPUT interface is mounted
// Report descriptor is also available for use. tuh_xinput_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_xinput_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t type, uint8_t subtype);
bool tuh_xinput_ready(uint8_t dev_addr, uint8_t instance);
bool tuh_xinput_send_report(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len);

// Invoked when device with XINPUT interface is un-mounted
TU_ATTR_WEAK void tuh_xinput_umount_cb(uint8_t dev_addr, uint8_t instance);

// Invoked when received report from device via interrupt endpoint
// Note: if there is report ID (composite), it is 1st byte of report
void tuh_xinput_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);

// Invoked when sent report to device successfully via interrupt endpoint
TU_ATTR_WEAK void tuh_xinput_report_sent_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);

//--------------------------------------------------------------------+
// Internal Class Driver API
//--------------------------------------------------------------------+
void xinputh_init(void);
bool xinputh_open(uint8_t rhport, uint8_t dev_addr, tusb_desc_interface_t const* desc_itf, uint16_t max_len);
bool xinputh_set_config(uint8_t dev_addr, uint8_t itf_num);
bool xinputh_xfer_cb(uint8_t dev_addr, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes);
void xinputh_close(uint8_t dev_addr);
#ifdef __cplusplus
}
#endif

#endif /* _TUSB_XINPUT_HOST_H_ */
