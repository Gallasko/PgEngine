#pragma once

#include "uisystem.h"

#include "../Input/inputcomponent.h"

namespace pg
{

    // TODO: manage the orientation of the component

    class SlideBar : UiComponent
    {
    public:
        SlideBar(const UiFrame& frame, UiSize* posToUpdate);

        void changeSliderTexture(const char* texture);

        //TODO: make the cursor more customizable -> texture / size
        void changeCursorTexture(const char* texture);

        void onPosChanged();

        void render(MasterRenderer *renderer);

    private:
        friend void renderer<>(MasterRenderer* renderer, SlideBar* slidebar);

        void mouseInput(Input* inputhandler, double deltaTime...);

        void updateCursorPos();

        TextureComponent* slider;
        TextureComponent* cursor;
        
        float min, max;
        float cursorPos;
        
        UiSize *posUpdate;

        MouseInputComponent* mouseArea;
    };

    class ListView : UiComponent
    {
    public:

    private:
        friend void renderer<>(MasterRenderer* renderer, SlideBar* slidebar);
        SlideBar* slide;
    };
}

