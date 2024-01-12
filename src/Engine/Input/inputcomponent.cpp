#include "inputcomponent.h"

#include "../UI/uisystem.h"

#include <iostream>

#ifdef __EMSCRIPTEN__
#include <SDL.h>
#include <SDL_opengl.h>
#include <emscripten.h>
#else
    #ifdef __linux__
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_opengl.h>
    #elif _WIN32
    #include <SDL.h>
    #include <SDL_opengl.h>
    #endif
#endif

#include "serialization.h"

namespace pg
{
    namespace
    {
    }

    template<>
    void serialize(Archive& archive, const MouseLeftClickComponent& component)
    {
        archive.startSerialization("Mouse Left Click Component");

        component.callback->serialize(archive);

        archive.endSerialization();
    }

    void MouseLeftClickSystem::execute()
    {
        LOG_THIS_MEMBER("MouseLeftClickSystem");

        int highestZ = INT_MIN;
        const auto& mousePos = inputHandler->getMousePos();

        static bool pressed = false;

        if(inputHandler->isButtonPressed(SDL_BUTTON_LEFT))
        {
            if(not pressed)
                ecsRef->sendEvent(OnMouseClick{});

            pressed = true;
        }

        if(not inputHandler->isButtonPressed(SDL_BUTTON_LEFT))
        {
            if(pressed)
            {
                for(const auto& mouseArea : mouseAreaHolder)
                {
                    UiComponent *ui = mouseArea.ui;

                    if(ui->pos.z < highestZ)
                        break;

                    if(ui->inClipBound(mousePos.x, mousePos.y))
                    {
                        highestZ = ui->pos.z;

                        auto comp = getComponent(mouseArea.id);

                        comp->callback->call(world());
                    }
                }
            }

            pressed = false;
        }
        
    }

    void MouseRightClickSystem::execute()
    {
        LOG_THIS_MEMBER("MouseRightClickSystem");

        int highestZ = INT_MIN;
        const auto& mousePos = inputHandler->getMousePos();

        static bool pressed = false;

        if(inputHandler->isButtonPressed(SDL_BUTTON_RIGHT))
        {
            if(not pressed)
                ecsRef->sendEvent(OnMouseClick{});

            pressed = true;
        }

        if(not inputHandler->isButtonPressed(SDL_BUTTON_RIGHT))
        {
            if(pressed)
            {
                for(const auto& mouseArea : mouseAreaHolder)
                {
                    UiComponent *ui = mouseArea.ui;

                    if(ui->pos.z < highestZ)
                        break;

                    if(ui->inClipBound(mousePos.x, mousePos.y))
                    {
                        highestZ = ui->pos.z;

                        auto comp = getComponent(mouseArea.id);

                        comp->callback->call(world());
                    }
                }
            }

            pressed = false;
        }
        
    }

    bool operator<(const MouseAreaZ& lhs, const MouseAreaZ& rhs)
    {
        const auto& z = lhs.ui->pos.z;
        const auto& rhsZ = rhs.ui->pos.z;

        if(z == rhsZ)
            return lhs.id < rhs.id;
        else
            return z < rhsZ;
    }

    bool operator>(const MouseAreaZ& lhs, const MouseAreaZ& rhs)
    {
        const auto& z = lhs.ui->pos.z;
        const auto& rhsZ = rhs.ui->pos.z;

        if(z == rhsZ)
            return lhs.id > rhs.id;
        else
            return z > rhsZ;
    }
}