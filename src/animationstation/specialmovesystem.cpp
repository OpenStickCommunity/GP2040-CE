#include "specialmovesystem.h"

#include "animationstation.h"

#include "enums.pb.h"

SpecialMoveOptions SpecialMoveSystem::Options = {};

SpecialMoveSystem::SpecialMoveSystem()
{
}

void SpecialMoveSystem::SetParentAnimationStation(class AnimationStation* InParentAnimationStation)
{
    ParentAnimationStation = InParentAnimationStation;
    ClearHistory();
}

void SpecialMoveSystem::Init()
{
    AdjustSpecialMoveProfileIndex(0);
}

void SpecialMoveSystem::ChangeSpecialMoveProfile(int changeSize) 
{
    AdjustSpecialMoveProfileIndex(changeSize);
}

void SpecialMoveSystem::AdjustSpecialMoveProfileIndex(int changeSize)
{
    std::vector<int> validIndexes;

    //if no profiles defined then return -1 to turn everything off
    if(Options.NumValidProfiles == 0)
    {
        this->Options.CurrentProfileIndex = 0;
        return;
    }

    for(int index = 0; index < MAX_SPECIALMOVE_PROFILES; ++index)
    {
        if(Options.profiles[index].bEnabled)
            validIndexes.push_back(index);
    }

    if(validIndexes.size() == 0)
    {
        this->Options.CurrentProfileIndex = 0;
        return;
    }

    //find index of current profile
    int indexOfCurrentProfile = -1;
    for(unsigned int index = 0; index < validIndexes.size(); ++index)
    {
        if(validIndexes[index] == (int)this->Options.CurrentProfileIndex)
        {
            indexOfCurrentProfile = index;
            break;
        }
    }

    //if we cant find it then this is probably the first call and the first profile isnt valid. Just return whichever is the first valid profile
    if(indexOfCurrentProfile == -1)
    {
        this->Options.CurrentProfileIndex = 0;
        return;
    }

    int newProfileIndex = indexOfCurrentProfile + changeSize;

    if (newProfileIndex >= (int)validIndexes.size())
    {
        this->Options.CurrentProfileIndex = validIndexes[0];
    }
    else if (newProfileIndex < 0) 
    {
        this->Options.CurrentProfileIndex = validIndexes[validIndexes.size() - 1];
    }
    else
    {
        this->Options.CurrentProfileIndex = validIndexes[newProfileIndex];
    }
}

void SpecialMoveSystem::Update()
{
    thisFrameTime = get_absolute_time();

    if(bMoveIsRunning == false)
    {
        if(TestAllMoves())
        {
            bMoveIsRunning = true;
        }
    }
}

bool SpecialMoveSystem::TestAllMoves()
{
    //Do we have any valid profiles?
    if(Options.NumValidProfiles == 0 || Options.CurrentProfileIndex > Options.NumValidProfiles ||  Options.profiles[Options.CurrentProfileIndex].bEnabled == false)
        return false;

    //Move is ready to fire
    if(bHasCachedSucceededMove && GetMsBetweenTimes(cachedSucceededMoveTime, thisFrameTime) > PRIORITY_LEEWAY_TIME)
    {
        bHasCachedSucceededMove = false;
        // Clear the history to prevent retrigger if animation is really quick
        ClearHistory();
        if(ParentAnimationStation)
            ParentAnimationStation->SetSpecialMoveAnimation(cachedSucceededMoveAnimation, cachedSucceededMoveParams);
        SwitchClearAwaitingAllRelease = true;
        return true;
    }

    //Test Each move and cache successes so higher priority ones can take preferrence
    uint32_t numMoves = Options.profiles[Options.CurrentProfileIndex].NumValidMoves;
    for(uint32_t moveIndex = 0; moveIndex < numMoves; ++moveIndex)
    {
        SpecialMoveDescription& thisMove = Options.profiles[Options.CurrentProfileIndex].AllSpecialMoves[moveIndex];
        uint32_t numCombos = thisMove.NumRequiredInputCombos;
        uint32_t numTriggers = thisMove.NumRequiredTriggerCombos;

        for(uint32_t comboIndex = 0; comboIndex < numCombos; ++comboIndex)
        {
            for(uint32_t triggerIndex = 0; triggerIndex < numTriggers; ++triggerIndex)
            {
                //360/720 inputs require us to try all possible start locations (although we only bother with cardinals as an optimisation)
                bool doCircleLoop = false; 
                if(thisMove.RequiredInputCombos[comboIndex] == SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_360 || thisMove.RequiredInputCombos[comboIndex] == SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_720)
                {
                    doCircleLoop = true;
                }

                for(int circleLoops = 0; circleLoops < (doCircleLoop ? (int)SPECIALMOVE_CIRCLETYPE_MAX : 1); ++circleLoops)
                {               
                    bool bSucceeded = TestForActivatedSpecialMove(&thisMove, comboIndex, triggerIndex, (SpecialMoveCircleType)circleLoops);
                    
                    if(bSucceeded)
                    {
                        if(!bHasCachedSucceededMove || thisMove.Priority > cachedSucceededMovePriority)
                        {
                            if(!bHasCachedSucceededMove)
                            {   
                                bHasCachedSucceededMove = true;
                                cachedSucceededMoveTime = thisFrameTime;
                            }
                            cachedSucceededMovePriority = thisMove.Priority;
                            cachedSucceededMoveAnimation = thisMove.Animation;
                            cachedSucceededMoveParams = thisMove.RequiredTriggerCombos[triggerIndex].OptionalParams;
                        }
                    }
                }
            }
        }
    }

    return false;
}

