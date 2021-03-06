#ifndef SERIALIZATION_H
#define SERIALIZATION_H

/**
 * @file serialization.h
 * @author Pigeon Codeur
 * @brief Definition of the serialization class
 * @version 0.1
 * @date 2022-04-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <string>
#include <memory>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "Files/filemanager.h"

namespace pg
{
    // TODO correctly ban ":" from class name "{" for attribute name and ATTRIBUTECONST from any litteral !
    // check and remove any \t not correctly placed
    // Indent always go up one by one so if we skip an increment their is a problem 
    // Test all those case !

    /**
     * @brief Archive class responsible for creating the serialized string to be written to file
     */
    class Archive
    {
        // Helper struct

        /**
         * @brief Helper struct for end of line in the serialized string.
         * 
         * This struct is used to put an end of line in the serialized string.
         * A specialized template catches this struct and can then put an end of line
         * and the matching indent thanks to it.
         */
        struct EndOfLine
        {
            size_t* indentLevel = nullptr; //< References to the current indentation level of the related Archive
        };

        // Public methods
    public:
        /**
         * @brief Construct a new Archive object
         * 
         * The archive object is an advanced container for a serialized string
         * It manages all the operation related to the creation of the serialized string from user defined data
         * The underlying stringstream hold the serialized string
         * 
         */
        Archive() { endOfLine.indentLevel = &indentLevel; }
        
        /**
         * @brief Function used to put an end of line in the serialized string
         * 
         * @return const EndOfLine& A reference to the end of line object of the Archive
         */
        const EndOfLine& endl() const { return endOfLine; }

        /** Start the serialization process of a class */
        void startSerialization(const std::string& className);

        /** Start the serialization process of a class */
        void endSerialization();

        /** Put an Attribute in the serialization process*/
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

        /**
         * @brief Operator used to put data into the serialized string
         * 
         * @tparam Type The type of data to be serialized
         * 
         * @param rhs A value to be serialized
         * @return Archive& A reference to the current Archive object
         * 
         * This operator put the data passed as the first parameter into the underlying container
         * It is responsible for putting new lines and setting the appropriate indent level,
         * to make it valid data for the parser !
         */
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

        /** The underlying container of the Archive. */
        std::stringstream container;

        /** Flag indicating whether a new line should be inserted */
        bool requestNewline = false;
        /** Flag indicating whether a comma should be inserted */
        bool requestComma = false;
        /** The indent level of the current data */
        size_t indentLevel = 0;
        /** The custom end of line object */
        EndOfLine endOfLine;
    };

    // TODO make a specialized renderer for std::nullptr_t to catch nullptr error ?; 

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

        const UnserializedObject& operator[](const std::string& key);
        const UnserializedObject& operator[](const std::string& key) const;

        const UnserializedObject& operator[](unsigned int id);
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
        Serializer(const TextFile& file);
        ~Serializer();

        static std::unique_ptr<Serializer>& getSerializer(const std::string& filename = "serialize.sz")
        {static std::unique_ptr<Serializer> serializer = std::unique_ptr<Serializer>(new Serializer(filename)); return serializer; }

        // Todo make a static_assert to check if ": " is present in the objectName and reject it at compile time
        template <typename Type>
        void serializeObject(const std::string& objectName, const Type& type) { ClassSerializer ar(objectName); serialize(ar.archive, type); }

        template <typename Type>
        Type deserializeObject(const std::string& objectName) const
        { 
            const auto& it = serializedMap.find(objectName); 
            if(it != serializedMap.end()) 
                return deserialize<Type>(UnserializedObject(it->second, objectName)); 
            else 
                return deserialize<Type>(UnserializedObject());
        }

        const std::unordered_map<std::string, std::string>& getSerializedMap() const { return serializedMap; }

    private:
        Serializer(const std::string& filename);
        void readFile(const std::string& data);

        void registerSerialized(const std::string& objectName, const std::stringstream& serializedString);
        void registerToFile() const;

        TextFile file;
        std::mutex mutex;

        std::unordered_map<std::string, std::string> serializedMap;
    };
}

#endif // SERIALIZATION_H
