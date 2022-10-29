#include "elementtype.h"

#include "logger.h"

namespace pg
{
    namespace
    {
        const char * DOM = "Expression";
    }

    Strfy& operator<<(Strfy& os, const ElementType& value)
    {
        return os << value.toString();
    }

    Strfy& operator<<(Strfy& os, const ElementType::UnionType& value)
    {
        switch(value)
        {
        case ElementType::UnionType::FLOAT:
            return os << "float";
            break;

        case ElementType::UnionType::INT:
            return os << "int";
            break;

        case ElementType::UnionType::STRING:
            os << "string";
            break;

        case ElementType::UnionType::BOOL:
            os << "boolean";
            break; 
        }
    }

    template<>
    void serialize(Archive& archive, const ElementType& element)
    {
        LOG_THIS(DOM);

        archive.startSerialization("Element Type");
        
        switch(element.type)
        {
        case ElementType::UnionType::FLOAT:
            serialize(archive, "type", element.enumTypeToString(element.type));
            serialize(archive, "data", element.data.f);
            break;

        case ElementType::UnionType::INT:
            serialize(archive, "type", element.enumTypeToString(element.type));
            serialize(archive, "data", element.data.i);
            break;

        case ElementType::UnionType::STRING:
            serialize(archive, "type", element.enumTypeToString(element.type));
            serialize(archive, "data", element.data.s);
            break;

        case ElementType::UnionType::BOOL:
            serialize(archive, "type", element.enumTypeToString(element.type));
            serialize(archive, "data", element.data.b);
            break; 
        }

        archive.endSerialization();
    }

