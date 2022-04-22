#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include <memory>

namespace pg
{
    struct OpenGLObject : protected QOpenGLFunctions
    {
        QOpenGLVertexArrayObject *VAO = nullptr;
        QOpenGLBuffer *VBO = nullptr;
        QOpenGLBuffer *EBO = nullptr;

        OpenGLObject() {}
        ~OpenGLObject() { delete VAO; delete VBO; delete EBO; }

        void initialize();
    };

    class MeshBuilder
    {
    public:

    private:
    };
}