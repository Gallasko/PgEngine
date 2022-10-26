#include "filemanager.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QTextStream>

#include <stdexcept>

#include "../logger.h"

namespace pg
{
    namespace
    {
        const char * DOM = "File Manager";

        TextFile openTxtFile(const std::string& filepath) noexcept
        {
            LOG_THIS(DOM);

            try
            {
                QFile file(filepath.c_str());

                if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    LOG_INFO(DOM, "Can't open file '" + filepath + "'.");
                    return TextFile{filepath, ""};
                }

                auto text = file.readAll().toStdString();

                file.close();

                return TextFile{filepath, text};
            }
            catch (const std::exception& e)
            {
                LOG_INFO(DOM, Strfy() << "Couldn't open file '" << filepath << "' : " << e.what());

                return TextFile{filepath, ""};
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

        foreach(const QString& fileName, QDir((":/" + foldername).c_str()).entryList())
        {
            folder.push_back(openTxtFile(":/" + foldername + fileName.toStdString()));
        }

        return folder;
    }

    TextFile FileAccessor::openTextFile(const std::string& filepath) noexcept
    {
        LOG_THIS(DOM);

        return openTxtFile(filepath);
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
            QFile f(file.filepath.c_str());

            if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                LOG_ERROR(DOM, "Can't open file '" + file.filepath + "'.");
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

        std::vector<TextFile> folder;

        foreach(const QString& fileName, QDir(foldername.c_str()).entryList() )
        {
            folder.push_back(openTxtFile(foldername + fileName.toStdString()));
        }

        return folder;
    }

    std::string UniversalFileAccessor::getFileName(const TextFile& file) noexcept
    {
        QFileInfo fileInfo(file.filepath.c_str());

        return fileInfo.baseName().toStdString();
    }

    std::string UniversalFileAccessor::getFoldername(const TextFile& file) noexcept
    {
        QFileInfo fileInfo(file.filepath.c_str());

        return fileInfo.dir().path().toStdString();
    }
}