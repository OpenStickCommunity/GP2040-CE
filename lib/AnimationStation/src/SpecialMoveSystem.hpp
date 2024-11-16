#ifndef _SPECIALMOVE_H_
#define _SPECIALMOVE_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include <vector>
#include <deque>
#include <map>

#define MAX_SPECIALMOVE_PROFILES 4
#define MAX_SPECIALMOVES 20
#define MAX_SPECIALMOVE_INPUTTYPES_PER_MOVE 4
#define MAX_SPECIALMOVE_TRIGGERCOMBOS 3
#define MAX_SPECIALMOVE_TRIGGERS_PER_MOVE 3
#define MAX_INPUT_HISTORY 100

//List of specialMove animation
typedef enum
{
  SPECIALMOVE_SMEFFECT_WAVE,
  SPECIALMOVE_SMEFFECT_SUPERWAVE,
  SPECIALMOVE_SMEFFECT_PULSECOLOR,
  SPECIALMOVE_SMEFFECT_CIRCLECOLOR,
  SPECIALMOVE_SMEFFECT_FLASHCOLOR,
  SPECIALMOVE_SMEFFECT_KNIGHTRIDER,
  SPECIALMOVE_SMEFFECT_RANDOMFLASH,
} SpecialMoveEffects;

// SpecialMove animation Duration Modifier
typedef enum
{
  SPECIALMOVE_DURATION_FAST,
  SPECIALMOVE_DURATION_MEDIUM,
  SPECIALMOVE_DURATION_LONG,
} SpecialMoveAnimationDuration;

// SpecialMove animation Direction Modifier
typedef enum
{
  SPECIALMOVE_DIRECTION_UP,
  SPECIALMOVE_DIRECTION_LEFT,
  SPECIALMOVE_DIRECTION_RIGHT,
  SPECIALMOVE_DIRECTION_DOWN,
} SpecialMoveAnimationDirection;

// SpecialMove input types
typedef enum
{
  SPECIALMOVE_INPUT_QUARTER_DOWN_RIGHT,
  SPECIALMOVE_INPUT_QUARTER_DOWN_LEFT,
  SPECIALMOVE_INPUT_DP_RIGHT,
  SPECIALMOVE_INPUT_DP_SHORTCUT_RIGHT,
  SPECIALMOVE_INPUT_DP_LEFT,
  SPECIALMOVE_INPUT_DP_SHORTCUT_LEFT,
} SpecialMoveInputTypes;

// Input History structure.
struct InputHistory
{
    uint32_t Input; //button mask
    absolute_time_t TimeSet;
    absolute_time_t TimeReleased;
    bool bIsHeld;
};

struct __attribute__ ((__packed__)) SpecialMoveInputTriggers
{
    uint32_t OptionalParams; //6 4byte params packed in here. Unique per effect type. See each effect for details
    uint8_t RequiredTriggers; //buttonmasks
};

struct __attribute__ ((__packed__)) SpecialMoveDescription 
{
    uint32_t NumRequiredInputCombos;
    SpecialMoveInputTypes RequiredInputCombos[MAX_SPECIALMOVE_INPUTTYPES_PER_MOVE];
    uint32_t NumRequiredTriggerCombos;
    SpecialMoveInputTriggers RequiredTriggerCombos[MAX_SPECIALMOVE_TRIGGERCOMBOS];

    SpecialMoveEffects Animation;
    bool bIsChargeMove;
};

struct __attribute__ ((__packed__)) SpecialMoveProfile 
{
    uint32_t NumValidMoves;
    SpecialMoveDescription AllSpecialMoves[MAX_SPECIALMOVES];
};

struct __attribute__ ((__packed__)) SpecialMoveOptions
{
    uint32_t NumValidProfiles;
    SpecialMoveProfile profiles[MAX_SPECIALMOVE_PROFILES];
    uint32_t ChargeTimeInMs;
    uint8_t CurrentProfileIndex;
};

class SpecialMoveSystem {
public:
    SpecialMoveSystem();
    virtual ~SpecialMoveSystem(){};

    //Which buttons are held at the moment
    void Update();
 
    //passed in user options
    static void SetOptions(SpecialMoveOptions InOptions);

    //What buttons (logical ones) are pressed this frame
    void HandlePressedButtons(uint32_t pressedButtons);

protected:

    void ClearHistory();
    void UpdateHistoryForInput(uint32_t buttonMask, bool bIsPressed);

    void TestForActivatedSpecialMove();
    void UpdateRunningSpecialMove();

    static SpecialMoveOptions Options;

    //history of all buttons/directions held. (newest entries first)
    std::deque<InputHistory> SwitchHistory;

    // after a special move we need to wait for all inputs to be released before continuing to avoid repeats
    bool SwitchClearAwaitingAllRelease = false;

    //Current running Special Move indexes
    int RunnningSpecialMoveIndex;
    int RunnningSpecialMoveAnimationIndex;

    absolute_time_t thisFrameTime = nil_time;
};

#endif
