#pragma once

#include <vector>

#include "uisystem.h"

#include "../Input/inputcomponent.h"

namespace pg
{
    //TODO make sliders part of the scrollable component cause not only the list view need them
    //Scrollable widgets includes : listview, long text, long images, maps, etc... 
    class SlideBar : public UiComponent
    {
    public:
        enum class Orientation
        {
            VERTICAL,
            HORIZONTAL
        };

    public:
        //TODO use the orientation data
        SlideBar(const UiFrame& frame, UiSize* posToUpdate, Orientation orientation = Orientation::VERTICAL);
        SlideBar(const UiFrame& frame, const UiFrame& boxToMonitor, const UiSize& maxPos, UiSize* posToUpdate, Orientation orientation = Orientation::VERTICAL);
        SlideBar(const SlideBar& slide);

        ~SlideBar();

        void changeSliderTexture(const char* texture);

        //TODO: make the cursor more customizable -> texture / size
        void changeCursorTexture(const char* texture);

        void onPosChanged();

        void mouseInput(Input* inputhandler, double deltaTime...);

        virtual void render(MasterRenderer* masterRenderer);

    private:
        friend void renderer<>(MasterRenderer* renderer, SlideBar* slidebar);

        void updateCursorSize(const UiSize& maxPos);
        void updateCursorPos();

        TextureComponent* slider;
        TextureComponent* cursor;

        // TODO: move this in the cursor class
        UiSize buttonHeight = height / 10.0f;
        
        //UiSize yMin, yMax;
        //float cursorPos;

        UiFrame boxToMonitor;
        
        UiSize *posUpdate;

        Orientation orientation;
        
        MouseInputPtr mouseArea = makeMouseArea(this, this, SlideBar::mouseInput);
    };

    // Make list view subclass from scrollable components
    class ListView : UiComponent
    {
    public:
        ListView(const UiFrame& frame, TextureComponent* backgroundTexture = nullptr);
        ListView(const UiFrame& frame, const SlideBar& slidebar, TextureComponent* backgroundTexture = nullptr); 

        void add(std::shared_ptr<UiComponent> child) { children.push_back(child); }

        virtual void render(MasterRenderer* masterRenderer);

    private:
        friend void renderer<>(MasterRenderer* renderer, ListView* listView);

        TextureComponent* background = nullptr;

        SlideBar slide;

        std::vector<std::shared_ptr<UiComponent>> children;
    };
}

