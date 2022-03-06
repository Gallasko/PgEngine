#include "configuration.h"

#include "logger.h"

namespace pg
{
    namespace
    {
        const char * DOM = "Configuration";
    }

    template<>
    void serialize(Archive& archive, const Configuration::ElementType& element)
    {
        archive.startSerialization("Element Type");
        
        switch(element.type)
        {
        case Configuration::ElementType::UnionType::FLOAT:
            serialize(archive, "type", element.enumTypeToString(element.type));
            serialize(archive, "data", element.data.f);
            break;

        case Configuration::ElementType::UnionType::INT:
            serialize(archive, "type", element.enumTypeToString(element.type));
            serialize(archive, "data", element.data.i);
            break;

        case Configuration::ElementType::UnionType::STRING:
            serialize(archive, "type", element.enumTypeToString(element.type));
            serialize(archive, "data", element.data.s);
            break;

        case Configuration::ElementType::UnionType::BOOL:
            serialize(archive, "type", element.enumTypeToString(element.type));
            serialize(archive, "data", element.data.b);
            break; 
        }

        archive.endSerialization();
    }

    template<>
    void serialize(Archive& archive, const Configuration& config)
    {
        archive.startSerialization("Configuration");

        for(const auto& element : config.elementMap)
        {
            serialize(archive, element.second);
        }

        archive.endSerialization();
    }

    template<>
    Configuration deserialize(const UnserializedObject& serializedString)
    {
        return Configuration();
    }

    std::string Configuration::ElementType::enumTypeToString(const Configuration::ElementType::UnionType& type) const
    {
        switch(type)
        {
            case UnionType::FLOAT: return "float"; break;
            case UnionType::INT: return "int"; break;
            case UnionType::STRING: return "string"; break;
            case UnionType::BOOL: return "bool"; break;
        }
    }

    Configuration::ElementType::operator float() const
    {
        if(this->type == UnionType::FLOAT)
            return data.f;
        else
        {
            LOG_ERROR(DOM, "Error in casting an element to float when defined as " + enumTypeToString(this->type));
            return 0.0f;
        }
    }

    Configuration::ElementType::operator int() const
    {
        if(this->type == UnionType::INT)
            return data.i;
        else
        {
            LOG_ERROR(DOM, "Error in casting an element to int when defined as " + enumTypeToString(this->type));
            return 0;
        }
    }

    Configuration::ElementType::operator std::string() const
    {
        if(this->type == UnionType::STRING)
            return data.s;
        else
        {
            LOG_ERROR(DOM, "Error in casting an element to string when defined as " + enumTypeToString(this->type));
            return "";
        }
    }

    Configuration::ElementType::operator bool() const
    {
        if(this->type == UnionType::BOOL)
            return data.b;
        else
        {
            LOG_ERROR(DOM, "Error in casting an element to bool when defined as " + enumTypeToString(this->type));
            return false;
        }
    }

}