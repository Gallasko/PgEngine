#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "../Renderer/renderer.h"
#include "../UI/uisystem.h"
#include "../Files/filemanager.h"
#include "../serialization.h"

namespace pg
{
    class Scene
    {
    public:
        Scene(const size_t& nbObjects);
        ~Scene();

        void addObject(const std::string& name, UiComponent* component);

        void onEnter();
        void onLeave();

    private:
        friend void renderer<>(MasterRenderer* masterRenderer, Scene* scene);

        std::vector<UiComponent* > sceneObjects;
        std::unordered_map<std::string, int> sceneObjectsRefs;

        int nbMaxObjects;
        int nbAllocatedObjects;
    };

    typedef std::function<UiComponent*(const UnserializedObject&)> UiCtorFunc;

    class SceneLoader
    {
    public:
        SceneLoader();
        ~SceneLoader();

        void addPrefab(const std::string& name, const UiCtorFunc& creator);

        Scene* load(const TextFile& sceneFile) const;
        static void unload(Scene *scene);

    private:
        std::unordered_map<std::string, UiCtorFunc> cTorLookupTable; 
    };
}