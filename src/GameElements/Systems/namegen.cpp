#include "namegen.h"

#include <fstream>

#include <dirent.h>

#include "../../logger.h"

namespace
{
    const char * DOM = "Name Generation";
}

std::string NameGenerator::getRandomName() const
{
    
}

void NameGenerator::listFiles(const std::string &path)
{
    LOG_THIS_MEMBER(DOM);

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
