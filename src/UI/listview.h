#pragma once

#include "uisystem.h"

#include "../Input/inputcomponent.h"

namespace pg
{

    // TODO: manage the orientation of the component

    class SlideBar : public UiComponent
    {
    public:
        SlideBar(const UiFrame& frame, UiSize* posToUpdate);

        void changeSliderTexture(const char* texture);

        //TODO: make the cursor more customizable -> texture / size
        void changeCursorTexture(const char* texture);

        void onPosChanged();

        void mouseInput(Input* inputhandler, double deltaTime...);

    private:
        friend void renderer<>(MasterRenderer* renderer, SlideBar* slidebar);

        void updateCursorPos();

        TextureComponent* slider;
        TextureComponent* cursor;
        
        UiSize yMin, yMax;
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

