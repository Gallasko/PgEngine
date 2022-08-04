#pragma once

#include <cstdint>

namespace pg
{
    namespace ecs
    {
        typedef uint_fast64_t _unique_id;
        typedef uint_fast64_t _entityId;

        _unique_id generateId();

        _unique_id generateSystemId();
    }
}