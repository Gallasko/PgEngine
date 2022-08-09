#pragma once

#include <cstdint>

namespace pg
{
    namespace ecs
    {
        typedef uint_fast64_t _unique_id;
        typedef uint_fast32_t _uint32; 

        _unique_id generateId();
    }
}