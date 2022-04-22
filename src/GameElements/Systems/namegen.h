#pragma once

#include <string>
#include <vector>
#include <memory>

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
    static const std::unique_ptr<NameGenerator>& generator() { static auto nameGenerator = std::unique_ptr<NameGenerator>(new NameGenerator()); return nameGenerator; }

    std::string getRandomName(const Gender& gender = Gender::MALE) const;

private:
    NameGenerator();

    void parseFiles(const std::string& path);

    std::vector<Name> maleList;
    std::vector<Name> femaleList;
    std::vector<Name> surnameList;
};