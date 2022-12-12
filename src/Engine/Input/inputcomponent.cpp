#include "inputcomponent.h"

#include "../UI/uisystem.h"

#include <iostream>

namespace pg
{
    namespace
    {
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

    MouseComponent* makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), void (*mouseLeave)(Input*, double), EntitySystem *ecs)
    {
        // Todo catch errors when world isn't initialized in Uicomponent (often happens when calling makemousearea on a newly created uicomponent)
        auto mouseArea = MouseInputPtr(new MouseInputBase<MouseInputComponent::Base>(component));
        mouseArea->registerFunc(mouseInput, mouseLeave);

        if(ecs)
            return ecs->attach<MouseComponent>(component, mouseArea);

        return component->world()->attach<MouseComponent>(component, mouseArea);
    }

    MouseComponent* makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), std::nullptr_t, EntitySystem *ecs)
    {
        auto mouseArea = MouseInputPtr(new MouseInputBase<MouseInputComponent::Base>(component));
        mouseArea->registerFunc(mouseInput, static_cast<void (*)(pg::Input*, double)>(nullptr));

        if(ecs)
            return ecs->attach<MouseComponent>(component, mouseArea);

        return component->world()->attach<MouseComponent>(component, mouseArea);
    }

    MouseComponent* makeMouseArea(UiComponent *component, void (*mouseInput)(Input*, double), EntitySystem *ecs)
    {
        return makeMouseArea(component, mouseInput, nullptr, ecs);
    }

    MouseComponent* makeMouseArea(UiComponent *component, const std::function<void (Input*, double)>& mouseInput, const std::function<void (Input*, double)>& mouseLeave, EntitySystem *ecs)
    {
        auto mouseArea = MouseInputPtr(new MouseInputBase<MouseInputComponent::Base>(component));
        mouseArea->registerFunc(mouseInput, mouseLeave);

        if(ecs)
            return ecs->attach<MouseComponent>(component, mouseArea);

        return component->world()->attach<MouseComponent>(component, mouseArea);
    }

    MouseComponent* makeMouseArea(UiComponent *component, const std::function<void (Input*, double)>& mouseInput, std::nullptr_t, EntitySystem *ecs)
    {
        auto mouseArea = MouseInputPtr(new MouseInputBase<MouseInputComponent::Base>(component));
        mouseArea->registerFunc(mouseInput, static_cast<std::function<void (Input*, double)>>(nullptr));

        if(ecs)
            return ecs->attach<MouseComponent>(component, mouseArea);

        return component->world()->attach<MouseComponent>(component, mouseArea);
    }

    MouseComponent* makeMouseArea(UiComponent *component, const std::function<void (Input*, double)>& mouseInput, EntitySystem *ecs)
    {
        return makeMouseArea(component, mouseInput, nullptr, ecs);
    }

}