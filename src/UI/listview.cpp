#include "listview.h"

#include <iostream>

namespace pg
{
    namespace
    {
        const char * DOM = "List View";

        float DEFAULT_SLIDER_WIDTH = 20;
    }

    template<>
    void renderer(MasterRenderer* masterRenderer, SlideBar* slideBar)
    {
        masterRenderer->render(slideBar->slider);
        masterRenderer->render(slideBar->cursor);
    }

    template<>
    void renderer(MasterRenderer* masterRenderer, ListView* listView)
    {
        auto rTable = masterRenderer->getParameter();
        const int screenHeight = rTable["ScreenHeight"];

        if(listView->backgroundTexture != nullptr)
            masterRenderer->render(listView->backgroundTexture);

        masterRenderer->render(&listView->slide);

        glEnable(GL_SCISSOR_TEST);
        //glScissor defined the box from the bottom left corner (x, y, w, h);
        glScissor(listView->pos.x, screenHeight - listView->height - listView->pos.y, listView->width, listView->height);

        for(auto& child : listView->children)
            child->render(masterRenderer);

        glDisable(GL_SCISSOR_TEST);
    }

    SlideBar::SlideBar(const UiComponent& frame, UiSize* posToUpdate, const UiOrientation& orientation) : UiComponent(frame), posUpdate(posToUpdate), orientation(orientation)
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

    SlideBar::SlideBar(const UiComponent& frame, const UiFrame& boxToMonitor, const UiSize& maxPos, UiSize* posToUpdate, const UiOrientation& orientation) : SlideBar(frame, posToUpdate, orientation)
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

    void SlideBar::mouseInput(Input* inputHandler, double...)
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
    
    void SlideBar::render(MasterRenderer* masterRenderer)
    { 
        renderer(masterRenderer, this); 
    }

    ListView::ListView(const UiComponent& frame, TextureComponent* backgroundTexture, const UiOrientation& orientation) : UiComponent(frame), slide(SlideBar(UiFrame{this->right, this->top, this->pos.z, DEFAULT_SLIDER_WIDTH, this->height}, this->frame, this->frame.pos.y, nullptr)), orientation(orientation), backgroundTexture(backgroundTexture)
    {
        if(backgroundTexture != nullptr)
        {
            backgroundTexture->setWidth(this->width);
            backgroundTexture->setHeight(this->height);

            backgroundTexture->setTopAnchor(this->top);
            backgroundTexture->setLeftAnchor(this->left);
        }
    }

    ListView::ListView(const UiComponent& frame, const SlideBar& slidebar, TextureComponent* backgroundTexture, const UiOrientation& orientation) : UiComponent(frame), slide(slidebar), orientation(orientation), backgroundTexture(backgroundTexture)
    {
        if(backgroundTexture != nullptr)
        {
            backgroundTexture->setWidth(this->width);
            backgroundTexture->setHeight(this->height);

            backgroundTexture->setTopAnchor(this->top);
            backgroundTexture->setLeftAnchor(this->left);
        }
    }

    //TODO when changing orientation reset all margin !

    void ListView::setSpacing(int spacing)
    {
        this->spacing = spacing;
        
        for(auto& child : this->children)
        {
            switch(orientation)
            {
            case UiOrientation::VERTICAL:
                child->setTopMargin(spacing);
                break;

            case UiOrientation::HORIZONTAL:
                child->setLeftMargin(spacing);
                break;
            }
        }

        calculateListSize();
    }

    void ListView::add(std::shared_ptr<UiComponent> child)
    {
        if(children.size() > 0)
        {
            switch(orientation)
            {
            case UiOrientation::VERTICAL:
                child->setTopAnchor(children.back()->bottom);
                child->setLeftAnchor(children.back()->left);

                child->setTopMargin(spacing);
                break;

            case UiOrientation::HORIZONTAL:
                child->setTopAnchor(children.back()->top);
                child->setLeftAnchor(children.back()->right);

                child->setLeftMargin(spacing);
                break;
            }
        }
        else
        {
            child->setTopAnchor(this->top);
            child->setLeftAnchor(this->left);
        }

        children.push_back(child); 

        calculateListSize();
    }

    void ListView::mouseInput(Input* inputhandler, double deltaTime...)
    {

    }

    void ListView::render(MasterRenderer* masterRenderer)
    { 
        renderer(masterRenderer, this); 
    }

    void ListView::calculateListSize()
    {
        listWidth = 0;
        listHeight = 0;

        for(const auto& child : children)
        {
            switch(orientation)
            {
            case UiOrientation::VERTICAL:
                listHeight += child->height + child->topMargin;

                if(listWidth < child->width)
                    listWidth = child->width;

                break;

            case UiOrientation::HORIZONTAL:
                listWidth += child->width + child->leftMargin;

                if(listHeight < child->height)
                    listHeight = child->height;

                break;
            }
        }

        std::cout << "listWidth: " << listWidth << " listHeight: " << listHeight << std::endl;
    }
}