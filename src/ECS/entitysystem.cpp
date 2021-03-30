#include "entitysystem.h"

void EntitySystem::GroupList::erase(unsigned int entityId)
{
    auto start = begin();
    EntitySystem::GroupList::GroupItem *previousGroupItem = nullptr;

    if(size() != 0)
    {
        while ((*start).next != nullptr && (*start).entityId != entityId)
        {
            previousGroupItem = &(*start);
            start++;
        }
            
        if((*start).entityId == entityId)
        {
            if(previousGroupItem != nullptr)
                previousGroupItem->next = (*start).next;

            if(start == begin())
                head.node = head.node->next;

            start.deleteNode();
            nbElement--;
        }
    }
}

bool EntitySystem::GroupList::changeId(unsigned int idToBeChanged, unsigned int newId)
{
    auto start = begin();

    if(size() != 0)
    {
        while ((*start).next != nullptr && (*start).entityId != idToBeChanged)
            start++;

        if((*start).entityId == idToBeChanged)
        {
            (*start).entityId = newId;
            return true;
        }
    }

    return false;
}

bool EntitySystem::GroupList::find(unsigned int entityId)
{
    auto start = begin();

    if(size() != 0)
    {
        while ((*start).next != nullptr && (*start).entityId != entityId)
            start++;

        return (*start).entityId == entityId;
    }

    return false;
    
}

void EntitySystem::removeEntity(EntitySystem::Entity* entity)
{
    if(entity == lastEntity)
    {
        auto it = entity->componentList.begin();

        while(it != entity->componentList.end())
            it = dettach(entity, it->first, it);

        lastEntity = lastEntity->previousEntity;
        if (lastEntity != nullptr)
            lastEntity->nextEntity = nullptr;
    }
    else
    {
        auto it = entity->componentList.begin();

        while(it != entity->componentList.end())
        {
            if(lastEntity->has(it->first))
            {
                auto toBeChanged = entity->getComponent(it->first);
                auto toBeRemove = lastEntity->getComponent(it->first);

                toBeChanged->data = toBeRemove->data;

                dettach(lastEntity, it->first, lastEntity->componentList.find(it->first));
                lastEntity->componentList[it->first] = toBeChanged;

                it++;
            }
            else
            {
                it = dettach(entity, it->first, it);
            }
        }

        EntitySystem::Entity *last;
        
        if(lastEntity->previousEntity != entity)
            last = lastEntity->previousEntity;
        else
            last = nullptr;

        auto lastId = lastEntity->id;

        lastEntity->id = entity->id;
        lastEntity->previousEntity = entity->previousEntity;
        lastEntity->nextEntity = entity->nextEntity;
        
        if(lastEntity->nextEntity != nullptr && last != nullptr)
            lastEntity->nextEntity->previousEntity = lastEntity;

        if(lastEntity->previousEntity != nullptr)
            lastEntity->previousEntity->nextEntity = lastEntity;

        for(auto it : lastEntity->componentList)
            if(it.second->entityId != lastEntity->id)
                moveBack(lastEntity, it.first, it.second);

        for(auto it : groupList)
        {
            if(isEntityInGroup(lastEntity, it.first))
            {
                if(!it.second->changeId(lastId, lastEntity->id))
                {
                    if(!it.second->find(entity))
                    {
                        auto item = new EntitySystem::GroupList::GroupItem(entity->id);

                        for(auto it2 : groupNameSpliceList[it.first])
                            item->componentList[it2] = lastEntity->getComponent(it2);

                        it.second->append(item);
                    }
                }
            }
        }

        if(last != nullptr)
            lastEntity = last;
        lastEntity->nextEntity = nullptr;
    }

    nbEntity--;
    delete entity;
}

std::unordered_map<std::string, EntitySystem::GenericComponent* >::iterator EntitySystem::dettach(EntitySystem::Entity *entity, std::string id, std::unordered_map<std::string, EntitySystem::GenericComponent* >::iterator it)
{
    if(componentMap.find(id) != componentMap.end())
    {
        EntitySystem::GenericComponent* component = entity->getComponent(id);

        if(component != nullptr)
        {
            // if prev is nullptr it means that it was the only element of the list
            if(component->prev != nullptr)
            {
                if(component->next == nullptr) // if component is the last element of the list
                {
                    component->prev->next = nullptr;

                    if(component->prev == componentMap[id])
                    {
                        componentMap[id]->prev = nullptr;
                    }
                    else
                    {
                        componentMap[id]->prev = component->prev;
                    }
                }
                else
                {
                    if(component->prev != component->next) // if prev == next it means that there are only 2 elements in the list;
                    {
                        if(component->prev->next == nullptr) // it means that the current component is the first one
                        {
                            componentMap[id] = component->next;
                        }
                        else
                        {
                            component->prev->next = component->next;
                        }

                        component->next->prev = component->prev;
                    }
                    else
                    {
                        componentMap[id] = component->next;
                        component->next->prev = nullptr;
                    }
                }
            }
            else
            {
                componentMap.erase(id);
            }

            for(auto it : groupList)
                if(isEntityInGroup(entity, it.first))
                    it.second->erase(entity->id);

            delete component;
            return entity->componentList.erase(it);
        }
    }

    return it++;
}

void EntitySystem::moveBack(EntitySystem::Entity *entity, std::string id, EntitySystem::GenericComponent *component)
{
    if(component->prev != nullptr)
    {
        if(component->prev->entityId > entity->id)
        {
            if(componentMap.find(id) != componentMap.end())
            {
                auto nextComponent = componentMap[id];

                while(nextComponent != nullptr)
                {
                    if(nextComponent->entityId > entity->id)
                    {
                        component->prev->next = component->next;

                        if(nextComponent->prev->next == nullptr) // it means that nextComponent is the first element
                            componentMap[id] = component;

                        if(component->next != nullptr)
                            component->next->prev = component->prev;
                        else
                           componentMap[id]->prev = component->prev;

                        if(nextComponent->prev != component) // if nextComponent->prev == component it means that component was the last element
                            component->prev = nextComponent->prev;

                        component->next = nextComponent;
                        nextComponent->prev = component;

                        if(component->prev->next != nullptr)
                            component->prev->next = component;

                        if(nextComponent->next == nullptr)
                            componentMap[id]->prev = nextComponent;

                        component->entityId = entity->id;

                        nextComponent = nullptr;
                    }

                    if(nextComponent != nullptr)
                        nextComponent = nextComponent->next;
                }
            }
        }
    }
}

bool EntitySystem::isEntityInGroup(EntitySystem::Entity *entity, std::string groupName)
{
    int nbComponent = 0;

    for(auto component : groupNameSpliceList[groupName])
        if(entity->has(component))
            nbComponent++;

    return nbComponent == static_cast<int>(groupNameSpliceList[groupName].size());
}