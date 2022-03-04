#pragma once

#include <typeinfo>
#include <string>
#include <memory>

namespace pg
{
    class Configuration
    {
    public:
        //TODO: Make this struct private -> rn public for testing purposes
        struct ElementType
        {
        private:
            union U
            {
                // Need to explicitly declare ctor and dtor cause they are non POD type in the union
                U() {}
                ~U() {}

                float f;
                int i;
                std::string s;
                bool b;
                // Big Int bi;
            } data;

            enum class UnionType
            {
                FLOAT,
                INT,
                STRING,
                BOOL
            };

            template<typename Type>
            struct ReturnUnion
            {
                Type value;

                ReturnUnion(const Type& value) : value(value) {}

                explicit operator Type() const
                {
                    return value;
                }
            };
        
        public:
            UnionType type;

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

            explicit operator float() const;

            explicit operator int() const;

            explicit operator std::string() const;

            explicit operator bool() const;

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

            friend std::string enumTypeToString(const Configuration::ElementType::UnionType& type);
        };

    };
}