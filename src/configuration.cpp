#include "configuration.h"

#include "logger.h"

namespace pg
{
    namespace
    {
        const char * DOM = "Configuration";
    }

    template<>
    void serialize(Archive& archive, const Configuration& config)
    {
        archive.startSerialization("Configuration");

        for(const auto& element : config.elementMap)
        {
            switch(element.second.type)
            {
            case Configuration::ElementType::UnionType::FLOAT:
                serialize(archive, element.first, element.second.get<float>());
                break;

            case Configuration::ElementType::UnionType::INT:
                serialize(archive, element.first, element.second.get<int>());
                break;

            case Configuration::ElementType::UnionType::STRING:
                serialize(archive, element.first, element.second.get<std::string>());
                break;

            case Configuration::ElementType::UnionType::BOOL:
                serialize(archive, element.first, element.second.get<bool>());
                break; 
            }
        }

        archive.endSerialization();
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
            LOG_ERROR(DOM, ("Error in casting an element to float when defined as " + enumTypeToString(this->type)).c_str());
            return 0.0f;
        }
    }

    Configuration::ElementType::operator int() const
    {
        if(this->type == UnionType::INT)
            return data.i;
        else
        {
            LOG_ERROR(DOM, ("Error in casting an element to int when defined as " + enumTypeToString(this->type)).c_str());
            return 0;
        }
    }

    Configuration::ElementType::operator std::string() const
    {
        if(this->type == UnionType::STRING)
            return data.s;
        else
        {
            LOG_ERROR(DOM, ("Error in casting an element to string when defined as " + enumTypeToString(this->type)).c_str());
            return "";
        }
    }

    Configuration::ElementType::operator bool() const
    {
        if(this->type == UnionType::BOOL)
            return data.b;
        else
        {
            LOG_ERROR(DOM, ("Error in casting an element to bool when defined as " + enumTypeToString(this->type)).c_str());
            return false;
        }
    }

    template<typename Type>
    Type Configuration::get(const std::string& name, const Type& defaultValue) const
    {
        const auto& it = elementMap.find(name);

        if(it == elementMap.end())
            return defaultValue;

        return it->second.get<Type>();
    }

    template<typename Type>
    void Configuration::set(const std::string& name, const Type& value)
    {
        elementMap[name] = value;
    }
}