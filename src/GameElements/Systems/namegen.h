#pragma once

#include <string>

class NameGenerator
{
    struct Name
    {
        const std::string country;
        const std::string name;
    };

public:

    
private:
    void listFiles(const std::string& path);
    void parseFile(const std::string& path);

    std::vector<Name> maleList;
    std::vector<Name> femaleList;
    std::vector<Name> surnameList;
};