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
        void changeCursorTexture(const char* texture);

        void onPosChanged();

        void render(MasterRenderer *renderer);

    private:
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
        SlideBar* slide;
    };
}