void SpecialMoveSystem::SetDirectionMasks(uint32_t UpMask, uint32_t DownMask, uint32_t LeftMask, uint32_t RightMask)
{
    SPECIAL_MOVE_DIRECTION_MASK_UP = UpMask;
    SPECIAL_MOVE_DIRECTION_MASK_DOWN = DownMask;
    SPECIAL_MOVE_DIRECTION_MASK_LEFT = LeftMask;
    SPECIAL_MOVE_DIRECTION_MASK_RIGHT = RightMask;
}

void SpecialMoveSystem::SetButtonMasks(uint32_t P1Mask)
{
    SPECIAL_MOVE_BUTTON_MASK_P1 = P1Mask;
}

SpecialMoveStickDirection SpecialMoveSystem::GetDirectionPressed(uint32_t PressedButtons)
{
    if((PressedButtons & (SPECIAL_MOVE_DIRECTION_MASK_LEFT | SPECIAL_MOVE_DIRECTION_MASK_UP)) == (SPECIAL_MOVE_DIRECTION_MASK_LEFT | SPECIAL_MOVE_DIRECTION_MASK_UP))
        return SpecialMoveStickDirection::SPECIALMOVE_STICK_UPLEFT;
    else if((PressedButtons & (SPECIAL_MOVE_DIRECTION_MASK_RIGHT | SPECIAL_MOVE_DIRECTION_MASK_UP)) == (SPECIAL_MOVE_DIRECTION_MASK_RIGHT | SPECIAL_MOVE_DIRECTION_MASK_UP))
        return SpecialMoveStickDirection::SPECIALMOVE_STICK_UPRIGHT;
    else if((PressedButtons & (SPECIAL_MOVE_DIRECTION_MASK_LEFT | SPECIAL_MOVE_DIRECTION_MASK_DOWN)) == (SPECIAL_MOVE_DIRECTION_MASK_LEFT | SPECIAL_MOVE_DIRECTION_MASK_DOWN))
        return SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT;
    else if((PressedButtons & (SPECIAL_MOVE_DIRECTION_MASK_RIGHT | SPECIAL_MOVE_DIRECTION_MASK_DOWN)) == (SPECIAL_MOVE_DIRECTION_MASK_RIGHT | SPECIAL_MOVE_DIRECTION_MASK_DOWN))
        return SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT;
    else if((PressedButtons & SPECIAL_MOVE_DIRECTION_MASK_UP) == SPECIAL_MOVE_DIRECTION_MASK_UP)
        return SpecialMoveStickDirection::SPECIALMOVE_STICK_UP;
    else if((PressedButtons & SPECIAL_MOVE_DIRECTION_MASK_DOWN) == SPECIAL_MOVE_DIRECTION_MASK_DOWN)
        return SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN;
    else if((PressedButtons & SPECIAL_MOVE_DIRECTION_MASK_LEFT) == SPECIAL_MOVE_DIRECTION_MASK_LEFT)
        return SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT;
    else if((PressedButtons & SPECIAL_MOVE_DIRECTION_MASK_RIGHT) == SPECIAL_MOVE_DIRECTION_MASK_RIGHT)
        return SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT;

    return SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID;
}

