#include "randomnumbergenerator.h"

#include <stdlib.h>

#include "../logger.h"
#include "../serialization.h"

namespace pg
{
    namespace
    {
        const char * DOM = "Number Generator";
    }

    template<>
    void serialize(Archive& archive, const RandomNumberGenerator& generator)
    {
        LOG_THIS(DOM);

        archive.startSerialization("Number Generator");

        serialize(archive, "seed", generator.getSeed());

        archive.endSerialization();
    }

    template<>
    RandomNumberGenerator deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        RandomNumberGenerator generator;

        auto element = serializedString["seed"];

        if(element.isNull())
            LOG_ERROR(DOM, "Element is null");
        else if(element.getObjectName() == "")
            LOG_ERROR(DOM, "Element has no name");
        else
        {
            LOG_INFO(DOM, "Deserializing " + element.getObjectName());

            auto seed = deserialize<unsigned int>(element);

            generator.setSeed(seed);
        }
        
        return generator;
    }


    int RandomNumberGenerator::getNumbers()
    {
        auto rng = rand();
        this->seed = rng;
        return rng;
    }

    void RandomNumberGenerator::setSeed(unsigned int seed)
    {
        srand(seed);
        this->seed = seed;
    }

    RandomNumberGenerator::RandomNumberGenerator(bool fromSerialization = false)
    {
        if(fromSerialization)
        {
            auto generator = Serializer::getSerializer()->deserializeObject<RandomNumberGenerator>("Random Number Generator");
            this->setSeed(generator.seed);
        }
    }
}