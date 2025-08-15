#pragma once

#include <vector>
#include <string>

// TODO add all helper function for file in here
// TODO Manage binary files
namespace pg
{
    struct DropFileEvent
    {
        std::string filepath;
    };

    struct TextFile
    {
        TextFile(const std::string& filepath, const std::string& data) : filepath(filepath), data(data) {}
        TextFile() {}

        TextFile(const TextFile& other) : filepath(other.filepath), data(other.data) {}
        TextFile(TextFile&& other) { filepath = std::move(other.filepath); data = std::move(other.data); }
        TextFile& operator=(const TextFile& other) { filepath = other.filepath; data = other.data; return *this; }
        TextFile& operator=(TextFile&& other) { filepath = std::move(other.filepath); data = std::move(other.data); return *this; }
        ~TextFile() {}

        std::string filepath;
        std::string data;
    };

    class ResourceAccessor
    {
    public:
        static TextFile openTextFile(const std::string& filepath) noexcept;
        static std::vector<TextFile> openTextFolder(const std::string& foldername) noexcept;
    };

    class FileAccessor
    {
    public:
        static TextFile openTextFile(const std::string& filepath) noexcept;
        static std::vector<TextFile> openTextFolder(const std::string& foldername, bool recursive = false) noexcept;

        static bool writeToFile(const TextFile& file, const std::string& data, bool truncate = false) noexcept;
    };

    class UniversalFileAccessor
    {
    public:
        static TextFile openTextFile(const std::string& filepath) noexcept;
        static std::vector<TextFile> openTextFolder(const std::string& foldername) noexcept;

        static bool writeToFile(const TextFile& file, const std::string& data, bool truncate = false) noexcept;

        static std::string getFileName(const TextFile& file) noexcept;
        static std::string getFoldername(const TextFile& file) noexcept;
        static std::string getRelativePath(const TextFile& file) noexcept;
    };
}