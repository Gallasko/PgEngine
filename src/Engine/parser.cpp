#include "parser.h"

#include <algorithm>

#include "../logger.h"

namespace pg
{
    namespace
    {
        const char * DOM = "Parser";
    }

    std::string FileParser::getNextLine()
    {
        LOG_THIS_MEMBER(DOM);

        if(!file.is_open())
        {
            LOG_ERROR(DOM, "File is not open");

            return "";
        }
        
        if(std::getline(file, nextLine))
        {
            return nextLine;
        }
        else
        {
            LOG_ERROR(DOM, "Requested a new line but none is available");
            
            return "";
        }
    }

    void FileParser::run()
    {
        LOG_THIS_MEMBER(DOM);

        file.open(filename, std::ifstream::in);
        
        for(std::string line; std::getline(file, line);)
            std::for_each(callbacks.begin(), callbacks.end(), [&line](const FileParser::ParsingCallback& element) { executeCallback(line, element); });

        file.close();
    }

    void executeCallback(const std::string& line, const FileParser::ParsingCallback& callback)
    {
        LOG_THIS(DOM);

        if(std::regex_search(line, callback.pattern))
            callback.callback(line);
    }

}

