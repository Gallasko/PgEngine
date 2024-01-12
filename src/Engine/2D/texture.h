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
        Texture2DComponent(const Texture2DComponent &rhs) : textureName(rhs.textureName) { }
        virtual ~Texture2DComponent() {}

        virtual void onCreation(EntityRef entity) { this->entity = entity; }

        inline static std::string getType() { return "Texture2DComponent"; } 

        inline void setTexture(const std::string& textureName)
        {
            if(entity)
                entity->world()->sendEvent(TextureChangeEvent{entity->id, this->textureName, textureName});
            
            this->textureName = textureName;
        }

        std::string textureName;

        Entity *entity = nullptr;
    };

    template <>
    void serialize(Archive& archive, const Texture2DComponent& value);

    template <>
    Texture2DComponent deserialize(const UnserializedObject& serializedString);

    struct Texture2DComponentSystem : public AbstractRenderer, System<Own<Texture2DComponent>, Listener<UiComponentChangeEvent>, Ref<UiComponent>, NamedSystem, InitSys, StoragePolicy>
    {
        Texture2DComponentSystem(MasterRenderer* masterRenderer) : AbstractRenderer(masterRenderer, RenderStage::Render) { }

        virtual std::string getSystemName() const override { return "Ui Texture System"; }

        virtual void render() override;

        virtual void init() override;

        virtual void onEvent(const UiComponentChangeEvent& event) override;

        Mesh* getTextureMesh(float width, float height, const std::string& name);
    };

    /** Helper that create an entity with an Ui component and a Texture component */
    CompList<UiComponent, Texture2DComponent> makeUiTexture(EntitySystem *ecs, float width, float height, const std::string& name);

}