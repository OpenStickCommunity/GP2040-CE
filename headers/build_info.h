/* generate build information (such as stuff auto-included in the .uf2) */
#include "pico/binary_info.h"
#include "pico/binary_info/code.h"

#include "BoardConfig.h"

// generate binary information for picotool info, etc.
#if(PIN_DPAD_UP >= 1)
bi_decl(bi_1pin_with_name(PIN_DPAD_UP, "Up"));
#endif

#if(PIN_DPAD_DOWN >= 1)
bi_decl(bi_1pin_with_name(PIN_DPAD_DOWN, "Down"));
#endif

#if(PIN_DPAD_LEFT >= 1)
bi_decl(bi_1pin_with_name(PIN_DPAD_LEFT, "Left"));
#endif

#if(PIN_DPAD_RIGHT >= 1)
bi_decl(bi_1pin_with_name(PIN_DPAD_RIGHT, "Right"));
#endif

#if(PIN_BUTTON_B1 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_B1, "B1"));
#endif

#if(PIN_BUTTON_B2 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_B2, "B2"));
#endif

#if(PIN_BUTTON_B3 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_B3, "B3"));
#endif

#if(PIN_BUTTON_B4 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_B4, "B4"));
#endif

#if(PIN_BUTTON_L1 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_L1, "L1"));
#endif

#if(PIN_BUTTON_R1 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_R1, "R1"));
#endif

#if(PIN_BUTTON_L2 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_L2, "L2"));
#endif

#if(PIN_BUTTON_R2 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_R2, "R2"));
#endif

#if(PIN_BUTTON_S1 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_S1, "S1"));
#endif

#if(PIN_BUTTON_S2 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_S2, "S2"));
#endif

#if(PIN_BUTTON_L3 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_L3, "L3"));
#endif

#if(PIN_BUTTON_R3 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_R3, "R3"));
#endif

#if(PIN_BUTTON_A1 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_A1, "A1"));
#endif

#if(PIN_BUTTON_A2 >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_A2, "A2"));
#endif

#if(PIN_BUTTON_FN >= 1)
bi_decl(bi_1pin_with_name(PIN_BUTTON_FN, "Fn"));
#endif
