#pragma once

#include <string>
#include <vector>

class NameGenerator
{
    struct Name
    {
        const std::string country;
        const std::string name;

        Name(const std::string& country, const std::string& name) : country(country), name(name) {}
        Name(const Name& other) : country(other.country), name(other.name) {}
    };

public:

    std::string getRandomName() const;
//private:
    void listFiles(const std::string& path);
    void parseFile(const std::string& path);
private:
    std::vector<Name> maleList;
    std::vector<Name> femaleList;
    std::vector<Name> surnameList;
};