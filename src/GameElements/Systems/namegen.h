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
    enum class Gender
    {
        MALE,
        FEMALE
    };

public:
    std::unique_ptr<NameGenerator>& nameGenerator() const { static auto nameGenerator = std::unique_ptr<NameGenerator>(new NameGenerator()); return nameGenerator; }

    std::string getRandomName(const Gender& gender = Gender::MALE) const;

private:
    NameGenerator();

    void listFiles(const std::string& path);
    void parseFile(const std::string& path);

    std::vector<Name> maleList;
    std::vector<Name> femaleList;
    std::vector<Name> surnameList;
};