#include "namegen.h"

#include <fstream>

#include <dirent.h>

#include "../../logger.h"
#include "../../Engine/randomnumbergenerator.h"


namespace
{
    const char * DOM = "Name Generation";
}

using pg::RandomNumberGenerator;

std::string NameGenerator::getRandomName(const Gender& gender) const
{
    LOG_THIS_MEMBER(DOM);
    
    std::string name = "";

    switch(gender)
    {
        case Gender::FEMALE:
            name = femaleList.at(RandomNumberGenerator::generator()->generateNumber() % femaleList.size()).name;
            break;

        case Gender::MALE:
        default:
            name = maleList.at(RandomNumberGenerator::generator()->generateNumber() % maleList.size()).name;
            break;
    }

    name += " " + surnameList.at(RandomNumberGenerator::generator()->generateNumber() % surnameList.size()).name;

    return name;
}

NameGenerator::NameGenerator()
{
    //TODO make this lockuped from somewhere
    listFiles("res/names");
}

void NameGenerator::listFiles(const std::string &path)
{
    LOG_THIS_MEMBER(DOM);

    // TODO Need to create a class for opening files !
    if (auto dir = opendir(path.c_str()))
    {
        while (auto f = readdir(dir))
        {
            if (!f->d_name || f->d_name[0] == '.') continue;

            if(std::string(f->d_name).find(".names") != std::string::npos)
                parseFile(path + f->d_name);

            listFiles(path + "/" + f->d_name + "/");
        }

        closedir(dir);
    }
}

void NameGenerator::parseFile(const std::string &path)
{
    LOG_THIS_MEMBER(DOM);

    std::ifstream file;
        
    file.open(path);

    if(file.is_open())
    {
        std::string line;

        std::string country;

        if(!std::getline(file, country))
        {
            LOG_ERROR(DOM, "Error parsing: " + path + " file is empty");
            return;
        }

        if(!std::getline(file, line))
        {
            LOG_ERROR(DOM, "Error parsing: " + path + " file is missing the gender type");
            return;
        }

        std::vector<Name> *nameVector;

        if(line == "female")
            nameVector = &femaleList;
        else if(line == "male")
            nameVector = &maleList;
        else if(line == "any")
            nameVector = &surnameList;
        else
        {
            LOG_ERROR(DOM, "Error in parsing the gender of the file: " + path);
            return;
        }

        while(std::getline(file, line))
        {
            nameVector->emplace_back(country, line);
        }
    }

}
