#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "../../Engine/constant.h"
#include "ECS/entitysystem.h"
#include "ECS/system.h"

using namespace pg;

namespace pg
{
    using namespace ecs;
}


struct Item : public Component
{
    Item() : Component("Item") {}

    unsigned int id;
    unsigned int amount = 1;
    unsigned int stackSize = 1;
};

struct Inventory : public Component
{
    Inventory(unsigned int capacity) : Component("Inventory"), capacity(capacity) {}

    unsigned int capacity;
};

template <typename Type>
bool addItemToInventory(Inventory* inventory, Type* item)
{
    auto ecs = inventory->world();
    auto heldItem = inventory->get<Type>();

    if(heldItem)
    {
        if(heldItem->amount + item->amount > heldItem->stackSize)
            return false;

        heldItem->amount += item->amount;
        return true;
    }

    ecs->attach(inventory, item);
    return true;
}

void lootInventory(Inventory* src, Inventory* dst);

// /**
//  * @struct Item
//  * 
//  * An object holding all information about a particular item
//  * 
//  */
// struct Item
// {
//     unsigned int id;
//     std::string name;

//     constant::RefracTable additionalInfo;
// };

// /**
//  * @class ItemLibrary
//  * 
//  * An ItemLibrary implementation which stores a list of items informations.
//  * 
//  * An Item Library is constructed by passing a file containing the item information.
//  * Then in the code, one can obtain an item from the library using the getters methods.
//  * 
//  */
// class ItemLibrary
// {
// public:
//     // [Constructor]
//     /** Create an Empty ItemLibrary */
//     ItemLibrary() { items.push_back(new Item{0, "None"}); itemsMap["None"] = items[0]; }
    
//     /** Create a Library from a file */
//     ItemLibrary(const std::string& filename);



//     // [Public Interfaces]

//     /** Read a file and extract its containing items informations */
//     void readFile(const std::string& filename);

//     // [Getter]
//     inline Item getItem(const std::string& name) const { if(itemsMap.find(name) != itemsMap.end()) return *(itemsMap.at(name)); return *items[0]; }
//     inline Item getItem(int id) const { if(items.size() < id) return *items[id]; return *items[0]; }

// private:
//     /** List of all items in the library, items[0] is a None item */
//     std::vector<Item*> items;

//     /** Map of the reference of the items in the library*/
//     std::unordered_map<std::string, Item*> itemsMap;
// };

// class Inventory
// {
//     struct ItemSlot
//     {
//         unsigned int itemId;

//         int stackSize;
//     };

// public:
//     Inventory(unsigned int inventorySize);

//     bool push(const Item& item);

// private:
//     std::vector<ItemSlot> inventorySlots;
// };