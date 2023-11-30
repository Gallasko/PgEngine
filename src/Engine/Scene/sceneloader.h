#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "ECS/entitysystem.h"
#include "Renderer/renderer.h"
#include "UI/uisystem.h"
#include "Files/filemanager.h"
#include "serialization.h"

namespace pg
{
    struct Scene
    {
        std::string sceneId;
    };

    class SceneLoader : public System<>
    {
    public:
        SceneLoader();
        ~SceneLoader();

        void load(const TextFile& sceneFile) const;
        static void unload(const std::string& id);

    private:
        Serializer sceneSerializer;

    };
}