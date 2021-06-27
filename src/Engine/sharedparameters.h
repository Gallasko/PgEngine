#pragma once

#include <string>
#include <unordered_map>
/*
class SharedParameters
{
public:
    class Data
    {
    public:
        void virtual value() = 0;
    };

    template<typename Type>
    class Variant : public Data
    {
    public:
        Variant(Type data) : data(data) {}

        inline Type value() { return data; } 

        inline Type& operator*() const { return value(); }

    private:
        Type data;
    };

    SharedParameters() {}

    template<typename Type>
    void registerParameter(std::string name, Type data) { Variant* var = new Variant<Type>(data); parametersList[name] = var; }

    Data* getParameter(std::string name) { if(parametersList.find(name) != parametersList.end()) return parametersList[name]; else return nullptr; }

private:
    std::unordered_map<std::string, SharedParameters::Data*> parametersList;
};
*/