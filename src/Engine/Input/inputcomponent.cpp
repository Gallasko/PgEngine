#include "inputcomponent.h"

#include "../UI/uisystem.h"

#include <iostream>

namespace pg
{
    namespace
    {
    }

    bool operator<(const MouseClickSystem::MouseAreaZ& lhs, const MouseClickSystem::MouseAreaZ& rhs)
    {
        const auto& z = lhs.ui->pos.z;
        const auto& rhsZ = rhs.ui->pos.z;

        if(z == rhsZ)
            return lhs.id < rhs.id;
        else
            return z < rhsZ;
    }

    bool operator>(const MouseClickSystem::MouseAreaZ& lhs, const MouseClickSystem::MouseAreaZ& rhs)
    {
        const auto& z = lhs.ui->pos.z;
        const auto& rhsZ = rhs.ui->pos.z;

        if(z == rhsZ)
            return lhs.id > rhs.id;
        else
            return z > rhsZ;
    }

    MouseInputComponent::MouseInputComponent(UiComponent *component) : pos(&component->pos), width(&component->width), height(&component->height), enable(&component->isVisible())
    {

    }

    MouseInputComponent::MouseInputComponent(const MouseInputComponent& component) : pos(component.pos), width(component.width), height(component.height), enable(component.enable), object(component.object), onPressed(component.onPressed), onLeave(component.onLeave), onPressedLambda(component.onPressedLambda), onLeaveLambda(component.onLeaveLambda)
    {

    }

    bool MouseInputComponent::inBound(int x, int y) const
    { 
        return x > static_cast<UiSize>(this->pos->x) && x < (this->pos->x + *this->width) && y < (this->pos->y + *this->height) && y > static_cast<UiSize>(this->pos->y); 
    }

    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, void (*mouseInput)(Input*, double), void (*mouseLeave)(Input*, double))
    {
        if(not ecs)
            return nullptr;

        MouseInputPtr mouseArea = std::make_shared<MouseInputBase<MouseInputComponent::Base>>(component);
        mouseArea->registerFunc(mouseInput, mouseLeave);

        auto ent = ecs->createEntity();

        ecs->attach<MouseComponent>(ent, mouseArea);

        return ent;
    }

    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, void (*mouseInput)(Input*, double), std::nullptr_t)
    {
        if(not ecs)
            return nullptr;

        MouseInputPtr mouseArea = std::make_shared<MouseInputBase<MouseInputComponent::Base>>(component);
        mouseArea->registerFunc(mouseInput, static_cast<void (*)(pg::Input*, double)>(nullptr));
        
        auto ent = ecs->createEntity();

        ecs->attach<MouseComponent>(ent, mouseArea);

        return ent;
    }

    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, void (*mouseInput)(Input*, double))
    {
        return makeMouseArea(ecs, component, mouseInput, nullptr);
    }

    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, const std::function<void (Input*, double)>& mouseInput, const std::function<void (Input*, double)>& mouseLeave)
    {
        if(not ecs)
            return nullptr;

        MouseInputPtr mouseArea = std::make_shared<MouseInputBase<MouseInputComponent::Base>>(component);
        mouseArea->registerFunc(mouseInput, mouseLeave);

        auto ent = ecs->createEntity();

        ecs->attach<MouseComponent>(ent, mouseArea);

        return ent;
    }

    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, const std::function<void (Input*, double)>& mouseInput, std::nullptr_t)
    {
        if(not ecs)
            return nullptr;

        MouseInputPtr mouseArea = std::make_shared<MouseInputBase<MouseInputComponent::Base>>(component);
        mouseArea->registerFunc(mouseInput, static_cast<std::function<void (Input*, double)>>(nullptr));

        auto ent = ecs->createEntity();

        ecs->attach<MouseComponent>(ent, mouseArea);

        return ent;
    }

    Entity* makeMouseArea(EntitySystem *ecs, UiComponent *component, const std::function<void (Input*, double)>& mouseInput)
    {
        return makeMouseArea(ecs, component, mouseInput, nullptr);
    }

}