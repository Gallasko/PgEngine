#include "renderer.h"

#include <QImage>

namespace pg
{
    void MasterRenderer::registerShader(const std::string& name, const char* vsPath, const char* fsPath)
    {
        auto shaderProgram = new QOpenGLShaderProgram();
        shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, vsPath);
        shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, fsPath);
        shaderProgram->link();

        registerShader(name, shaderProgram);
    }

    //TODO mirror or not the texture
    void MasterRenderer::registerTexture(const std::string& name, const char* texturePath)
    { 
        QImage textureAtlas = QImage(QString(texturePath));
        textureAtlas = textureAtlas.convertToFormat(QImage::Format_RGBA8888); //.mirrored(); // TODO check mirrored

        unsigned int texture;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // set the texture wrapping parameters
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // load image, create texture and generate mipmaps
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureAtlas.width(), textureAtlas.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureAtlas.bits());
        glGenerateMipmap(GL_TEXTURE_2D);

        registerTexture(name, texture);
    }

    void MasterRenderer::initializeGlObject(QOpenGLContext *context)
    {
        initializeOpenGLFunctions(); 
        extraFunctions = new QOpenGLExtraFunctions(context); 

        instanceVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        instanceVBO->create();

        squareObject = new OpenGLObject();
        squareObject->initialize();

        auto tileVertices = new float[20];

        //                 x                         y                         z                      texpos x                 texpos y
        tileVertices[0]  = 0.0f; tileVertices[1]  =  0.0f; tileVertices[2]  = 0.0f; tileVertices[3]  = 0.0f; tileVertices[4]  = 0.0f;   
        tileVertices[5]  = 1.0f; tileVertices[6]  =  0.0f; tileVertices[7]  = 0.0f; tileVertices[8]  = 1.0f; tileVertices[9]  = 0.0f;
        tileVertices[10] = 1.0f; tileVertices[11] = -1.0f; tileVertices[12] = 0.0f; tileVertices[13] = 0.0f; tileVertices[14] = 1.0f;
        tileVertices[15] = 0.0f; tileVertices[16] = -1.0f; tileVertices[17] = 0.0f; tileVertices[18] = 1.0f; tileVertices[19] = 1.0f;

        unsigned int nbTileVertices = 20;

        auto tileVerticesIndice = new unsigned int[6];

        tileVerticesIndice[0] = 0; tileVerticesIndice[1] = 1; tileVerticesIndice[2] = 2;
        tileVerticesIndice[3] = 1; tileVerticesIndice[4] = 2; tileVerticesIndice[5] = 3;

        unsigned int nbOfElements = 6;

        squareObject->VAO->bind();

        // position attribute
        
        squareObject->VBO->bind();
        squareObject->VBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
        squareObject->VBO->allocate(tileVertices, nbTileVertices * sizeof(float));

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

        // texture coord attribute
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        squareObject->EBO->bind();
        squareObject->EBO->setUsagePattern(QOpenGLBuffer::StaticDraw);
        squareObject->EBO->allocate(tileVerticesIndice, nbOfElements * sizeof(unsigned int));

        squareObject->VAO->release();
    }

    void MasterRenderer::initializeParameters()
    {
        systemParameters["ScreenWidth"] = 1;
        systemParameters["ScreenHeight"] = 1;
        systemParameters["CurrentTime"] = 1;
    }
}