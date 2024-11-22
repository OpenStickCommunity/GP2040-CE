#include "SpecialMoveSystem.hpp"

#include "AnimationStation.hpp"

SpecialMoveOptions SpecialMoveSystem::Options = {};

SpecialMoveSystem::SpecialMoveSystem()
{
}

void SpecialMoveSystem::SetParentAnimationStation(class AnimationStation* InParentAnimationStation)
{
    ParentAnimationStation = InParentAnimationStation;
    SwitchHistory.resize(0);
}

void SpecialMoveSystem::Update()
{
    thisFrameTime = get_absolute_time();

    //trim history if required
//    if(SwitchHistory.size() > MAX_INPUT_HISTORY)
//        SwitchHistory.resize(MAX_INPUT_HISTORY);

    //Debug
    AnimationStation::printfs[0] = std::to_string(SwitchHistory.size()) + " ";
    if(SwitchHistory.size() > 0)
    {
        AnimationStation::printfs[1] = (SwitchHistory[0].bIsHeld ? "true " : "false ") + std::to_string(SwitchHistory[0].ButtonMaskInput) + " ";
        AnimationStation::printfs[2] = std::to_string(SwitchHistory[0].ButtonMaskInput) + " " + std::to_string(SwitchHistory[0].DirectionInput);
        AnimationStation::printfs[3] = std::to_string(to_us_since_boot(SwitchHistory[0].TimeSet)) + " " + std::to_string(to_us_since_boot(SwitchHistory[0].TimeReleased));
    }

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
    //Test Each move
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
                bool bSucceeded = TestForActivatedSpecialMove(&thisMove, comboIndex, triggerIndex);
                if(bSucceeded)
                {
                    // Clear the history to prevent retrigger if animation is really quick
                    ClearHistory();
                    if(ParentAnimationStation)
                        ParentAnimationStation->SetSpecialMoveAnimation(thisMove.Animation, thisMove.RequiredTriggerCombos[triggerIndex].OptionalParams);
                    SwitchClearAwaitingAllRelease = true;

                    return true;
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
    SwitchHistory.resize(0);
}

void SpecialMoveSystem::UpdateHistoryForInput(uint32_t buttonMask, SpecialMoveStickDirection directionHeld, bool bIsPressed)
{
    //search history for recent instance of this button
    for(unsigned int historyIndex = 0; historyIndex < SwitchHistory.size() ; ++historyIndex)
    {
        if(SwitchHistory[historyIndex].ButtonMaskInput == buttonMask && SwitchHistory[historyIndex].DirectionInput == directionHeld)
        {
            //found button. now check if its pressed state is correct
            if(bIsPressed && SwitchHistory[historyIndex].bIsHeld == false)
            {
                //create new entry at start of history array
                InputHistory newHistory;
                newHistory.bIsHeld = true;
                newHistory.TimeSet = thisFrameTime;
                newHistory.ButtonMaskInput = buttonMask;
                newHistory.DirectionInput = directionHeld;
                SwitchHistory.push_front(newHistory);
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

    //no previous entry found. Add new one
    InputHistory newHistory;
    newHistory.bIsHeld = true;
    newHistory.TimeSet = thisFrameTime;
    newHistory.ButtonMaskInput = buttonMask;
    newHistory.DirectionInput = directionHeld;
    SwitchHistory.push_front(newHistory); 
}

void SpecialMoveSystem::GetComboArrayForMove(SpecialMoveInputTypes InputType, std::vector<ComboEntry>& comboArray)
{ 
    switch(InputType)
    {
        case SpecialMoveInputTypes::SPECIALMOVE_INPUT_QUARTER_DOWN_LEFT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
        } break;

        case SpecialMoveInputTypes::SPECIALMOVE_INPUT_DP_RIGHT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_RIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWN, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
        } break;
        
        case SpecialMoveInputTypes::SPECIALMOVE_INPUT_DP_SHORTCUT_RIGHT:
        {
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_DOWNRIGHT, 0));
        } break;
        
        default:
            break;
    }
}

bool SpecialMoveSystem::DoesInputMatch(int HistoryIndex, ComboEntry ComboInput, bool bIsChargeMove)
{
    if(ComboInput.DirectionInput != SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID)
    {
        //direction input check
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

        //other wise just an exact match will do
        return SwitchHistory[HistoryIndex].DirectionInput == ComboInput.DirectionInput;
    }

    //otherwise its button matching
    return SwitchHistory[HistoryIndex].ButtonMaskInput == ComboInput.ButtonMaskInput;
}

