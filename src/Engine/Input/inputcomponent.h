#pragma once

#include "input.h"

#include "constant.h"

#include "ECS/entitysystem.h"
#include "ECS/callable.h"
#include "UI/uisystem.h"

#include <functional>
#include <memory>

namespace pg
{
    enum class MouseStateTrigger : uint8_t
    {
        OnPress = 0,
        OnRelease,
        Both
    };

    struct MouseLeftClickComponent
    {
        MouseLeftClickComponent(CallablePtr callback, const MouseStateTrigger& trigger = MouseStateTrigger::OnRelease) : callback(callback), trigger(trigger) { LOG_THIS_MEMBER("MouseLeftClickComponent"); }
        MouseLeftClickComponent(const MouseLeftClickComponent& rhs) : callback(rhs.callback), trigger(rhs.trigger) { LOG_THIS_MEMBER("MouseLeftClickComponent"); }
        virtual ~MouseLeftClickComponent() { LOG_THIS_MEMBER("MouseLeftClickComponent"); }

        CallablePtr callback;
        MouseStateTrigger trigger;
    };

    template<>
    void serialize(Archive& archive, const MouseLeftClickComponent& component);

    struct MouseRightClickComponent
    {
        MouseRightClickComponent(CallablePtr callback, const MouseStateTrigger& trigger = MouseStateTrigger::OnRelease) : callback(callback), trigger(trigger) { LOG_THIS_MEMBER("MouseRightClickComponent"); }
        MouseRightClickComponent(const MouseRightClickComponent& rhs) : callback(rhs.callback), trigger(rhs.trigger) { LOG_THIS_MEMBER("MouseRightClickComponent"); }
        virtual ~MouseRightClickComponent() { LOG_THIS_MEMBER("MouseRightClickComponent"); }

        CallablePtr callback;
        MouseStateTrigger trigger;
    };

    struct MouseLeaveClickComponent
    {
        MouseLeaveClickComponent(CallablePtr callback) : callback(callback) { LOG_THIS_MEMBER("MouseLeaveClickComponent"); }
        MouseLeaveClickComponent(const MouseLeaveClickComponent& rhs) : callback(rhs.callback) { LOG_THIS_MEMBER("MouseLeaveClickComponent"); }
        virtual ~MouseLeaveClickComponent() { LOG_THIS_MEMBER("MouseLeaveClickComponent"); }

        CallablePtr callback;
    };

    struct OnMouseClick
    {
        OnMouseClick(const MousePos& pos, const MouseButton& button) : pos(pos), button(button) { }
        OnMouseClick(const OnMouseClick& other) : pos(other.pos), button(other.button) { }

        OnMouseClick& operator=(const OnMouseClick& other)
        {
            pos = other.pos;
            button = other.button;

            return *this;
        }

        MousePos pos;
        MouseButton button;
    };

    struct OnMouseMove
    {
        MousePos pos;
        Input *inputHandler;
    };

    struct OnSDLTextInput
    {
        std::string text;
    };

    struct OnSDLScanCode
    {
        SDL_Scancode key;
    };

    struct OnSDLScanCodeReleased
    {
        SDL_Scancode key;
    };

    struct OnSDLGamepadPressed
    {
        int id;

        unsigned int button;
    };

    struct OnSDLGamepadReleased
    {
        int id;

        unsigned int button;
    };

    struct OnSDLGamepadAxisChanged
    {
        int id;

        unsigned int axis;

        int value;
    };

    struct MouseAreaZ
    {
        MouseAreaZ(_unique_id id, CompRef<UiComponent> ui) : id(id), ui(ui) { LOG_THIS_MEMBER("MouseArea"); }

        _unique_id id;
        CompRef<UiComponent> ui;
    };

    struct MouseClickSystem : public System<Own<MouseLeftClickComponent>, Own<MouseRightClickComponent>, Ref<UiComponent>, NamedSystem, InitSys>
    {
        MouseClickSystem(Input* inputHandler) : inputHandler(inputHandler) { LOG_THIS_MEMBER("MouseClickSystem"); }

        virtual std::string getSystemName() const override { return "Mouse Click System"; }

        virtual void init() override;

        virtual void execute() override;

        void handleClick(const MouseButton& button, const std::set<MouseAreaZ, std::greater<>>& pressAreas, const std::set<MouseAreaZ, std::greater<>>& releaseAreas);

        void callCallback(const MouseButton& button, _unique_id id);

        Input *inputHandler;
        MousePos oldMousePos;
        std::set<MouseAreaZ, std::greater<>> mouseLeftAreaPressHolder;
        std::set<MouseAreaZ, std::greater<>> mouseLeftAreaReleaseHolder;
        std::set<MouseAreaZ, std::greater<>> mouseRightAreaPressHolder;
        std::set<MouseAreaZ, std::greater<>> mouseRightAreaReleaseHolder;
        std::unordered_map<MouseButton, bool> pressedList;
    };

    // Todo combine this in the MouseClickSystem
    struct MouseLeaveClickSystem : public System<Listener<OnMouseClick>, Own<MouseLeaveClickComponent>, Ref<UiComponent>, NamedSystem, InitSys, StoragePolicy>
    {
        MouseLeaveClickSystem(Input* inputHandler) : inputHandler(inputHandler) { LOG_THIS_MEMBER("MouseLeaveClickSystem"); }

        virtual std::string getSystemName() const override { return "Mouse Leave Click System"; }

        virtual void init() override
        {
            LOG_THIS_MEMBER("MouseLeaveClickSystem");

            auto group = registerGroup<UiComponent, MouseLeaveClickComponent>();

            group->addOnGroup([this](EntityRef entity) {
                LOG_MILE("MouseLeaveClickSystem", "Add entity " << entity->id << " to ui - mouse leave click group !");

                auto ui = entity->get<UiComponent>();
                
                mouseAreaHolder.emplace(entity->id, ui);
            });

            group->removeOfGroup([this](EntitySystem*, _unique_id id) {
                LOG_MILE("MouseLeaveClickSystem", "Add entity " << id << " to ui - mouse leave click group !");

                const auto& it = std::find_if(mouseAreaHolder.begin(), mouseAreaHolder.end(), [id](const MouseAreaZ& area) { return area.id == id; });

                if (it != mouseAreaHolder.end())
                {
                    mouseAreaHolder.erase(it);
                }
            });
        }

        virtual void onEvent(const OnMouseClick&) override
        {
            LOG_THIS_MEMBER("MouseLeaveClickSystem");

            auto mousePos = inputHandler->getMousePos();

            for (auto mouseArea : mouseAreaHolder)
            {
                UiComponent *ui = mouseArea.ui;

                if (not ui->inClipBound(mousePos.x, mousePos.y))
                {
                    auto comp = getComponent(mouseArea.id);

                    comp->callback->call(world());
                }
            }
        }

        Input *inputHandler;
        std::set<MouseAreaZ, std::less<>> mouseAreaHolder;
    };

    bool operator<(MouseAreaZ lhs, MouseAreaZ rhs);
    bool operator>(MouseAreaZ lhs, MouseAreaZ rhs);

}