#pragma once

#include <unordered_map>

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>

#include <QMatrix4x4>
#include <cstdarg>

#include "..\constant.h"
#include "meshbuilder.h"

namespace pg
{
    typedef constant::RefracTable RefracRef;
    typedef std::unordered_map<std::string, QOpenGLShaderProgram*> ShaderRef;
    typedef std::unordered_map<std::string, unsigned int> TextureRef;

    class MasterRenderer;

    // TODO make a specialized renderer for std::nullptr_t to catch nullptr error;

    template <typename... Args>
    void renderer(MasterRenderer* masterRender, Args... args);

    //[TODO] Multiple FBO -> 1 for a whole screen capture and other for batch rendering on a texture 
    // Add Particle systeme with instancing already done / create an alternative if needed

    class MasterRenderer : protected QOpenGLFunctions
    {
    public:
        MasterRenderer() {}
        ~MasterRenderer() { delete extraFunctions; delete squareObject; delete instanceVBO; }

        void initialize(QOpenGLContext *m_context) { initializeGlObject(m_context); initializeParameters(); }

        void registerShader(const std::string& name, QOpenGLShaderProgram *shaderProgram) { shaderList[name] = shaderProgram; }
        void registerShader(const std::string& name, const char* vsPath, const char* fsPath);

        void registerTexture(const std::string& name, unsigned int textureId) { textureList[name] = textureId; }
        void registerTexture(const std::string& name, const char* texturePath);

        //TODO raise exception on none presence of attribute
        QOpenGLShaderProgram* getShader(const std::string& name) { return shaderList[name]; }
        unsigned int getTexture(const std::string& name) { return textureList[name]; }

        //TODO check if we need to make a special case UiComponent

        template <typename... Args>
        void render(const Args&... args) { renderer(this, args...); }

        template <typename Renderable>
        MasterRenderer& operator<<(Renderable* toRender) { renderer(this, toRender); return *this; }

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
        
        RefracRef systemParameters;
        ShaderRef shaderList;
        TextureRef textureList;
    };
}