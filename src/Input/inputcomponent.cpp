#include "inputcomponent.h"

namespace pg
{
    namespace
    {
        template <typename Container, typename Value>
        int findInputPos(const Value& value, const Container& container)
        {
            for (long long unsigned int i = 0; i < container.size(); i++)
                if(value == container.at(i))
                    return i;

            return -1;
        }

        template<typename InputHolder>
        bool compareZValueFromComponents(const InputHolder& left, const InputHolder& right)
        {
            return left.component->pos->z > right.component->pos->z;
        }
    }

    void InputSystem::deleteInput(const MouseComponent& component)
    {
        auto it = findInputPos(component, mouseComponents);

        if(it != -1)
            mouseComponents.erase(mouseComponents.begin() + it);
    }

    void InputSystem::deleteInput(const KeyComponent& component)
    {
        auto it = findInputPos(component, keyComponents);

        if(it != -1)
            keyComponents.erase(keyComponents.begin() + it);
    }

    void InputSystem::updateState(Input* inputHandler, double deltaTime)
    {
        // Lowest value of an integer
        int highestZ = INT_MIN;
        const auto& mousePos = inputHandler->getMousePos();

        for( auto& component : mouseComponents)
        {
            const auto& mouseArea = component.component;

            // Break of the loop if the current z value is lower than the highest z value in bound
            // Possible because mouseComponents is sorted from highest to lowest Z
            if(highestZ > mouseArea->pos->z)
                break;

            if(mouseArea->inBound(mousePos.x(), mousePos.y()) and *mouseArea->enable)
            {
                highestZ = mouseArea->pos->z;
                component.callback(inputHandler, deltaTime);
            }
        }

        for(auto& component : keyComponents)
            component.callback(inputHandler, deltaTime);
    }

    const MouseComponent& InputSystem::registerMouseArea(MouseInputPtr component, std::function<void(Input*, double)> callback)
    { 
        mouseComponents.emplace_back(component, callback);
        MouseComponent& returnComponent = mouseComponents.back();

        std::sort(mouseComponents.begin(), mouseComponents.end(), compareZValueFromComponents<MouseComponent>);

        return returnComponent; 
    }
    const KeyComponent& InputSystem::registerKeyInput(KeyInputPtr component, std::function<void(Input*, double)> callback)
    {
        keyComponents.emplace_back(component, callback);
        KeyComponent& returnComponent = keyComponents.back();

        return returnComponent;
    }

}