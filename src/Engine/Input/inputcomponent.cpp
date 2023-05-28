#include "inputcomponent.h"

#include "../UI/uisystem.h"

#include <iostream>

#ifdef __linux__
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#elif _WIN32
#include <SDL.h>
#include <SDL_opengl.h>
#endif

namespace pg
{
    namespace
    {
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

        if(not inputHandler->isButtonPressed(SDL_BUTTON_RIGHT))
        {
            if(pressed)
            {
                for(const auto& mouseArea : mouseAreaHolder)
                {
                    UiComponent *ui = mouseArea.ui;

                    if(ui->pos.z < highestZ)
                        break;

                    if(ui->inBound(mousePos.x, mousePos.y))
                    {
                        highestZ = static_cast<UiSize>(ui->pos.z);

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

                    if(ui->inBound(mousePos.x, mousePos.y))
                    {
                        highestZ = static_cast<UiSize>(ui->pos.z);

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