    template<>
    ElementType deserialize(const UnserializedObject& serializedString)
    {
        LOG_THIS(DOM);

        ElementType value;

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

    bool ElementType::isTrue() const
    {
        switch(type)
        {
            case UnionType::FLOAT: return get<float>() != 0.0f; break;
            case UnionType::INT: return get<int>() != 0; break;
            case UnionType::STRING: return get<std::string>() != ""; break;
            case UnionType::BOOL: return get<bool>(); break;

            default:
                LOG_ERROR(DOM, "Error in casting type to a boolean");
                return false;
                break;
        }
    }

    ElementType ElementType::operator+(const ElementType& other) const
    {
        if(type == UnionType::FLOAT && other.type == UnionType::FLOAT)
        {
            return ElementType { get<float>() + other.get<float>() };
        }
        else if(type == UnionType::FLOAT && other.type == UnionType::INT)
        {
            return ElementType { get<float>() + other.get<int>() };
        }
        else if(type == UnionType::INT && other.type == UnionType::INT)
        {
            return ElementType { get<int>() + other.get<int>() };
        }
        else if(type == UnionType::INT && other.type == UnionType::FLOAT)
        {
            return ElementType { get<int>() + other.get<float>() };
        }
        else if(type == UnionType::STRING && other.type == UnionType::STRING)
        {
            return ElementType { get<std::string>() + other.get<std::string>() };
        }
        else
        {
            throw std::runtime_error(Strfy() << "Operator + between two incompatible operand: " << type << " and " << other.type);
        }
    }

    ElementType ElementType::operator-(const ElementType& other) const
    {
        if(type == UnionType::FLOAT && other.type == UnionType::FLOAT)
        {
            return ElementType { get<float>() - other.get<float>() };
        }
        else if(type == UnionType::FLOAT && other.type == UnionType::INT)
        {
            return ElementType { get<float>() - other.get<int>() };
        }
        else if(type == UnionType::INT && other.type == UnionType::INT)
        {
            return ElementType { get<int>() - other.get<int>() };
        }
        else if(type == UnionType::INT && other.type == UnionType::FLOAT)
        {
            return ElementType { get<int>() - other.get<float>() };
        }
        else
        {
            throw std::runtime_error(Strfy() << "Operator - between two incompatible operand: " << type << " and " << other.type);
        }
    }

    ElementType ElementType::operator*(const ElementType& other) const
    {
        if(type == UnionType::FLOAT && other.type == UnionType::FLOAT)
        {
            return ElementType { get<float>() * other.get<float>() };
        }
        else if(type == UnionType::FLOAT && other.type == UnionType::INT)
        {
            return ElementType { get<float>() * other.get<int>() };
        }
        else if(type == UnionType::INT && other.type == UnionType::INT)
        {
            return ElementType { get<int>() * other.get<int>() };
        }
        else if(type == UnionType::INT && other.type == UnionType::FLOAT)
        {
            return ElementType { get<int>() * other.get<float>() };
        }
        else
        {
            throw std::runtime_error(Strfy() << "Operator * between two incompatible operand: " << type << " and " << other.type);
        }
    }

    ElementType ElementType::operator/(const ElementType& other) const
    {
        if(type == UnionType::FLOAT && other.type == UnionType::FLOAT)
        {
            if(other.get<float>() == 0.0f)
            {
                throw std::runtime_error("Division by zero");
            }

            return ElementType { get<float>() / other.get<float>() };
        }
        else if(type == UnionType::FLOAT && other.type == UnionType::INT)
        {
            if(other.get<int>() == 0.0f)
            {
                throw std::runtime_error("Division by zero");
            }

            return ElementType { get<float>() / other.get<int>() };
        }
        else if(type == UnionType::INT && other.type == UnionType::INT)
        {
            if(other.get<int>() == 0.0f)
            {
                throw std::runtime_error("Division by zero");
            }

            return ElementType { static_cast<float>(get<int>()) / other.get<int>() };
        }
        else if(type == UnionType::INT && other.type == UnionType::FLOAT)
        {
            if(other.get<float>() == 0.0f)
            {
                throw std::runtime_error("Division by zero");
            }

            return ElementType { get<int>() / other.get<float>() };
        }
        else
        {
            throw std::runtime_error(Strfy() << "Operator / between two incompatible operand: " << type << " and " << other.type);
        }
    }

    ElementType ElementType::operator%(const ElementType& other) const
    {
        if(type == UnionType::INT && other.type == UnionType::INT)
        {
            return ElementType { get<int>() % other.get<int>() };
        }
        else
        {
            throw std::runtime_error(Strfy() << "Operator % between two incompatible operand: " << type << " and " << other.type);
        }
    }

    ElementType ElementType::operator> (const ElementType& other) const
    {
        if(type == UnionType::FLOAT && other.type == UnionType::FLOAT)
        {
            return ElementType { get<float>() > other.get<float>() };
        }
        else if(type == UnionType::FLOAT && other.type == UnionType::INT)
        {
            return ElementType { get<float>() > other.get<int>() };
        }
        else if(type == UnionType::INT && other.type == UnionType::INT)
        {
            return ElementType { get<int>() > other.get<int>() };
        }
        else if(type == UnionType::INT && other.type == UnionType::FLOAT)
        {
            return ElementType { get<int>() > other.get<float>() };
        }
        else
        {
            throw std::runtime_error(Strfy() << "Operator > between two incompatible operand: " << type << " and " << other.type);
        }
    }

    ElementType ElementType::operator>=(const ElementType& other) const
    {
        try
        {
            return ElementType { not (other > *this) };
        }
        catch (const std::runtime_error& e)
        {
            throw std::runtime_error(Strfy() << "Operator >= between two incompatible operand: " << type << " and " << other.type);
        }
    }

    ElementType ElementType::operator< (const ElementType& other) const
    {
        try
        {
            return ElementType { (other > *this) };
        }
        catch (const std::runtime_error& e)
        {
            throw std::runtime_error(Strfy() << "Operator < between two incompatible operand: " << type << " and " << other.type);
        }
    }

    ElementType ElementType::operator<=(const ElementType& other) const
    {
        try
        {
            return ElementType { not (*this > other) };
        }
        catch (const std::runtime_error& e)
        {
            throw std::runtime_error(Strfy() << "Operator <= between two incompatible operand: " << type << " and " << other.type);
        }
    }

    ElementType ElementType::operator==(const ElementType& other) const
    {
        if(type == UnionType::FLOAT && other.type == UnionType::FLOAT)
        {
            return ElementType { get<float>() == other.get<float>() };
        }
        else if(type == UnionType::FLOAT && other.type == UnionType::INT)
        {
            return ElementType { get<float>() == other.get<int>() };
        }
        else if(type == UnionType::INT && other.type == UnionType::INT)
        {
            return ElementType { get<int>() == other.get<int>() };
        }
        else if(type == UnionType::INT && other.type == UnionType::FLOAT)
        {
            return ElementType { get<int>() == other.get<float>() };
        }
        else if(type == UnionType::STRING && other.type == UnionType::STRING)
        {
            return ElementType { get<std::string>() == other.get<std::string>() };
        }
        else if(type == UnionType::BOOL && other.type == UnionType::BOOL)
        {
            return ElementType { get<bool>() == other.get<bool>() };
        }
        else
        {
            throw std::runtime_error(Strfy() << "Operator == between two incompatible operand: " << type << " and " << other.type);
        }
    }

    ElementType ElementType::operator!=(const ElementType& other) const
    {
        try
        {
            return ElementType { not (*this == other) };
        }
        catch (const std::runtime_error& e)
        {
            throw std::runtime_error(Strfy() << "Operator != between two incompatible operand: " << type << " and " << other.type);
        }
    }

    ElementType ElementType::operator-() const
    {
        if(type == UnionType::INT)
        {
            return ElementType { -get<int>() };
        }
        else if (type == UnionType::FLOAT)
        {
            return ElementType { -get<float>() };
        }
        else
        {
            throw std::runtime_error(Strfy() << "Operator unary - on an incompatible operand: " << type);
        }
    }

    std::string ElementType::toString() const
    {
        switch(type)
        {
            case UnionType::STRING: return get<std::string>(); break;
            case UnionType::INT: return std::to_string(get<int>()); break;
            case UnionType::FLOAT: return std::to_string(get<float>()); break;
            case UnionType::BOOL: return get<bool>() ? "true" : "false"; break;

            default:
                LOG_ERROR(DOM, "Error in casting type to string");
                return "Invalid";
                break;
        }
    }

    std::string ElementType::enumTypeToString(const ElementType::UnionType& type) const
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

    ElementType::operator float() const
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

    ElementType::operator int() const
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

    ElementType::operator std::string() const
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

    ElementType::operator bool() const
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