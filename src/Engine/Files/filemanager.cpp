#include "filemanager.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <filesystem>
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
                // Todo put the path of the file in the textFile struct
                fs::path p {filename};

                if (!fs::exists(p))
                {
                    LOG_INFO(DOM, "Couldn't open file '" << filename << "' : File doesn't exist.");
                    return TextFile{filename, ""};
                }

                LOG_INFO(DOM, "Reading file '" << filename << "'");

                std::fstream file;

                file.open(filename, std::ios::in);

                bool firstLine = false;

                if (file.is_open())
                {
                    std::string temp;
                    std::string buffer;
                    
                    while(std::getline(file, buffer))
                    {
                        if(firstLine)
                            temp += "\n";
                        else
                            firstLine = true;

                        temp += buffer;
                    }

                    file.close();

                    return TextFile{p.relative_path().string(), temp};
                }
                return TextFile{p.relative_path().string(), ""};
            }
            catch (const std::exception& e)
            {
                LOG_INFO(DOM, "Couldn't open file '" << filename << "' : " << e.what());

                return TextFile{filename, ""};
            }
        }
    }

    TextFile ResourceAccessor::openTextFile(const std::string& filepath) noexcept
    {
        LOG_THIS(DOM);

        return openTxtFile(":/" + filepath);
    }

    std::vector<TextFile> ResourceAccessor::openTextFolder(const std::string& foldername) noexcept
    {
        LOG_THIS(DOM);

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
        LOG_THIS(DOM);

        return openTxtFile(filepath);
    }

    std::vector<TextFile> FileAccessor::openTextFolder(const std::string& foldername, bool recursive) noexcept
    {
        LOG_THIS(DOM);

        std::vector<TextFile> folder;

        // Todo

        fs::path p {foldername};

        if (!fs::exists(p))
        {
            LOG_INFO(DOM, "Couldn't open folder '" << foldername << "' : Folder doesn't exist.");
            return folder;
        }

        LOG_INFO(DOM, "Opening folder '" << foldername << "'");

        if (recursive)
        {
            for (const auto& file : fs::recursive_directory_iterator(foldername))
            {
                LOG_INFO(DOM, "Opening file '" << file.path().string() << "'");

                folder.push_back(openTextFile(file.path().string()));
            }
        }
        else
        {
            for (const auto& file : fs::directory_iterator(foldername))
            {
                LOG_INFO(DOM, "Opening file '" << file.path().string() << "'");

                folder.push_back(openTextFile(file.path().string()));
            }
        }

        return folder;
    }

    void FileAccessor::writeToFile(const TextFile& file, const std::string& data) noexcept
    {
        LOG_THIS(DOM);

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
        LOG_THIS(DOM);

        auto file = openTxtFile(filepath);

        if(file.data == "")
            file = openTxtFile(":/" + filepath);

        return file;
    }

    std::vector<TextFile> UniversalFileAccessor::openTextFolder(const std::string& foldername) noexcept
    {
        LOG_THIS(DOM);

        std::vector<TextFile> folder = FileAccessor::openTextFolder(foldername);

        if(folder.size() == 0)
            folder = ResourceAccessor::openTextFolder(foldername);

        return folder;
    }

    std::string UniversalFileAccessor::getFileName(const TextFile& file) noexcept
    {
        LOG_THIS(DOM);

        fs::path p {file.filepath};

        return p.filename().string();
    }

    std::string UniversalFileAccessor::getFoldername(const TextFile& file) noexcept
    {
        LOG_THIS(DOM);

        fs::path p {file.filepath};

        return p.relative_path().remove_filename().string();
    }

    std::string UniversalFileAccessor::getRelativePath(const TextFile& file) noexcept
    {
        LOG_THIS(DOM);

        return file.filepath;
    }
}