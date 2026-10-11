#ifndef _HE_Trigger_H
#define _HE_Trigger_H

#include "gpaddon.h"

#include "pico/time.h"

#define HETRIGGER_COUNT 32

#ifndef HETRIGGER_ENABLED
#define HETRIGGER_ENABLED 0
#endif

#ifndef HETRIGGER_MUX_CHANNELS
#define HETRIGGER_MUX_CHANNELS 8
#endif

#ifndef HETRIGGER_S0_PIN
#define HETRIGGER_S0_PIN -1
#endif
#ifndef HETRIGGER_S1_PIN
#define HETRIGGER_S1_PIN -1
#endif
#ifndef HETRIGGER_S2_PIN
#define HETRIGGER_S2_PIN -1
#endif
#ifndef HETRIGGER_S3_PIN
#define HETRIGGER_S3_PIN -1
#endif

#ifndef HETRIGGER_SEPARATE_SELECT_PINS
#define HETRIGGER_SEPARATE_SELECT_PINS 0
#endif

#ifndef HETRIGGER_MUX1_S0_PIN
#define HETRIGGER_MUX1_S0_PIN -1
#endif
#ifndef HETRIGGER_MUX1_S1_PIN
#define HETRIGGER_MUX1_S1_PIN -1
#endif
#ifndef HETRIGGER_MUX1_S2_PIN
#define HETRIGGER_MUX1_S2_PIN -1
#endif
#ifndef HETRIGGER_MUX1_S3_PIN
#define HETRIGGER_MUX1_S3_PIN -1
#endif

#ifndef HETRIGGER_MUX2_S0_PIN
#define HETRIGGER_MUX2_S0_PIN -1
#endif
#ifndef HETRIGGER_MUX2_S1_PIN
#define HETRIGGER_MUX2_S1_PIN -1
#endif
#ifndef HETRIGGER_MUX2_S2_PIN
#define HETRIGGER_MUX2_S2_PIN -1
#endif
#ifndef HETRIGGER_MUX2_S3_PIN
#define HETRIGGER_MUX2_S3_PIN -1
#endif

#ifndef HETRIGGER_MUX3_S0_PIN
#define HETRIGGER_MUX3_S0_PIN -1
#endif
#ifndef HETRIGGER_MUX3_S1_PIN
#define HETRIGGER_MUX3_S1_PIN -1
#endif
#ifndef HETRIGGER_MUX3_S2_PIN
#define HETRIGGER_MUX3_S2_PIN -1
#endif
#ifndef HETRIGGER_MUX3_S3_PIN
#define HETRIGGER_MUX3_S3_PIN -1
#endif

#ifndef HETRIGGER_ADC0
#define HETRIGGER_ADC0 -1
#endif
#ifndef HETRIGGER_ADC1
#define HETRIGGER_ADC1 -1
#endif
#ifndef HETRIGGER_ADC2
#define HETRIGGER_ADC2 -1
#endif
#ifndef HETRIGGER_ADC3
#define HETRIGGER_ADC3 -1
#endif

#ifndef HETRIGGER_SMOOTHING_ENABLED
#define HETRIGGER_SMOOTHING_ENABLED 1
#endif

#ifndef HETRIGGER_SMOOTHING_FACTOR
#define HETRIGGER_SMOOTHING_FACTOR 5
#endif

#ifndef HETRIGGER_DEFAULT_IDLE
#define HETRIGGER_DEFAULT_IDLE 150
#endif

#ifndef HETRIGGER_DEFAULT_ACTIVE
#define HETRIGGER_DEFAULT_ACTIVE 2000
#endif

#ifndef HETRIGGER_DEFAULT_PRESSED
#define HETRIGGER_DEFAULT_PRESSED 3500
#endif

#ifndef HETRIGGER_DEFAULT_POLARITY
#define HETRIGGER_DEFAULT_POLARITY 0
#endif

#ifndef HETRIGGER_DEFAULT_RELEASE
#define HETRIGGER_DEFAULT_RELEASE 2000
#endif

#ifndef HETRIGGER_DEFAULT_NOISE
#define HETRIGGER_DEFAULT_NOISE 30
#endif

#ifndef HETRIGGER_DEFAULT_RAPID
#define HETRIGGER_DEFAULT_RAPID 0
#endif

