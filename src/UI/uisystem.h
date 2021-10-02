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
    class UiAnchor
    {
    friend class UiComponent;
    public:
        UiAnchor(UiComponent* component, UiSize* refPos, UiSize *refSize = nullptr) : component(component), refPos(refPos), refSize(refSize) {}

        operator float()
        {
            if(refSize != nullptr)
            {
                return static_cast<float>(*refPos + *refSize);
            }
                
            return static_cast<float>(*refPos);
        }

    protected:
        UiComponent *component;

    private:
        UiSize* refPos = nullptr;
        UiSize *refSize = nullptr;
    };

    bool visible = true;

    //TODO create a int wrapper that call update on change (overload operator=) and have a pointer to the component like the anchor class so i can write UiComponent.x = 10 for exemple and it update children
    //TODO change x y z to be UISize so i can position it using other component
    UiPosition pos;

    UiSize width = UiSize(0, 0, nullptr);
    UiSize height = UiSize(0, 0, nullptr);
    float scale = 1.0f;

    UiAnchor *topAnchor = nullptr;
    UiAnchor *rightAnchor = nullptr;
    UiAnchor *bottomAnchor = nullptr;
    UiAnchor *leftAnchor = nullptr;

    UiAnchor top = UiAnchor(this, &pos.y);
    UiAnchor right = UiAnchor(this, &pos.x, &width);
    UiAnchor bottom = UiAnchor(this, &pos.y, &height);
    UiAnchor left = UiAnchor(this, &pos.x);

    int topMargin = 0;
    int rightMargin = 0;
    int bottomMargin = 0;
    int leftMargin = 0;

    std::vector<UiComponent*> children;

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

    void inline setTopAnchor(UiAnchor *anchor) { topAnchor = anchor; auto it = std::find(anchor->component->children.begin(), anchor->component->children.end(), this); if(it == anchor->component->children.end()) anchor->component->children.push_back(this); update(); }
    void inline setRightAnchor(UiAnchor *anchor) { rightAnchor = anchor; auto it = std::find(anchor->component->children.begin(), anchor->component->children.end(), this); if(it == anchor->component->children.end()) anchor->component->children.push_back(this); update(); }
    void inline setBottomAnchor(UiAnchor *anchor) { bottomAnchor = anchor; auto it = std::find(anchor->component->children.begin(), anchor->component->children.end(), this); if(it == anchor->component->children.end()) anchor->component->children.push_back(this); update(); }
    void inline setLeftAnchor(UiAnchor *anchor) { leftAnchor = anchor; auto it = std::find(anchor->component->children.begin(), anchor->component->children.end(), this); if(it == anchor->component->children.end()) anchor->component->children.push_back(this); update(); }
    
    bool updated = false;

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
    //this->visible = rhs.visible;
    //this->x = rhs.x;
    //this->y = rhs.y;
    //this->z = rhs.z;
    //this->width = rhs.width;
    //this->height = rhs.height;
    //this->scale = rhs.scale;
    //this->topAnchor = rhs.topAnchor;
    //this->rightAnchor = rhs.rightAnchor;
    //this->bottomAnchor = rhs.bottomAnchor;
    //this->leftAnchor = rhs.leftAnchor;
    //this->topMargin = rhs.topMargin;
    //this->rightMargin = rhs.rightMargin;
    //this->bottomMargin = rhs.bottomMargin;
    //this->leftMargin = rhs.leftMargin;
    //this->children = rhs.children;
    //this->scale = scale;

    this->id = rhs.id;

    update();
}

struct LoaderRenderer : public Renderer
{
    using Renderer::Renderer;
    virtual ~LoaderRenderer() {}

    void render(MasterRenderer* masterRenderer...);
};
