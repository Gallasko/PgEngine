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
        FileParser(const TextFile& file);
        ~FileParser() {}

        std::string getFileVersion() const { return version; }
        
        void addCallback(const std::regex& regex, const std::function<void(const std::string&)>& callback) { callbacks.emplace_back(regex, callback); }
        void addCallback(const std::string& pattern, const std::function<void(const std::string&)>& callback) { callbacks.emplace_back(pattern, callback); }
        
        std::string getCurrentLine() const { return currentLine; }

        std::string getNextLine() const { return nextLine; }

        /**
         * @brief Advance the parser to the next line
         * 
         * @return true if the parser could advance to the next line, false otherwise (End of the file reached)
         */
        bool advance();

        void run();

    private:
        friend void executeCallback(const std::string& line, const FileParser::ParsingCallback& callback);

        const TextFile file;
        std::istringstream stream;
        std::string currentLine;
        std::string nextLine;

        std::string version = "1.0.0";

        std::vector<ParsingCallback> callbacks;
    };
}
