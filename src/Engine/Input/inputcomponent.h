#pragma once

#include "input.h"

#include "../constant.h"

#include "ECS/entitysystem.h"
#include "ECS/callable.h"
#include "UI/uisystem.h"

#include <functional>
#include <memory>

namespace pg
{
    struct MouseLeftClickComponent
    {
        MouseLeftClickComponent(CallablePtr callback) : callback(callback) { LOG_THIS_MEMBER("MouseLeftClickComponent"); }
        MouseLeftClickComponent(const MouseLeftClickComponent& rhs) : callback(rhs.callback) { LOG_THIS_MEMBER("MouseLeftClickComponent"); }
        virtual ~MouseLeftClickComponent() { LOG_THIS_MEMBER("MouseLeftClickComponent"); }

        CallablePtr callback;
    };

    struct MouseRightClickComponent
    {
        MouseRightClickComponent(CallablePtr callback) : callback(callback) { LOG_THIS_MEMBER("MouseRightClickComponent"); }
        MouseRightClickComponent(const MouseRightClickComponent& rhs) : callback(rhs.callback) { LOG_THIS_MEMBER("MouseRightClickComponent"); }
        virtual ~MouseRightClickComponent() { LOG_THIS_MEMBER("MouseRightClickComponent"); }

        CallablePtr callback;
    };

    struct MouseLeaveClickComponent
    {
        MouseLeaveClickComponent(CallablePtr callback) : callback(callback) { LOG_THIS_MEMBER("MouseLeaveClickComponent"); }
        MouseLeaveClickComponent(const MouseLeaveClickComponent& rhs) : callback(rhs.callback) { LOG_THIS_MEMBER("MouseLeaveClickComponent"); }
        virtual ~MouseLeaveClickComponent() { LOG_THIS_MEMBER("MouseLeaveClickComponent"); }

        CallablePtr callback;
    };

    struct OnMouseClick {};

    struct OnSDLTextInput
    {
        std::string text;
    };

    struct OnSDLScanCode
    {
        SDL_Scancode key;
    };

    struct OnSDLScanCodePressed
    {
        SDL_Scancode key;
    };

    struct MouseAreaZ
    {
        MouseAreaZ(_unique_id id, CompRef<UiComponent> ui) : id(id), ui(ui) { LOG_THIS_MEMBER("MouseArea"); }

        _unique_id id;
        CompRef<UiComponent> ui;
    };

    struct MouseLeftClickSystem : public System<Own<MouseLeftClickComponent>, NamedSystem, InitSys>
    {
        MouseLeftClickSystem(Input* inputHandler) : inputHandler(inputHandler) { LOG_THIS_MEMBER("MouseLeftClickSystem"); }

        virtual std::string getSystemName() const override { return "Mouse Left Click System"; }

        virtual void init() override
        {
            LOG_THIS_MEMBER("MouseLeftClickSystem");

            auto group = registerGroup<UiComponent, MouseLeftClickComponent>();

            group->addOnGroup([](EntityRef entity) {
                LOG_MILE("MouseLeftClickSystem", "Add entity " << entity->id << " to ui - mouse left click group !");

                auto sys = entity->world()->getSystem<MouseLeftClickSystem>();

                const auto& ui = entity->get<UiComponent>();
                
                sys->mouseAreaHolder.emplace(entity->id, ui);
            });
        }

        virtual void execute() override;

        Input *inputHandler;
        std::set<MouseAreaZ, std::greater<>> mouseAreaHolder;
    };

    struct MouseRightClickSystem : public System<Own<MouseRightClickComponent>, NamedSystem, InitSys>
    {
        MouseRightClickSystem(Input* inputHandler) : inputHandler(inputHandler) { LOG_THIS_MEMBER("MouseRightClickSystem"); }

        virtual std::string getSystemName() const override { return "Mouse Right Click System"; }

        virtual void init() override
        {
            LOG_THIS_MEMBER("MouseRightClickSystem");

            auto group = registerGroup<UiComponent, MouseRightClickComponent>();

            group->addOnGroup([](EntityRef entity) {
                LOG_MILE("MouseRightClickSystem", "Add entity " << entity->id << " to ui - mouse right click group !");

                auto sys = entity->world()->getSystem<MouseRightClickSystem>();

                const auto& ui = entity->get<UiComponent>();
                
                sys->mouseAreaHolder.emplace(entity->id, ui);
            });
        }

        virtual void execute() override;

        Input *inputHandler;
        std::set<MouseAreaZ, std::greater<>> mouseAreaHolder;
    };

    struct MouseLeaveClickSystem : public System<Listener<OnMouseClick>, Own<MouseLeaveClickComponent>, NamedSystem, InitSys, StoragePolicy>
    {
        MouseLeaveClickSystem(Input* inputHandler) : inputHandler(inputHandler) { LOG_THIS_MEMBER("MouseLeaveClickSystem"); }

        virtual std::string getSystemName() const override { return "Mouse Leave Click System"; }

        virtual void init() override
        {
            LOG_THIS_MEMBER("MouseLeaveClickSystem");

            auto group = registerGroup<UiComponent, MouseLeaveClickComponent>();

            group->addOnGroup([](EntityRef entity) {
                LOG_MILE("MouseLeaveClickSystem", "Add entity " << entity->id << " to ui - mouse leave click group !");

                auto sys = entity->world()->getSystem<MouseLeaveClickSystem>();

                const auto& ui = entity->get<UiComponent>();
                
                sys->mouseAreaHolder.emplace(entity->id, ui);
            });
        }

        virtual void onEvent(const OnMouseClick&) override
        {
            LOG_THIS_MEMBER("MouseLeaveClickSystem");

            const auto& mousePos = inputHandler->getMousePos();

            for(const auto& mouseArea : mouseAreaHolder)
            {
                UiComponent *ui = mouseArea.ui;

                if(not ui->inBound(mousePos.x, mousePos.y))
                {
                    auto comp = getComponent(mouseArea.id);

                    comp->callback->call(world());
                }
            }
        }

        Input *inputHandler;
        std::set<MouseAreaZ, std::less<>> mouseAreaHolder;
    };

    

    bool operator<(const MouseAreaZ& lhs, const MouseAreaZ& rhs);
    bool operator>(const MouseAreaZ& lhs, const MouseAreaZ& rhs);

}