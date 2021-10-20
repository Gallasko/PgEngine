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
struct UiComponent : public Base
{
    bool visible = true;

    UiPosition pos;

    UiSize width = UiSize(0, 0, nullptr);
    UiSize height = UiSize(0, 0, nullptr);
    float scale = 1.0f;

    const UiSize *topAnchor = nullptr;
    const UiSize *rightAnchor = nullptr;
    const UiSize *bottomAnchor = nullptr;
    const UiSize *leftAnchor = nullptr;

    UiSize top = &pos.y;
    UiSize right = pos.x + width;
    UiSize bottom = pos.y + height;
    UiSize left = &pos.x;

    int topMargin = 0;
    int rightMargin = 0;
    int bottomMargin = 0;
    int leftMargin = 0;

    std::vector<UiComponent*> children; // todo remove this

    UiComponent() { }
    UiComponent(const UiComponent& rhs);

    void inline setX(const int& value) { pos.x = value; update(); }
    void inline setY(const int& value) { pos.y = value; update(); }
    void inline setZ(const int& value) { pos.z = value; update(); }

    void inline setWidth(const int &value) { width = value; update(); }
    void inline setWidth(const UiSize &value) { width = value; update(); }
    void inline setHeight(const int &value) { height = value; update(); }
    void inline setHeight(const UiSize &value) { height = value; update(); }

    void inline setTopMargin(const int& value) { topMargin = value; update(); }
    void inline setRightMargin(const int& value) { rightMargin = value; update(); }
    void inline setBottomMargin(const int& value) { bottomMargin = value; update(); }
    void inline setLeftMargin(const int& value) { leftMargin = value; update(); }

    void inline setTopAnchor(UiSize *anchor) { topAnchor = anchor; update(); }
    void inline setRightAnchor(UiSize *anchor) { rightAnchor = anchor; update(); }
    void inline setBottomAnchor(UiSize *anchor) { bottomAnchor = anchor; update(); }
    void inline setLeftAnchor(UiSize *anchor) { leftAnchor = anchor; update(); }

    void inline setTopAnchor(const UiSize& anchor) { topAnchor = &anchor; update(); }
    void inline setRightAnchor(const UiSize& anchor) { rightAnchor = &anchor; update(); }
    void inline setBottomAnchor(const UiSize& anchor) { bottomAnchor = &anchor; update(); }
    void inline setLeftAnchor(const UiSize& anchor) { leftAnchor = &anchor; update(); }
    
    bool updated = true; // todo remove this 

    bool inBound(int x, int y) { return x > this->pos.x / this->scale && x < (this->pos.x + this->width) / this->scale && y < (this->pos.y + this->height) / this->scale && y > this->pos.y / this->scale; }
    bool inBound(constant::Vector2D vec2) { return inBound(vec2.x, vec2.y); }
    
    void update();
};

struct TextureComponent : public UiComponent, private QOpenGLFunctions
{
    TextureComponent(UiSize width, UiSize height, const char* path);
    TextureComponent(const TextureComponent &rhs);
    ~TextureComponent();

    void generateMesh();

    unsigned int texture;

    constant::SquareInfo modelInfo;

    QOpenGLVertexArrayObject *VAO = nullptr;
	QOpenGLBuffer *VBO = nullptr;
	QOpenGLBuffer *EBO = nullptr;

    float oldWidth = width, oldHeight = height;

    bool initialised = false;
};

struct TextureRenderer : public Renderer
{
    using Renderer::Renderer;
    virtual ~TextureRenderer() {}

    void render(MasterRenderer* masterRenderer...);
};

//TODO Copy Constructor
template <typename LoaderId> 
struct LoaderRenderComponent : public UiComponent
{
    LoaderRenderComponent(LoaderId *id) : id(id) {}
    LoaderRenderComponent(const LoaderRenderComponent& rhs);

    LoaderId *id;
};

template <typename LoaderId> 
LoaderRenderComponent<LoaderId>::LoaderRenderComponent(const LoaderRenderComponent &rhs) : UiComponent(rhs)
{
    this->id = rhs.id;

    update();
}

struct LoaderRenderer : public Renderer
{
    using Renderer::Renderer;
    virtual ~LoaderRenderer() {}

    void render(MasterRenderer* masterRenderer...);
};
