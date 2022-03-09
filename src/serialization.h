#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <string>
#include <memory>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace pg
{
    // TODO correctly ban ":" from class name "{" for attribute name and ATTRIBUTECONST from any litteral !
    // check and remove any \t not correctly placed
    // Indent always go up one by one so if we skip an increment their is a problem 
    // Test all those case !

    class Archive
    {
        struct EndOfLine { size_t* indentLevel = nullptr; };

    public:
        Archive() { endOfLine.indentLevel = &indentLevel; }
        
        const EndOfLine& endl() const { return endOfLine; }

        void startSerialization(const std::string& className);

        void endSerialization();

        void setAttribute(const std::string& value, const std::string& type = "");

        // Todo specialize this with std::endl() to disable it by making static assert !
        // Archive require the usage of EndOfLine otherwise it can break the whole serialization file !!!
        Archive& operator<<(const EndOfLine&)
        {
            requestComma = true;
            requestNewline = true;
            return *this;
        }

        Archive& operator<<(std::ostream & (*)(std::ostream &))
        {
            static_assert(true, "Can't use any std:: manipulator on this class !");

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

        std::stringstream container;

        bool requestNewline = false;
        bool requestComma = false;
        size_t indentLevel = 0;
        EndOfLine endOfLine;
    };

    template<typename Type>
    void serialize(Archive& archive, const Type& value);

    // Todo make a static_assert to check if ": " is present in the name and reject it at compile time
    template<typename Type>
    void serialize(Archive& archive, const std::string& name, const Type& value)
    {
        archive << name << ": ";

        serialize(archive, value);
    }

    template<typename Type>
    void serialize(Archive& archive, const char* value)
    {
        // Todo store the number of characters in the string
        serialize(archive, std::string(value));
    }

    class UnserializedObject
    {
        struct Attribute
        {
            std::string name = "";
            std::string value = "";
        };
    public:
        UnserializedObject() : isNullObject(true), isClass(false) { }
        UnserializedObject(const std::string& serializedString, const std::string& objectName = "", bool isClass = true) : objectName(objectName), serializedString(serializedString), isNullObject(false), isClass(isClass) { if(isClass) parseString(); }

        const std::string& getObjectName() const { return objectName; }

        /**
         * @brief Return the object as an Attribute object.
         * Can only be called if isClass is set to false.
         * 
         * @return The representation of the string in an attribute form
         */
        UnserializedObject::Attribute getAsAttribute() const;

        inline bool isNull() const { return isNullObject; }

        UnserializedObject operator[](const std::string& key);
        const UnserializedObject& operator[](const std::string& key) const;

        UnserializedObject operator[](unsigned int id);
        const UnserializedObject& operator[](unsigned int id) const;

    public:
        std::vector<UnserializedObject> children;

    private:
        void parseString();

        std::string objectName = "";
        std::string serializedString = "";
        
        bool isNullObject = false;
        bool isClass = true;
    };

    template<typename Type>
    Type deserialize(const UnserializedObject& name);

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
        Serializer(const std::string& filename);
        ~Serializer();

        static std::unique_ptr<Serializer>& getSerializer(const std::string& filename = "serialize.sz")
        {static std::unique_ptr<Serializer> serializer = std::unique_ptr<Serializer>(new Serializer(filename)); return serializer; }

        // Todo make a static_assert to check if ": " is present in the objectName and reject it at compile time
        template <typename Type>
        void serializeObject(const std::string& objectName, const Type& type) { ClassSerializer ar(objectName); serialize(ar.archive, type); }

        template <typename Type>
        Type deserializeObject(const std::string& objectName) const { const auto& it = serializedMap.find(objectName); if(it != serializedMap.end()) return deserialize<Type>(UnserializedObject(it->second, objectName)); else return deserialize<Type>(UnserializedObject()); }

    private:
        void registerSerialized(const std::string& objectName, const std::stringstream& serializedString);
        void registerToFile() const;

        std::string filename;
        std::mutex mutex;

        std::unordered_map<std::string, std::string> serializedMap;
    };
}

#endif // SERIALIZATION_H
