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

#define COMBO_INPUT_TIME_WINDOW 250 //Maximum time between combo movements that arent a charge
#define COMBO_TRIGGER_INPUT_TIME_WINDOW 200 //Maximum time between combined button inputs eg PP, PPP, KK or PK etc
#define COMBO_INPUT_COUNT_FOR_ONE_OUT_OF_TWO 7 //if the combo input is 7  or more moves then the input system will allow you to succeed if you dont miss 2 in a row window. eg LDRU would be fine for a SPD
#define COMBO_INPUT_COUNT_FOR_ONE_MISSING_IN_MIDDLE 5 //if the combo input is 5 (HCF) or more moves then the input system will allow you to succeed if you dont miss more than 1 in the middle
#define PRIORITY_LEEWAY_TIME 75

//List of specialMove animation
typedef enum
{
  SPECIALMOVE_SMEFFECT_WAVE,
  SPECIALMOVE_SMEFFECT_PULSECOLOR,
  SPECIALMOVE_SMEFFECT_CIRCLECOLOR,
  SPECIALMOVE_SMEFFECT_KNIGHTRIDER,
  SPECIALMOVE_SMEFFECT_RANDOMFLASH,
} SpecialMoveEffects;

// SpecialMove animation Duration Modifier
typedef enum
{
  SPECIALMOVE_DURATION_VERYSHORT,
  SPECIALMOVE_DURATION_SHORT,
  SPECIALMOVE_DURATION_MEDIUM,
  SPECIALMOVE_DURATION_LONG,
  SPECIALMOVE_DURATION_VERYLONG,
} SpecialMoveAnimationDuration;

// SpecialMove colour fade Modifier
typedef enum
{
  SPECIALMOVE_COLOURFADE_INSTANT,
  SPECIALMOVE_COLOURFADE_VERYFAST,
  SPECIALMOVE_COLOURFADE_FAST,
  SPECIALMOVE_COLOURFADE_MEDIUM,
  SPECIALMOVE_COLOURFADE_SLOW,
  SPECIALMOVE_COLOURFADE_VERYSLOW,
} SpecialMoveAnimationColourFadeSpeed;

// SpecialMove animation Direction Modifier
typedef enum
{
  SPECIALMOVE_DIRECTION_UP,
  SPECIALMOVE_DIRECTION_LEFT,
  SPECIALMOVE_DIRECTION_RIGHT,
  SPECIALMOVE_DIRECTION_DOWN,
} SpecialMoveAnimationDirection;

// SpecialMove animation Divider Modifier
typedef enum
{
  SPECIALMOVE_DIVIDER_ALL_LIGHTS,
  SPECIALMOVE_DIVIDER_FIRSTHALF_LIGHTS,
  SPECIALMOVE_DIVIDER_SECONDHALF_LIGHTS,
} SpecialMoveAnimationDivider;

// SpecialMove input types
typedef enum
{
  SPECIALMOVE_INPUT_QUARTER_DOWN_RIGHT,
  SPECIALMOVE_INPUT_QUARTER_DOWN_LEFT,
  SPECIALMOVE_INPUT_DP_RIGHT,
  SPECIALMOVE_INPUT_DP_SHORTCUT_RIGHT,
  SPECIALMOVE_INPUT_DP_LEFT,
  SPECIALMOVE_INPUT_DP_SHORTCUT_LEFT,
  SPECIALMOVE_INPUT_HALFCIRCLE_LEFT_RIGHT,
  SPECIALMOVE_INPUT_HALFCIRCLE_RIGHT_LEFT,
  SPECIALMOVE_INPUT_DOUBLE_DOWN,
  SPECIALMOVE_INPUT_DOUBLE_QUARTER_DOWN_RIGHT,
  SPECIALMOVE_INPUT_DOUBLE_QUARTER_DOWN_LEFT,
  SPECIALMOVE_INPUT_CHARGE_LEFT_RIGHT,
  SPECIALMOVE_INPUT_CHARGE_RIGHT_LEFT,
  SPECIALMOVE_INPUT_CHARGE_DOWN_UP,
  SPECIALMOVE_INPUT_SUPERCHARGE_LEFT_RIGHT,
  SPECIALMOVE_INPUT_SUPERCHARGE_RIGHT_LEFT,
  SPECIALMOVE_INPUT_360,
  SPECIALMOVE_INPUT_720,
  SPECIALMOVE_INPUT_RAGING_DEMON_RIGHT,
  SPECIALMOVE_INPUT_RAGING_DEMON_LEFT,
  SPECIALMOVE_INPUT_NONE,
} SpecialMoveInputTypes;

// SpecialMove joystick directions
typedef enum
{
  SPECIALMOVE_STICK_INVALID,
  SPECIALMOVE_STICK_UP,
  SPECIALMOVE_STICK_UPLEFT,
  SPECIALMOVE_STICK_UPRIGHT,
  SPECIALMOVE_STICK_LEFT,
  SPECIALMOVE_STICK_DOWNLEFT,
  SPECIALMOVE_STICK_DOWN,
  SPECIALMOVE_STICK_DOWNRIGHT,
  SPECIALMOVE_STICK_RIGHT,
} SpecialMoveStickDirection;

// CircleType
typedef enum
{
  SPECIALMOVE_CIRCLETYPE_LEFT_LEFT,
  SPECIALMOVE_CIRCLETYPE_LEFT_DOWN,
  SPECIALMOVE_CIRCLETYPE_LEFT_RIGHT,
  SPECIALMOVE_CIRCLETYPE_LEFT_UP,
  SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT,
  SPECIALMOVE_CIRCLETYPE_RIGHT_DOWN,
  SPECIALMOVE_CIRCLETYPE_RIGHT_RIGHT,
  SPECIALMOVE_CIRCLETYPE_RIGHT_UP,
  SPECIALMOVE_CIRCLETYPE_MAX,
} SpecialMoveCircleType;

