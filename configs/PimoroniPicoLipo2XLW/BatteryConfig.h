/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

// Minimal, dependency-free battery-sense definitions for the
// Pimoroni Pico Lipo 2 XL W. This header must not include anything
// that pulls in TinyUSB or BTstack — it is consumed by both BoardConfig.h
// (full firmware build) and src/BLEHIDManager.cpp (BTstack-only TU), and
// those two worlds disagree on `hid_report_type_t`.

#ifndef PIMORONI_PICO_LIPO_2_XL_W_BATTERY_CONFIG_H_
#define PIMORONI_PICO_LIPO_2_XL_W_BATTERY_CONFIG_H_

// Battery voltage monitoring (LiPo, GP43 / ADC channel 3 on RP2350B).
// The onboard 3:1 voltage divider routes battery voltage to ADC3.
// Actual battery voltage = ADC_voltage * BATTERY_VOLTAGE_DIVIDER
// LiPo range: 3.0 V (0%) to 4.2 V (100%)
//
// Usage:
//   adc_init();
//   adc_gpio_init(BATTERY_ADC_GPIO);
//   adc_select_input(BATTERY_ADC_CHANNEL);
//   uint16_t raw = adc_read();  // 12-bit, 0–4095
//   float v_adc = (raw / 4095.0f) * 3.3f;
//   float v_bat = v_adc * BATTERY_VOLTAGE_DIVIDER;
//   float pct   = (v_bat - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) * 100.0f;
#define BATTERY_ADC_GPIO         43
#define BATTERY_ADC_CHANNEL      3
#define BATTERY_VOLTAGE_DIVIDER  3.0f
#define BATTERY_MIN_VOLTAGE      3.0f
#define BATTERY_MAX_VOLTAGE      4.2f

#endif // PIMORONI_PICO_LIPO_2_XL_W_BATTERY_CONFIG_H_
