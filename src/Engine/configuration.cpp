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
        LOG_THIS(DOM);

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
        LOG_THIS(DOM);

        archive.startSerialization("Configuration");

        for(const auto& element : config.elementMap)
        {
            serialize(archive, element.first, element.second);
        }

        archive.endSerialization();
    }

    template<>
    Configuration::ElementType deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        Configuration::ElementType value;

        std::string type = "";

        if(serializedString.isNull())
            LOG_ERROR(DOM, "Element is null");
        else
        {
            LOG_INFO(DOM, "Deserializing an Element Type");

            type = deserialize<std::string>(serializedString["type"]);

            if(type == "float")
                value.setValue(deserialize<float>(serializedString["data"]));
            else if(type == "int")
                value.setValue(deserialize<int>(serializedString["data"]));
            else if(type == "string")
                value.setValue(deserialize<std::string>(serializedString["data"]));
            else if(type == "bool")
                value.setValue(deserialize<bool>(serializedString["data"]));
            else
            {
                LOG_ERROR(DOM, "Error in casting an string to UnionType");
                return value;
            }
        }

        return value;
    }

    template<>
    Configuration deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        Configuration value;

        for(auto& element : serializedString.children)
        {
            if(element.isNull())
                LOG_ERROR(DOM, "Element is null");
            else if(element.getObjectName() == "")
                LOG_ERROR(DOM, "Element has no name");
            else
            {
                LOG_INFO(DOM, "Deserializing " + element.getObjectName());

                auto child = deserialize<Configuration::ElementType>(element);
                value.elementMap[element.getObjectName()] = child;
            }
        }
        
        return value;
    }

    std::string Configuration::ElementType::enumTypeToString(const Configuration::ElementType::UnionType& type) const
    {
        LOG_THIS_MEMBER(DOM);

        switch(type)
        {
            case UnionType::FLOAT: return "float"; break;
            case UnionType::INT: return "int"; break;
            case UnionType::STRING: return "string"; break;
            case UnionType::BOOL: return "bool"; break;
            default:
                LOG_ERROR(DOM, "Error in casting type to a string");
                return "int";
                break;
        }
    }

    Configuration::ElementType::operator float() const
    {
        LOG_THIS_MEMBER(DOM);

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
        LOG_THIS_MEMBER(DOM);

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
        LOG_THIS_MEMBER(DOM);

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
        LOG_THIS_MEMBER(DOM);

        if(this->type == UnionType::BOOL)
            return data.b;
        else
        {
            LOG_ERROR(DOM, "Error in casting an element to bool when defined as " + enumTypeToString(this->type));
            return false;
        }
    }

}