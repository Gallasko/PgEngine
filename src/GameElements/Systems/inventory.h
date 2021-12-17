#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct Item
{
    unsigned int id;
    std::string name;
};

class ItemLibrary
{
public:
    // [Constructor]
    /** Create an Empty ItemLibrary */
    ItemLibrary() { items.push_back(Item{0, "None"}); itemsMap["None"] = &items[0]; }

    // [Getter]
    inline Item getItem(const std::string& name) const { if(itemsMap.find(name) != itemsMap.end()) return *(itemsMap[name]); return items[0]; }
    inline Item getItem(int id) const { if(items.size() < id) return items[id]; return items[0]; }

private:
    /** List of all items in the library, items[0] is a None item */
    std::vector<Item> items;

    /** Map to the reference of the items in the library*/
    std::unordered_map<std::string, Item*> itemsMap;
};



class Inventory
{
    struct ItemSlot
    {
        unsigned int itemId;

        int stackSize;
    };

public:
    Inventory(unsigned int inventorySize);

    bool push(const Item& item);

private:
    std::vector<ItemSlot> inventorySlots;
};