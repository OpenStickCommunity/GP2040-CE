#include "SpecialMoveSystem.hpp"

#include "AnimationStation.hpp"

SpecialMoveOptions SpecialMoveSystem::Options = {};

SpecialMoveSystem::SpecialMoveSystem()
{
}

void SpecialMoveSystem::Update()
{
    thisFrameTime = get_absolute_time();

    //trim history if required
    while(SwitchHistory.size() > MAX_INPUT_HISTORY)
        SwitchHistory.pop_front();
}

void SpecialMoveSystem::SetOptions(SpecialMoveOptions InOptions)
{
    Options = InOptions;
}

void SpecialMoveSystem::HandlePressedButtons(uint32_t pressedButtons)
{
    //check each input and update history
    for(int buttonMask = 1; buttonMask < (1 << 20); buttonMask = buttonMask << 1)
    {
        UpdateHistoryForInput(buttonMask, (pressedButtons & buttonMask) != 0);
    }
}

void SpecialMoveSystem::ClearHistory()
{
    SwitchHistory.clear();
}

void SpecialMoveSystem::UpdateHistoryForInput(uint32_t buttonMask, bool bIsPressed)
{
    //search history for recent instance of this button
    for(unsigned int historyIndex = 0; historyIndex < SwitchHistory.size() ; ++historyIndex)
    {
        if(SwitchHistory[historyIndex].Input == buttonMask)
        {
            //found button. now check if its pressed state is correct
            if(bIsPressed && SwitchHistory[historyIndex].bIsHeld == false)
            {
                //create new entry at start of history array
                InputHistory newHistory;
                newHistory.bIsHeld = true;
                newHistory.TimeSet = thisFrameTime;
                newHistory.Input = buttonMask;
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
    newHistory.Input = buttonMask;
    SwitchHistory.push_front(newHistory); 
}

void SpecialMoveSystem::TestForActivatedSpecialMove()
{

}

void SpecialMoveSystem::UpdateRunningSpecialMove()
{

}