#pragma once

#include "UI/sentencesystem.h"

namespace pg
{
    struct MockSentenceSystem : public SentenceSystem
    {
        MockSentenceSystem(MasterRenderer* masterRenderer) : SentenceSystem(masterRenderer, "res/font/fontmap.ft") { }

        size_t getCurrentSize() const { return currentSize; }
        size_t getElementIndex() const { return elementIndex; }
        size_t getVisibleElements() const { return visibleElements; }
        size_t getNbAttributes() const { return nbAttributes; }

        std::unordered_map<_unique_id, size_t>& getIdToIndexMap() { return idToIndexMap; }

        float * getBuffer() { return bufferData; }
    };
}