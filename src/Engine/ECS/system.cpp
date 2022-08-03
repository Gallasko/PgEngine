#include "system.h"

namespace pg
{
    namespace ecs
    {
        void addCreationFunction(std::unordered_map<std::string, componentCreateFunction> *cTorLookupTable)
        {
            // Does nothing, terminator class for addCreationFunction
        }
    }
}