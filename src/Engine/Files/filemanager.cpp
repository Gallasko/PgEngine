#include "filemanager.h"

#include <QFile>
#include <QDir>
#include <QString>
#include <QTextStream>

#include <stdexcept>

#include "../logger.h"

namespace pg
{
    namespace
    {
        const char * DOM = "File Manager";

        TextFile openTxtFile(const std::string& filename) noexcept
        {
            LOG_THIS(DOM);

            try
            {
                QFile file(filename.c_str());

                if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    LOG_INFO(DOM, "Can't open file '" + filename + "'.");
                    return TextFile{filename, ""};
                }

                auto text = file.readAll().toStdString();

                file.close();

                return TextFile{filename, text};
            }
            catch (const std::exception& e)
            {
                LOG_INFO(DOM, e.what());

                return TextFile{filename, ""};
            }
        }
    }

    TextFile ResourceAccessor::openTextFile(const std::string& filename) noexcept
    {
        LOG_THIS(DOM);

        return openTxtFile(":/" + filename);
    }

    std::vector<TextFile> ResourceAccessor::openTextFolder(const std::string& foldername) noexcept
    {
        LOG_THIS(DOM);

        std::vector<TextFile> folder;

        foreach(const QString& fileName, QDir((":/" + foldername).c_str()).entryList())
        {
            folder.push_back(openTxtFile(":/" + foldername + fileName.toStdString()));
        }

        return folder;
    }

    TextFile FileAccessor::openTextFile(const std::string& filename) noexcept
    {
        LOG_THIS(DOM);

        return openTxtFile(filename);
    }

    std::vector<TextFile> FileAccessor::openTextFolder(const std::string& foldername) noexcept
    {
        LOG_THIS(DOM);

        std::vector<TextFile> folder;

        foreach(const QString& fileName, QDir(foldername.c_str()).entryList() )
        {
            folder.push_back(openTxtFile(foldername + fileName.toStdString()));
        }

        return folder;
    }

    void FileAccessor::writeToFile(const TextFile& file, const std::string& data) noexcept
    {
        LOG_THIS(DOM);

        try
        {
            QFile f(file.filename.c_str());

            if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                LOG_ERROR(DOM, "Can't open file '" + file.filename + "'.");
                return;
            }

            f.write(data.c_str());

            f.close();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR(DOM, e.what());
        }
    }

    TextFile UniversalFileAccessor::openTextFile(const std::string& filename) noexcept
    {
        LOG_THIS(DOM);

        auto file = openTxtFile(":/" + filename);

        if(file.data == "")
            file = openTxtFile(filename);

        return file;
    }

    std::vector<TextFile> UniversalFileAccessor::openTextFolder(const std::string& foldername) noexcept
    {
        LOG_THIS(DOM);

        std::vector<TextFile> folder;

        foreach(const QString& fileName, QDir(foldername.c_str()).entryList() )
        {
            folder.push_back(openTxtFile(foldername + fileName.toStdString()));
        }

        return folder;
    }

}