#include "fileparser.h"

#include <algorithm>

#include "../logger.h"

namespace pg
{
    namespace
    {
        static constexpr char const * DOM = "Parser";
    }

    std::string FileParser::getNextLine()
    {
        LOG_THIS_MEMBER(DOM);

        if(std::getline(stream, nextLine))
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
        
        for(std::string line; std::getline(stream, line);)
            std::for_each(callbacks.begin(), callbacks.end(), [&line](const FileParser::ParsingCallback& element) { executeCallback(line, element); });
    }

    void executeCallback(const std::string& line, const FileParser::ParsingCallback& callback)
    {
        LOG_THIS(DOM);

        if(std::regex_search(line, callback.pattern))
            callback.callback(line);
    }

}

