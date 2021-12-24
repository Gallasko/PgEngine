#pragma once

#include <unordered_map>

#include <QImage>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>

#include <QMatrix4x4>
#include <cstdarg>

#include "..\constant.h"

typedef constant::RefracTable RefracRef;
typedef std::unordered_map<std::string, QOpenGLShaderProgram*> ShaderRef;
typedef std::unordered_map<std::string, unsigned int> TextureRef;

struct OpenGLObject : protected QOpenGLFunctions
{
    QOpenGLVertexArrayObject *VAO = nullptr;
	QOpenGLBuffer *VBO = nullptr;
	QOpenGLBuffer *EBO = nullptr;

    OpenGLObject() { initializeOpenGLFunctions(); }
    ~OpenGLObject() { delete VAO; delete VBO; delete EBO; }

    void initialize();
};

class MasterRenderer;

struct Renderer : protected QOpenGLFunctions
{
    Renderer() { initializeOpenGLFunctions(); }
    Renderer(const Renderer&) : QOpenGLFunctions() { initializeOpenGLFunctions(); }
    virtual ~Renderer() {}

    virtual void render(MasterRenderer*...) = 0; 
};

//[TODO] Multiple FBO -> 1 for a whole screen capture and other for batch rendering on a texture 
// Add Particle systeme with instancing already done / create an alternative if needed

class MasterRenderer : protected QOpenGLFunctions
{
public:
    MasterRenderer() {}
    ~MasterRenderer() { delete extraFunctions; delete squareObject; delete instanceVBO; }

    void initialize(QOpenGLContext *m_context) { initializeGlObject(m_context); initializeParameters(); }

    template<typename Renderer, typename... Args>
    void registerRederer(Args... args) { auto rendererName = typeid(Renderer).name(); rendererList[rendererName] = new Renderer(args...); }

    void registerShader(const std::string& name, QOpenGLShaderProgram *shaderProgram) { shaderList[name] = shaderProgram; }
    void registerShader(const std::string& name, const char* vsPath, const char* fsPath);

    void registerTexture(const std::string& name, unsigned int textureId) { textureList[name] = textureId; }
    void registerTexture(const std::string& name, const char* texturePath);

    //TODO raise exception on none presence of attribute
    QOpenGLShaderProgram* getShader(std::string name) { return shaderList[name]; }
    unsigned int getTexture(std::string name) { return textureList[name]; }

    template<typename Renderer, typename... Args>
    void render(Args... args) { auto rendererName = typeid(Renderer).name(); if(rendererList.find(rendererName) != rendererList.end()) rendererList[rendererName]->render(this, args...); }

    template<typename Renderable>
    MasterRenderer& operator<<(Renderable* toRender) { toRender->render(this); return *this; }

    //TODO make the setting of a numerical free of memory leak currently the new here is making a memory leak each time it is called
    inline void setWindowSize(const int& width, const int& height) { systemParameters["ScreenWidth"] = width; systemParameters["ScreenHeight"] = height; }
    inline void setCurrentTime(const unsigned int& time) { systemParameters["CurrentTime"] = static_cast<int>(time); }

    RefracRef getParameter() const { return systemParameters; }

    QOpenGLExtraFunctions* getExtraFunctions() const { return extraFunctions; }
    QOpenGLVertexArrayObject* getSquareVAO() const { return squareObject->VAO; }
    QOpenGLBuffer* getInstanceVBO() const { return instanceVBO; }
private:
    void initializeGlObject(QOpenGLContext *context);

    void initializeParameters();

    QOpenGLExtraFunctions *extraFunctions;
    OpenGLObject *squareObject;
    QOpenGLBuffer *instanceVBO;
    std::unordered_map<std::string, Renderer*> rendererList;
    
    RefracRef systemParameters;
    ShaderRef shaderList;
    TextureRef textureList;
};