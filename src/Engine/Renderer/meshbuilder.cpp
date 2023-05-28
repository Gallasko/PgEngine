#include "meshbuilder.h"

#include "UI/sentencesystem.h"

#include "Helpers/openglobject.h"

namespace pg
{
    void OpenGLObject::initialize()
    {
        LOG_THIS_MEMBER("OpenGLObject");
        
        VAO = new OpenGLVertexArrayObject();
        VBO = new OpenGLBuffer(OpenGLBuffer::VertexBuffer);
        EBO = new OpenGLBuffer(OpenGLBuffer::IndexBuffer); 

        VAO->create();
        VBO->create();
        EBO->create();
    }

    void MeshBuilder::Mesh::bind()
    { 
        LOG_THIS_MEMBER("Mesh");

        if(not initialized)
            generateMesh();
        
        OpenGLMesh.VAO->bind();
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

    void MeshBuilder::SentenceMesh::generateMesh()
    {
        LOG_THIS_MEMBER("Sentence Mesh");

        OpenGLMesh.initialize();

        OpenGLMesh.VAO->bind();

        // position attribute
        OpenGLMesh.VBO->bind();
        OpenGLMesh.VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
        OpenGLMesh.VBO->allocate(modelInfo.vertices, modelInfo.nbVertices * sizeof(float));

        OpenGLMesh.glEnableVertexAttribArray(0);
        OpenGLMesh.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 18 * sizeof(float), (void*)0);

        // texture coord attribute
        OpenGLMesh.glEnableVertexAttribArray(1);
        OpenGLMesh.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 18 * sizeof(float), (void*)(3 * sizeof(float)));

