/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 Open Stick Community (gp2040-ce.info)
 */

#ifndef _USBDRIVER_CPP_
#define _USBDRIVER_CPP_

#include "tusb.h"
#include "drivermanager.h"

static bool usb_mounted;
static bool usb_suspended;

bool get_usb_mounted(void) {
	return usb_mounted;
}

bool get_usb_suspended(void) {
	return usb_suspended;
}

const usbd_class_driver_t *usbd_app_driver_get_cb(uint8_t *driver_count) {
	*driver_count = 1;
	GPDriver * d = DriverManager::getInstance().getDriver();
	if (d == nullptr) {
		*driver_count = 0;
		return nullptr;
	}
	return d->get_class_driver();
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
	GPDriver * d = DriverManager::getInstance().getDriver();
	if (d == nullptr) return 0;
	return d->get_report(report_id, report_type, buffer, reqlen);
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
	GPDriver * d = DriverManager::getInstance().getDriver();
	if (d == nullptr) return;
	d->set_report(report_id, report_type, buffer, bufsize);
}

// Invoked when device is mounted
void tud_mount_cb(void)
{
	usb_mounted = true;
	usb_suspended = false;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
	usb_mounted = false;
	usb_suspended = false;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
	(void)remote_wakeup_en;
	usb_suspended = true;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
	usb_suspended = false;
}

// Vendor Controlled XFER occured
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage,
                                tusb_control_request_t const *request) {
	GPDriver * d = DriverManager::getInstance().getDriver();
	if (d == nullptr) return false;
	return d->vendor_control_xfer_cb(rhport, stage, request);
}


// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
	GPDriver * d = DriverManager::getInstance().getDriver();
	if (d == nullptr) return nullptr;
	return d->get_descriptor_string_cb(index, langid);
}

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb() {
	GPDriver * d = DriverManager::getInstance().getDriver();
	if (d == nullptr) return nullptr;
	return d->get_descriptor_device_cb();
}

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf) {
	GPDriver * d = DriverManager::getInstance().getDriver();
	if (d == nullptr) return nullptr;
	return d->get_hid_descriptor_report_cb(itf);
}

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
	GPDriver * d = DriverManager::getInstance().getDriver();
	if (d == nullptr) return nullptr;
	return d->get_descriptor_configuration_cb(index);
}

uint8_t const* tud_descriptor_device_qualifier_cb() {
	GPDriver * d = DriverManager::getInstance().getDriver();
	if (d == nullptr) return nullptr;
	return d->get_descriptor_device_qualifier_cb();
}

#endif
