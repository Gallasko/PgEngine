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
        typedef std::function<void(const UiSize&)> PositionCallback;
        
    public:
        //TODO use the orientation data
        SlideBar(const UiComponent& frame, const PositionCallback& posToUpdate, const UiOrientation& orientation = UiOrientation::VERTICAL);
        SlideBar(const UiComponent& frame, const UiFrame& boxToMonitor, const UiSize& maxPos, const PositionCallback& posToUpdate, const UiOrientation& orientation = UiOrientation::VERTICAL);
        SlideBar(const SlideBar& slide);

        ~SlideBar();

        void changeSliderTexture(const char* texture);

        //TODO: make the cursor more customizable -> texture / size
        void changeCursorTexture(const char* texture);

        void updateCursorSize(const UiSize& maxPos);
        void onPosChanged();

        void mouseInput(Input* inputhandler, double deltaTime...);
        void mouseLeave(Input* inputhandler, double deltaTime...);

        virtual void render(MasterRenderer* masterRenderer);

    private:
        friend void renderer<>(MasterRenderer* renderer, SlideBar* slidebar);

        void updateCursorPos(const QPoint& pos);

        TextureComponent* slider;
        TextureComponent* cursor;

        // TODO: move this in the cursor class
        UiSize buttonHeight; // = height / 10.0f;

        UiFrame boxToMonitor;
        UiSize maxPos;

        PositionCallback posUpdate = [](const UiSize&){};

        UiOrientation orientation;
        
        MouseComponent mouseArea = makeMouseArea(this, this, SlideBar::mouseInput, SlideBar::mouseLeave);
        bool pressed = false;
    };

    // Make list view subclass from scrollable components
    class ListView : public UiComponent
    {
    public:
        ListView(const UiComponent& frame, TextureComponent* backgroundTexture = nullptr, const UiOrientation& orientation = UiOrientation::VERTICAL);
        ListView(const UiComponent& frame, const SlideBar& slidebar, TextureComponent* backgroundTexture = nullptr, const UiOrientation& orientation = UiOrientation::VERTICAL); 

        void setSpacing(int spacing);

        void add(std::shared_ptr<UiComponent> child);

        void mouseInput(Input* inputhandler, double deltaTime...);
        void mouseLeave(Input* inputhandler, double deltaTime...);

        virtual void render(MasterRenderer* masterRenderer);

    private:
        friend void renderer<>(MasterRenderer* renderer, ListView* listView);

        void updateListPos(const UiSize& pos);

        void calculateListSize();
        void updateRenderList();

        //Todo have 2 slidebar: 1 vertical and 1 horizontal, and hide them if unessesary
        SlideBar slide;

        /** Orientation of the element put in the list */
        UiOrientation orientation;
        TextureComponent* backgroundTexture;
        std::vector<std::shared_ptr<UiComponent>> children;
        std::vector<std::shared_ptr<UiComponent>> renderList;

        MouseComponent mouseArea = makeMouseArea(this, this, ListView::mouseInput, ListView::mouseLeave);

        int spacing = 5;

        UiSize firstMargin;

        UiSize listWidth;
        UiSize listHeight;

        bool pressed = false;
    };
}

