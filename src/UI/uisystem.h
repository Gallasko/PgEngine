#pragma once

#include "uiconstant.h"

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include <algorithm>
#include <vector>

#include "../Engine/basesystem.h"
#include "../constant.h"

#include "../Engine/renderer.h"

//Todo parenting, better anchoring
//TODO refactor all the struct that need to be a class and make the element private

//TODO make the UiComponent a class
//TODO create a destructor that go through the children and remove this ? or remove the child recursively ?
namespace pg
{
    struct UiComponent
    {
        bool visible = true;

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

        int topMargin = 0;
        int rightMargin = 0;
        int bottomMargin = 0;
        int leftMargin = 0;

        UiComponent() { }
        UiComponent(const UiFrame& frame) : pos(&frame.pos), width(&frame.w), height(&frame.h) { }
        UiComponent(const UiComponent& rhs);

        virtual ~UiComponent() { }

        inline void setX(const int& value) { pos.x = value; update(); }
        inline void setY(const int& value) { pos.y = value; update(); }
        inline void setZ(const int& value) { pos.z = value; update(); }

        inline void setWidth(const int &value) { width = value; update(); }
        inline void setWidth(const UiSize &value) { width = value; update(); }
        inline void setHeight(const int &value) { height = value; update(); }
        inline void setHeight(const UiSize &value) { height = value; update(); }

        inline void setTopMargin(const int& value) { topMargin = value; update(); }
        inline void setRightMargin(const int& value) { rightMargin = value; update(); }
        inline void setBottomMargin(const int& value) { bottomMargin = value; update(); }
        inline void setLeftMargin(const int& value) { leftMargin = value; update(); }

        inline void setTopAnchor(UiSize *anchor) { topAnchor = anchor; update(); }
        inline void setRightAnchor(UiSize *anchor) { rightAnchor = anchor; update(); }
        inline void setBottomAnchor(UiSize *anchor) { bottomAnchor = anchor; update(); }
        inline void setLeftAnchor(UiSize *anchor) { leftAnchor = anchor; update(); }

        inline void setTopAnchor(const UiSize& anchor) { topAnchor = &anchor; update(); }
        inline void setRightAnchor(const UiSize& anchor) { rightAnchor = &anchor; update(); }
        inline void setBottomAnchor(const UiSize& anchor) { bottomAnchor = &anchor; update(); }
        inline void setLeftAnchor(const UiSize& anchor) { leftAnchor = &anchor; update(); }
        
        bool updated = true; // todo remove this 

        bool inBound(int x, int y) const;
        bool inBound(const constant::Vector2D& vec2) const { return inBound(vec2.x, vec2.y); }

        virtual void render(MasterRenderer* masterRenderer);
        
        void update();
    };

    struct TextureComponent : public UiComponent, private QOpenGLFunctions
    {
        TextureComponent(const UiSize& width, const UiSize& height, const char* path);
        TextureComponent(const TextureComponent &rhs);
        ~TextureComponent();

        void generateMesh();

        virtual void render(MasterRenderer* masterRenderer);

        unsigned int texture;

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
        LoaderRenderComponent(LoaderId *id) : id(id) {}
        LoaderRenderComponent(const LoaderRenderComponent& rhs);

        //virtual void render(MasterRenderer* masterRenderer) { renderer(masterRenderer, this); }

        LoaderId *id;
    };

    template <typename LoaderId> 
    LoaderRenderComponent<LoaderId>::LoaderRenderComponent(const LoaderRenderComponent &rhs) : UiComponent(rhs)
    {
        this->id = rhs.id;

        update();
    }

}