#include "filemanager.h"

#include <QFile>
#include <QDir>
#include <QString>
#include <QTextStream>

#include <stdexcept>

#include "../../logger.h"

namespace pg
{
    namespace
    {
        const char * DOM = "File Manager";

        TextFile openTxtFile(const std::string& filename)
        {
            LOG_THIS(DOM);

            QFile file(filename.c_str());

            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                LOG_ERROR(DOM, "Can't open file '" + filename + "'.");
                throw std::runtime_error("Can't open file " + filename + "'.");
            }

            auto text = file.readAll().toStdString();

            file.close();

            return TextFile{filename, text};
        }
    }

    TextFile ResourceManager::openTextFile(const std::string& filename)
    {
        return openTxtFile(":/" + filename);
    }

    std::vector<TextFile> ResourceManager::openTextFolder(const std::string& foldername)
    {
        std::vector<TextFile> folder;

        foreach(const QString& fileName, QDir((":/" + foldername).c_str()).entryList())
        {
            folder.push_back(openTxtFile(":/" + foldername + fileName.toStdString()));
        }

        return folder;
    }

    TextFile FileManager::openTextFile(const std::string& filename)
    {
        return openTxtFile(filename);
    }

    std::vector<TextFile> FileManager::openTextFolder(const std::string& foldername)
    {
        std::vector<TextFile> folder;

        foreach(const QString& fileName, QDir(foldername.c_str()).entryList() )
        {
            folder.push_back(openTxtFile(fileName.toStdString()));
        }

        return folder;
    }
}