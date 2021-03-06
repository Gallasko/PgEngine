#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include <algorithm>
#include <vector>

#include "uiconstant.h"
#include "../constant.h"

#include "../Renderer/renderer.h"

//Todo parenting, better anchoring
//TODO refactor all the struct that need to be a class and make the element private

//TODO make the UiComponent a class
//TODO create a destructor that go through the children and remove this ? or remove the child recursively ?
namespace pg
{
    struct UiComponent
    {
        UiPosition pos;

        UiSize width;
        UiSize height;

        const UiFrame frame = UiFrame(pos.x, pos.y, pos.z, width, height);

        const UiSize *topAnchor = nullptr;
        const UiSize *rightAnchor = nullptr;
        const UiSize *bottomAnchor = nullptr;
        const UiSize *leftAnchor = nullptr;

        const UiSize top = &pos.y;
        const UiSize right = pos.x + width;
        const UiSize bottom = pos.y + height;
        const UiSize left = &pos.x;

        UiSize topMargin;
        UiSize rightMargin;
        UiSize bottomMargin;
        UiSize leftMargin;

        UiComponent() { }
        UiComponent(const UiFrame& frame) : pos(&frame.pos), width(&frame.w), height(&frame.h) { }
        UiComponent(const UiComponent& rhs);

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
        bool inBound(const constant::Vector2D& vec2) const { return inBound(vec2.x, vec2.y); }

        const bool& isVisible() const { return visible; }

        virtual void show() { this->visible = true; }
        virtual void hide() { this->visible = false; }

        virtual void render(MasterRenderer* masterRenderer);
        
        void update();

    protected:
        bool visible = true;
    };

    struct TextureComponent : public UiComponent, private QOpenGLFunctions
    {
        TextureComponent(const UiSize& width, const UiSize& height, const std::string& textureName);
        TextureComponent(const UiComponent& component, const std::string& textureName);
        TextureComponent(const TextureComponent &rhs);
        ~TextureComponent();

        void generateMesh();

        virtual void render(MasterRenderer* masterRenderer);

        std::string textureName;

        constant::SquareInfo modelInfo;

        QOpenGLVertexArrayObject *VAO = nullptr;
        QOpenGLBuffer *VBO = nullptr;
        QOpenGLBuffer *EBO = nullptr;

        float oldWidth = width, oldHeight = height;

        bool initialised = false;
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