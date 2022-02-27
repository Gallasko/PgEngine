#include "inputcomponent.h"

namespace pg
{
    void InputSystem::updateState(Input* inputHandler, double deltaTime)
    {
        // Lowest value of an integer
        int highestZ = INT_MIN;
        const auto& mousePos = inputHandler->getMousePos();

        for(const auto& mouseArea : mouseComponents)
        {
            if(mouseArea->inBound(mousePos.x(), mousePos.y()) and *mouseArea->enable)
                if (mouseArea->pos->z > highestZ)
                    highestZ = mouseArea->pos->z;
        }

        for(unsigned int i = 0; i < mouseComponents.size(); i++)
        {
            const auto& mouseArea = mouseComponents[i];

            if(mouseArea->inBound(mousePos.x(), mousePos.y()) and *mouseArea->enable and mouseArea->pos->z == highestZ)
            {
                mouseCallbacks[i](inputHandler, deltaTime);
            }
        }

        for(auto& callback : keyCallbacks)
            callback(inputHandler, deltaTime);
    }
}