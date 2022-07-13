#include "inputcomponent.h"

#include "../UI/uisystem.h"

#include <iostream>

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
                std::cout << "Indice " << left.indice->index << " with z = " << left.component->pos->z <<
                " is swapping with Indice " << right.indice->index << " with z = " << right.component->pos->z << std::endl;
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

        if(not mouseDeleteList.empty())
        {
            for(int i = 0; i < mouseDeleteList.size(); i++)
            {
                auto indice = mouseDeleteList[i];

                auto index = indice->index;

                std::cout << "Removing index: " << index << std::endl;

                if(indice->prev != nullptr)
                    indice->prev->next = indice->next;

                InputIndice* fIndice = firstMouseIndice;

                while(firstMouseIndice != nullptr && fIndice->next != nullptr)
                {
                    if(fIndice->index > index)
                        fIndice->index--;

                    fIndice = fIndice->next;
                }

                mouseComponents.erase(mouseComponents.begin() + index);

                delete indice;
            }

            mouseDeleteList.clear();

            reorderMouse();
        }

        if(not keyDeleteList.empty())
        {
            for(int i = 0; i < keyDeleteList.size(); i++)
            {
                auto indice = keyDeleteList[i];

                auto index = indice->index;

                if(indice->prev != nullptr)
                    indice->prev->next = indice->next;

                InputIndice* fIndice = firstKeyIndice;

                while(firstKeyIndice != nullptr && fIndice->next != nullptr)
                {
                    if(fIndice->index > index)
                        fIndice->index--;

                    fIndice = fIndice->next;
                }

                keyComponents.erase(keyComponents.begin() + index);

                delete indice;
            }

            keyDeleteList.clear();
        }

        // for(auto& component : mouseComponents)
        for(int i = 0; i < mouseComponents.size(); i++)
        {
            auto& component = mouseComponents[i];
            const auto& mouseArea = component.component;

            // Break of the loop if the current z value is lower than the highest z value in bound
            // Possible because mouseComponents is sorted from highest to lowest Z 
            // TODO need to reorder the list when a Z value is modified
            if(highestZ > mouseArea->pos->z) // care some edge case exist like listview promoting a Z value so list is not always sorted ! 
                break;

            std::cout << "[" << i << "]: "  << mouseComponents[i].indice->index << " with z=" << mouseArea->pos->z << std::endl;

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
        MouseInput input;
        input.indice = new InputIndice();

        auto indice = findLastMouseIndice();
        if(indice == nullptr)
        {
            firstMouseIndice = input.indice;
        }
        else
        {
            input.indice->prev = indice;
            indice->next = input.indice;
            input.indice->index = mouseComponents.size();
        }

        mouseComponents.emplace_back(component, input.indice, inputCallback, leaveCallback);

        reorderMouse();

        return input;
    }

    KeyInput InputSystem::registerKeyInput(KeyInputPtr component, const std::function<void(Input*, double)>& callback)
    {
        KeyInput input;
        input.indice = new InputIndice();

        auto indice = findLastKeyIndice();
        if(indice == nullptr)
        {
            firstKeyIndice = input.indice;
        }
        else
        {
            input.indice->prev = indice;
            indice->next = input.indice;
            input.indice->index = keyComponents.size();
        }

        keyComponents.emplace_back(component, input.indice, callback);

        return input;
    }

    void InputSystem::reorderMouse()
    {
        // Usage of stable_sort to preserve the order of inputs that has the same z value !
        std::stable_sort(mouseComponents.begin(), mouseComponents.end(), compareZValueFromComponents<InputSystem::MouseComponent>);
    };

    void MouseInput::changeZ(const UiSize& zOrder) const
    {
        auto& system = InputSystem::system();
        system->mouseComponents[indice->index].component->pos->z = zOrder;
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

    void deleteInput(const MouseInput& input)
    {
        input.deleteInput();
    }

    void deleteInput(const KeyInput& input)
    {
        input.deleteInput();
    }

}