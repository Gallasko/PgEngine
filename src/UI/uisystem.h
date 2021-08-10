#pragma once

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

struct UiSize
{
    int pixelSize = 0;
    float scaleValue = 0.0f;
    UiSize *refSize;

    UiSize() {}
    UiSize(int pixelSize = 0, float scaleValue = 0, UiSize* ref = nullptr) : pixelSize(pixelSize), scaleValue(scaleValue), refSize(ref) {}
    UiSize(const UiSize& size) : pixelSize(size.pixelSize), scaleValue(size.scaleValue), refSize(size.refSize) {}

    static float returnCurrentSize(const UiSize* size)
    {
        if(size == nullptr)
            return 0;
        else
            return size->pixelSize + returnCurrentSize(size->refSize) * size->scaleValue;
    }

    void operator=(const UiSize& rhs)
    {
        this->pixelSize = rhs.pixelSize;
        this->scaleValue = rhs.scaleValue;
        this->refSize = rhs.refSize;
    }

    void operator=(const int& rhs)
    {
        this->pixelSize = rhs;
        this->scaleValue = 0.0;
        this->refSize = nullptr;
    }

    UiSize operator*(const float& rhs)
    {
        return UiSize(0, rhs, this);
    }

    UiSize operator+(const int& rhs)
    {
        return UiSize(rhs, this->scaleValue, this);
    }

    UiSize operator-(const int& rhs)
    {
        return UiSize(-rhs, this->scaleValue, this);
    }

    float operator-()
    {
        return -UiSize::returnCurrentSize(this);
    }

    template<typename Type>
    friend Type operator+(const Type& lhs, const UiSize& rhs);

    template<typename Type>
    friend Type operator-(const Type& lhs, const UiSize& rhs);

    operator float()
    {
        return UiSize::returnCurrentSize(this);
    }
};

enum class UiAnchor
{
    TOP,
    RIGHT,
    BOTTOM,
    LEFT
};

struct UiComponent : public Base
{
    bool visible = true;

    int x = 0;
    int y = 0;
    int z = 0; // stack indice

    UiSize width = UiSize(0, 0, nullptr);
    UiSize height = UiSize(0, 0, nullptr);
    float scale = 1.0f;

    UiComponent *topAnchor = nullptr;
    UiComponent *rightAnchor = nullptr;
    UiComponent *bottomAnchor = nullptr;
    UiComponent *leftAnchor = nullptr;

    UiAnchor top = UiAnchor::TOP;
    UiAnchor right = UiAnchor::RIGHT;
    UiAnchor bottom = UiAnchor::BOTTOM;
    UiAnchor left = UiAnchor::LEFT;

    int topMargin = 0;
    int rightMargin = 0;
    int bottomMargin = 0;
    int leftMargin = 0;

    std::vector<UiComponent*> children;

    void inline setX(const int& value) { x = value; update(); }
    void inline setY(const int& value) { y = value; update(); }
    void inline setZ(const int& value) { z = value; update(); }

    void inline setWidth(const int &value) { width = value; update(); }
    void inline setWidth(const UiSize &value) { width = value; update(); }
    void inline setHeight(const int &value) { height = value; update(); }
    void inline setHeight(const UiSize &value) { height = value; update(); }

    void inline setTopMargin(int value) { topMargin = value; update(); }
    void inline setRightMargin(int value) { rightMargin = value; update(); }
    void inline setBottomMargin(int value) { bottomMargin = value; update(); }
    void inline setLeftMargin(int value) { leftMargin = value; update(); }

    void inline setTopAnchor(UiComponent* component, UiAnchor side = UiAnchor::TOP) { top = side; topAnchor = component; auto it = std::find(component->children.begin(), component->children.end(), this); if(it == component->children.end()) component->children.push_back(this); update(); }
    void inline setRightAnchor(UiComponent* component, UiAnchor side = UiAnchor::RIGHT) { right = side; rightAnchor = component; auto it = std::find(component->children.begin(), component->children.end(), this); if(it == component->children.end()) component->children.push_back(this); update(); }
    void inline setBottomAnchor(UiComponent* component, UiAnchor side = UiAnchor::BOTTOM) { bottom = side; bottomAnchor = component; auto it = std::find(component->children.begin(), component->children.end(), this); if(it == component->children.end()) component->children.push_back(this); update(); }
    void inline setLeftAnchor(UiComponent* component, UiAnchor side = UiAnchor::LEFT) { left = side; leftAnchor = component; auto it = std::find(component->children.begin(), component->children.end(), this); if(it == component->children.end()) component->children.push_back(this); update(); }
    
    bool updated = false;
    
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

    bool initialised = false;
};

struct TextureRenderer : public Renderer
{
    using Renderer::Renderer;
    virtual ~TextureRenderer() {}

    void render(std::string rendererName...);
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
LoaderRenderComponent<LoaderId>::LoaderRenderComponent(const LoaderRenderComponent &rhs)
{
    this->visible = rhs.visible;
    this->x = rhs.x;
    this->y = rhs.y;
    this->z = rhs.z;
    this->width = rhs.width;
    this->height = rhs.height;
    this->scale = rhs.scale;
    this->topAnchor = rhs.topAnchor;
    this->rightAnchor = rhs.rightAnchor;
    this->bottomAnchor = rhs.bottomAnchor;
    this->leftAnchor = rhs.leftAnchor;
    this->topMargin = rhs.topMargin;
    this->rightMargin = rhs.rightMargin;
    this->bottomMargin = rhs.bottomMargin;
    this->leftMargin = rhs.leftMargin;
    this->children = rhs.children;
    this->scale = scale;

    this->id = rhs.id;

    update();
}

struct LoaderRenderer : public Renderer
{
    using Renderer::Renderer;
    virtual ~LoaderRenderer() {}

    void render(std::string rendererName...);
};