bool SpecialMoveSystem::CheckJoystickDirection(SpecialMoveStickDirection& FoundDirection, SpecialMoveStickDirection TestDirection, uint32_t PressedButtons, uint32_t DirectionOne, uint32_t DirectionTwo)
{
    bool bWasPressed = false;
    if(FoundDirection == SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID && ((PressedButtons & (DirectionOne | DirectionTwo)) == (DirectionOne | DirectionTwo)))
    {
        bWasPressed = true;
        FoundDirection = TestDirection;
        UpdateHistoryForInput(0, TestDirection, true);
    }
    else
    {
        UpdateHistoryForInput(0, TestDirection, false);
    } 

    return bWasPressed;
}

void SpecialMoveSystem::HandlePressedButtons(uint32_t pressedButtons)
{
    bool bWasAnythingPressed = false;

    //do directions explicitly due to needing to do diagonals
    SpecialMoveStickDirection FoundDirection = SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID;
    bWasAnythingPressed |= CheckJoystickDirection(FoundDirection, SpecialMoveStickDirection::SPECIALMOVE_STICK_UPLEFT, pressedButtons, SPECIAL_MOVE_DIRECTION_MASK_LEFT, SPECIAL_MOVE_DIRECTION_MASK_UP);
    bWasAnythingPressed |= CheckJoystickDirection(FoundDirection, SpecialMoveStickDirection::SPECIALMOVE_STICK_UPRIGHT, pressedButtons, SPECIAL_MOVE_DIRECTION_MASK_RIGHT, SPECIAL_MOVE_DIRECTION_MASK_UP);
    bWasAnythingPressed |= CheckJoystickDirection(FoundDirection, SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, pressedButtons, SPECIAL_MOVE_DIRECTION_MASK_LEFT, SPECIAL_MOVE_DIRECTION_MASK_DOWN);
    bWasAnythingPressed |= CheckJoystickDirection(FoundDirection, SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, pressedButtons, SPECIAL_MOVE_DIRECTION_MASK_RIGHT, SPECIAL_MOVE_DIRECTION_MASK_DOWN);
    bWasAnythingPressed |= CheckJoystickDirection(FoundDirection, SpecialMoveStickDirection::SPECIALMOVE_STICK_UP, pressedButtons, SPECIAL_MOVE_DIRECTION_MASK_UP, 0);
    bWasAnythingPressed |= CheckJoystickDirection(FoundDirection, SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, pressedButtons, SPECIAL_MOVE_DIRECTION_MASK_DOWN, 0);
    bWasAnythingPressed |= CheckJoystickDirection(FoundDirection, SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, pressedButtons, SPECIAL_MOVE_DIRECTION_MASK_LEFT, 0);
    bWasAnythingPressed |= CheckJoystickDirection(FoundDirection, SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, pressedButtons, SPECIAL_MOVE_DIRECTION_MASK_RIGHT, 0);

    //check each button input and update history
    for(uint32_t buttonMask = 1; buttonMask < (1 << 20); buttonMask = buttonMask << 1)
    {
        if(buttonMask == SPECIAL_MOVE_DIRECTION_MASK_UP || buttonMask == SPECIAL_MOVE_DIRECTION_MASK_DOWN || buttonMask == SPECIAL_MOVE_DIRECTION_MASK_LEFT || buttonMask == SPECIAL_MOVE_DIRECTION_MASK_RIGHT)
            continue;

        bool bIsPressed = (pressedButtons & buttonMask) != 0;
        bWasAnythingPressed |= bIsPressed;
        UpdateHistoryForInput(buttonMask, SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID, bIsPressed);
    }

    //After a special move we need to wait for all buttons to be released before we trigger another or we could potentially chain two together by accident. 
    //If nothing is pressed this frame then we can remove this block.
    if(SwitchClearAwaitingAllRelease)
    {
        if(bWasAnythingPressed)
        {
            ClearHistory();
        }
        else
        {
            SwitchClearAwaitingAllRelease = false;
        }
    }
}

void SpecialMoveSystem::ClearHistory()
{
  for(int historyIndex = 0; historyIndex < MAX_INPUT_HISTORY; ++historyIndex)
  {
    SwitchHistory[historyIndex].ButtonMaskInput = 0;
    SwitchHistory[historyIndex].DirectionInput = SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID;
  }
}

