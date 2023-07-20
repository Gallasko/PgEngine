#pragma once

#include <memory>
#include <unordered_map>

#include "constant.h"
#include "logger.h"

#include "2D/simple2dobject.h"

namespace pg
{
    // Forward declarations
    class OpenGLVertexArrayObject;
    class OpenGLBuffer;

    struct OpenGLObject
    {
        OpenGLVertexArrayObject *VAO = nullptr;
        OpenGLBuffer *VBO = nullptr;
        OpenGLBuffer *EBO = nullptr;

        bool initialized = false;

        OpenGLObject();
        ~OpenGLObject();

        void initialize();
    };

    // Todo remove when Mesh specializations are implemented in their own headers
    class SentenceText;
    class FontLoader;

    struct Mesh
    {
        Mesh() { LOG_THIS_MEMBER("Mesh"); }
        Mesh(const Mesh &other) = delete;
        Mesh& operator=(const Mesh &other) = delete;
        virtual ~Mesh() {}

        void bind();

        virtual void generateMesh() = 0;

        OpenGLObject OpenGLMesh;
        constant::ModelInfo modelInfo;
        bool initialized = false;
    };

    class MeshBuilder
    {
    public:
        // Todo may need to count the number of same Meshes created to free up the memory when unused

        struct SimpleMesh : public Mesh
        {
            SimpleMesh() : Mesh() { LOG_THIS_MEMBER("Simple Mesh"); modelInfo = constant::SquareInfo{}; }
            ~SimpleMesh() { LOG_THIS_MEMBER("Simple Mesh"); }

            void generateMesh();
        };

        
    public:
        // ~MeshBuilder() { clear(); }

        // Todo make a general "createMesh" function so the user can provide his own MeshConstruct which inherit Mesh like TextureMesh

        
        Mesh* getSimpleMesh(float width, float height, const Simple2DObject::Type& type);

        // void clear() { LOG_THIS_MEMBER("MeshBuilder"); for(auto mesh : m_meshes) delete mesh.second; m_meshes.clear(); }

    private:
        
    };
}