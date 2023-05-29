#pragma once

#include <unordered_map>

#include <mutex>

#include <cstdarg>

#include "ECS/system.h"

#include "..\constant.h"
#include "meshbuilder.h"

namespace pg
{
    // Forwarding
    class OpenGLShaderProgram;
    class OpenGLContext;
    class MasterRenderer;

    // Type def
    typedef constant::RefracTable RefracRef;
    typedef std::unordered_map<std::string, OpenGLShaderProgram*> ShaderRef;
    typedef std::unordered_map<std::string, unsigned int> TextureRef;

    // TODO make a specialized renderer for std::nullptr_t to catch nullptr error;

    template <typename... Args>
    void renderer(MasterRenderer* masterRender, Args... args);

    class UiComponent;
    class TextureComponent;

    struct RenderableTexture
    {
        _unique_id entityId;
        CompRef<UiComponent> uiRef;
        MeshBuilder::MeshRef meshRef;
    };

    //[TODO] Multiple FBO -> 1 for a whole screen capture and other for batch rendering on a texture 
    // Add Particle system with instancing already done / create an alternative if needed

    class MasterRenderer : public System<NamedSystem>
    {
    public:
        MasterRenderer();
        ~MasterRenderer();

        virtual std::string getSystemName() const override { return "Renderer System"; }

        virtual void execute() override;

        void renderAll();

        void registerShader(const std::string& name, OpenGLShaderProgram *shaderProgram);
        void registerShader(const std::string& name, const char* vsPath, const char* fsPath);

        void registerTexture(const std::string& name, unsigned int textureId) { textureList[name] = textureId; }
        void registerTexture(const std::string& name, const char* texturePath);

        //TODO raise exception on none presence of attribute
        OpenGLShaderProgram* getShader(const std::string& name) { return shaderList[name]; }
        unsigned int getTexture(const std::string& name) { return textureList[name]; }

        template <typename... Args>
        void render(const Args&... args) { renderer(this, args...); }

        template <typename Renderable>
        MasterRenderer& operator<<(Renderable* toRender) { renderer(this, toRender); return *this; }

        inline void setWindowSize(const int& width, const int& height) { systemParameters["ScreenWidth"] = width; systemParameters["ScreenHeight"] = height; }
        inline void setCurrentTime(const unsigned int& time) { systemParameters["CurrentTime"] = static_cast<int>(time); }

        RefracRef getParameter() const { return systemParameters; }
        
    private:
        void initializeParameters();

    public:
        bool changed = false;

        std::mutex modificationMutex;
        std::mutex renderMutex;

        std::map<std::string, std::map<std::string, std::vector<RenderableTexture>>> tempRenderList;
        std::map<std::string, std::map<std::string, std::vector<RenderableTexture>>> currentRenderList;
        
        RefracRef systemParameters;
        ShaderRef shaderList;
        TextureRef textureList;

        MeshBuilder meshBuilder;

        size_t nbRenderedFrames = 0;
    };
}