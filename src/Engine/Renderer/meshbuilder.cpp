#include "meshbuilder.h"

namespace pg
{
    void OpenGLObject::initialize()
    {
        LOG_THIS_MEMBER("OpenGLObject");

        initializeOpenGLFunctions();
        
        VAO = new QOpenGLVertexArrayObject();
        VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer); 

        VAO->create();
        VBO->create();
        EBO->create();
    }

    void MeshBuilder::TextureMesh::generateMesh()
    {
        LOG_THIS_MEMBER("Texture Mesh");

        OpenGLMesh.initialize();

        OpenGLMesh.VAO->bind();

        // position attribute
        OpenGLMesh.VBO->bind();
        OpenGLMesh.VBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
        OpenGLMesh.VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

        OpenGLMesh.glEnableVertexAttribArray(0);
        OpenGLMesh.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

        // texture coord attribute
        OpenGLMesh.glEnableVertexAttribArray(1);
        OpenGLMesh.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        OpenGLMesh.EBO->bind();
        OpenGLMesh.EBO->setUsagePattern(QOpenGLBuffer::StreamDraw);
        OpenGLMesh.EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

        OpenGLMesh.VAO->release();

        initialized = true;
    }

    MeshBuilder::MeshRef MeshBuilder::getTextureMesh(float width, float height, const std::string& name)
    {
        LOG_THIS_MEMBER("MeshBuilder");

        auto meshName = "_texture_" + name + "_" + std::to_string(width) + "_" + std::to_string(height);

        LOG_MILE("MeshBuilder", "Creating a new texture mesh: " << meshName);

        const auto& it = m_meshes.find(meshName);

        if(it == m_meshes.end())
        {
            auto mesh = new TextureMesh();

            mesh->modelInfo.vertices[0] =  0.0f;  mesh->modelInfo.vertices[1] =    0.0f;   mesh->modelInfo.vertices[2] =  0.0f;
            mesh->modelInfo.vertices[5] =  width; mesh->modelInfo.vertices[6] =    0.0f;   mesh->modelInfo.vertices[7] =  0.0f;
            mesh->modelInfo.vertices[10] = 0.0f;  mesh->modelInfo.vertices[11] =  -height; mesh->modelInfo.vertices[12] = 0.0f;
            mesh->modelInfo.vertices[15] = width; mesh->modelInfo.vertices[16] =  -height; mesh->modelInfo.vertices[17] = 0.0f;

            m_meshes.emplace(meshName, mesh);
        }
        else
        {
            // Todo increment the number of time this mesh is used
            // m_meshes[meshName].count++;
        }

        return MeshRef{this, meshName};
    }
}