#pragma once

#include <unordered_map>

#include <QImage>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>

#include "..\constant.h"

typedef constant::RefracTable RefracRef;
typedef std::unordered_map<std::string, QOpenGLShaderProgram*> ShaderRef;
typedef std::unordered_map<std::string, unsigned int> TextureRef;

struct OpenGLObject : protected QOpenGLFunctions
{
    QOpenGLVertexArrayObject *VAO = nullptr;
	QOpenGLBuffer *VBO = nullptr;
	QOpenGLBuffer *EBO = nullptr;

    OpenGLObject() {}
    ~OpenGLObject() { delete VAO; delete VBO; delete EBO; }

    void initialize() {
        VAO = new QOpenGLVertexArrayObject();
	    VBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	    EBO = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer); 

        VAO->create();
        VBO->create();
        EBO->create();
        }
};

class MasterRenderer;

struct Renderer : protected QOpenGLFunctions
{
    Renderer() { initializeOpenGLFunctions(); }
    Renderer(const Renderer& renderer) : QOpenGLFunctions() { initializeOpenGLFunctions(); }
    virtual ~Renderer() {}

    virtual void render(MasterRenderer*...) = 0; 
};

//[TODO] Multiple FBO -> 1 for a whole screen capture and other for batch rendering on a texture 
// Add Particle systeme with instancing already done / create an alternative if needed

//TODO remove all the iostream for std::cout on release
#include <iostream>
class MasterRenderer : protected QOpenGLFunctions
{
public:
    MasterRenderer() {}
    ~MasterRenderer() { delete extraFunctions; delete squareObject; delete instanceVBO; }

    void initialize(QOpenGLContext *m_context) { initializeGlObject(m_context); initializeParameters(); }

    template<typename Renderer, typename... Args>
    void registerRederer(Args... args) { auto rendererName = typeid(Renderer).name(); rendererList[rendererName] = new Renderer(args...); }

    void registerShader(std::string name, QOpenGLShaderProgram *shaderProgram) { shaderList[name] = shaderProgram; }
    void registerShader(std::string name, const char* vsPath, const char* fsPath) {
        auto shaderProgram = new QOpenGLShaderProgram();
        shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, vsPath);
        shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, fsPath);
        shaderProgram->link();

        std::cout << name << ": "<< glGetError() << std::endl; 

        registerShader(name, shaderProgram);
    }

    void registerTexture(std::string name, unsigned int textureId) { textureList[name] = textureId; }
    void registerTexture(std::string name, const char* texturePath) { 
        QImage textureAtlas = QImage(QString(texturePath));
        textureAtlas = textureAtlas.convertToFormat(QImage::Format_RGBA8888).mirrored(); // TODO check mirrored

        unsigned int texture;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // load image, create texture and generate mipmaps
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureAtlas.width(), textureAtlas.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureAtlas.bits());
        glGenerateMipmap(GL_TEXTURE_2D);

        registerTexture(name, texture);
    }

    //TODO raise exception on none presence of attribute
    QOpenGLShaderProgram* getShader(std::string name) { return shaderList[name]; }
    unsigned int getTexture(std::string name) { return textureList[name]; }

    template<typename Renderer, typename... Args>
    void render(Args... args) { auto rendererName = typeid(Renderer).name(); if(rendererList.find(rendererName) != rendererList.end()) rendererList[rendererName]->render(this, args...); }

    template<typename Renderable>
    MasterRenderer& operator<<(Renderable* toRender) { toRender->render(this); return *this; }

    //TODO make the setting of a numerical free of memory leak currently the new here is making a memory leak each time it is called
    inline void setWindowSize(const int& width, const int& height) { systemParameters["ScreenWidth"] = new constant::NumericalInt(width); systemParameters["ScreenHeight"] = new constant::NumericalInt(height); }
    inline void setCurrentTime(const unsigned int& time) { systemParameters["CurrentTime"] = new constant::NumericalInt(time); }

    RefracRef getParameter() const { return systemParameters; }

    QOpenGLExtraFunctions* getExtraFunctions() const { return extraFunctions; }
    QOpenGLVertexArrayObject* getSquareVAO() const { return squareObject->VAO; }
    QOpenGLBuffer* getInstanceVBO() const { return instanceVBO; }
private:
    void initializeGlObject(QOpenGLContext *m_context) {
        initializeOpenGLFunctions(); 
        extraFunctions = new QOpenGLExtraFunctions(m_context); 

        instanceVBO = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
        instanceVBO->create();

        squareObject = new OpenGLObject();
        squareObject->initialize();

        auto tileVertices = new float[20];

        //                 x                         y                         z                      texpos x                 texpos y
        tileVertices[0]  = 0.0f; tileVertices[1]  =  0.0f; tileVertices[2]  = 0.0f; tileVertices[3]  = 0.0f; tileVertices[4]  = 1.0f;   
        tileVertices[5]  = 1.0f; tileVertices[6]  =  0.0f; tileVertices[7]  = 0.0f; tileVertices[8]  = 1.0f; tileVertices[9]  = 1.0f;
        tileVertices[10] = 0.0f; tileVertices[11] = -1.0f; tileVertices[12] = 0.0f; tileVertices[13] = 0.0f; tileVertices[14] = 0.0f;
        tileVertices[15] = 1.0f; tileVertices[16] = -1.0f; tileVertices[17] = 0.0f; tileVertices[18] = 1.0f; tileVertices[19] = 0.0f;

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

    void initializeParameters() {
        systemParameters["ScreenWidth"] = new constant::NumericalInt(1);
        systemParameters["ScreenHeight"] = new constant::NumericalInt(1);
        systemParameters["CurrentTime"] = new constant::NumericalInt(1);
    }

    QOpenGLExtraFunctions *extraFunctions;
    OpenGLObject *squareObject;
    QOpenGLBuffer *instanceVBO;
    std::unordered_map<std::string, Renderer*> rendererList;
    
    RefracRef systemParameters;
    ShaderRef shaderList;
    TextureRef textureList;
};