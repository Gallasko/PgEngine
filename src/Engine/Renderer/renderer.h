#pragma once

#include <unordered_map>

#include <mutex>

#include <cstdarg>

#include "ECS/entitysystem.h"

#include "..\constant.h"
#include "mesh.h"

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

    struct RenderableTexture
    {
        _unique_id entityId;
        CompRef<UiComponent> uiRef;
        Mesh* meshRef;
    };

    enum class RenderStage : uint8_t
    {
        PreRender,
        Render,
        PostProcess
    };

    class BaseAbstractRenderer
    {
    public:
        BaseAbstractRenderer(MasterRenderer* masterRenderer, const RenderStage& stage);
        virtual ~BaseAbstractRenderer() {}

        RenderStage getRenderStage() const { return renderStage; }
    
        virtual void render() = 0;

        virtual void updateMeshes() = 0;

    protected:
        MasterRenderer *masterRenderer;
    
        RenderStage renderStage;
    };

    class AbstractRenderer : public BaseAbstractRenderer
    {
    public:
        AbstractRenderer(MasterRenderer* masterRenderer, const RenderStage& stage) : BaseAbstractRenderer(masterRenderer, stage) {}
        virtual ~AbstractRenderer() {}

        RenderStage getRenderStage() const { return renderStage; }
    
        virtual void render() = 0;

        virtual void updateMeshes() override
        {
            if(changed)
            {
                std::lock_guard<std::mutex> lock(modificationMutex);

                std::lock_guard<std::mutex> lock2(renderMutex);

                currentRenderList = tempRenderList;

                changed = false;
            }
        }

    protected:
        bool changed = false;

        std::mutex modificationMutex;
        std::mutex renderMutex;

        std::map<unsigned int, std::vector<RenderableTexture>> tempRenderList;
        std::map<unsigned int, std::vector<RenderableTexture>> currentRenderList;

        std::unordered_map<std::string, Mesh*> meshes;
    };

    class AbstractInstanceRenderer : public AbstractRenderer
    {
    public:
        AbstractInstanceRenderer(MasterRenderer* masterRenderer, const RenderStage& stage, size_t nbAttributes) : AbstractRenderer(masterRenderer, stage),  nbAttributes(nbAttributes) {}

        virtual ~AbstractInstanceRenderer() { if(bufferData) delete[] bufferData; }

        virtual void removeElement(_unique_id id);

        virtual void increaseSize();

        virtual void swapIndex(size_t origin, size_t destination);

    protected:
        bool sizeChanged = false;

        std::atomic<size_t> elementIndex {0};
        std::atomic<size_t> visibleElements {0};
        size_t currentSize = 0;

        float *bufferData = nullptr;
        const size_t nbAttributes; // x, y, z, w(r), h(o), r, g, b 

        std::unordered_map<_unique_id, size_t> idToIndexMap;
    };

    //[TODO] Multiple FBO -> 1 for a whole screen capture and other for batch rendering on a texture 
    // Add Particle system with instancing already done / create an alternative if needed

    class MasterRenderer : public System<NamedSystem, Listener<ResizeEvent>>
    {
    public:
        MasterRenderer();
        ~MasterRenderer();

        virtual std::string getSystemName() const override { return "Renderer System"; }

        virtual void execute() override;

        inline void startResizing() { resizeMutex.lock(); }
        inline void stopResizing()  { resizeMutex.unlock(); }

        void renderAll();

        void registerShader(const std::string& name, OpenGLShaderProgram *shaderProgram);
        void registerShader(const std::string& name, const std::string& vsPath, const std::string& fsPath);

        void registerTexture(const std::string& name, unsigned int textureId) { textureList[name] = textureId; }
        void registerTexture(const std::string& name, const char* texturePath);

        //TODO raise exception on none presence of attribute
        OpenGLShaderProgram* getShader(const std::string& name) { return shaderList[name]; }
        unsigned int getTexture(const std::string& name) { return textureList[name]; }

        template <typename... Args>
        void render(const Args&... args) { renderer(this, args...); }

        template <typename Renderable>
        MasterRenderer& operator<<(Renderable* toRender) { renderer(this, toRender); return *this; }

        virtual void onEvent(const ResizeEvent& event) override
        {
            LOG_INFO("Ui internals", "Window resizing");

            std::lock_guard<std::mutex> lock(resizeMutex);

            systemParameters["ScreenWidth"] = static_cast<int>(event.width); systemParameters["ScreenHeight"] = static_cast<int>(event.height);
        }

        inline void setWindowSize(const int& width, const int& height) { systemParameters["ScreenWidth"] = width; systemParameters["ScreenHeight"] = height; }
        inline void setCurrentTime(const unsigned int& time) { systemParameters["CurrentTime"] = static_cast<int>(time); }

        RefracRef getParameter() const { return systemParameters; }

        inline void addRenderer(BaseAbstractRenderer* renderer) { renderers.push_back(renderer); }

        inline size_t getNbRenderedFrames() const { return nbRenderedFrames; }
        
    private:
        void initializeParameters();

    private:
        bool changed = false;
        
        RefracRef systemParameters;
        ShaderRef shaderList;
        TextureRef textureList;

        size_t nbRenderedFrames = 0;

        std::vector<BaseAbstractRenderer*> renderers;

        std::mutex resizeMutex;
    };
}