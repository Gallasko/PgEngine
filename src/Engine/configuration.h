
#pragma once

#include <typeinfo>
#include <string>
#include <memory>
#include <unordered_map>

#include "serialization.h"

namespace pg
{
    class Configuration
    {
    public:
        /**
         * @struct ElementType
         * @brief Variant strut to hold the element type of a configuration element.
         * 
         * This structure can represent a configuration element which can be any of the following types:
         * float, int, string, bool.
         * 
         * This structure can be serialized.
         */
        struct ElementType
        {
        private:
            /**
             * @union U
             * 
             * @brief Hold the data of the element type.
             */
            union U
            {
                /**
                 * @brief Construct a new union U object.
                 * 
                 * Need to explicitly declare ctor and dtor cause they are non POD type in the union.
                 * By default the union construct itself as an int with the value of 0;
                 */
                U() { i = 0; }

                /**
                 * @brief Destroy the U object.
                 */
                ~U() {}

                float f;        ///< Float representation of the element type.
                int i;          ///< Int representation of the element type.
                std::string s;  ///< String representation of the element type.
                bool b;         ///< Bool representation of the element type.
                // Big Int bi;
            };

        public:
            enum class UnionType
            {
                FLOAT,
                INT,
                STRING,
                BOOL
            };
        
        public:
            /** The type of the internal representation of the union. */
            UnionType type = UnionType::INT;

            /**
             * @brief Construct a new Element Type object.
             * 
             * By default it is a int set as 0.
             */
            ElementType() { this->setValue(0); }

            /**
             * @brief Construct a new Element Type object.
             * 
             * @tparam Type The type of the value to be constructed.
             * @param value The value to be constructed.
             * 
             * This constructor is explicit to avoid any problems with const char* being implicitly converted to int !
             * Currently this class only support 4 types of data:
             *  -   float
             *  -   int
             *  -   string
             *  -   bool
             */
            template <typename Type>
            explicit ElementType(const Type& value) { this->setValue(value); }

            ElementType(const ElementType& other)
            {
                switch (other.type)
                {
                    case UnionType::FLOAT: this->setValue(other.data.f); break;
                    case UnionType::INT: this->setValue(other.data.i); break;
                    case UnionType::STRING: this->setValue(other.data.s); break;
                    case UnionType::BOOL: this->setValue(other.data.b); break;
                }
            }

            ~ElementType() { clearPreviousType(); }

            void operator=(const ElementType& other)
            {
                switch (other.type)
                {
                    case UnionType::FLOAT: this->setValue(other.data.f); break;
                    case UnionType::INT: this->setValue(other.data.i); break;
                    case UnionType::STRING: this->setValue(other.data.s); break;
                    case UnionType::BOOL: this->setValue(other.data.b); break;
                }
            }

            void setValue(float value)
            {
                clearPreviousType();
                
                data.f = value;
                this->type = UnionType::FLOAT;
            }

            void setValue(int value)
            {
                clearPreviousType();
                
                data.i = value;
                this->type = UnionType::INT;
            }

            void setValue(const char* value)
            {
                clearPreviousType();

                new(&data.s) std::string(value);
                this->type = UnionType::STRING;
            }

            void setValue(const std::string& value)
            {
                clearPreviousType();

                new(&data.s) std::string(value);
                this->type = UnionType::STRING;
            }

            void setValue(bool value)
            {
                clearPreviousType();
                
                data.b = value;
                this->type = UnionType::BOOL;
            }

            template <typename Type>
            Type get() const
            {
                return static_cast<Type>(*this);
            }

        private:
            friend void serialize<>(Archive& archive, const Configuration::ElementType& element);

            void clearPreviousType()
            {
                if(this->type == UnionType::STRING)
                    data.s.~basic_string();

                // TODO add Big Int clear too here
            }

            std::string enumTypeToString(const Configuration::ElementType::UnionType& type) const;

            explicit operator float() const;
            explicit operator int() const;
            explicit operator std::string() const;
            explicit operator bool() const;

            U data;
        };

        typedef std::unique_ptr<Configuration> ConfigurationPtr;

    public:
        //Configuration(const Configuration&& other) : elementMap(std::move(other.elementMap)) {}
        Configuration(const Configuration& other) : elementMap(other.elementMap) {}

        inline static const ConfigurationPtr& config() { static ConfigurationPtr config = ConfigurationPtr(new Configuration(true)); return config; }

        template <typename Type>
        Type get(const std::string& name, const Type& defaultValue) const;

        template <typename Type>
        void set(const std::string& name, const Type& value);

        template <typename Type>
        void set(const char* name, const Type& value);

    private:
        /**
         * @brief Construct a new Configuration object.
         * 
         * @param fromSerialization boolean to know if the configuration should be built from serialization or not.
         */
        Configuration(bool fromSerialization = false)
        {
            if(fromSerialization)
                *this = Serializer::getSerializer()->deserializeObject<Configuration>("Config");
        }

        friend void serialize<>(Archive& archive, const Configuration& config);
        friend Configuration deserialize<>(const UnserializedObject& serializedString);

        Configuration& operator=(const Configuration& config)
        {
            for(const auto& element : config.elementMap)
                this->elementMap[element.first] = element.second;

            return *this;
        }

        std::unordered_map<std::string, Configuration::ElementType> elementMap;
    };

    template <typename Type>
    Type Configuration::get(const std::string& name, const Type& defaultValue) const
    {
        const auto& it = elementMap.find(name);

        if(it == elementMap.end())
            return defaultValue;

        return it->second.get<Type>();
    }

    template <typename Type>
    void Configuration::set(const std::string& name, const Type& value)
    {
        elementMap[name] = ElementType(value);

        // Save the current configuration file inside the serializer
        Serializer::getSerializer()->serializeObject("Config", *this);
    }

    template <typename Type>
    void Configuration::set(const char* name, const Type& value)
    {
        elementMap[std::string(name)] = ElementType(value);

        // Save the current configuration file inside the serializer
        Serializer::getSerializer()->serializeObject("Config", *this);
    }
}