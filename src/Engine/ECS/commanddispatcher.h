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

            template <typename Type>
            ComponentCommand(Type *component, const ComponentCommandType& type) : type(type)
            {
                struct Delegate : public Storage, public Type {};

                this->component = static_cast<Storage*>(static_cast<Delegate*>(component));
            }

            Storage *component;
            ComponentCommandType type;
        };

    public:
        CommandDispatcher(EntitySystem *ecs) : ecsRef(ecs) {}

        EntityRef createEntity();

        void deleteEntity(Entity* entity);

        template <typename Type, typename... Args>
        Type* attachComp(const Args&... args)
        {
            LOG_THIS_MEMBER("Command Dispatcher");

            auto comp = new Type(args...);

            if(not componentQueue.enqueue(ComponentCommand{comp, ComponentCommand::ComponentCommandType::creation}))
            {
                LOG_ERROR("Command Dispatcher", Strfy() << "Could not enqueue the creation of component " << typeid(Type).name());
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