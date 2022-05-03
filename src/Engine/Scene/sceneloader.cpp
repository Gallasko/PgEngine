#include "sceneloader.h"

#include <sstream>

#include "../logger.h"

namespace pg
{
    namespace
    {
        const char * DOM = "Scene Loader";

        /**
         * @brief A function that trim all whitespace characters from the beginning of a string
         * 
         * @param str The string to trim
         * @param whitespace Characters used as whitespace characters
         * 
         * @return std::string The resulting string trimmed of the whitespace characters
         */
        std::string trim(const std::string& str, const std::string& whitespace = " \t")
        {
            LOG_THIS(DOM);

            const auto strBegin = str.find_first_not_of(whitespace);
            if (strBegin == std::string::npos)
                return ""; // no content

            const auto strEnd = str.find_last_not_of(whitespace);
            const auto strRange = strEnd - strBegin + 1;

            return str.substr(strBegin, strRange);
        }
    }

    template<>
    void renderer(MasterRenderer* masterRenderer, Scene* scene)
    {
        LOG_THIS(DOM);

        for(int i = 0; i < scene->nbAllocatedObjects; i++)
        {
            scene->sceneObjects[i]->render(masterRenderer);
        }
    }

    Scene::Scene(const size_t& nbObjects) : nbMaxObjects(nbObjects), nbAllocatedObjects(0)
    {
        LOG_THIS_MEMBER(DOM);

        sceneObjects.reserve(nbObjects);
    }

    Scene::~Scene()
    {
        LOG_THIS_MEMBER(DOM);

        for(int i = 0; i < nbAllocatedObjects; i++)
        {
            delete sceneObjects[i];
        }
    }

    void Scene::addObject(const std::string& name, UiComponent* component)
    {
        LOG_THIS_MEMBER(DOM);

        if(nbAllocatedObjects >= nbMaxObjects)
        {
            LOG_ERROR(DOM, "Try to add more objects than allowed in the scene, '" + name + "'");
            delete component;
            return;
        }

        sceneObjects[nbAllocatedObjects] = component;
        sceneObjectsRefs[name] = nbAllocatedObjects;

        nbAllocatedObjects++;
    }

    void Scene::onEnter()
    {
        LOG_THIS_MEMBER(DOM);
    }

    void Scene::onLeave()
    {
        LOG_THIS_MEMBER(DOM);
    }

    SceneLoader::SceneLoader()
    {
        LOG_THIS_MEMBER(DOM);
    }

    SceneLoader::~SceneLoader()
    {
        LOG_THIS_MEMBER(DOM);
    }

    void SceneLoader::addPrefab(const std::string& name, const uiCtorFunc& creator)
    {
        cTorLookupTable[name] = creator;
    }

    Scene* SceneLoader::load(const TextFile& sceneFile) const
    {
        LOG_THIS_MEMBER(DOM);

        Serializer serializer(sceneFile);

        const auto& serializedSceneObjects = serializer.getSerializedMap();

        Scene *scene = new Scene(serializedSceneObjects.size());

        for(auto serializedObject : serializedSceneObjects)
        {
            const auto serializedString = serializedObject.second;

            std::istringstream stream(serializedObject.second);
            std::string firstLine;

            if(std::getline(stream, firstLine))
            {
                const auto pos = firstLine.find(':');

                if(pos == std::string::npos)
                    LOG_ERROR(DOM, "Can't create '" + serializedObject.first + "', serialized string doesn't name a class !");
                else
                {
                    const auto className = trim(firstLine.substr(0, pos));

                    const auto it = cTorLookupTable.find(className);

                    if(it != cTorLookupTable.end())
                    {
                        scene->addObject(serializedObject.first, it->second(serializedString));
                    }
                    else
                    {
                        LOG_ERROR(DOM, "Can't create '" + serializedObject.first + "', no constructor for class: '" + className + "' exist in the scene loader !");
                    }
                }
            }
            else
            {
                LOG_ERROR(DOM, "Can't create '" + serializedObject.first + "', serialized string is empty !");
            }
        }

        return scene;
    }

    void SceneLoader::unload(Scene *scene)
    {
        LOG_THIS(DOM);

        delete scene;
    }
}