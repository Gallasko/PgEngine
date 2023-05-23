#include "texture.h"

#include "logger.h"
#include "serialization.h"

#include "Renderer/renderer.h"

namespace pg
{
    static constexpr char const * DOM = "Texture";

    template <>
    void serialize(Archive& archive, const TextureComponent& value)
    {
        LOG_THIS(DOM);

        archive.startSerialization("TextureComponent");

        serialize(archive, "textureName", value.textureName);
    
        archive.endSerialization();
    }

    template <>
    TextureComponent deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        std::string type = "";

        if(serializedString.isNull())
            LOG_ERROR(DOM, "Element is null");
        else
        {
            LOG_INFO(DOM, "Deserializing an TextureComponent");

            auto textureName = deserialize<std::string>(serializedString["textureName"]);

            return TextureComponent{textureName};
        }

        return TextureComponent{""};
    }

    void TextureComponentSystem::init()
    {
        auto group = registerGroup<UiComponent, TextureComponent>();

        group->addOnGroup([](EntityRef entity) {
            LOG_INFO("Texture Component System", "Add entity " << entity->id << " to ui - tex group !");

            auto ui = entity->get<UiComponent>();
            auto tex = entity->get<TextureComponent>();

            auto tName = tex->textureName;

            auto sys = entity->world()->getSystem<MasterRenderer>();

            auto mesh = sys->meshBuilder.getTextureMesh(ui->width, ui->height, tName);

            auto rTex = RenderableTexture{entity->id, ui, mesh};

            sys->tempRenderList["default"][tName].push_back(rTex);

            sys->changed = true;
        });
    }

    void TextureComponentSystem::onEvent(const UiComponentChangeEvent& event)
    {
        auto entity = ecsRef->getEntity(event.id);

        if(not entity->has<TextureComponent>())
            return;

        auto ui = entity->get<UiComponent>();

        // Todo check if the entity has a sentence text before trying to modify it
        auto tex = entity->get<TextureComponent>();

        auto tName = tex->textureName;

        auto sys = entity->world()->getSystem<MasterRenderer>();

        auto mesh = sys->meshBuilder.getTextureMesh(ui->width, ui->height, tName);

        auto rTex = RenderableTexture{event.id, ui, mesh};

        LOG_MILE("Texture Component System", "Modification of id: " << entity->id << " texture");

        std::lock_guard<std::mutex> lock (sys->modificationMutex);

        auto first = sys->tempRenderList["default"][tName].begin();
        auto last = sys->tempRenderList["default"][tName].end();

        while (first != last)
        {
            if (first->entityId == event.id)
            {
                *first = rTex;
                break;
            }

            ++first;
        }

        sys->changed = true;
    }

    CompList<UiComponent, TextureComponent> makeUiTexture(EntitySystem *ecs, float width, float height, const std::string& name)
    {
        auto entity = ecs->createEntity();

        auto ui = ecs->attach<UiComponent>(entity);

        ui->setWidth(width);
        ui->setHeight(height);

        auto tex = ecs->attach<TextureComponent>(entity, name);

        return CompList<UiComponent, TextureComponent>(entity, ui, tex);
    }
}