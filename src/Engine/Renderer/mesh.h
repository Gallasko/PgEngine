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
    struct SentenceText;
    class FontLoader;

    struct Mesh
    {
        Mesh() { LOG_THIS_MEMBER("Mesh"); }
        Mesh(const Mesh &other) = delete;
        Mesh& operator=(const Mesh &other) = delete;
        virtual ~Mesh() {}

        void bind();

        virtual void generateMesh() = 0;

        OpenGLObject openGLMesh;
        constant::ModelInfo modelInfo;
        bool initialized = false;
    };
}