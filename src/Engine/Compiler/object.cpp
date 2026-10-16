#include "stdafx.h"

#include "object.h"

#include "chunk.h"

#include "vm.h"

namespace pg
{
    Closure::Closure(ObjFunction* func) : function(func)
    {
        upvalues.resize(func->upvalueCount);
    }

    void ObjInstance::setField(const std::string& name, Value value, VM *vm, bool deleteOld)
    {
        const size_t slot = findField(name);

        if (slot != npos)
        {
            if (deleteOld and vm)
            {
                // Release old value if it requires ref counting
                Value oldValue = fieldValueList[slot];
                if (requiresRefCount(oldValue))
                {
                    vm->releaseValue(oldValue);
                }
            }

            // Update existing field
            fieldValueList[slot] = value;
        }
        else
        {
            appendField(name, value);
        }
    }
}