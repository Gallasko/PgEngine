#include "filemanager.h"

#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

#include "../logger.h"

namespace pg
{
    namespace
    {
        static constexpr char const * DOM = "File Manager";

        TextFile openTxtFile(const std::string& filename) noexcept
        {
            LOG_THIS(DOM);

            try
            {
                fs::path p {filename};

                if (!fs::exists(p))
                {
                    LOG_INFO(DOM, Strfy() << "Couldn't open file '" << filename << "' : File doesn't exist.");
                    return TextFile{filename, ""};
                }

                std::ifstream ifs(filename.c_str(), std::ios::in);

                std::ifstream::pos_type fileSize = ifs.tellg();
                if (fileSize < 0)
                    return TextFile{filename, ""};

                ifs.seekg(0, std::ios::beg);

                std::vector<char> bytes(fileSize);
                ifs.read(&bytes[0], fileSize);

                return TextFile{filename, std::string(&bytes[0], fileSize)};
            }
            catch (const std::exception& e)
            {
                LOG_INFO(DOM, Strfy() << "Couldn't open file '" << filename << "' : " << e.what());

                return TextFile{filename, ""};
            }
        }
    }

    TextFile ResourceAccessor::openTextFile(const std::string& filepath) noexcept
    {
        LOG_THIS_MEMBER(DOM);

        return openTxtFile(":/" + filepath);
    }

    std::vector<TextFile> ResourceAccessor::openTextFolder(const std::string& foldername) noexcept
    {
        LOG_THIS_MEMBER(DOM);

        std::vector<TextFile> folder;

        // Todo

        // foreach(const QString& fileName, QDir((":/" + foldername).c_str()).entryList())
        // {
        //     folder.push_back(openTxtFile(":/" + foldername + fileName.toStdString()));
        // }

        return folder;
    }

    TextFile FileAccessor::openTextFile(const std::string& filepath) noexcept
    {
        LOG_THIS_MEMBER(DOM);

        return openTxtFile(filepath);
    }

    std::vector<TextFile> FileAccessor::openTextFolder(const std::string& foldername) noexcept
    {
        LOG_THIS_MEMBER(DOM);

        std::vector<TextFile> folder;

        // Todo

        // foreach(const QString& fileName, QDir(foldername.c_str()).entryList() )
        // {
        //     folder.push_back(openTxtFile(foldername + fileName.toStdString()));
        // }

        return folder;
    }

    void FileAccessor::writeToFile(const TextFile& file, const std::string& data) noexcept
    {
        LOG_THIS_MEMBER(DOM);

        // Todo

        // try
        // {
        //     QFile f(file.filename.c_str());

        //     if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        //     {
        //         LOG_ERROR(DOM, "Can't open file '" + file.filename + "'.");
        //         return;
        //     }

        //     f.write(data.c_str());

        //     f.close();
        // }
        // catch (const std::exception& e)
        // {
        //     LOG_ERROR(DOM, e.what());
        // }
    }

    TextFile UniversalFileAccessor::openTextFile(const std::string& filepath) noexcept
    {
        LOG_THIS_MEMBER(DOM);

        auto file = openTxtFile(filepath);

        if(file.data == "")
            file = openTxtFile(":/" + filepath);

        return file;
    }

    std::vector<TextFile> UniversalFileAccessor::openTextFolder(const std::string& foldername) noexcept
    {
        LOG_THIS_MEMBER(DOM);

        std::vector<TextFile> folder;

        // foreach(const QString& fileName, QDir(foldername.c_str()).entryList() )
        // {
        //     folder.push_back(openTxtFile(foldername + fileName.toStdString()));
        // }

        return folder;
    }

    std::string UniversalFileAccessor::getFileName(const TextFile& file) noexcept
    {
        // QFileInfo fileInfo(file.filepath.c_str());

        // return fileInfo.baseName().toStdString();

        return "";
    }

    std::string UniversalFileAccessor::getFoldername(const TextFile& file) noexcept
    {
        // QFileInfo fileInfo(file.filepath.c_str());

        // return fileInfo.dir().path().toStdString();
        return "";
    }
}