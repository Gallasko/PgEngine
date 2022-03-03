#pragma once

#include <typeinfo>
#include <string>

namespace pg
{
    class Configuration
    {
        struct ElementType
        {
        private:
            union 
            {
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
        
        public:
            UnionType type;

            template <typename Type>
            ElementType(const Type& value) : setValue(value) {} 

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
            Type getValue() const
            {
                if(typeid(Type) == typeid(float) and this->type == UnionType::FLOAT)
                    return data.f;
                else if(typeid(Type) == typeid(int) and this->type == UnionType::INT)
                    return data.i;
                else if(typeid(Type) == typeid(std::string) and this->type == UnionType::STRING)
                    return data.s;
                else if(typeid(Type) == typeid(bool) and this->type == UnionType::BOOL)
                    return data.b;
                else
                {
                    // TODO LOG_ERROR
                    return Type();
                }
            }

        private:
            void clearPreviousType()
            {
                if(this->type == UnionType::STRING)
                    data.s.~basic_string();

                // TODO add Big Int clear too here
            }
        };

    };
}