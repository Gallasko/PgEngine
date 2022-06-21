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
            int index = 0;
            if(left.component->pos->z > right.component->pos->z)
            {
                index = left.indice->index;
                left.indice->index = right.indice->index;
                right.indice->index = index;
            }

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

    MouseInput InputSystem::registerMouseArea(MouseInputPtr component, const std::function<void(Input*, double)>& inputCallback, const std::function<void(Input*, double)>& leaveCallback)
    {
        // This create an unique ptr of the component to not invalidate the ref to the component
        // mouseComponents.push_back(new InputSystem::MouseComponent(component, inputCallback, leaveCallback));
        mouseComponents.emplace_back(component, inputCallback, leaveCallback);
        MouseInput input;

        auto indice = findLastIndice();
        if(indice == nullptr)
        {
            firstIndice = &(input.indice);
        }
        else
        {
            indice->next = &(input.indice);
            input.indice.index = indice->index + 1;
        }

        std::sort(mouseComponents.begin(), mouseComponents.end(), compareZValueFromComponents<InputSystem::MouseComponent>);

        return input;
    }

    const InputSystem::KeyComponent& InputSystem::registerKeyInput(KeyInputPtr component, const std::function<void(Input*, double)>& callback)
    {
        // This create an unique ptr of the component to not invalidate the ref to the component
        // keyComponents.emplace_back(new InputSystem::KeyComponent(component, callback));
        

        // TODO

        //mouseComponents.push_back(std::make_shared<InputSystem::KeyComponent>(component, callback));
        //const InputSystem::KeyComponent& returnComponent = *keyComponents.back(); // TODO change this cause an element of a vector can be invalidate at any point of the app !

        //return returnComponent;
    }

    void MouseInput::changeZ(const UiSize& zOrder) const
    {
        auto& system = InputSystem::system();
        system->mouseComponents[indice.index].component->pos->z = zOrder;
        system->reorderMouse();
    }

    MouseInput makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), void (*mouseLeave)(Input*, double))
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

    MouseInput makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), std::nullptr_t)
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<MouseInputComponent::Base>(component));
        mouseArea->registerFunc(mouseInput, static_cast<void (*)(pg::Input*, double)>(nullptr));

        auto inputCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->call(inputHandler, deltaTime); };

        return system->registerMouseArea(mouseArea, inputCallback, static_cast<std::function<void(pg::Input*, double)>>(nullptr));
    }

    MouseInput makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double))
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<MouseInputComponent::Base>(component));
        mouseArea->registerFunc(mouseInput, static_cast<void (*)(pg::Input*, double)>(nullptr));

        auto inputCallback = [=](Input* inputHandler, double deltaTime) { mouseArea->call(inputHandler, deltaTime); };

        return system->registerMouseArea(mouseArea, inputCallback, static_cast<std::function<void(pg::Input*, double)>>(nullptr));
    }

    MouseInput makeMouseArea(UiComponent *component, const std::function<void(pg::Input*, double)>& mouseInput)
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<MouseInputComponent::Base>(component));

        return system->registerMouseArea(mouseArea, mouseInput, static_cast<std::function<void(pg::Input*, double)>>(nullptr));
    }

    MouseInput makeMouseArea(UiComponent *component, const std::function<void(pg::Input*, double)>& mouseInput, const std::function<void(pg::Input*, double)>& mouseLeave)
    {
        auto& system = InputSystem::system();

        auto mouseArea = MouseInputPtr(new MouseInputBase<MouseInputComponent::Base>(component));

        return system->registerMouseArea(mouseArea, mouseInput, mouseLeave);
    }

    MouseInput makeMouseArea(UiComponent *component, const InputSystem::MouseComponent& mouseArea)
    {
        auto& system = InputSystem::system();

        auto mArea = MouseInputPtr(new MouseInputBase<MouseInputComponent::Base>(component));

        return system->registerMouseArea(mArea, mouseArea.inputCallback, mouseArea.leaveCallback);
    }

}