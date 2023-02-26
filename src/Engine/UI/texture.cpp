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

    EntityRef makeUiTexture(EntitySystem *ecs, float width, float height, const std::string& name)
    {
        auto entity = ecs->createEntity();

        auto ui = ecs->attach<UiComponent>(entity);

        ui->setWidth(width);
        ui->setHeight(height);

        ecs->attach<TextureComponent>(entity, name);

        return entity;
    }
}