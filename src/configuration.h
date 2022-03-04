
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
    private:
        struct ElementType
        {
        private:
            union U
            {
                /**
                 * @brief Construct a new union U object
                 * 
                 * Need to explicitly declare ctor and dtor cause they are non POD type in the union
                 * By default the union construct itself as an int with the value of 0;
                 */
                U() { i = 0; }
                ~U() {}

                float f;
                int i;
                std::string s;
                bool b;
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
            UnionType type;

            /**
             * @brief Construct a new Element Type object
             * 
             * By default it is a int set as 0
             */
            ElementType() { this->setValue(0); }

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

            template<typename Type>
            Type get() const
            {
                return static_cast<Type>(*this);
            }

        private:
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
        inline static const ConfigurationPtr& config() { static ConfigurationPtr config = ConfigurationPtr(new Configuration()); return config; }

        template<typename Type>
        Type get(const std::string& name, const Type& defaultValue) const;

        template<typename Type>
        void set(const std::string& name, const Type& value);

        template<typename Type>
        void set(const char* name, const Type& value);

    private:
        friend void serialize<>(Archive& archive, const Configuration& config);

        std::unordered_map<std::string, Configuration::ElementType> elementMap;
    };

    template<typename Type>
    void Configuration::set(const std::string& name, const Type& value)
    {
        elementMap[name] = ElementType(value);
    }

    template<typename Type>
    void Configuration::set(const char* name, const Type& value)
    {
        elementMap[std::string(name)] = ElementType(value);
    }
}