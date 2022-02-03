#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <string>
#include <memory>
#include <mutex>
#include <sstream>

#include <iostream>

namespace pg
{
    class Archive
    {
    public:
        void indent()
        {
            //TODO optimize this

            for(unsigned int i = 0; i < indentLevel; i++)
                container << "\t";
        }

        void startSerialization(const std::string& className)
        {
            indent();
            container << className << " {\n";
            
            indentLevel++;
        }

        void endSerialization()
        {
            indentLevel--;
            indent();

            container << "}\n";
        }

        template <typename Type>
        Archive& operator<<(const Type& rhs)
        {
            container << rhs;
            return *this;
        }

        std::stringstream container;

        unsigned int indentLevel = 0;
    };

    template<typename Type>
    void serialize(Archive& archive, const Type& value);

    template<typename Type>
    void serialize(Archive& archive, const std::string& name, const Type& value)
    {
        //TODO make this automatically after a new line;
        archive.indent();
        
        archive << name << ": ";

        serialize(archive, value);
    }

    class Serializer
    {
        class ClassSerializer
        {
        friend class Serializer;
            //ClassSerializer(const std::string& className) { archive << className << " {\n"; }
            //~ClassSerializer() { archive << "}\n"; auto& serializer = Serializer::getSerializer(); serializer->registerToFile(archive.container, mutex); }
            ~ClassSerializer() { auto& serializer = Serializer::getSerializer(); serializer->registerToFile(archive.container, mutex); }
        
        public:
            Archive archive;

        private:
            std::recursive_mutex mutex;
        };

    public:
        static std::unique_ptr<Serializer>& getSerializer()
        {static std::unique_ptr<Serializer> serializer = std::unique_ptr<Serializer>(new Serializer); return serializer; }

        template <typename Type>
        void serializeObject(const Type& type) { ClassSerializer ar; serialize(ar.archive, type); }

    private:
        void registerToFile(const std::stringstream& serializedString, std::recursive_mutex& mutex)
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);

            std::cout << serializedString.str() << std::endl;
        }

        static std::string filename;
    };
}

#endif // SERIALIZATION_H