void SpecialMoveSystem::SwitchHistoryCreateNew(uint32_t buttonMask, SpecialMoveStickDirection directionHeld)
{
  //Shift all entries up by 1
  for(int historyIndex = (MAX_INPUT_HISTORY-1) ; historyIndex > 0; --historyIndex)
  {
    SwitchHistory[historyIndex].ButtonMaskInput = SwitchHistory[historyIndex-1].ButtonMaskInput;
    SwitchHistory[historyIndex].DirectionInput = SwitchHistory[historyIndex-1].DirectionInput;
    SwitchHistory[historyIndex].TimeSet = SwitchHistory[historyIndex-1].TimeSet;
    SwitchHistory[historyIndex].TimeReleased = SwitchHistory[historyIndex-1].TimeReleased;
    SwitchHistory[historyIndex].bIsHeld = SwitchHistory[historyIndex-1].bIsHeld;
  }

  //Stick new input onto front of array and set the held start time
  SwitchHistory[0].ButtonMaskInput = buttonMask;
  SwitchHistory[0].DirectionInput = directionHeld;
  SwitchHistory[0].TimeSet = thisFrameTime;
  //SwitchHistory[0].TimeReleased = (absolute_time_t)0;
  SwitchHistory[0].bIsHeld = true;
}

void SpecialMoveSystem::UpdateHistoryForInput(uint32_t buttonMask, SpecialMoveStickDirection directionHeld, bool bIsPressed)
{
    //search history for recent instance of this button
    for(unsigned int historyIndex = 0; historyIndex < MAX_INPUT_HISTORY; ++historyIndex)
    {
        if(SwitchHistory[historyIndex].ButtonMaskInput == buttonMask && SwitchHistory[historyIndex].DirectionInput == directionHeld)
        {
            //found button. now check if its pressed state is correct
            if(bIsPressed && SwitchHistory[historyIndex].bIsHeld == false)
            {
                //create new entry at start of history array
                SwitchHistoryCreateNew(buttonMask, directionHeld);
            }
            else if(!bIsPressed && SwitchHistory[historyIndex].bIsHeld)
            {
                //button has been released. Store off when this happened
                SwitchHistory[historyIndex].bIsHeld = false;
                SwitchHistory[historyIndex].TimeReleased = thisFrameTime;
            }

            return;
        }
    }

    //no previous entry found. Add new one if its being held
    if(bIsPressed)
        SwitchHistoryCreateNew(buttonMask, directionHeld);
}

bool SpecialMoveSystem::DoesInputMatch(int HistoryIndex, ComboEntry& ComboInput, bool bIsChargeMove)
{
    if(ComboInput.DirectionInput != SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID)
    {
        //exact match always true
        if(SwitchHistory[HistoryIndex].DirectionInput == ComboInput.DirectionInput)
            return true;

        //charge moves are allowed to use diagonals
        if(bIsChargeMove)
        {
             //Charge moves allow the surrounding directions to also count
            if(ComboInput.DirectionInput == SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT)
            {
                return SwitchHistory[HistoryIndex].DirectionInput == SpecialMoveStickDirection::SPECIALMOVE_STICK_UPLEFT || SwitchHistory[HistoryIndex].DirectionInput == SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT;
            }
            else if(ComboInput.DirectionInput == SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN)
            {
                return SwitchHistory[HistoryIndex].DirectionInput == SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT || SwitchHistory[HistoryIndex].DirectionInput == SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT;
            }
            else if(ComboInput.DirectionInput == SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT)
            {
                return SwitchHistory[HistoryIndex].DirectionInput == SpecialMoveStickDirection::SPECIALMOVE_STICK_UPRIGHT || SwitchHistory[HistoryIndex].DirectionInput == SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT;
            }
            else if(ComboInput.DirectionInput == SpecialMoveStickDirection::SPECIALMOVE_STICK_UP)
            {
                return SwitchHistory[HistoryIndex].DirectionInput == SpecialMoveStickDirection::SPECIALMOVE_STICK_UPLEFT || SwitchHistory[HistoryIndex].DirectionInput == SpecialMoveStickDirection::SPECIALMOVE_STICK_UPRIGHT;
            }
        }

        //other wise we didnt find a match
        return false;
    }

    //otherwise its button matching
    return SwitchHistory[HistoryIndex].ButtonMaskInput == ComboInput.ButtonMaskInput;
}

