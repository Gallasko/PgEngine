#include "filemanager.h"

/*
#include <QFile>
#include <QString>
#include <QTextStream>


QFile file(":/res/names/american/female.names");

if (!file.open(QIODevice::ReadOnly))
{
    std::cout << "Couldn't open file" << std::endl;
    return -1;
}

QTextStream in(&file);
while (!in.atEnd()) {
    QString line = in.readLine();
    std::cout << line.toStdString() << std::endl;
}
*/

namespace pg
{
    
    TextFile RessourceManager::openTextFile(const std::string& filename)
    {

    }

    BinaryFile RessourceManager::openBinaryFile(const std::string& filename)
    {

    }

    std::vector<TextFile> RessourceManager::openTextFolder(const std::string& foldername)
    {

    }

    std::vector<BinaryFile> RessourceManager::openBinaryFolder(const std::string& foldername)
    {

    }

    TextFile FileManager::openTextFile(const std::string& filename)
    {

    }

    BinaryFile FileManager::openBinaryFile(const std::string& filename)
    {

    }

    std::vector<TextFile> FileManager::openTextFolder(const std::string& foldername)
    {

    }

    std::vector<BinaryFile> FileManager::openBinaryFolder(const std::string& foldername)
    {

    }

}