#include "listview.h"

#include <iostream>

namespace pg
{
    template<>
    void renderer(MasterRenderer* masterRenderer, SlideBar* slideBar)
    {
        masterRenderer->render(slideBar->slider);
        masterRenderer->render(slideBar->cursor);
    }

    SlideBar::SlideBar(const UiFrame& frame, UiSize* posToUpdate, Orientation orientation) : UiComponent(frame), posUpdate(posToUpdate), orientation(orientation)
    {
        // Default slider

        // TODO create a unique instance of a texture creator and call it to create all textures
        // TODO make the texture creator load the texture only once and call the same texture when needed (avoid duplication of a texture in memory)
        // TODO make them dependent on the texture create and keep only shared ptr of it
        slider = new TextureComponent(this->width, this->height, "res/object/slider.png");
        slider->setTopAnchor(this->top);
        slider->setLeftAnchor(this->left);

        cursor = new TextureComponent(this->width, this->buttonHeight, "res/object/cursor.png");
        cursor->setTopAnchor(this->top);
        cursor->setLeftAnchor(this->left);
    }

    SlideBar::SlideBar(const UiFrame& frame, const UiFrame& boxToMonitor, const UiSize& maxPos, UiSize* posToUpdate, Orientation orientation) : SlideBar(frame, posToUpdate, orientation)
    {
        this->boxToMonitor = boxToMonitor;

        updateCursorSize(maxPos);
    }

    SlideBar::SlideBar(const SlideBar& rhs) : UiComponent(rhs), slider(rhs.slider), cursor(rhs.cursor), buttonHeight(rhs.buttonHeight), boxToMonitor(rhs.boxToMonitor), posUpdate(rhs.posUpdate), orientation(rhs.orientation)
    {
    }

    SlideBar::~SlideBar()
    {
        deleteInput(mouseArea);
        delete slider;
        delete cursor;
    }

    void SlideBar::mouseInput(Input* inputHandler, double deltaTime...)
    {
        static bool pressed = false;

        if(not inputHandler->isButtonGrabbed(Qt::LeftButton))
            pressed = false;

        if(inputHandler->isButtonPressed(Qt::LeftButton) or pressed) 
        {
            inputHandler->grabMouse(Qt::LeftButton);

            const auto pos = inputHandler->getMousePos();

            auto currentPos = pos.y() - this->pos.y - this->buttonHeight / 2.0f;

            if(currentPos < 0)
                currentPos = 0;

            if(currentPos > height - this->buttonHeight)
                currentPos = height - this->buttonHeight;

            if(posUpdate)
                *posUpdate = currentPos;

            cursor->setTopMargin(currentPos);

            pressed = true;
        }
    }

    void SlideBar::updateCursorSize(const UiSize& maxPos)
    {
        if(maxPos > 0 && this->boxToMonitor.h > 0)
            this->buttonHeight = (this->boxToMonitor.h / maxPos) * height;
        else
            this->buttonHeight = height;

        cursor->setHeight(this->buttonHeight);
    }

    template<>
    void renderer(MasterRenderer* masterRenderer, ListView* listView)
    {
        if(listView->background != nullptr)
            masterRenderer->render(listView->background);

        masterRenderer->render(&listView->slide);

        //TODO gl scissor for list views 
        glEnable(GL_SCISSOR_TEST);
        glScissor(listView->pos.x, listView->pos.y, listView->width, listView->height);

        //for(auto child : listView->children)
        //    masterRenderer->render(child);

        glDisable(GL_SCISSOR_TEST);
    }

    // TODO set a base size for the slide + fix the magic number in this line -->                                                                     this is a magic number  V 
    ListView::ListView(const UiFrame& frame, TextureComponent* backgroundTexture) : UiComponent(frame), slide(SlideBar(UiFrame{this->right, this->top, this->pos.z, 20, this->height}, this->frame, this->frame.pos.y, nullptr)), background(backgroundTexture)
    {

    }

    ListView::ListView(const UiFrame& frame, const SlideBar& slidebar, TextureComponent* backgroundTexture) : UiComponent(frame), slide(slidebar), background(backgroundTexture)
    {

    }
    
}