#include "inputcomponent.h"

#include "../UI/uisystem.h"

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

    void InputSystem::deleteInput(const InputSystem::MouseComponent& component)
    {
        auto it = findInputPos(component, mouseComponents);

        if(it != -1)
            mouseComponents.erase(mouseComponents.begin() + it);
    }

    void InputSystem::deleteInput(const InputSystem::KeyComponent& component)
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

        for(auto& component : mouseComponents)
        {
            const auto& mouseArea = component.component;

            // Break of the loop if the current z value is lower than the highest z value in bound
            // Possible because mouseComponents is sorted from highest to lowest Z 
            // TODO need to reorder the list when a Z value is modified
            // if(highestZ > mouseArea->pos->z) // care some edge case exist like listview promoting a Z value so list is not always sorted ! 
            //    break;

            if(mouseArea->inBound(mousePos.x(), mousePos.y()) and *mouseArea->enable and mouseArea->pos->z >= highestZ)
            {
                highestZ = mouseArea->pos->z;
                component.inputCallback(inputHandler, deltaTime);
            }
            else
            {
                if(component.leaveCallback != nullptr)
                    component.leaveCallback(inputHandler, deltaTime);
            }
        }

        for(auto& component : keyComponents)
            component.callback(inputHandler, deltaTime);
    }

    MouseInputComponent::MouseInputComponent(UiComponent *component) : pos(&component->pos), width(&component->width), height(&component->height), enable(&component->isVisible())
    {

    }

    MouseInputComponent::MouseInputComponent(const MouseInputComponent& component) : pos(component.pos), width(component.width), height(component.height), enable(component.enable), object(component.object), onPressed(component.onPressed), onLeave(component.onLeave), onPressedLambda(component.onPressedLambda), onLeaveLambda(component.onLeaveLambda)
    {

    }

    bool MouseInputComponent::inBound(int x, int y) const
    { 
        return x > this->pos->x && x < (this->pos->x + *this->width) && y < (this->pos->y + *this->height) && y > this->pos->y; 
    }

    const InputSystem::MouseComponent& InputSystem::registerMouseArea(MouseInputPtr component, const std::function<void(Input*, double)>& inputCallback, const std::function<void(Input*, double)>& leaveCallback)
    { 
        mouseComponents.emplace_back(component, inputCallback, leaveCallback);
        InputSystem::MouseComponent& returnComponent = mouseComponents.back();

        std::sort(mouseComponents.begin(), mouseComponents.end(), compareZValueFromComponents<MouseComponent>);

        return returnComponent; 
    }

    const InputSystem::KeyComponent& InputSystem::registerKeyInput(KeyInputPtr component, const std::function<void(Input*, double)>& callback)
    {
        keyComponents.emplace_back(component, callback);
        InputSystem::KeyComponent& returnComponent = keyComponents.back();

        return returnComponent;
    }

    const InputSystem::MouseComponent& makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), void (*mouseLeave)(Input*, double) = nullptr)
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<MouseInputComponent::Base>(component));
        mouseArea->registerFunc(mouseInput, mouseLeave);

        auto inputCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->call(inputHandler, deltaTime); };

        std::function<void(Input*, double)> leaveCallback;
        if(mouseLeave != nullptr)
            leaveCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->leave(inputHandler, deltaTime); };
        else
            leaveCallback = nullptr;

        return system->registerMouseArea(mouseArea, inputCallback, leaveCallback);
    }

    const InputSystem::MouseComponent& makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), std::nullptr_t)
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<MouseInputComponent::Base>(component));
        mouseArea->registerFunc(mouseInput, static_cast<void (*)(pg::Input*, double)>(nullptr));

        auto inputCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->call(inputHandler, deltaTime); };

        return system->registerMouseArea(mouseArea, inputCallback, static_cast<std::function<void(pg::Input*, double)>>(nullptr));
    }

}