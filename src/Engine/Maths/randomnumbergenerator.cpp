#include "randomnumbergenerator.h"

#include <stdlib.h>

#include <mutex>

#include "../logger.h"

namespace pg
{
    namespace
    {
        static constexpr char const * DOM = "Number Generator";
    }

    template <>
    void serialize(Archive& archive, const RandomNumberGenerator& generator)
    {
        LOG_THIS(DOM);

        archive.startSerialization("Number Generator");

        serialize(archive, "seed", generator.getSeed());

        archive.endSerialization();
    }

    template <>
    RandomNumberGenerator deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        RandomNumberGenerator generator;

        if(serializedString.isNull())
        {
            LOG_INFO(DOM, "Random Number Generator is not in the serialized file.");
            return generator;
        }

        auto element = serializedString["seed"];

        if (element.isNull())
        {
            LOG_ERROR(DOM, "Element is null");
        }
        else if (element.getObjectName() == "")
        {
            LOG_ERROR(DOM, "Element has no name");
        }
        else
        {
            LOG_INFO(DOM, "Deserializing " + element.getObjectName());

            auto seed = deserialize<unsigned int>(element);

            generator.setSeed(seed);
        }
        
        return generator;
    }

    int RandomNumberGenerator::generateNumber()
    {
        static std::mutex syncMutex;
        std::lock_guard<std::mutex> lock(syncMutex);

        auto rng = rand();
        this->setSeed(rng);
        return rng;
    }

    void RandomNumberGenerator::setSeed(unsigned int seed)
    {
        srand(seed);
        this->seed = seed;
    }

    RandomNumberGenerator::RandomNumberGenerator(bool fromSerialization)
    {
        if(fromSerialization)
        {
            auto generator = Serializer::getSerializer()->deserializeObject<RandomNumberGenerator>("Random Number Generator");
            this->setSeed(generator.seed);
        }
    }
}