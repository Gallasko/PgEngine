#include "listview.h"

#include "texture.h"
#include "logger.h"

namespace pg
{
    namespace
    {
        static constexpr char const * DOM = "List View";

        float DEFAULT_SLIDER_WIDTH = 20;
    }

    template <>
    void renderer(MasterRenderer* masterRenderer, SlideBar* slideBar)
    {
        masterRenderer->render(slideBar->slider);
        masterRenderer->render(slideBar->cursor);
    }

    template <>
    void renderer(MasterRenderer* masterRenderer, ListView* listView)
    {
        auto rTable = masterRenderer->getParameter();
        const int screenHeight = rTable["ScreenHeight"];

        if(listView->backgroundTexture != nullptr)
            masterRenderer->render(listView->backgroundTexture);

        masterRenderer->render(&listView->slide);

        //Avoid multiple lockup
        float listViewHeight = listView->height;

        glEnable(GL_SCISSOR_TEST);
        //glScissor defined the box from the bottom left corner (x, y, w, h);
        glScissor(static_cast<UiSize>(listView->pos.x), (screenHeight - listViewHeight) - listView->pos.y, listView->width, listViewHeight);

        //TODO see if the square calculation couldn t be optimized
        for(auto& child : listView->renderList)
            child->render(masterRenderer);
            
        glDisable(GL_SCISSOR_TEST);
    }

    SlideBar::SlideBar(const UiComponent& frame, const PositionCallback& posToUpdate, const UiOrientation& orientation) : UiComponent(frame), posUpdate(posToUpdate), orientation(orientation)
    {
        // Default slider

        // TODO create a unique instance of a texture creator and call it to create all textures
        // TODO make the texture creator load the texture only once and call the same texture when needed (avoid duplication of a texture in memory)
        // TODO make them dependent on the texture create and keep only shared ptr of it
        slider = new TextureComponent(this->width, this->height, "slider");
        slider->setTopAnchor(this->top);
        slider->setLeftAnchor(this->left);

        cursor = new TextureComponent(this->width, this->buttonHeight, "cursor");
        cursor->setTopAnchor(this->top);
        cursor->setLeftAnchor(this->left);
    }

    SlideBar::SlideBar(const UiComponent& frame, const UiFrame& boxToMonitor, const UiSize& maxPos, const PositionCallback& posToUpdate, const UiOrientation& orientation) : SlideBar(frame, posToUpdate, orientation)
    {
        this->boxToMonitor = &boxToMonitor;
        updateCursorSize(maxPos);
    }

    SlideBar::SlideBar(const SlideBar& rhs) : UiComponent(rhs), slider(rhs.slider), cursor(rhs.cursor), buttonHeight(rhs.buttonHeight), boxToMonitor(rhs.boxToMonitor), posUpdate(rhs.posUpdate), orientation(rhs.orientation)
    {
    }

    SlideBar::~SlideBar()
    {
        delete slider;
        delete cursor;
    }

    //TODO this is needed we cant rely on the auto computation of the size cause they are some restriction
    void SlideBar::updateCursorSize(const UiSize& maxPos)
    {
        // TODO do it for a horizontal slider

        // Do this if the orientation is vertical

        this->maxPos = maxPos;

        if(this->maxPos > 0 && this->boxToMonitor.h > 0)
        {
            this->buttonHeight = (this->boxToMonitor.h / this->maxPos) * this->height;
            
            if(this->buttonHeight > this->height)
                this->buttonHeight = this->height;
        }
        else
        {
            this->buttonHeight = this->height;
        }
            
        cursor->setHeight(this->buttonHeight);
    }

    void SlideBar::mouseInput(Input* inputHandler, double...)
    {
        if(not inputHandler->isButtonGrabbed(Qt::LeftButton))
            pressed = false;

        if(inputHandler->isButtonPressed(Qt::LeftButton) or pressed) 
        {
            inputHandler->grabMouse(Qt::LeftButton);

            const auto& pos = inputHandler->getMousePos();

            updateCursorPos(pos);

            pressed = true;
        }
    }

    void SlideBar::mouseLeave(Input* inputHandler, double)
    {
        if(inputHandler->isButtonGrabbed(Qt::LeftButton) and pressed) 
        {
            const auto& pos = inputHandler->getMousePos();

            updateCursorPos(pos);
        }
        else
            pressed = false;
    }
    
    void SlideBar::render(MasterRenderer* masterRenderer)
    { 
        renderer(masterRenderer, this); 
    }

