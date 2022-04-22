#pragma once

#include <vector>
#include <string>

namespace pg
{
    struct TextFile
    {
        std::string filename;
    };

    struct BinaryFile
    {
        std::string filename;
    };

    class RessourceManager
    {
    public:
        static TextFile openTextFile(const std::string& filename);
        static BinaryFile openBinaryFile(const std::string& filename);
        
        static std::vector<TextFile> openTextFolder(const std::string& foldername);
        static std::vector<BinaryFile> openBinaryFolder(const std::string& foldername);
    };

    class FileManager
    {
    public:
        static TextFile openTextFile(const std::string& filename);
        static BinaryFile openBinaryFile(const std::string& filename);
        
        static std::vector<TextFile> openTextFolder(const std::string& foldername);
        static std::vector<BinaryFile> openBinaryFolder(const std::string& foldername);
    };
}