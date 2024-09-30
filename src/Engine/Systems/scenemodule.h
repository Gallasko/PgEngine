#pragma once

#include "Scene/scenemanager.h"

#include "Interpreter/pginterpreter.h"

namespace pg
{
    class LoadSceneFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef)
        {
            setArity(1, 1);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto sceneName = args.front()->getElement();
            args.pop();

            if (not sceneName.isLitteral())
            {
                LOG_ERROR("Load Scene Function", "Cannot load a scene, no name specified");

                return nullptr;
            }

            ecsRef->sendEvent(LoadScene{sceneName.toString()});

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
    };

    class SaveSceneFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef)
        {
            setArity(0, 0);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue&) override
        {
            ecsRef->sendEvent(SaveScene{});

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
    };

    class PrintSceneFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(EntitySystem* ecsRef)
        {
            setArity(0, 0);

            this->ecsRef = ecsRef;
        }

        virtual ValuablePtr call(ValuableQueue&) override
        {
            auto sys = ecsRef->getSystem<SceneElementSystem>();

            if (not sys)
            {
                LOG_ERROR("Print Scene Function", "Scene system is not loaded");
                
                return nullptr;
            }

            // Todo fix this
            // for (const auto& elem : sys->serializer.getSerializedMap())
            // {
            //     // TODO: change this
            //     std::cout << "[Interpreter]: " << sys->serializer.getVersion() << std::endl;

            //     std::cout << "[Interpreter]: Scene object named " << elem.first << std::endl;
            //     std::cout << "[Interpreter]: serial str --> " << elem.second << std::endl;
            // }

            return nullptr;
        }

        EntitySystem* ecsRef = nullptr;
    };

    struct SceneModule : public SysModule
    {
        SceneModule(EntitySystem *ecsRef)
        {
            addSystemFunction<LoadSceneFunction>("loadScene", ecsRef);
            addSystemFunction<SaveSceneFunction>("saveScene", ecsRef);
            addSystemFunction<PrintSceneFunction>("printScene", ecsRef);            
        }
    };
}