int64_t SpecialMoveSystem::GetMsBetweenTimes(absolute_time_t start, absolute_time_t end)
{
    return absolute_time_diff_us(start, end) / 1000;
}

bool SpecialMoveSystem::TestForActivatedSpecialMove(SpecialMoveDescription* MoveToTestFor, int ComboIndex, int TriggerIndex, SpecialMoveCircleType CircleStartDirection)
{
    int lastIndexComboInputWasFound = -1;
    bool bHasFinishedDoingTriggerButtons = false;
    absolute_time_t timeNow = thisFrameTime;
    absolute_time_t timeLastInputSet = thisFrameTime;
    int numMissedConsecutive = 0;
    int numMissedMiddle = 0;
    bool bIsChargeCombo = MoveToTestFor->bIsChargeMove;
    int64_t OptionsChargeTime = Options.ChargeTimeInMs;

    // Get combo array
    std::vector<ComboEntry> comboArray;
    GetComboArrayForMove(MoveToTestFor->RequiredInputCombos[ComboIndex], comboArray, CircleStartDirection);
    int moveLength = comboArray.size();
    bool bAllowNoConsecutiveMiss = moveLength >= COMBO_INPUT_COUNT_FOR_ONE_OUT_OF_TWO;
    bool bAllowMissInMiddle = moveLength >= COMBO_INPUT_COUNT_FOR_ONE_MISSING_IN_MIDDLE;
    
    // put triggers on the end for easier processing
    int triggerLength = 0;
    uint32_t thisCombinedTrigger = MoveToTestFor->RequiredTriggerCombos[TriggerIndex].RequiredTriggers;
    SpecialMoveStickDirection triggerDirection = GetDirectionPressed(thisCombinedTrigger);
    if(triggerDirection != SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID)
    {
        comboArray.push_back(ComboEntry(triggerDirection, 0));
        triggerLength++;
    }
    for(uint32_t buttonMask = 1; buttonMask < (1 << 20); buttonMask = buttonMask << 1)
    {
        if(buttonMask == SPECIAL_MOVE_DIRECTION_MASK_UP || buttonMask == SPECIAL_MOVE_DIRECTION_MASK_DOWN || buttonMask == SPECIAL_MOVE_DIRECTION_MASK_LEFT || buttonMask == SPECIAL_MOVE_DIRECTION_MASK_RIGHT)
            continue;

        if((thisCombinedTrigger & buttonMask) != 0)
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID, buttonMask));
            triggerLength++;
        }
    }
    if(triggerLength == 0)
        return false;

    int comboLength = moveLength + triggerLength;
    if (comboLength > MAX_INPUT_HISTORY)
        comboLength = MAX_INPUT_HISTORY;

    // Moves are specified in normal order. History is newest first. So start from the end of the combo and work back
    int triggersFound = 0;
    for (int comboSearchIndex = comboLength - 1; comboSearchIndex >= 0; --comboSearchIndex)
    {
        // First see if the button(s) and the final movement that finish this combo routine have recently been pressed
        if (bHasFinishedDoingTriggerButtons == false)
        {
            // Is this a trigger button
            if (triggersFound == triggerLength)
            {
                bHasFinishedDoingTriggerButtons = true;
                //we dont return here as the last movement is also counted as a trigger action.
            }

            // In which case, we only care if it was pressed within the last COMBO_TRIGGER_INPUT_TIME_WINDOW
            bool bFoundTrigger = false;
            for (unsigned int historyIndex = 0; historyIndex < MAX_INPUT_HISTORY; ++historyIndex)
            {
                // are we past the time window?
                if (GetMsBetweenTimes(SwitchHistory[historyIndex].TimeSet, timeNow) > (bHasFinishedDoingTriggerButtons ? COMBO_INPUT_TIME_WINDOW : COMBO_TRIGGER_INPUT_TIME_WINDOW))
                {
                    return false; // we havent managed to find this trigger in time
                }

                if (DoesInputMatch(historyIndex, comboArray[comboSearchIndex], bIsChargeCombo))
                {
                    // we found this trigger. Move onto the next one
                    bFoundTrigger = true;
                    triggersFound++;
                    lastIndexComboInputWasFound = historyIndex;
                    break;
                }
            }

            if (bFoundTrigger == false)
            {
                return false; // we havent managed to find this trigger
            }

            // found trigger so move onto next combo entry
            continue;
        }

        // search history for next input
        bool bFoundInput = false;
        for (int historyIndex = lastIndexComboInputWasFound + 1; historyIndex < MAX_INPUT_HISTORY; ++historyIndex)
        {
            if (DoesInputMatch(historyIndex, comboArray[comboSearchIndex], bIsChargeCombo))
            {   
                bFoundInput = true;

                // Is this the last input and if so is it a charge input as they're special and
                if (comboSearchIndex == 0 && bIsChargeCombo)
                {
                    // Its a charge move. In which case, this move must be held for CHARGE_COMBO_INPUT_TIME_WINDOW as well as released within COMBO_INPUT_TIME_WINDOW of the last one
                    if (SwitchHistory[historyIndex].bIsHeld == false && GetMsBetweenTimes(SwitchHistory[historyIndex].TimeReleased, timeLastInputSet) < COMBO_INPUT_TIME_WINDOW && GetMsBetweenTimes(SwitchHistory[historyIndex].TimeSet, SwitchHistory[historyIndex].TimeReleased) > OptionsChargeTime)
                    {
                        // Clear the history to prevent retrigger if animation is really quick
                        ClearHistory();
                        return true; // success
                    }
                    else
                    {
                        return false; // released too early
                    }
                }

                // regular input
                absolute_time_t timeToUse = SwitchHistory[historyIndex].bIsHeld ? SwitchHistory[historyIndex].TimeSet : SwitchHistory[historyIndex].TimeReleased;
                if (GetMsBetweenTimes(timeToUse, timeLastInputSet) > COMBO_INPUT_TIME_WINDOW)
                {
                    if (bAllowNoConsecutiveMiss)
                    {
                        // too long between inputs on this one. but might be ok to continue if this was an older input in the history
                        numMissedConsecutive++;
                        if (numMissedConsecutive == 2)
                            return false; // too many missing
                    }
                    else if(bAllowMissInMiddle)
                    {
                        if(comboSearchIndex != 0 &&numMissedMiddle == 0)
                            numMissedMiddle++;
                        else
                            return false;
                    }
                    else
                        return false; // too long between inputs
                }

                // Store this time and location and move onto next input
                timeLastInputSet = SwitchHistory[historyIndex].TimeSet;
                lastIndexComboInputWasFound = historyIndex;
                numMissedConsecutive = 0;

                break;
            }
        }

        if (bFoundInput == false)
        {
                if (bAllowNoConsecutiveMiss)
                {
                    // too long between inputs on this one. but might be ok to continue if this was an older input in the history
                    numMissedConsecutive++;
                    if (numMissedConsecutive == 2)
                        return false; // too many missing
                }
                else if(bAllowMissInMiddle)
                {
                    if(comboSearchIndex != 0 &&numMissedMiddle == 0)
                        numMissedMiddle++;
                    else
                        return false;
                }
                else
                    return false; // we havent found the input at all
        }
    }

    // if we get here we've found every single input! Combo passed
    return true;
}