    void SlideBar::updateCursorPos(const QPoint& pos)
    {
        auto currentPos = pos.y() - this->pos.y - this->buttonHeight / 2.0f;

        if(currentPos < 0)
            currentPos = 0;

        if(currentPos > height - this->buttonHeight)
            currentPos = height - this->buttonHeight;

        if(posUpdate && this->boxToMonitor.h > 0)
            posUpdate(currentPos * (this->maxPos / this->boxToMonitor.h));

        cursor->setTopMargin(currentPos);
    }

    //TODO create a 2nd slider like said in the header
    // TODO add anchor to UiPosition 
    ListView::ListView(const UiComponent& frame, TextureComponent* backgroundTexture, const UiOrientation& orientation) :
        UiComponent(frame),
        slide(SlideBar(UiFrame{this->right.anchorPoint, this->top.anchorPoint, this->pos.z, DEFAULT_SLIDER_WIDTH, this->height}, this->frame, this->pos.y, [&](const UiSize& pos){ this->updateListPos(pos); })), orientation(orientation), backgroundTexture(backgroundTexture)
    {
        LOG_THIS_MEMBER(DOM);

        if(backgroundTexture != nullptr)
        {
            backgroundTexture->setWidth(this->width);
            backgroundTexture->setHeight(this->height);

            backgroundTexture->setTopAnchor(this->top);
            backgroundTexture->setLeftAnchor(this->left);
        }
    }

    //TODO don't pass a slider but the slider parameters and then contruct the slider to be relevent to this list view
    ListView::ListView(const UiComponent& frame, const SlideBar& slidebar, TextureComponent* backgroundTexture, const UiOrientation& orientation) : UiComponent(frame), slide(slidebar), orientation(orientation), backgroundTexture(backgroundTexture)
    {
        LOG_THIS_MEMBER(DOM);

        // This is for the vertical slider
        //Todo create the slider here (horizontalSilder = SlideBar(SlideParam, this->frame, this->listHeight, his->firstMargin))
        slide.setHeight(this->height);
        slide.setTopAnchor(this->top);
        slide.setLeftAnchor(this->right);

        //TODO make it for the horizontal one

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
        LOG_THIS_MEMBER(DOM);

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
        LOG_THIS_MEMBER(DOM);

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
            child->setTopAnchor(this->top); // Todo - this->firstMargin);
            child->setLeftAnchor(this->left);
        }

        //This make it so the mouse area of the elements can t be used directly because they will be beneath the Screen Ui ! (Which is the bottom line sitting at -1)
        child->setZ(-2);

        children.push_back(child);

        calculateListSize();
        updateRenderList();
    }

    void ListView::mouseInput(Input* inputhandler, double...)
    {
        if(inputhandler->isButtonPressed(Qt::LeftButton) or inputhandler->isButtonGrabbed(Qt::LeftButton))
        {
            LOG_THIS_MEMBER(DOM);

            pressed = true;

            const auto& mousePos = inputhandler->getMousePos();

            for(auto& child : children)
            {
                child->pos.z = -2;

                if (child->inBound(mousePos.x(), mousePos.y()))
                    child->pos.z = this->pos.z;
            }
        }
        else
        {
            if(pressed)
            {
                for(auto& child : children)
                {
                    child->pos.z = -2;
                }
            }
                
            pressed = false;
        }
    }

    void ListView::mouseLeave(Input*, double)
    {
        if(pressed)
        {
            LOG_THIS_MEMBER(DOM);

            for(auto& child : children)
            {
                child->pos.z = -2;
            }

            pressed = false;
        }
    }

    void ListView::clear()
    {
        children.clear();
        children.shrink_to_fit();

        calculateListSize();
        updateRenderList();
    }

    void ListView::render(MasterRenderer* masterRenderer)
    { 
        renderer(masterRenderer, this); 
    }

    void ListView::updateListPos(const UiSize& pos)
    {
        LOG_THIS_MEMBER(DOM);

        this->firstMargin = pos;
        this->updateRenderList();
    }

    void ListView::calculateListSize()
    {
        LOG_THIS_MEMBER(DOM);

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

        //this is for vertical slider
        slide.updateCursorSize(listHeight);
    }

    void ListView::updateRenderList()
    {
        LOG_THIS_MEMBER(DOM);

        renderList.clear();
		renderList.shrink_to_fit();

        for(auto& child : children)
        {
            //Cast once to avoid multiple value lockup
            float childTop = child->top.anchorPoint;
            float childBottom = child->bottom.anchorPoint;
            float childLeft = child->left.anchorPoint;
            float childRight = child->right.anchorPoint;

            child->hide();

            if(this->inBound(childLeft, childTop) or this->inBound(childLeft, childBottom) or this->inBound(childRight, childTop) or this->inBound(childRight, childBottom))
            {
                renderList.push_back(child);
                child->show();
            }
        }
    }
}