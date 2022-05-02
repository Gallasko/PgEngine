#include "meshbuilder.h"

namespace pg
{
    void OpenGLObject::initialize()
    {
        initializeOpenGLFunctions();
        
        VAO = new QOpenGLVertexArrayObject();
        VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer); 

        VAO->create();
        VBO->create();
        EBO->create();
    }
}