// Rapid trigger v2 defaults, in percent of travel. These match the "Standard"
// calibration preset (see HE_PRESETS below).
#ifndef HETRIGGER_DEFAULT_ACTUATION
#define HETRIGGER_DEFAULT_ACTUATION 35
#endif

#ifndef HETRIGGER_DEFAULT_RT_PRESS
#define HETRIGGER_DEFAULT_RT_PRESS 10
#endif

#ifndef HETRIGGER_DEFAULT_RT_RELEASE
#define HETRIGGER_DEFAULT_RT_RELEASE 10
#endif

#ifndef HETRIGGER_DEFAULT_RT_CONTINUOUS
#define HETRIGGER_DEFAULT_RT_CONTINUOUS 0
#endif

#ifndef HETRIGGER_DEFAULT_DEADZONE
#define HETRIGGER_DEFAULT_DEADZONE 3
#endif

// 32 possible HE triggers
#ifndef HETRIGGER_HE0_ACTION
#define HETRIGGER_HE0_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE0_ACTIVE
#define HETRIGGER_HE0_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE0_IDLE
#define HETRIGGER_HE0_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE0_PRESSED
#define HETRIGGER_HE0_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE0_POLARITY
#define HETRIGGER_HE0_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE0_RELEASE
#define HETRIGGER_HE0_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE0_NOISE
#define HETRIGGER_HE0_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE0_RAPID
#define HETRIGGER_HE0_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE1_ACTION
#define HETRIGGER_HE1_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE1_ACTIVE
#define HETRIGGER_HE1_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE1_IDLE
#define HETRIGGER_HE1_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE1_PRESSED
#define HETRIGGER_HE1_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE1_POLARITY
#define HETRIGGER_HE1_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE1_RELEASE
#define HETRIGGER_HE1_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE1_NOISE
#define HETRIGGER_HE1_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE1_RAPID
#define HETRIGGER_HE1_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE2_ACTION
#define HETRIGGER_HE2_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE2_ACTIVE
#define HETRIGGER_HE2_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE2_IDLE
#define HETRIGGER_HE2_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE2_PRESSED
#define HETRIGGER_HE2_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE2_POLARITY
#define HETRIGGER_HE2_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE2_RELEASE
#define HETRIGGER_HE2_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE2_NOISE
#define HETRIGGER_HE2_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE2_RAPID
#define HETRIGGER_HE2_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE3_ACTION
#define HETRIGGER_HE3_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE3_ACTIVE
#define HETRIGGER_HE3_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE3_IDLE
#define HETRIGGER_HE3_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE3_PRESSED
#define HETRIGGER_HE3_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE3_POLARITY
#define HETRIGGER_HE3_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE3_RELEASE
#define HETRIGGER_HE3_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE3_NOISE
#define HETRIGGER_HE3_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE3_RAPID
#define HETRIGGER_HE3_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE4_ACTION
#define HETRIGGER_HE4_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE4_ACTIVE
#define HETRIGGER_HE4_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE4_IDLE
#define HETRIGGER_HE4_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE4_PRESSED
#define HETRIGGER_HE4_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE4_POLARITY
#define HETRIGGER_HE4_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE4_RELEASE
#define HETRIGGER_HE4_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE4_NOISE
#define HETRIGGER_HE4_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE4_RAPID
#define HETRIGGER_HE4_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE5_ACTION
#define HETRIGGER_HE5_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE5_ACTIVE
#define HETRIGGER_HE5_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE5_IDLE
#define HETRIGGER_HE5_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE5_PRESSED
#define HETRIGGER_HE5_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE5_POLARITY
#define HETRIGGER_HE5_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE5_RELEASE
#define HETRIGGER_HE5_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE5_NOISE
#define HETRIGGER_HE5_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE5_RAPID
#define HETRIGGER_HE5_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE6_ACTION
#define HETRIGGER_HE6_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE6_ACTIVE
#define HETRIGGER_HE6_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE6_IDLE
#define HETRIGGER_HE6_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE6_PRESSED
#define HETRIGGER_HE6_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE6_POLARITY
#define HETRIGGER_HE6_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE6_RELEASE
#define HETRIGGER_HE6_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE6_NOISE
#define HETRIGGER_HE6_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE6_RAPID
#define HETRIGGER_HE6_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE7_ACTION
#define HETRIGGER_HE7_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE7_ACTIVE
#define HETRIGGER_HE7_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE7_IDLE
#define HETRIGGER_HE7_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE7_PRESSED
#define HETRIGGER_HE7_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE7_POLARITY
#define HETRIGGER_HE7_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE7_RELEASE
#define HETRIGGER_HE7_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE7_NOISE
#define HETRIGGER_HE7_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE7_RAPID
#define HETRIGGER_HE7_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE8_ACTION
#define HETRIGGER_HE8_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE8_ACTIVE
#define HETRIGGER_HE8_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE8_IDLE
#define HETRIGGER_HE8_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE8_PRESSED
#define HETRIGGER_HE8_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE8_POLARITY
#define HETRIGGER_HE8_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE8_RELEASE
#define HETRIGGER_HE8_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE8_NOISE
#define HETRIGGER_HE8_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE8_RAPID
#define HETRIGGER_HE8_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE9_ACTION
#define HETRIGGER_HE9_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE9_ACTIVE
#define HETRIGGER_HE9_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE9_IDLE
#define HETRIGGER_HE9_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE9_PRESSED
#define HETRIGGER_HE9_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE9_POLARITY
#define HETRIGGER_HE9_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE9_RELEASE
#define HETRIGGER_HE9_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE9_NOISE
#define HETRIGGER_HE9_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE9_RAPID
#define HETRIGGER_HE9_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE10_ACTION
#define HETRIGGER_HE10_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE10_ACTIVE
#define HETRIGGER_HE10_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE10_IDLE
#define HETRIGGER_HE10_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE10_PRESSED
#define HETRIGGER_HE10_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE10_POLARITY
#define HETRIGGER_HE10_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE10_RELEASE
#define HETRIGGER_HE10_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE10_NOISE
#define HETRIGGER_HE10_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE10_RAPID
#define HETRIGGER_HE10_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE11_ACTION
#define HETRIGGER_HE11_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE11_ACTIVE
#define HETRIGGER_HE11_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE11_IDLE
#define HETRIGGER_HE11_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE11_PRESSED
#define HETRIGGER_HE11_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE11_POLARITY
#define HETRIGGER_HE11_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE11_RELEASE
#define HETRIGGER_HE11_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE11_NOISE
#define HETRIGGER_HE11_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE11_RAPID
#define HETRIGGER_HE11_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE12_ACTION
#define HETRIGGER_HE12_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE12_ACTIVE
#define HETRIGGER_HE12_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE12_IDLE
#define HETRIGGER_HE12_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE12_PRESSED
#define HETRIGGER_HE12_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE12_POLARITY
#define HETRIGGER_HE12_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE12_RELEASE
#define HETRIGGER_HE12_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE12_NOISE
#define HETRIGGER_HE12_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE12_RAPID
#define HETRIGGER_HE12_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE13_ACTION
#define HETRIGGER_HE13_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE13_ACTIVE
#define HETRIGGER_HE13_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE13_IDLE
#define HETRIGGER_HE13_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE13_PRESSED
#define HETRIGGER_HE13_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE13_POLARITY
#define HETRIGGER_HE13_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE13_RELEASE
#define HETRIGGER_HE13_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE13_NOISE
#define HETRIGGER_HE13_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE13_RAPID
#define HETRIGGER_HE13_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE14_ACTION
#define HETRIGGER_HE14_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE14_ACTIVE
#define HETRIGGER_HE14_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE14_IDLE
#define HETRIGGER_HE14_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE14_PRESSED
#define HETRIGGER_HE14_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE14_POLARITY
#define HETRIGGER_HE14_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE14_RELEASE
#define HETRIGGER_HE14_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE14_NOISE
#define HETRIGGER_HE14_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE14_RAPID
#define HETRIGGER_HE14_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE15_ACTION
#define HETRIGGER_HE15_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE15_ACTIVE
#define HETRIGGER_HE15_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE15_IDLE
#define HETRIGGER_HE15_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE15_PRESSED
#define HETRIGGER_HE15_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE15_POLARITY
#define HETRIGGER_HE15_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE15_RELEASE
#define HETRIGGER_HE15_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE15_NOISE
#define HETRIGGER_HE15_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE15_RAPID
#define HETRIGGER_HE15_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE16_ACTION
#define HETRIGGER_HE16_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE16_ACTIVE
#define HETRIGGER_HE16_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE16_IDLE
#define HETRIGGER_HE16_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE16_PRESSED
#define HETRIGGER_HE16_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE16_POLARITY
#define HETRIGGER_HE16_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE16_RELEASE
#define HETRIGGER_HE16_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE16_NOISE
#define HETRIGGER_HE16_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE16_RAPID
#define HETRIGGER_HE16_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE17_ACTION
#define HETRIGGER_HE17_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE17_ACTIVE
#define HETRIGGER_HE17_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE17_IDLE
#define HETRIGGER_HE17_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE17_PRESSED
#define HETRIGGER_HE17_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE17_POLARITY
#define HETRIGGER_HE17_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE17_RELEASE
#define HETRIGGER_HE17_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE17_NOISE
#define HETRIGGER_HE17_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE17_RAPID
#define HETRIGGER_HE17_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE18_ACTION
#define HETRIGGER_HE18_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE18_ACTIVE
#define HETRIGGER_HE18_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE18_IDLE
#define HETRIGGER_HE18_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE18_PRESSED
#define HETRIGGER_HE18_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE18_POLARITY
#define HETRIGGER_HE18_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE18_RELEASE
#define HETRIGGER_HE18_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE18_NOISE
#define HETRIGGER_HE18_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE18_RAPID
#define HETRIGGER_HE18_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE19_ACTION
#define HETRIGGER_HE19_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE19_ACTIVE
#define HETRIGGER_HE19_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE19_IDLE
#define HETRIGGER_HE19_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE19_PRESSED
#define HETRIGGER_HE19_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE19_POLARITY
#define HETRIGGER_HE19_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE19_RELEASE
#define HETRIGGER_HE19_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE19_NOISE
#define HETRIGGER_HE19_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE19_RAPID
#define HETRIGGER_HE19_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE20_ACTION
#define HETRIGGER_HE20_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE20_ACTIVE
#define HETRIGGER_HE20_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE20_IDLE
#define HETRIGGER_HE20_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE20_PRESSED
#define HETRIGGER_HE20_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE20_POLARITY
#define HETRIGGER_HE20_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE20_RELEASE
#define HETRIGGER_HE20_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE20_NOISE
#define HETRIGGER_HE20_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE20_RAPID
#define HETRIGGER_HE20_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE21_ACTION
#define HETRIGGER_HE21_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE21_ACTIVE
#define HETRIGGER_HE21_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE21_IDLE
#define HETRIGGER_HE21_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE21_PRESSED
#define HETRIGGER_HE21_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE21_POLARITY
#define HETRIGGER_HE21_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE21_RELEASE
#define HETRIGGER_HE21_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE21_NOISE
#define HETRIGGER_HE21_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE21_RAPID
#define HETRIGGER_HE21_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE22_ACTION
#define HETRIGGER_HE22_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE22_ACTIVE
#define HETRIGGER_HE22_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE22_IDLE
#define HETRIGGER_HE22_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE22_PRESSED
#define HETRIGGER_HE22_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE22_POLARITY
#define HETRIGGER_HE22_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE22_RELEASE
#define HETRIGGER_HE22_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE22_NOISE
#define HETRIGGER_HE22_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE22_RAPID
#define HETRIGGER_HE22_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE23_ACTION
#define HETRIGGER_HE23_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE23_ACTIVE
#define HETRIGGER_HE23_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE23_IDLE
#define HETRIGGER_HE23_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE23_PRESSED
#define HETRIGGER_HE23_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE23_POLARITY
#define HETRIGGER_HE23_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE23_RELEASE
#define HETRIGGER_HE23_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE23_NOISE
#define HETRIGGER_HE23_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE23_RAPID
#define HETRIGGER_HE23_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE24_ACTION
#define HETRIGGER_HE24_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE24_ACTIVE
#define HETRIGGER_HE24_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE24_IDLE
#define HETRIGGER_HE24_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE24_PRESSED
#define HETRIGGER_HE24_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE24_POLARITY
#define HETRIGGER_HE24_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE24_RELEASE
#define HETRIGGER_HE24_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE24_NOISE
#define HETRIGGER_HE24_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE24_RAPID
#define HETRIGGER_HE24_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE25_ACTION
#define HETRIGGER_HE25_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE25_ACTIVE
#define HETRIGGER_HE25_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE25_IDLE
#define HETRIGGER_HE25_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE25_PRESSED
#define HETRIGGER_HE25_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE25_POLARITY
#define HETRIGGER_HE25_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE25_RELEASE
#define HETRIGGER_HE25_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE25_NOISE
#define HETRIGGER_HE25_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE25_RAPID
#define HETRIGGER_HE25_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE26_ACTION
#define HETRIGGER_HE26_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE26_ACTIVE
#define HETRIGGER_HE26_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE26_IDLE
#define HETRIGGER_HE26_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE26_PRESSED
#define HETRIGGER_HE26_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE26_POLARITY
#define HETRIGGER_HE26_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE26_RELEASE
#define HETRIGGER_HE26_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE26_NOISE
#define HETRIGGER_HE26_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE26_RAPID
#define HETRIGGER_HE26_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE27_ACTION
#define HETRIGGER_HE27_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE27_ACTIVE
#define HETRIGGER_HE27_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE27_IDLE
#define HETRIGGER_HE27_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE27_PRESSED
#define HETRIGGER_HE27_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE27_POLARITY
#define HETRIGGER_HE27_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE27_RELEASE
#define HETRIGGER_HE27_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE27_NOISE
#define HETRIGGER_HE27_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE27_RAPID
#define HETRIGGER_HE27_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE28_ACTION
#define HETRIGGER_HE28_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE28_ACTIVE
#define HETRIGGER_HE28_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE28_IDLE
#define HETRIGGER_HE28_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE28_PRESSED
#define HETRIGGER_HE28_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE28_POLARITY
#define HETRIGGER_HE28_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE28_RELEASE
#define HETRIGGER_HE28_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE28_NOISE
#define HETRIGGER_HE28_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE28_RAPID
#define HETRIGGER_HE28_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE29_ACTION
#define HETRIGGER_HE29_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE29_ACTIVE
#define HETRIGGER_HE29_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE29_IDLE
#define HETRIGGER_HE29_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE29_PRESSED
#define HETRIGGER_HE29_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE29_POLARITY
#define HETRIGGER_HE29_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE29_RELEASE
#define HETRIGGER_HE29_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE29_NOISE
#define HETRIGGER_HE29_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE29_RAPID
#define HETRIGGER_HE29_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE30_ACTION
#define HETRIGGER_HE30_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE30_ACTIVE
#define HETRIGGER_HE30_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE30_IDLE
#define HETRIGGER_HE30_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE30_PRESSED
#define HETRIGGER_HE30_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE30_POLARITY
#define HETRIGGER_HE30_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE30_RELEASE
#define HETRIGGER_HE30_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE30_NOISE
#define HETRIGGER_HE30_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE30_RAPID
#define HETRIGGER_HE30_RAPID HETRIGGER_DEFAULT_RAPID
#endif
#ifndef HETRIGGER_HE31_ACTION
#define HETRIGGER_HE31_ACTION GpioAction::NONE
#endif
#ifndef HETRIGGER_HE31_ACTIVE
#define HETRIGGER_HE31_ACTIVE HETRIGGER_DEFAULT_ACTIVE
#endif
#ifndef HETRIGGER_HE31_IDLE
#define HETRIGGER_HE31_IDLE HETRIGGER_DEFAULT_IDLE
#endif
#ifndef HETRIGGER_HE31_PRESSED
#define HETRIGGER_HE31_PRESSED HETRIGGER_DEFAULT_PRESSED
#endif
#ifndef HETRIGGER_HE31_POLARITY
#define HETRIGGER_HE31_POLARITY HETRIGGER_DEFAULT_POLARITY
#endif
#ifndef HETRIGGER_HE31_RELEASE
#define HETRIGGER_HE31_RELEASE HETRIGGER_DEFAULT_RELEASE
#endif
#ifndef HETRIGGER_HE31_NOISE
#define HETRIGGER_HE31_NOISE HETRIGGER_DEFAULT_NOISE
#endif
#ifndef HETRIGGER_HE31_RAPID
#define HETRIGGER_HE31_RAPID HETRIGGER_DEFAULT_RAPID
#endif

