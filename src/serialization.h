#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <string>
#include <memory>
#include <mutex>
#include <sstream>

namespace pg
{
    class Archive
    {
        struct EndOfLine { size_t* indentLevel = nullptr; };

    public:
        Archive() { endOfLine.indentLevel = &indentLevel; }
        
        const EndOfLine& endl() const { return endOfLine; }

        void startSerialization(const std::string& className);

        void endSerialization();

        Archive& operator<<(const EndOfLine&)
        {
            container << std::endl;
            requestNewline = true;
            return *this;
        }

        template <typename Type>
        Archive& operator<<(const Type& rhs)
        {
            if(requestNewline)
            {
                container << std::string(*endOfLine.indentLevel, '\t');
                requestNewline = false;
            }

            container << rhs;
            return *this;
        }

        //friend std::ostream& operator<<(std::ostream& stream, const Archive::EndOfLine& endOfLine);

        std::stringstream container;

        bool requestNewline = false;
        size_t indentLevel = 0;
        EndOfLine endOfLine;
    };

    template<typename Type>
    void serialize(Archive& archive, const Type& value);

    template<typename Type>
    void serialize(Archive& archive, const std::string& name, const Type& value)
    {
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
        Serializer(const std::string& filename) : filename(filename) { }

        static std::unique_ptr<Serializer>& getSerializer(const std::string& filename = "serialize.sz")
        {static std::unique_ptr<Serializer> serializer = std::unique_ptr<Serializer>(new Serializer(filename)); return serializer; }

        template <typename Type>
        void serializeObject(const Type& type) { ClassSerializer ar; serialize(ar.archive, type); }

    private:
        void registerToFile(const std::stringstream& serializedString, std::recursive_mutex& mutex);

        std::string filename;
    };
}

#endif // SERIALIZATION_H