bool SpecialMoveSystem::TestForActivatedSpecialMove(SpecialMoveDescription* MoveToTestFor, int ComboIndex, int TriggerIndex)
{
    int lastIndexComboInputWasFound = -1;
    bool bHasFinishedDoingTriggerButtons = false;
    absolute_time_t timeNow = thisFrameTime;
    absolute_time_t timeLastInputSet = thisFrameTime;
    int numMissed = 0;
    bool bAllowMiss = MoveToTestFor->NumRequiredInputCombos >= COMBO_INPUT_COUNT_FOR_ONE_OUT_OF_TWO;
    bool bIsChargeCombo = MoveToTestFor->bIsChargeMove;
    int64_t OptionsChargeTime = Options.ChargeTimeInMs;

    // Get combo array
    std::vector<ComboEntry> comboArray;
    GetComboArrayForMove(MoveToTestFor->RequiredInputCombos[ComboIndex], comboArray);
    int moveLength = comboArray.size();
    
    // put triggers on the end for easier processing
    int triggerLength = 0;
    for(int triggerIndex = 0; triggerIndex < 3; ++triggerIndex)
    {
        int32_t thisTrigger = ((MoveToTestFor->RequiredTriggerCombos[TriggerIndex].RequiredTriggers) >> (4*triggerIndex)) & 0xFF;
        if(thisTrigger != 0)
        {
            triggerLength++;
            comboArray.push_back(ComboEntry(SpecialMoveStickDirection::SPECIALMOVE_STICK_INVALID, thisTrigger));
        }
    }

    int comboLength = moveLength + triggerLength;
    if (comboLength > MAX_INPUT_HISTORY)
        comboLength = MAX_INPUT_HISTORY;

    // Moves are specified in normal order. History is newest first. So start from the end of the combo and work back
    int triggersFound = 0;
    for (int comboIndex = comboLength - 1; comboIndex >= 0; --comboIndex)
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
            for (unsigned int historyIndex = 0; historyIndex < SwitchHistory.size(); ++historyIndex)
            {
                // are we past the time window?
                if (absolute_time_diff_us(SwitchHistory[historyIndex].TimeSet, timeNow) > COMBO_TRIGGER_INPUT_TIME_WINDOW)
                {
                    return false; // we havent managed to find this trigger in time
                }

                if (DoesInputMatch(historyIndex, comboArray[comboIndex], bIsChargeCombo))
                {
                    // we found this trigger. Move onto the next one
                    bFoundTrigger = true;
                    triggersFound++;
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
            if (DoesInputMatch(historyIndex, comboArray[comboIndex], bIsChargeCombo))
            {
                bFoundInput = true;

                // Is this the last input and if so is it a charge input as they're special and
                if (comboIndex == 0 && bIsChargeCombo)
                {
                    // Its a charge move. In which case, this move must be held for CHARGE_COMBO_INPUT_TIME_WINDOW as well as released within COMBO_INPUT_TIME_WINDOW of the last one
                    if (SwitchHistory[historyIndex].bIsHeld == false && absolute_time_diff_us(SwitchHistory[historyIndex].TimeReleased, timeLastInputSet) < COMBO_INPUT_TIME_WINDOW && absolute_time_diff_us(SwitchHistory[historyIndex].TimeSet, SwitchHistory[historyIndex].TimeReleased) > OptionsChargeTime)
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
                if (absolute_time_diff_us(timeToUse, timeLastInputSet) > COMBO_INPUT_TIME_WINDOW)
                {
                    if (bAllowMiss)
                    {
                        // too long between inputs on this one. but might be ok to continue if this was an older input in the history
                        numMissed++;
                        if (numMissed == 2)
                            return false; // too many missing
                    }
                    else
                        return false; // too long between inputs
                }

                // Store this time and location and move onto next input
                timeLastInputSet = SwitchHistory[historyIndex].TimeSet;
                lastIndexComboInputWasFound = historyIndex;
                numMissed = 0;

                break;
            }
        }

        if (bFoundInput == false)
        {
            if (bAllowMiss)
            {
                // didnt find this one. but might be ok to continue
                numMissed++;
                if (numMissed == 2)
                    return false; // too many missing
            }
            else
                return false; // we havent found the input at all
        }
    }

    // if we get here we've found every single input! Combo passed

    return true;
}

void SpecialMoveSystem::UpdateRunningSpecialMove()
{

}