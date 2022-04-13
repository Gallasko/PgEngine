#pragma once

#include <memory>

#include "../serialization.h"

namespace pg
{
    class RandomNumberGenerator
    {
    public:
        static std::unique_ptr<RandomNumberGenerator>& generator() { static auto generator = std::unique_ptr<RandomNumberGenerator>(new RandomNumberGenerator(true)); return generator; }

        int generateNumber();

        void setSeed(unsigned int seed);
        inline unsigned int getSeed() const { return seed; }

    private:
        friend RandomNumberGenerator deserialize<>(const UnserializedObject& serializedString);

        RandomNumberGenerator(bool fromSerialization = false);
        unsigned int seed = 0;
    };

}