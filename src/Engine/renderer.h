#pragma once

#include <unordered_map>

#include <QImage>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include "..\constant.h"

typedef constant::RefracTable RefracRef;
typedef std::unordered_map<std::string, QOpenGLShaderProgram*> ShaderRef;
typedef std::unordered_map<std::string, unsigned int> TextureRef;

struct Renderer : protected QOpenGLFunctions
{
    Renderer() { initializeOpenGLFunctions(); }
    Renderer(const Renderer& renderer) : QOpenGLFunctions() { initializeOpenGLFunctions(); }
    virtual ~Renderer() {}

    virtual void render(RefracRef, ShaderRef, TextureRef...) = 0; 
};

//[TODO] Multiple FBO -> 1 for a whole screen capture and other for batch rendering on a texture 
// Add Particle systeme with instancing already done / create an alternative if needed

class MasterRenderer : protected QOpenGLFunctions
{
public:
    MasterRenderer() {}

    void initialize() { initializeOpenGLFunctions(); systemParameters["ScreenWidth"] = new constant::NumericalInt(1); systemParameters["ScreenHeight"] = new constant::NumericalInt(1); systemParameters["CurrentTime"] = new constant::NumericalInt(1); }

    template<typename Renderer, typename... Args>
    void registerRederer(Args... args) { auto rendererName = typeid(Renderer).name(); rendererList[rendererName] = new Renderer(args...); }

    void registerShader(std::string name, QOpenGLShaderProgram *shaderProgram) { shaderList[name] = shaderProgram; }
    void registerShader(std::string name, const char* vsPath, const char* fsPath) {
        auto shaderProgram = new QOpenGLShaderProgram();
        shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, vsPath);
        shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, fsPath);
        shaderProgram->link();

        registerShader(name, shaderProgram);
    }

    void registerTexture(std::string name, unsigned int textureId) { textureList[name] = textureId; }
    void registerTexture(std::string name, const char* texturePath) { 
        QImage textureAtlas = QImage(QString(texturePath));
        textureAtlas = textureAtlas.convertToFormat(QImage::Format_RGBA8888);

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
    void render(Args... args) { auto rendererName = typeid(Renderer).name(); if(rendererList.find(rendererName) != rendererList.end()) rendererList[rendererName]->render(systemParameters, shaderList, textureList, args...); }

    template<typename Renderable>
    MasterRenderer& operator<<(Renderable* toRender) { toRender->render(systemParameters, shaderList, textureList); return *this; }

    inline void setWindowSize(const int& width, const int& height) { systemParameters["ScreenWidth"] = new constant::NumericalInt(width); systemParameters["ScreenHeight"] = new constant::NumericalInt(height); }
    inline void setCurrentTime(const unsigned int& time) { systemParameters["CurrentTime"] = new constant::NumericalInt(time); }
private:
    std::unordered_map<std::string, Renderer*> rendererList;
    
    RefracRef systemParameters;
    ShaderRef shaderList;
    TextureRef textureList;
};