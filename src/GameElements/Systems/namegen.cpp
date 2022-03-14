#include "namegen.h"

#include <fstream>

#include <dirent.h>

#include "../../logger.h"

namespace
{
    const char * DOM = "Name Generation";
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

        if()
    }

}
