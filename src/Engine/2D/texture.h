#pragma once

#include "Renderer/renderer.h"

#include "UI/uisystem.h"
#include "constant.h"

#include "logger.h"

namespace pg
{
    struct TextureChangeEvent
    {
        _unique_id id;
        std::string oldTextureName;
        std::string newTextureName;
    };

    struct TextureMesh : public Mesh
    {
        TextureMesh() : Mesh() { LOG_THIS_MEMBER("Texture Mesh"); modelInfo = constant::SquareInfo{}; }
        ~TextureMesh() { LOG_THIS_MEMBER("Texture Mesh"); }

        void generateMesh();
    };

    struct Texture2DComponent : public Ctor
    {
        Texture2DComponent(const std::string& textureName) : textureName(textureName) { }

        template <typename... Values>
        Texture2DComponent(const std::string& textureName, Values... args) : textureName(textureName) { addParameter(args...); }

        Texture2DComponent(const Texture2DComponent &rhs) : textureName(rhs.textureName), additionnalParameters(rhs.additionnalParameters) { }
        virtual ~Texture2DComponent() {}

        template <typename... Values>
        void addParameter(float value, Values... args) { additionnalParameters.push_back(value); addParameter(args...); }

        // Do nothing, end of the template recursion
        void addParameter() { }

        virtual void onCreation(EntityRef entity) { this->entity = entity; }

        inline static std::string getType() { return "Texture2DComponent"; } 

        inline void setTexture(const std::string& textureName)
        {
            if(entity)
                entity->world()->sendEvent(TextureChangeEvent{entity->id, this->textureName, textureName});
            
            this->textureName = textureName;
        }

        std::string textureName;

        std::vector<float> additionnalParameters;

        Entity *entity = nullptr;
    };

    template <>
    void serialize(Archive& archive, const Texture2DComponent& value);

    template <>
    Texture2DComponent deserialize(const UnserializedObject& serializedString);

    struct Texture2DComponentSystem : public AbstractRenderer, System<Own<Texture2DComponent>, Listener<UiComponentChangeEvent>, Ref<UiComponent>, NamedSystem, InitSys, StoragePolicy>
    {
        struct TextureBuffer
        {
            struct SimpleSquareMesh : public Mesh
            {
                SimpleSquareMesh() : Mesh()
                { 
                    LOG_THIS_MEMBER("Shape 2D Mesh");
                    modelInfo = constant::SquareInfo{};

                    modelInfo.vertices[0] =  0.0f; modelInfo.vertices[1] =   0.0f;
                    modelInfo.vertices[5] =  1.0f; modelInfo.vertices[6] =   0.0f;
                    modelInfo.vertices[10] = 0.0f; modelInfo.vertices[11] = -1.0f;
                    modelInfo.vertices[15] = 1.0f; modelInfo.vertices[16] = -1.0f;
                    
                }

                virtual ~SimpleSquareMesh();

                void generateMesh();

                OpenGLBuffer *instanceVBO = nullptr;
            };

            TextureBuffer(size_t nbAttributes) : nbAttributes(nbAttributes) {}

            void swapIndex(size_t origin, size_t destination);

            bool sizeChanged = false;

            std::atomic<size_t> elementIndex {0};
            std::atomic<size_t> visibleElements {0};
            size_t currentSize = 0;

            std::mutex renderMutex;
            float *bufferData = nullptr;
            const size_t nbAttributes; // Should be always at least equal to 7 ! (x, y, z, tx, ty w, h)

            SimpleSquareMesh basicSquareMesh;

            std::unordered_map<_unique_id, size_t> idToIndexMap;
        };

        Texture2DComponentSystem(MasterRenderer* masterRenderer) : AbstractRenderer(masterRenderer, RenderStage::Render) { }

        virtual std::string getSystemName() const override { return "Ui Texture System"; }

        virtual void render() override;

        virtual void init() override;

        virtual void onEvent(const UiComponentChangeEvent& event) override;

        Mesh* getTextureMesh(float width, float height, const std::string& name);

        void addElement(const CompRef<UiComponent>& ui, const CompRef<Texture2DComponent>& obj);

        void removeElement(_unique_id id);

        std::shared_ptr<TextureBuffer> getTexBuffer(const std::string& mainTexName);

        std::shared_ptr<TextureBuffer> getTexBuffer(_unique_id id);

        bool squareMeshInitialized = false;

        // Map texture name to its corresponding texture buffer
        std::map<std::string, std::shared_ptr<TextureBuffer>> textureBuffers;
    };

    /** Helper that create an entity with an Ui component and a Texture component */
    CompList<UiComponent, Texture2DComponent> makeUiTexture(EntitySystem *ecs, float width, float height, const std::string& name);

}