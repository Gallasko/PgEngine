#include "mesh.h"

#include "UI/sentencesystem.h"

#include "Helpers/openglobject.h"

namespace pg
{
    OpenGLObject::OpenGLObject()
    {
        LOG_THIS_MEMBER("OpenGLObject");
    }

    OpenGLObject::~OpenGLObject()
    {
        LOG_THIS_MEMBER("OpenGLObject");
        
        if(initialized)
        {
            delete VAO; 
            delete VBO; 
            delete EBO;
        }
    }

    void OpenGLObject::initialize()
    {
        LOG_THIS_MEMBER("OpenGLObject");
        
        VAO = new OpenGLVertexArrayObject();
        VBO = new OpenGLBuffer(OpenGLBuffer::VertexBuffer);
        EBO = new OpenGLBuffer(OpenGLBuffer::IndexBuffer); 

        VAO->create();
        VBO->create();
        EBO->create();

        initialized = true;
    }

    void Mesh::bind()
    { 
        LOG_THIS_MEMBER("Mesh");

        if(not initialized)
            generateMesh();
        
        OpenGLMesh.VAO->bind();
    }

    // Todo add a mutex to protect m_meshes of any race conditions
}