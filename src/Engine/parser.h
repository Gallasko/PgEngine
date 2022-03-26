#pragma once

#include <regex>
#include <functional>
#include <vector>
#include <fstream>

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
        FileParser(const std::string& filename) : filename(filename) {}
        ~FileParser() { if(file.is_open()) file.close(); }
        
        void addCallback(const std::regex& regex, const std::function<void(const std::string&)>& callback) { callbacks.emplace_back(regex, callback); }
        void addCallback(const std::string& pattern, const std::function<void(const std::string&)>& callback) { callbacks.emplace_back(pattern, callback); }
        
        std::string getNextLine();

        void run();

    private:
        friend void executeCallback(const std::string& line, const FileParser::ParsingCallback& callback);

        const std::string filename;
        std::ifstream file;
        std::string nextLine = "";

        std::vector<ParsingCallback> callbacks;
    };
}