void SpecialMoveSystem::GetComboArrayForMove(SpecialMoveInputTypes InputType, std::vector<ComboEntry>& comboArray, SpecialMoveCircleType CircleStartDirection)
{ 
    switch(InputType)
    {
        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_NONE:
        {
            //Nothing. Used for stuff like Giefs Lariat
        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_QUARTER_DOWN_RIGHT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_QUARTER_DOWN_LEFT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
        } break;
        
        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_DP_RIGHT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
        } break;
        
        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_DP_LEFT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
        } break;
        
        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_DP_SHORTCUT_RIGHT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
        } break;
        
        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_DP_SHORTCUT_LEFT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
        } break;
        
        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_HALFCIRCLE_LEFT_RIGHT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_HALFCIRCLE_RIGHT_LEFT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_DOUBLE_DOWN:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));

        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_DOUBLE_QUARTER_DOWN_RIGHT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_DOUBLE_QUARTER_DOWN_LEFT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_CHARGE_LEFT_RIGHT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_CHARGE_RIGHT_LEFT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_CHARGE_DOWN_UP:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UP, 0));
        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_SUPERCHARGE_LEFT_RIGHT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_SUPERCHARGE_RIGHT_LEFT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_RAGING_DEMON_RIGHT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID, SPECIAL_MOVE_BUTTON_MASK_P1));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID, SPECIAL_MOVE_BUTTON_MASK_P1));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_RAGING_DEMON_LEFT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID, SPECIAL_MOVE_BUTTON_MASK_P1));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID, SPECIAL_MOVE_BUTTON_MASK_P1));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
        } break;

        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_360:
        {
            if(CircleStartDirection == SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT || CircleStartDirection == SPECIALMOVE_CIRCLETYPE_RIGHT_DOWN ||CircleStartDirection == SPECIALMOVE_CIRCLETYPE_RIGHT_UP ||CircleStartDirection == SPECIALMOVE_CIRCLETYPE_RIGHT_RIGHT)
            {
                if(CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
                }
                if(CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT || CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_DOWN)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
                }
                if(CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT || CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_DOWN || CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_RIGHT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UPRIGHT, 0));
                }

                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UP, 0));

                if(CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UPLEFT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
                }
                if(CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT && CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_DOWN)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
                }
                if(CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT && CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_DOWN && CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_RIGHT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
                }
            }
            else
            {
                if(CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_RIGHT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
                }
                if(CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_RIGHT || CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_DOWN)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
                }
                if(CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_RIGHT || CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_DOWN || CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_LEFT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UPLEFT, 0));
                }

                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UP, 0));

                if(CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_RIGHT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UPRIGHT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
                }
                if(CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_RIGHT && CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_DOWN)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
                }
                if(CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_RIGHT && CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_DOWN && CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_LEFT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
                }
            }
        } break;
        
        case SpecialMoveInputTypes::SpecialMoveInputTypes_INPUT_720:
        {
            if(CircleStartDirection == SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT || CircleStartDirection == SPECIALMOVE_CIRCLETYPE_RIGHT_DOWN ||CircleStartDirection == SPECIALMOVE_CIRCLETYPE_RIGHT_UP ||CircleStartDirection == SPECIALMOVE_CIRCLETYPE_RIGHT_RIGHT)
            {
                if(CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
                }
                if(CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT || CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_DOWN)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
                }
                if(CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT || CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_DOWN || CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_RIGHT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UPRIGHT, 0));
                }

                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UP, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UPLEFT, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UPRIGHT, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UP, 0));

                if(CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UPLEFT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
                }
                if(CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT && CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_DOWN)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
                }
                if(CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_LEFT && CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_DOWN && CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_RIGHT_RIGHT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
                }
            }
            else
            {
                if(CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_RIGHT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
                }
                if(CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_RIGHT || CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_DOWN)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
                }
                if(CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_RIGHT || CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_DOWN || CircleStartDirection == SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_LEFT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UPLEFT, 0));
                }

                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UP, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UPRIGHT, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UPLEFT, 0));
                comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UP, 0));

                if(CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_RIGHT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_UPRIGHT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
                }
                if(CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_RIGHT && CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_DOWN)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
                }
                if(CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_RIGHT && CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_DOWN && CircleStartDirection != SpecialMoveCircleType::SPECIALMOVE_CIRCLETYPE_LEFT_LEFT)
                {
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNLEFT, 0));
                    comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_LEFT, 0));
                }
            }
        } break;

        default:
            break;
    }
}

void SpecialMoveSystem::CheckForOptionsUpdate()
{
/*    bool bChangeDetected = false;
    SpecialMoveOptions_Proto& optionsProto = Storage::getInstance().getSpecialMoveOptions();

     //Any changes?
	if(optionsProto.CurrentProfileIndex != options.CurrentProfileIndex)
      bChangeDetected = true;

    //only change settings if they've been static for a little while
    if(bChangeDetected)
    {
        if(timeAnimationSaveSet == 0)
        {
            timeAnimationSaveSet = get_absolute_time();
        }

        if(absolute_time_diff_us(timeAnimationSaveSet, get_absolute_time()) / 1000) > 1000) // 1 second delay on saves
        {
            timeAnimationSaveSet = 0;
            optionsProto.CurrentProfileIndex			= options.CurrentProfileIndex;
        }
    }
    else 
    {
        timeAnimationSaveSet = 0;
    }*/
}