struct ComboEntry
{
    SpecialMoveStickDirection DirectionInput;
    uint32_t ButtonMaskInput; //button mask

    ComboEntry(SpecialMoveStickDirection InDirection, uint32_t InButton)
    {
      DirectionInput = InDirection;
      ButtonMaskInput = InButton;
    }
};

// Input History structure.
struct InputHistory
{
    SpecialMoveStickDirection DirectionInput;
    uint32_t ButtonMaskInput; //button mask

    absolute_time_t TimeSet;
    absolute_time_t TimeReleased;
    bool bIsHeld;
};

struct __attribute__ ((__packed__)) SpecialMoveInputTriggers
{
    uint32_t OptionalParams = 0; //6 4byte params packed in here. Unique per effect type. See each effect for details
    uint32_t RequiredTriggers = 0; //buttonmasks
};

struct __attribute__ ((__packed__)) SpecialMoveDescription 
{
    uint32_t NumRequiredInputCombos = 0;
    SpecialMoveInputTypes RequiredInputCombos[MAX_SPECIALMOVE_INPUTTYPES_PER_MOVE];
    uint32_t NumRequiredTriggerCombos = 0;
    SpecialMoveInputTriggers RequiredTriggerCombos[MAX_SPECIALMOVE_TRIGGERCOMBOS];

    SpecialMoveEffects Animation;
    bool bIsChargeMove = false;
    int Priority;
};

struct __attribute__ ((__packed__)) SpecialMoveProfile 
{
    bool bEnabled = false;
    uint32_t NumValidMoves = 0;
    SpecialMoveDescription AllSpecialMoves[MAX_SPECIALMOVES];
};

struct __attribute__ ((__packed__)) SpecialMoveOptions
{
    uint32_t checksum;
    uint32_t NumValidProfiles = 0;
    SpecialMoveProfile profiles[MAX_SPECIALMOVE_PROFILES];
    uint32_t ChargeTimeInMs = 0;
    uint8_t CurrentProfileIndex = 0;
};

class SpecialMoveSystem {
public:
    SpecialMoveSystem();
    virtual ~SpecialMoveSystem(){};

    void Init();
    void ChangeSpecialMoveProfile(int changeSize);

    //Which buttons are held at the moment
    void Update();
 
    //passed in user options
    void SetDirectionMasks(uint32_t UpMask, uint32_t DownMask, uint32_t LeftMask, uint32_t RightMask);
    void SetButtonMasks(uint32_t P1Mask);

    //What buttons (logical ones) are pressed this frame
    void HandlePressedButtons(uint32_t pressedButtons);

    void SetParentAnimationStation(class AnimationStation* InParentAnimationStation);
    void SetSpecialMoveAnimationOver() { bMoveIsRunning = false; }

    static SpecialMoveOptions Options;

protected:

    void AdjustSpecialMoveProfileIndex(int changeSize);
    void ClearHistory();
    int64_t GetMsBetweenTimes(absolute_time_t start, absolute_time_t end);
    void UpdateHistoryForInput(uint32_t buttonMask, SpecialMoveStickDirection directionHeld, bool bIsPressed);
    bool CheckJoystickDirection(SpecialMoveStickDirection& FoundDirection, SpecialMoveStickDirection TestDirection, uint32_t PressedButtons, uint32_t DirectionOne, uint32_t DirectionTwo);
    SpecialMoveStickDirection GetDirectionPressed(uint32_t PressedButtons);
    bool DoesInputMatch(int HistoryIndex, ComboEntry& ComboInput, bool bIsChargeMove);
    void SwitchHistoryCreateNew(uint32_t buttonMask, SpecialMoveStickDirection directionHeld);

    bool TestAllMoves();
    bool TestForActivatedSpecialMove(SpecialMoveDescription* MoveToTestFor, int ComboIndex, int TriggerIndex, SpecialMoveCircleType CircleStartDirection);
    void StartMoveAnimation(SpecialMoveEffects AnimationToPlay, uint32_t OptionalParams);

    void GetComboArrayForMove(SpecialMoveInputTypes InputType, std::vector<ComboEntry>& comboArray, SpecialMoveCircleType CircleStartDirection);

    class AnimationStation* ParentAnimationStation;

    //history of all buttons/directions held. (newest entries first)
    InputHistory SwitchHistory[MAX_INPUT_HISTORY];

    // after a special move we need to wait for all inputs to be released before continuing to avoid repeats
    bool SwitchClearAwaitingAllRelease = false;

    absolute_time_t thisFrameTime = nil_time;

    uint32_t SPECIAL_MOVE_DIRECTION_MASK_UP;
    uint32_t SPECIAL_MOVE_DIRECTION_MASK_DOWN;
    uint32_t SPECIAL_MOVE_DIRECTION_MASK_LEFT;
    uint32_t SPECIAL_MOVE_DIRECTION_MASK_RIGHT;

    uint32_t SPECIAL_MOVE_BUTTON_MASK_P1;

    bool bMoveIsRunning = false;

    absolute_time_t cachedSucceededMoveTime;
    int cachedSucceededMovePriority;
    SpecialMoveEffects cachedSucceededMoveAnimation;
    uint32_t cachedSucceededMoveParams;
    bool bHasCachedSucceededMove = false;
};

#endif
