#include "configuration.h"

#include "logger.h"

namespace pg
{
    namespace
    {
        const char * DOM = "Configuration";
    }

    explicit Configuration::ElementType::operator float() const
    {
        if(this->type == UnionType::FLOAT)
            return data.f;
        else
        {
            LOG_ERROR(DOM, "Error in casting an element to float when defined as " + enumTypeToString(this->type));
            return 0.0f;
        }
    }

    explicit Configuration::ElementType::operator int() const
    {
        if(this->type == UnionType::INT)
            return data.i;
        else
        {
            LOG_ERROR(DOM, "Error in casting an element to int when defined as " + enumTypeToString(this->type));
            return 0;
        }
    }

    explicit Configuration::ElementType::operator std::string() const
    {
        if(this->type == UnionType::STRING)
            return data.s;
        else
        {
            LOG_ERROR(DOM, "Error in casting an element to string when defined as " + enumTypeToString(this->type));
            return "";
        }
    }

    explicit Configuration::ElementType::operator bool() const
    {
        if(this->type == UnionType::BOOL)
            return data.b;
        else
        {
            LOG_ERROR(DOM, "Error in casting an element to bool when defined as " + enumTypeToString(this->type));
            return false;
        }
    }

    std::string enumTypeToString(const Configuration::ElementType::UnionType& type)
    {
        switch(type)
        {
            case Configuration::ElementType::UnionType::FLOAT: return "float"; break;
            case Configuration::ElementType::UnionType::INT: return "int"; break;
            case Configuration::ElementType::UnionType::STRING: return "string"; break;
            case Configuration::ElementType::UnionType::BOOL: return "bool"; break;
        }
    }
}