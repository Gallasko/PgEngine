#pragma once

#include <regex>
#include <functional>
#include <vector>
#include <sstream>

#include "filemanager.h"

namespace pg
{
    class FileParser
    {
        struct ParsingCallback
        {
            ParsingCallback(const std::regex& regex, const std::function<void(const std::string&)>& callback) : pattern(regex), callback(callback) {}
            ParsingCallback(const std::string& regex, const std::function<void(const std::string&)>& callback) : pattern(regex), callback(callback) {}
            
            const std::regex pattern;
            const std::function<void(const std::string&)> callback;
        };

    public:
        FileParser(const TextFile& file) : file(file), stream(file.data, std::ios::binary) {}
        ~FileParser() {}
        
        void addCallback(const std::regex& regex, const std::function<void(const std::string&)>& callback) { callbacks.emplace_back(regex, callback); }
        void addCallback(const std::string& pattern, const std::function<void(const std::string&)>& callback) { callbacks.emplace_back(pattern, callback); }
        
        std::string getNextLine();

        void run();

    private:
        friend void executeCallback(const std::string& line, const FileParser::ParsingCallback& callback);

        const TextFile file;
        std::istringstream stream;
        std::string nextLine = "";

        std::vector<ParsingCallback> callbacks;
    };
}
