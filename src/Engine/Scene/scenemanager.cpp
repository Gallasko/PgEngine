#include "scenemanager.h"

namespace pg
{
    namespace
    {
    }

    template <>
    void renderer(MasterRenderer* masterRenderer, SceneManager* manager)
    {
        if(manager->currentScene)
        {
            // renderer(masterRenderer, manager->currentScene);
        }
    }

    SceneManager::SceneManager()
    {
        // Add all the basic ui elements in the loader !
        //loader.addPrefab("Texture", createTexture);
        // loader.addPrefab("Loader", [](const UnserializedObject& serializedString) { return deserialize<LoaderRenderComponent>(serializedString); });
        //loader.addPrefab("Sentence", [](const UnserializedObject& serializedString) { return deserialize<Sentence>(serializedString); })
    }

}