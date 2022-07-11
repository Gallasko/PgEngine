#pragma once

#include "uiconstant.h"

//Todo parenting, better anchoring
//TODO refactor all the struct that need to be a class and make the element private

//TODO make the UiComponent a class
//TODO create a destructor that go through the children and remove this ? or remove the child recursively ?
namespace pg
{
    // Forward declaration
    namespace constant
    {
        class Vector2D;
    }
    class MasterRenderer;

    /**
     * @struct UiComponent
     *
     * Base struct of all the ui components.
     * This struct all data about the position of the object
     * as well as a render function
     */
    struct UiComponent
    {
        /** The position of the object */
        UiPosition pos;

        /** The width of the object */
        UiSize width;
        /** The height of the object */
        UiSize height;

        /** The bounding box of the object */
        const UiFrame frame = UiFrame(pos.x, pos.y, pos.z, width, height);

        // Pointer to anchors where this object is tied
        const UiSize *topAnchor = nullptr;
        const UiSize *rightAnchor = nullptr;
        const UiSize *bottomAnchor = nullptr;
        const UiSize *leftAnchor = nullptr;

        // The 4 anchors points of the object
        /** Top anchor points of the object */
        const UiSize top = &pos.y;
        /** Right anchor points of the object */
        const UiSize right = pos.x + width;
        /** Bottom anchor points of the object */
        const UiSize bottom = pos.y + height;
        /** Left anchor points of the object */
        const UiSize left = &pos.x;

        // The margin to the given anchor point
        UiSize topMargin;
        UiSize rightMargin;
        UiSize bottomMargin;
        UiSize leftMargin;

        /**
         * @brief Construct a new Ui Component object
         * 
         * This construct an empty ui component at coord (0.0f, 0.0f, 0.0f) of size 0, 0
         */
        UiComponent() { }
        
        /**
         * @brief Construct a new Ui Component object
         * 
         * @param frame The reference frame for this object
         * 
         * This construct an ui component from a given reference
         * The position and size of the newly created object is automatically updated on the fly
         */
        UiComponent(const UiFrame& frame) : pos(&frame.pos), width(&frame.w), height(&frame.h) { }

        /**
         * @brief Construct a new Ui Component object
         * 
         * @param rhs Another UiComponent
         * 
         * This creates a copy of the rhs ui component
         */
        UiComponent(const UiComponent& rhs);

        /**
         * @brief Destroy the Ui Component object
         */
        virtual ~UiComponent() { }

        inline void setX(const int& value) { pos.x = value; update(); }
        inline void setY(const int& value) { pos.y = value; update(); }
        inline void setZ(const int& value) { pos.z = value; update(); }

        inline void setX(const UiSize& value) { pos.x = value; update(); }
        inline void setY(const UiSize& value) { pos.y = value; update(); }
        inline void setZ(const UiSize& value) { pos.z = value; update(); }

        inline void setWidth(const int &value) { width = value; update(); }
        inline void setWidth(const UiSize &value) { width = value; update(); }
        inline void setHeight(const int &value) { height = value; update(); }
        inline void setHeight(const UiSize &value) { height = value; update(); }

        inline void setTopMargin(const int& value) { topMargin = value; update(); }
        inline void setRightMargin(const int& value) { rightMargin = value; update(); }
        inline void setBottomMargin(const int& value) { bottomMargin = value; update(); }
        inline void setLeftMargin(const int& value) { leftMargin = value; update(); }

        inline void setTopMargin(const UiSize& value) { topMargin = value; update(); }
        inline void setRightMargin(const UiSize& value) { rightMargin = value; update(); }
        inline void setBottomMargin(const UiSize& value) { bottomMargin = value; update(); }
        inline void setLeftMargin(const UiSize& value) { leftMargin = value; update(); }

        inline void setTopAnchor(const UiSize *anchor) { topAnchor = anchor; update(); }
        inline void setRightAnchor(const UiSize *anchor) { rightAnchor = anchor; update(); }
        inline void setBottomAnchor(const UiSize *anchor) { bottomAnchor = anchor; update(); }
        inline void setLeftAnchor(const UiSize *anchor) { leftAnchor = anchor; update(); }

        inline void setTopAnchor(const UiSize& anchor) { topAnchor = &anchor; update(); }
        inline void setRightAnchor(const UiSize& anchor) { rightAnchor = &anchor; update(); }
        inline void setBottomAnchor(const UiSize& anchor) { bottomAnchor = &anchor; update(); }
        inline void setLeftAnchor(const UiSize& anchor) { leftAnchor = &anchor; update(); }

        bool inBound(int x, int y) const;
        bool inBound(const constant::Vector2D& vec2) const;

        const bool& isVisible() const { return visible; }

        virtual void show() { this->visible = true; }
        virtual void hide() { this->visible = false; }

        virtual void render(MasterRenderer* masterRenderer);
        
        void update();

    protected:
        bool visible = true;
    };

    //TODO Copy Constructor
    template <typename LoaderId> 
    struct LoaderRenderComponent : public UiComponent
    {
        LoaderRenderComponent(const LoaderId *id) : id(id) {}
        LoaderRenderComponent(const LoaderRenderComponent& rhs);

        //virtual void render(MasterRenderer* masterRenderer) { renderer(masterRenderer, this); }

        const LoaderId *id;
    };

    template <typename LoaderId> 
    LoaderRenderComponent<LoaderId>::LoaderRenderComponent(const LoaderRenderComponent &rhs) : UiComponent(rhs), id(rhs.id)
    {
        update();
    }

}