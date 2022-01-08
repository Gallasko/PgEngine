#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <sstream>

#include <iostream>

namespace pg
{
    template <typename Type>
    std::stringstream& serialize(const std::string& name, const Type& value, const std::stringstream& stream)
    {
        stream << name << ": " << value << ",\n";
        return stream;
    }

    class Serializer
    {
        class ClassSerializer
        {
        friend class Serializer;
            ClassSerializer(const std::string& className) { serializedString << className << " {\n"; }
            ~ClassSerializer() { serializedString << "}\n"; auto& serializer = Serializer::getSerializer(); serializer->registerToFile(serializedString, mutex); }
        
        public:
            template <typename Type>
            ClassSerializer& operator()(const std::string& name, const Type& value) { serialize(name, value); return *this; }

        private:
            std::stringstream serializedString;
            std::recursive_mutex mutex;
        };

    public:
        static std::unique_ptr<Serializer>& getSerializer()
        {static std::unique_ptr<Serializer> serializer = std::make_unique<Serializer>(); return serializer; }

        ClassSerializer serializeClass(const std::string& className) const { return ClassSerializer(className); }

    private:
        void registerToFile(const std::stringstream& serializedString, std::recursive_mutex& mutex)
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);

            std::cout << serializedString.str() << std::endl;
        }

        static std::string filename;
    };
}
