#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <string>
#include <memory>
#include <mutex>
#include <sstream>
#include <unordered_map>

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
            requestComma = true;
            requestNewline = true;
            return *this;
        }

        template <typename Type>
        Archive& operator<<(const Type& rhs)
        {
            if(requestNewline)
            {
                if(requestComma)
                    container << ",";
                
                container << std::endl;
                container << std::string(*endOfLine.indentLevel, '\t');
                requestNewline = false;
            }

            container << rhs;
            return *this;
        }

        //friend std::ostream& operator<<(std::ostream& stream, const Archive::EndOfLine& endOfLine);

        std::stringstream container;

        bool requestNewline = false;
        bool requestComma = false;
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
            ClassSerializer(const std::string& objectName) : objectName(objectName) {}
            ~ClassSerializer() { archive.container << std::endl; auto& serializer = Serializer::getSerializer(); serializer->registerSerialized(objectName, archive.container); }
        
        public:
            Archive archive;

        private:
            std::string objectName;
        };

    public:
        Serializer(const std::string& filename) : filename(filename) { }
        ~Serializer();

        static std::unique_ptr<Serializer>& getSerializer(const std::string& filename = "serialize.sz")
        {static std::unique_ptr<Serializer> serializer = std::unique_ptr<Serializer>(new Serializer(filename)); return serializer; }

        template <typename Type>
        void serializeObject(const std::string& objectName, const Type& type) { ClassSerializer ar(objectName); serialize(ar.archive, type); }

    private:
        void registerSerialized(const std::string& objectName, const std::stringstream& serializedString);
        void registerToFile() const;

        std::string filename;
        std::mutex mutex;

        std::unordered_map<std::string, std::string> serializedMap;
    };
}

#endif // SERIALIZATION_H
