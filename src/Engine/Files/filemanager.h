#pragma once

#include <vector>
#include <string>


// TODO add all helper function for file in here
// TODO Manage binary files
namespace pg
{   
    struct TextFile
    {
        std::string filename;
        std::string data;
    };

    class ResourceManager
    {
    public:
        static TextFile openTextFile(const std::string& filename);
        static std::vector<TextFile> openTextFolder(const std::string& foldername);
    };

    class FileManager
    {
    public:
        static TextFile openTextFile(const std::string& filename);
        static std::vector<TextFile> openTextFolder(const std::string& foldername);
    };
}