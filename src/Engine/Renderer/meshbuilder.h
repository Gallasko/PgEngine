#pragma once

#include <memory>
#include <unordered_map>

#include "constant.h"
#include "logger.h"

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

    class MeshBuilder
    {
    public:
        // Todo may need to count the number of same Meshes created to free up the memory when unused
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

        struct TextureMesh : public Mesh
        {
            TextureMesh() : Mesh() { LOG_THIS_MEMBER("Texture Mesh"); modelInfo = constant::SquareInfo{}; }
            ~TextureMesh() { LOG_THIS_MEMBER("Texture Mesh"); }

            void generateMesh();
        };

        struct SentenceMesh : public Mesh
        {
            SentenceMesh() : Mesh() { LOG_THIS_MEMBER("Sentence Mesh"); modelInfo = constant::SquareInfo{}; }
            ~SentenceMesh() { LOG_THIS_MEMBER("Sentence Mesh"); }

            void generateMesh();
        };

        struct MeshRef
        {
            MeshRef(MeshBuilder *builder, const std::string &name) : builderRef(builder), textureName(name) { LOG_THIS_MEMBER("MeshBuilder"); }
            MeshRef(const MeshRef &other) : builderRef(other.builderRef), textureName(other.textureName) { LOG_THIS_MEMBER("MeshBuilder"); }
            ~MeshRef() { LOG_THIS_MEMBER("MeshBuilder"); }

            MeshRef& operator=(const MeshRef& rhs) { LOG_THIS_MEMBER("MeshBuilder"); builderRef = rhs.builderRef; textureName = rhs.textureName; return *this; }

            inline Mesh* getMesh() const noexcept
            {
                LOG_THIS_MEMBER("MeshBuilder");

                try
                {
                    return builderRef->m_meshes.at(textureName);
                }
                catch(const std::exception& e)
                {
                    LOG_ERROR("MeshBuilder", "Mesh was not found at " << textureName << ", error: " << e.what());
                    return nullptr;
                }
            }

            MeshBuilder *builderRef;
            std::string textureName;
        };
    public:
        ~MeshBuilder() { clear(); }

        // Todo make a general "createMesh" function so the user can provide his own MeshConstruct which inherit Mesh like TextureMesh

        MeshRef getTextureMesh(float width, float height, const std::string& name);
        MeshRef getSentenceMesh(SentenceText& sentence, FontLoader *font);

        void clear() { LOG_THIS_MEMBER("MeshBuilder"); for(auto mesh : m_meshes) delete mesh.second; m_meshes.clear(); }

    private:
        std::unordered_map<std::string, Mesh*> m_meshes;
    };
}