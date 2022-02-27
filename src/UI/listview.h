#pragma once

#include "uisystem.h"

#include "../Input/inputcomponent.h"

namespace pg
{   
    class SlideBar : public UiComponent
    {
    public:
        enum class Orientation
        {
            VERTICAL,
            HORIZONTAL
        };

    public:
        SlideBar(const UiFrame& frame, UiSize* posToUpdate, Orientation orientation = Orientation::VERTICAL);
        SlideBar(const UiFrame& frame, const UiFrame& boxToMonitor, const UiSize& maxPos, UiSize* posToUpdate, Orientation orientation = Orientation::VERTICAL);

        void changeSliderTexture(const char* texture);

        //TODO: make the cursor more customizable -> texture / size
        void changeCursorTexture(const char* texture);

        void onPosChanged();

        void mouseInput(Input* inputhandler, double deltaTime...);

        virtual void render(MasterRenderer* masterRenderer) { renderer(masterRenderer, this); }

    private:
        friend void renderer<>(MasterRenderer* renderer, SlideBar* slidebar);

        void updateCursorSize(const UiSize& maxPos);
        void updateCursorPos();

        TextureComponent* slider;
        TextureComponent* cursor;

        UiSize buttonHeight = height / 10.0f;
        
        //UiSize yMin, yMax;
        //float cursorPos;

        UiFrame boxToMonitor;
        
        UiSize *posUpdate;

        Orientation orientation;

        //Todo when creating a mouse area it should automatically be registered in the game state loop
        //MouseInputComponent* mouseArea;
    };

    class ListView : UiComponent
    {
    public:

        virtual void render(MasterRenderer* masterRenderer) { renderer(masterRenderer, this); }

    private:
        friend void renderer<>(MasterRenderer* renderer, SlideBar* slidebar);
        SlideBar* slide;
    };
}

