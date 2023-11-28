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
        
    };

    class SceneLoader : public System<>
    {
    public:
        SceneLoader();
        ~SceneLoader();

        void addPrefab(const std::string& name, const UiCtorFunc& creator);

        Scene* load(const TextFile& sceneFile) const;
        static void unload(Scene *scene);

    private:
        Serializer sceneSerializer;

    private:
        std::unordered_map<std::string, UiCtorFunc> cTorLookupTable; 
    };
}