        // texture coord attribute
        OpenGLMesh.glEnableVertexAttribArray(2);
        OpenGLMesh.glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 18 * sizeof(float), (void*)(5 * sizeof(float)));

        // texture coord attribute
        OpenGLMesh.glEnableVertexAttribArray(3);
        OpenGLMesh.glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 18 * sizeof(float), (void*)(9 * sizeof(float)));

        // texture coord attribute
        OpenGLMesh.glEnableVertexAttribArray(4);
        OpenGLMesh.glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 18 * sizeof(float), (void*)(13 * sizeof(float)));

        // texture coord attribute
        OpenGLMesh.glEnableVertexAttribArray(5);
        OpenGLMesh.glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 18 * sizeof(float), (void*)(17 * sizeof(float)));

        OpenGLMesh.EBO->bind();
        OpenGLMesh.EBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
        OpenGLMesh.EBO->allocate(modelInfo.indices, modelInfo.nbIndices * sizeof(unsigned int));

        OpenGLMesh.VAO->release();

        initialized = true;
    }

    // Todo add a mutex to protect m_meshes of any race conditions

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

    MeshBuilder::MeshRef MeshBuilder::getSentenceMesh(SentenceText& sentence, FontLoader *font)
    {
        LOG_THIS_MEMBER("MeshBuilder");

        // Todo add font name in mesh name
        auto meshName = "_sentence_" + sentence.text + "_" + std::to_string(sentence.scale);

        LOG_MILE("MeshBuilder", "Lookup for the texture: " << meshName);

        const auto& it = m_meshes.find(meshName);

        if(it != m_meshes.end())
        {
            return MeshRef{this, meshName};
        }

        LOG_MILE("MeshBuilder", "Creating a new texture mesh: " << meshName);

        auto mesh = new SentenceMesh();

        auto nbChara = sentence.text.length();
        
        mesh->modelInfo.nbVertices = 72 * nbChara;
        mesh->modelInfo.nbIndices = 6 * nbChara;

        if(mesh->modelInfo.vertices != nullptr)
            delete[] mesh->modelInfo.vertices;
        if(mesh->modelInfo.indices != nullptr)
            delete[] mesh->modelInfo.indices;

        mesh->modelInfo.vertices = new float [mesh->modelInfo.nbVertices];
        mesh->modelInfo.indices = new unsigned int [mesh->modelInfo.nbIndices];

        float currentX = 0.0f;
        float outO = 0.0f; //Outline Offset

        constant::ModelInfo letterModel; 

        for(size_t i = 0; i < nbChara; i++)
        {
            const auto letter = font->getChara(std::string(1, sentence.text.at(i)));

            outO = 0.0f;

            // Todo see how to add back the scale
            // auto w = letter->getWidth() * scale;
            // auto h = letter->getHeight() * scale;
            // auto o = letter->getOffset() * scale;

            auto w = letter->getWidth() * sentence.scale;
            auto h = letter->getHeight() * sentence.scale;
            auto o = letter->getOffset() * sentence.scale;

            if(sentence.outline1.w == 0.0f)
                outO += 1.0 * sentence.scale;

            if(sentence.outline2.w == 0.0f)
                outO += 1.0 * sentence.scale;

            letterModel = letter->getModelInfo();

            // Todo maybe also get the z to add it in the vertices
            // Coord
            mesh->modelInfo.vertices[i * 72 + 0]  = currentX - outO    ; mesh->modelInfo.vertices[i * 72 + 1]  =     -o + outO; mesh->modelInfo.vertices[i * 72 + 2]  = 0.0f;
            mesh->modelInfo.vertices[i * 72 + 18] = currentX + w + outO; mesh->modelInfo.vertices[i * 72 + 19] =     -o + outO; mesh->modelInfo.vertices[i * 72 + 20] = 0.0f;
            mesh->modelInfo.vertices[i * 72 + 36] = currentX - outO    ; mesh->modelInfo.vertices[i * 72 + 37] = -h - o - outO; mesh->modelInfo.vertices[i * 72 + 38] = 0.0f;
            mesh->modelInfo.vertices[i * 72 + 54] = currentX + w + outO; mesh->modelInfo.vertices[i * 72 + 55] = -h - o - outO; mesh->modelInfo.vertices[i * 72 + 56] = 0.0f;

            // Tex Coord
            mesh->modelInfo.vertices[i * 72 + 3]  = letterModel.vertices[3]  - outO / font->getAtlasWidth(); mesh->modelInfo.vertices[i * 72 + 4]  = letterModel.vertices[4]  - outO / font->getAtlasHeight();  
            mesh->modelInfo.vertices[i * 72 + 21] = letterModel.vertices[8]  + outO / font->getAtlasWidth(); mesh->modelInfo.vertices[i * 72 + 22] = letterModel.vertices[9]  - outO / font->getAtlasHeight();
            mesh->modelInfo.vertices[i * 72 + 39] = letterModel.vertices[13] - outO / font->getAtlasWidth(); mesh->modelInfo.vertices[i * 72 + 40] = letterModel.vertices[14] + outO / font->getAtlasHeight();
            mesh->modelInfo.vertices[i * 72 + 57] = letterModel.vertices[18] + outO / font->getAtlasWidth(); mesh->modelInfo.vertices[i * 72 + 58] = letterModel.vertices[19] + outO / font->getAtlasHeight();

            //Main Color
            mesh->modelInfo.vertices[i * 72 + 5]  = sentence.mainColor.x; mesh->modelInfo.vertices[i * 72 + 6]  = sentence.mainColor.y; mesh->modelInfo.vertices[i * 72 + 7]  = sentence.mainColor.z; mesh->modelInfo.vertices[i * 72 + 8]  = sentence.mainColor.w;
            mesh->modelInfo.vertices[i * 72 + 23] = sentence.mainColor.x; mesh->modelInfo.vertices[i * 72 + 24] = sentence.mainColor.y; mesh->modelInfo.vertices[i * 72 + 25] = sentence.mainColor.z; mesh->modelInfo.vertices[i * 72 + 26] = sentence.mainColor.w;
            mesh->modelInfo.vertices[i * 72 + 41] = sentence.mainColor.x; mesh->modelInfo.vertices[i * 72 + 42] = sentence.mainColor.y; mesh->modelInfo.vertices[i * 72 + 43] = sentence.mainColor.z; mesh->modelInfo.vertices[i * 72 + 44] = sentence.mainColor.w;
            mesh->modelInfo.vertices[i * 72 + 59] = sentence.mainColor.x; mesh->modelInfo.vertices[i * 72 + 60] = sentence.mainColor.y; mesh->modelInfo.vertices[i * 72 + 61] = sentence.mainColor.z; mesh->modelInfo.vertices[i * 72 + 62] = sentence.mainColor.w;

            //Outline 1
            mesh->modelInfo.vertices[i * 72 + 9]  = sentence.outline1.x; mesh->modelInfo.vertices[i * 72 + 10] = sentence.outline1.y; mesh->modelInfo.vertices[i * 72 + 11] = sentence.outline1.z; mesh->modelInfo.vertices[i * 72 + 12] = sentence.outline1.w;
            mesh->modelInfo.vertices[i * 72 + 27] = sentence.outline1.x; mesh->modelInfo.vertices[i * 72 + 28] = sentence.outline1.y; mesh->modelInfo.vertices[i * 72 + 29] = sentence.outline1.z; mesh->modelInfo.vertices[i * 72 + 30] = sentence.outline1.w;
            mesh->modelInfo.vertices[i * 72 + 45] = sentence.outline1.x; mesh->modelInfo.vertices[i * 72 + 46] = sentence.outline1.y; mesh->modelInfo.vertices[i * 72 + 47] = sentence.outline1.z; mesh->modelInfo.vertices[i * 72 + 48] = sentence.outline1.w;
            mesh->modelInfo.vertices[i * 72 + 63] = sentence.outline1.x; mesh->modelInfo.vertices[i * 72 + 64] = sentence.outline1.y; mesh->modelInfo.vertices[i * 72 + 65] = sentence.outline1.z; mesh->modelInfo.vertices[i * 72 + 66] = sentence.outline1.w;

            //Outline 2
            mesh->modelInfo.vertices[i * 72 + 13] = sentence.outline2.x; mesh->modelInfo.vertices[i * 72 + 14] = sentence.outline2.y; mesh->modelInfo.vertices[i * 72 + 15] = sentence.outline2.z; mesh->modelInfo.vertices[i * 72 + 16] = sentence.outline2.w;
            mesh->modelInfo.vertices[i * 72 + 31] = sentence.outline2.x; mesh->modelInfo.vertices[i * 72 + 32] = sentence.outline2.y; mesh->modelInfo.vertices[i * 72 + 33] = sentence.outline2.z; mesh->modelInfo.vertices[i * 72 + 34] = sentence.outline2.w;
            mesh->modelInfo.vertices[i * 72 + 49] = sentence.outline2.x; mesh->modelInfo.vertices[i * 72 + 50] = sentence.outline2.y; mesh->modelInfo.vertices[i * 72 + 51] = sentence.outline2.z; mesh->modelInfo.vertices[i * 72 + 52] = sentence.outline2.w;
            mesh->modelInfo.vertices[i * 72 + 67] = sentence.outline2.x; mesh->modelInfo.vertices[i * 72 + 68] = sentence.outline2.y; mesh->modelInfo.vertices[i * 72 + 69] = sentence.outline2.z; mesh->modelInfo.vertices[i * 72 + 70] = sentence.outline2.w;

            //Effect
            mesh->modelInfo.vertices[i * 72 + 17] = (float)sentence.effect;
            mesh->modelInfo.vertices[i * 72 + 35] = (float)sentence.effect;
            mesh->modelInfo.vertices[i * 72 + 53] = (float)sentence.effect;
            mesh->modelInfo.vertices[i * 72 + 71] = (float)sentence.effect;

            mesh->modelInfo.indices[i * 6 + 0] = 4 * i + 0; mesh->modelInfo.indices[i * 6 + 1] = 4 * i + 1; mesh->modelInfo.indices[i * 6 + 2] = 4 * i + 2;
            mesh->modelInfo.indices[i * 6 + 3] = 4 * i + 1; mesh->modelInfo.indices[i * 6 + 4] = 4 * i + 2; mesh->modelInfo.indices[i * 6 + 5] = 4 * i + 3;

            currentX += w + 1;

            if(h + o > sentence.textHeight)
                sentence.textHeight = h + o;

        }

        sentence.textWidth = currentX;

        m_meshes.emplace(meshName, mesh);

        return MeshRef{this, meshName};
    }
}