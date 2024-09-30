#pragma once

#include "2D/simple2dobject.h"

namespace pg
{
    struct Mock2DSimpleShape : public Simple2DObjectSystem
    {
        Mock2DSimpleShape(MasterRenderer* masterRenderer) : Simple2DObjectSystem(masterRenderer) { }

        size_t getCurrentSize() const { return currentSize; }
        size_t getElementIndex() const { return elementIndex; }
        size_t getVisibleElements() const { return visibleElements; }
        size_t getNbAttributes() const { return nbAttributes; }

        std::unordered_map<_unique_id, size_t>& getIdToIndexMap() { return idToIndexMap; }

        float * getBuffer() { return bufferData; }
    };
}