#include "filemanager.h"

#include <stdexcept>
#include <fstream>

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
                constexpr auto read_size = std::size_t(4096);
                auto stream = std::ifstream(filename.data());
                stream.exceptions(std::ios_base::badbit);

                if (not stream) {
                    throw std::ios_base::failure("File does not exist");
                }
                
                auto out = std::string();
                auto buf = std::string(read_size, '\0');

                while (stream.read(& buf[0], read_size)) 
                {
                    out.append(buf, 0, stream.gcount());
                }

                out.append(buf, 0, stream.gcount());

                return TextFile{filename, out};
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

        // Todo

        // foreach(const QString& fileName, QDir((":/" + foldername).c_str()).entryList())
        // {
        //     folder.push_back(openTxtFile(":/" + foldername + fileName.toStdString()));
        // }

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

        // Todo

        // foreach(const QString& fileName, QDir(foldername.c_str()).entryList() )
        // {
        //     folder.push_back(openTxtFile(foldername + fileName.toStdString()));
        // }

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

}