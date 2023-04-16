#pragma once

#include "entity.h"

#include "concurrentqueue.h"

#include "logger.h"

namespace pg
{
    // Type forwarding
    class EntitySystem;
    class Entity;

    struct SysCommand
    {
        enum class SysCommandType
        {
            createEntity,
            deleteEntity,
            startSystem,
            stopSystem,
            log
        };

        SysCommandType type;
    };

    /**
     * @brief A base ECS system responsible of system commands generated from other systems.
     * 
     * This system is responsible for creating and running ECS system commands
     * Should be used for every commands that need to be executed asynchronously such as
     * creating or deleting an Entity or a component.
     */
    class CommandDispatcher
    {
    public:
        typedef moodycamel::ConcurrentQueue<SysCommand>::producer_token_t CommandToken;

        struct EntityCommand
        {
            enum class EntityCommandType
            {
                creation = 0,
                deletion = 1
            };

            EntityCommand(Entity *entity, const EntityCommandType& type) : entity(entity), type(type) {}

            Entity *entity;
            EntityCommandType type;
        };

        struct ComponentCommand
        {
            enum class ComponentCommandType
            {
                creation = 0,
                deletion = 1
            };

            struct Storage {};

            ComponentCommand() : component(nullptr), type(ComponentCommandType::creation) {}

            template <typename Type>
            ComponentCommand(EntityRef entity, Type *component, const ComponentCommandType& type) : entity(entity), type(type)
            {
                struct Delegate : public Storage, public Type {};

                this->component = static_cast<Storage*>(static_cast<Delegate*>(component));

                setupFunctions<Type>();
            }

            template <typename Type>
            void setupFunctions();

            EntityRef entity;
            Storage *component;
            ComponentCommandType type;

            void(*addInEcs)(EntitySystem*, EntityRef, Storage*);
            void(*deleteComp)(Storage*);
        };

    public:
        CommandDispatcher(EntitySystem *ecs) : ecsRef(ecs) { LOG_THIS_MEMBER("Command Dispatcher"); }

        EntityRef createEntity();

        void deleteEntity(Entity* entity);

        template <typename Type, typename... Args>
        Type* attachComp(EntityRef entity, Args&&... args)
        {
            LOG_THIS_MEMBER("Command Dispatcher");

            auto comp = new Type(std::forward<Args>(args)...);

            if(not componentQueue.enqueue(ComponentCommand{entity, comp, ComponentCommand::ComponentCommandType::creation}))
            {
                LOG_ERROR("Command Dispatcher", "Could not enqueue the creation of component " << typeid(Type).name());
                return nullptr;
            }

            return comp;
        }

        inline bool enqueueCommand(const SysCommand& cmd)
        {
            return sysQueue.enqueue(cmd);
        }

        inline bool enqueueCommand(SysCommand&& cmd)
        {
            return sysQueue.enqueue(cmd);
        }

        inline bool enqueueCommand(const CommandToken& token, const SysCommand& cmd)
        {
            return sysQueue.enqueue(token, cmd);
        }

        void process();

    private:
        EntitySystem *const ecsRef;

        moodycamel::ConcurrentQueue<EntityCommand> entityQueue;

        moodycamel::ConcurrentQueue<ComponentCommand> componentQueue;

        moodycamel::ConcurrentQueue<SysCommand> sysQueue;
    };
}