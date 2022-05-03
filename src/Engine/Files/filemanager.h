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

    class ResourceAccessor
    {
    public:
        static TextFile openTextFile(const std::string& filename) noexcept;
        static std::vector<TextFile> openTextFolder(const std::string& foldername) noexcept;
    };

    class FileAccessor
    {
    public:
        static TextFile openTextFile(const std::string& filename) noexcept;
        static std::vector<TextFile> openTextFolder(const std::string& foldername) noexcept;

        static void writeToFile(const TextFile& file, const std::string& data) noexcept;
    };
}