// Per-channel board-config defaults, gathered into one table so that config
// hydration can loop instead of unrolling 32 blocks by hand. Board configs
// (e.g. configs/GranolaBeacon/BoardConfig.h) override the macros above; this
// table just collects whatever they resolved to.
#define HETRIGGER_DEFAULT_ENTRY(n) \
    { HETRIGGER_HE##n##_ACTION, HETRIGGER_HE##n##_ACTIVE, HETRIGGER_HE##n##_IDLE, \
      HETRIGGER_HE##n##_PRESSED, HETRIGGER_HE##n##_POLARITY, HETRIGGER_HE##n##_RELEASE, \
      HETRIGGER_HE##n##_NOISE, HETRIGGER_HE##n##_RAPID }

struct HETriggerDefaults {
    int32_t action;
    int32_t active;
    int32_t idle;
    int32_t pressed;
    int32_t polarity;
    int32_t release;
    int32_t noise;
    int32_t rapid;
};

static const HETriggerDefaults HE_TRIGGER_DEFAULTS[HETRIGGER_COUNT] = {
    HETRIGGER_DEFAULT_ENTRY(0),  HETRIGGER_DEFAULT_ENTRY(1),  HETRIGGER_DEFAULT_ENTRY(2),
    HETRIGGER_DEFAULT_ENTRY(3),  HETRIGGER_DEFAULT_ENTRY(4),  HETRIGGER_DEFAULT_ENTRY(5),
    HETRIGGER_DEFAULT_ENTRY(6),  HETRIGGER_DEFAULT_ENTRY(7),  HETRIGGER_DEFAULT_ENTRY(8),
    HETRIGGER_DEFAULT_ENTRY(9),  HETRIGGER_DEFAULT_ENTRY(10), HETRIGGER_DEFAULT_ENTRY(11),
    HETRIGGER_DEFAULT_ENTRY(12), HETRIGGER_DEFAULT_ENTRY(13), HETRIGGER_DEFAULT_ENTRY(14),
    HETRIGGER_DEFAULT_ENTRY(15), HETRIGGER_DEFAULT_ENTRY(16), HETRIGGER_DEFAULT_ENTRY(17),
    HETRIGGER_DEFAULT_ENTRY(18), HETRIGGER_DEFAULT_ENTRY(19), HETRIGGER_DEFAULT_ENTRY(20),
    HETRIGGER_DEFAULT_ENTRY(21), HETRIGGER_DEFAULT_ENTRY(22), HETRIGGER_DEFAULT_ENTRY(23),
    HETRIGGER_DEFAULT_ENTRY(24), HETRIGGER_DEFAULT_ENTRY(25), HETRIGGER_DEFAULT_ENTRY(26),
    HETRIGGER_DEFAULT_ENTRY(27), HETRIGGER_DEFAULT_ENTRY(28), HETRIGGER_DEFAULT_ENTRY(29),
    HETRIGGER_DEFAULT_ENTRY(30), HETRIGGER_DEFAULT_ENTRY(31),
};

// HE-local pseudo-actions. These live in the same int32 `action` field as
// GpioAction values but sit far above GpioAction's range (max 130), so they can
// never collide. They are inert to the gamepad -- applyAction() ignores them --
// and are handled explicitly before the action switch. Deliberately NOT
// GpioAction values: a positive GpioAction would show up in the GPIO pin
// mapping UI and flow into GPIO init paths, and HE channels are not GPIOs.
// Analog directions can be bound in two ways. The plain GpioAction snaps the
// axis to full tilt on actuation; the same action plus this offset drives the
// axis proportionally to how far the switch is pressed. Encoding it in the
// action keeps it a per-binding choice, so it works per profile like anything
// else, rather than needing a separate per-switch flag.
#define HE_ANALOG_PROPORTIONAL_OFFSET 2000

#define HE_ACTION_PROFILE_CYCLE 1000
#define HE_ACTION_PROFILE_1     1001
#define HE_ACTION_PROFILE_2     1002
#define HE_ACTION_PROFILE_3     1003
#define HE_ACTION_PROFILE_4     1004

// Total HE binding profiles: base (triggers[].action) + profileSets[0..2].
#define HE_PROFILE_COUNT 4

// How long to wait after the last profile change before persisting it. Storage
// saves are not debounced and rewrite the whole config block, so this keeps a
// burst of cycling from turning into a burst of flash erases.
#ifndef HETRIGGER_PROFILE_SAVE_DELAY_MS
#define HETRIGGER_PROFILE_SAVE_DELAY_MS 10000
#endif

// Calibration tuning. The idle window needs enough samples for a stable mean and
// standard deviation; at roughly 1kHz per channel two seconds is ~2000 samples.
#define HETRIGGER_CAL_IDLE_MS 2000
// A channel counts as "pressed" once it deviates well clear of its own noise. The
// absolute floor keeps an unusually quiet channel from latching on a light brush.
#define HETRIGGER_CAL_MOVED_SIGMA 10
#define HETRIGGER_CAL_MOVED_FLOOR 300
// Idle noise beyond this suggests a wiring problem rather than a real signal.
#define HETRIGGER_CAL_UNSTABLE_STDDEV 100
// Calibration suppresses gamepad output, so a user who wanders off mid-session
// would otherwise be left with a dead controller.
#define HETRIGGER_CAL_TIMEOUT_MS 300000

// Tuning that a profile may override per channel.
enum class HETuningField : uint8_t {
    RAPID_TRIGGER = 0,
    ACTUATION_POINT = 1,
    RT_PRESS = 2,
    RT_RELEASE = 3,
};

enum class HECalMode : uint8_t {
    OFF = 0,
    IDLE_BASELINE = 1,
    PRESS_CAPTURE = 2,
    DONE = 3,
    // Live monitoring for the test view: samples and runs the normal actuation
    // logic so the UI can show real travel and trigger state, but produces no
    // gamepad input. Shares the sweep with calibration since the config-mode loop
    // has to tick it either way.
    MONITOR = 4,
};

// Per-channel accumulators for the calibration sweep.
struct HECalChannel {
    uint32_t sampleCount;
    uint64_t sum;           // for the idle mean
    uint64_t sumSquares;    // for the idle standard deviation
    uint16_t idleMean;
    uint16_t idleStdDev;
    uint16_t lastRaw;
    int32_t  maxDeviation;  // signed: the sign is what reveals switch polarity
    bool     moved;
    bool     unstable;
};

// HETrigger Module Name
#define HETriggerAddonName "Hall Effect Trigger"

class HETriggerAddon : public GPAddon {
public:
    // The web config handlers need to drive calibration, but AddonManager is a
    // private member of GP2040 and is not reachable from webconfig.cpp. Rather
    // than duplicate the mux addressing logic there (which is what the previous
    // calibration path did, and it drifted out of sync), the addon publishes
    // itself here on construction.
    static HETriggerAddon* getInstance() { return instance; }

    HETriggerAddon() { instance = this; }
    // AddonManager::LoadAddon deletes the addon when available() is false, so the
    // pointer must be cleared here or the web handlers would dereference freed
    // memory whenever the addon is disabled.
    ~HETriggerAddon() { if (instance == this) instance = nullptr; }

    virtual bool available();
    virtual void setup();
    virtual void process() {}
    virtual void preprocess();
    virtual void postprocess(bool sent) {}
    virtual void reinit();
    virtual std::string name() { return HETriggerAddonName; }

    uint8_t getActiveProfile() { return activeProfile; }
    void setHEProfile(uint8_t profile);
    void cycleHEProfile();

    // --- calibration, driven by the web config wizard ---
    // The sweep runs here rather than in webconfig.cpp so it samples at full loop
    // speed. A 32-channel sweep over HTTP would take most of a second per pass,
    // which is longer than a button press, so presses would simply be missed.
    void startCalibration();
    // Samples every assigned channel once. Called from preprocess() in gamepad
    // mode, and directly from the config-mode loop, which skips add-ons.
    void runCalibrationSweep();
    void advanceCalibration();          // idle baseline -> press capture
    void finishCalibration();           // press capture -> done
    void abortCalibration();
    // Derives idle/pressed/polarity/noise from the sweep and writes them to config.
    // Actuation and sensitivities come from the caller's chosen preset.
    void applyCalibration(uint8_t actuationPercent, uint8_t pressPercent,
                          uint8_t releasePercent, bool continuousRT);
    bool isCalibrating() { return calMode != HECalMode::OFF; }
    void startMonitor();
    void stopMonitor();
    bool isMonitoring() { return calMode == HECalMode::MONITOR; }
    // Live per-channel state for the test view.
    int16_t getChannelTravel(uint8_t he) { return lastTravel[he]; }
    bool isChannelActive(uint8_t he) { return triggerActive[he]; }
    HECalMode getCalibrationMode() { return calMode; }
    uint32_t getCalibrationElapsedMs();
    const HECalChannel& getCalibrationChannel(uint8_t he) { return calData[he]; }
    bool isChannelAssigned(uint8_t he);
private:
    void selectChannel(uint8_t mux, uint8_t channel);
    uint16_t emaSmoothing(uint16_t value, uint16_t previous);

    // Rebuilds the cached travel geometry from config. Must be called whenever
    // calibration values change (setup, reinit).
    void rebuildGeometry();
    // Maps a raw ADC reading to monotonic travel: 0 = released, 1000 = pressed.
    // This is where polarity is handled, exactly once.
    int16_t toTravel(uint8_t he, uint16_t raw);
    // Runs the actuation/rapid-trigger state machine for one channel.
    void updateTrigger(uint8_t he, int16_t travel);
    // Resolves the binding for a channel under the active profile.
    int32_t actionFor(uint8_t he);
    // Resolves one tuning value through the active profile, falling back to the
    // base switch when the profile does not override it.
    uint32_t tuningFor(uint8_t he, HETuningField field);
    void applyAction(Gamepad* gamepad, uint8_t he, int32_t action);
    uint16_t analogDeflection(uint8_t he, bool positive);
    // Commits a pending profile change once cycling has settled.
    void updateProfilePersistence();
    // Accumulates one sample into the calibration state for a channel.
    void accumulateCalibration(uint8_t he, uint16_t raw);

    static const int16_t TRAVEL_MAX = 1000;

    int muxTotal = 0;
    int selectPins = 0;
    Pin_t muxPinArray[4] = { -1, -1, -1, -1 };
    // Select pins indexed by [mux][selectBit]. Upstream made these per-mux so a
    // board whose muxes do not share a select bus can still address every channel.
    Pin_t selectPinArray[4][4] = {
        { -1, -1, -1, -1 }, { -1, -1, -1, -1 },
        { -1, -1, -1, -1 }, { -1, -1, -1, -1 },
    };
    Pin_t lastADCSelected = -1;

    // Per-trigger runtime state. These are initialized unconditionally: the
    // previous code only initialized them when EMA smoothing was enabled, which
    // left rapid trigger running on uninitialized memory when it was not.
    uint16_t emaSmoothingReads[HETRIGGER_COUNT] = {};
    bool     triggerActive[HETRIGGER_COUNT] = {};
    int16_t  travelPeak[HETRIGGER_COUNT] = {};    // deepest press since direction change
    int16_t  travelTrough[HETRIGGER_COUNT] = {};  // shallowest release since direction change
    bool     rtArmed[HETRIGGER_COUNT] = {};       // has crossed the actuation point at least once
    bool     menuActionHeld[HETRIGGER_COUNT] = {};    // edge detection for menu events
    bool     profileActionHeld[HETRIGGER_COUNT] = {}; // edge detection for profile switching

    // Cached travel geometry, rebuilt by rebuildGeometry(). Precomputing the
    // reciprocal keeps a 32-bit divide out of the per-channel hot path.
    int16_t  travelIdle[HETRIGGER_COUNT] = {};
    int32_t  travelSpanRecip[HETRIGGER_COUNT] = {};
    int16_t  actuationTravel[HETRIGGER_COUNT] = {};
    int16_t  pressSensTravel[HETRIGGER_COUNT] = {};
    int16_t  releaseSensTravel[HETRIGGER_COUNT] = {};
    int16_t  deadzoneTravel[HETRIGGER_COUNT] = {};
    int16_t  noiseTravel[HETRIGGER_COUNT] = {};
    // Resolved rapid trigger state per channel; cached because it can come from
    // the active profile rather than the switch record.
    bool     rapidTriggerOn[HETRIGGER_COUNT] = {};

    float emaSmoothingFactor = 0.0f;

    uint8_t activeProfile = 0;
    bool profileSavePending = false;
    absolute_time_t profileSaveDeadline = {};

    HECalMode calMode = HECalMode::OFF;
    // Most recent travel per channel, in 0..TRAVEL_MAX. Written on every sample
    // in both the gamepad and monitor paths: the test view reports it, and
    // proportional analog output scales the axis by it.
    int16_t lastTravel[HETRIGGER_COUNT] = {};
    // Travel from the unsmoothed reading. The EMA exists to keep the trigger
    // state stable, but it is a lag filter: at the default smoothing factor it
    // needs ~70 samples to settle, so an axis driven from the smoothed value
    // trails the finger and a brisk press never reaches the rail. Proportional
    // analog output uses this instead.
    int16_t lastRawTravel[HETRIGGER_COUNT] = {};
    HECalChannel calData[HETRIGGER_COUNT] = {};
    absolute_time_t calPhaseStart = {};
    absolute_time_t calTimeout = {};

    static HETriggerAddon* instance;
};

#endif  // _HE_Trigger_H