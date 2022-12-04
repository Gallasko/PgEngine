#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include <memory>

#include "ECS/system.h"
#include "ECS/entitysystem.h"

namespace pg
{
    struct OpenGLObject : protected QOpenGLFunctions, public ecs::Component
    {
        QOpenGLVertexArrayObject *VAO = nullptr;
        QOpenGLBuffer *VBO = nullptr;
        QOpenGLBuffer *EBO = nullptr;

        OpenGLObject() : ecs::Component("OpenGLObject") {}
        ~OpenGLObject() { delete VAO; delete VBO; delete EBO; }

        void initialize();
    };

    class MeshBuilder : public ecs::System<ecs::Own<OpenGLObject>>
    {
    public:

    private:
    };
}