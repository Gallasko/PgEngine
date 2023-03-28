#include "inventory.h"

#include <fstream>

void lootInventory(Inventory* src, Inventory* dst)
{
    // auto srcEcs = src->world();
    // auto dstEcs = dst->world();

    // for(auto item : src->componentList)
    {
        // dstEcs->move(item, src, srcEcs, dst);
    }
}

// /**
//  * @brief Construct a new ItemLibrary object with the content of a given file
//  * 
//  * @param filename Path to the file to be loaded
//  */
// ItemLibrary::ItemLibrary(const std::string& filename) : ItemLibrary()
// {
//     readFile(filename);
// }

// /**
//  * @brief Read all the contents of a given file and create Item reference from it
//  * 
//  * @param filename Path to the file to be loaded
//  */
// void ItemLibrary::readFile(const std::string& filename)
// {
//     // File opening
//     std::ifstream f;

// 	f.open(filename, std::ifstream::in);

// 	if(f.is_open())
// 	{
// 		for(std::string line; std::getline(f, line);)
//         {

//         }
//     }

//     f.close();
// }

