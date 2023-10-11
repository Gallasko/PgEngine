#pragma once

#include "logger.h"

#include "uisystem.h"

#include "Renderer/renderer.h"

#include "constant.h"

namespace pg
{
    enum class Shape2D : uint8_t
    {
        Triangle = 0,
        Square,
        Circle,
        None
    };
    

    struct Shape2DMesh : public Mesh
    {
        Shape2DMesh() : Mesh() { LOG_THIS_MEMBER("Shape 2D Mesh"); modelInfo = constant::SquareInfo{}; }
        ~Shape2DMesh() { LOG_THIS_MEMBER("Shape 2D Mesh"); }

        void generateMesh();

        constant::Vector3D colors{ 255.0f, 255.0f, 255.0f };
    };

    struct Simple2DObject : public Ctor
    {
        Simple2DObject(const Shape2D& shape) : shape(shape) { }
        Simple2DObject(const Shape2D& shape, float width, float height, const constant::Vector3D& c) : shape(shape)
        {
            size.x = width;
            size.y = height;

            colors = c;
        }

        Simple2DObject(const Simple2DObject &rhs) : shape(rhs.shape), size(rhs.size), colors(rhs.colors) { }
        virtual ~Simple2DObject() {}

        virtual void onCreation(EntityRef entity) { this->entity = entity; }

        Shape2D shape;

        // Todo be specific for each shape (rect = [width, height], circle = [origin, radius], triangle = [base, height] + rotation arg)
        constant::Vector2D size{10.0f, 10.0f};

        // Todo make the colors normalized (0.0f <-> 1.0f) to not do the division in the shader
        constant::Vector3D colors{ 255.0f, 255.0f, 255.0f };

        Entity *entity = nullptr;
    };

    struct Simple2DObjectSystem : public AbstractRenderer, System<Own<Simple2DObject>, Ref<UiComponent>, Listener<UiComponentChangeEvent>, NamedSystem, InitSys, StoragePolicy>
    {
        struct SimpleSquareMesh : public Mesh
        {
            SimpleSquareMesh() : Mesh()
            { 
                LOG_THIS_MEMBER("Shape 2D Mesh");
                modelInfo = constant::SquareInfo{};

                modelInfo.vertices[0] =  0.0f; modelInfo.vertices[1] =   0.0f;
				modelInfo.vertices[5] =  1.0f; modelInfo.vertices[6] =   0.0f;
				modelInfo.vertices[10] = 0.0f; modelInfo.vertices[11] = -1.0f;
				modelInfo.vertices[15] = 1.0f; modelInfo.vertices[16] = -1.0f;
                
            }
            virtual ~SimpleSquareMesh();

            void generateMesh();

            OpenGLBuffer *instanceVBO = nullptr;
        };

        Simple2DObjectSystem(MasterRenderer* masterRenderer) : AbstractRenderer(masterRenderer, RenderStage::Render) { }
        virtual ~Simple2DObjectSystem() { if(bufferData) delete[] bufferData; }

        virtual std::string getSystemName() const override { return "Shape 2D System"; }

        virtual void render() override;

        virtual void init() override;

        void addElement(const CompRef<UiComponent>& ui, const CompRef<Simple2DObject>& obj);

        virtual void onEvent(const UiComponentChangeEvent& event) override;

        virtual void updateMeshes() override;

        Mesh* get2DMesh(const Simple2DObject& shape);

        SimpleSquareMesh basicSquareMesh;
        bool squareMeshInitialized = false;

        bool sizeChanged = false;

        size_t elementIndex = 0;
        size_t currentSize = 0;

        float *bufferData = nullptr;
        const size_t nbAttributes = 8; // x, y, z, w(r), h(o), r, g, b 

        std::unordered_map<_unique_id, size_t> idToIndexMap;
        std::mutex tableMutex;
    };

    CompList<UiComponent, Simple2DObject> makeSimple2DShape(EntitySystem *ecs, const Shape2D& shape, float width, float height, const constant::Vector3D& colors);

}