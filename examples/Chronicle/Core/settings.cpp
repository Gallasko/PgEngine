#include "settings.h"

#include <filesystem>

#include "logger.h"
#include "serialization.h"
#include "Files/filemanager.h"

namespace pg
{
    template <>
    void serialize(Archive& archive, const chronicle::Settings& value)
    {
        archive.startSerialization("Settings");

        serialize(archive, "version", chronicle::Settings::Version);
        serialize(archive, "beginOnFirstClick", value.beginOnFirstClick);
        serialize(archive, "theme", value.theme);

        archive.endSerialization();
    }

    template <>
    chronicle::Settings deserialize(const UnserializedObject& serialized)
    {
        chronicle::Settings data;

        if (serialized.isNull())
            return data;

        defaultDeserialize(serialized, "beginOnFirstClick", data.beginOnFirstClick);
        defaultDeserialize(serialized, "theme", data.theme);

        // A theme this build does not have is the day's
        if (data.theme != "day" and data.theme != "candle")
            data.theme = "day";

        return data;
    }
}

namespace chronicle
{
    namespace
    {
        constexpr const char * const DOM = "Chronicle.Settings";

        constexpr const char * const ObjectName = "settings";
    }

    bool Settings::load(const std::string& path)
    {
        *this = Settings{};

        if (path.empty() or not pg::UniversalFileAccessor::exists(path))
            return false;

        pg::Serializer serializer(false);
        serializer.setFile(path);

        if (serializer.getSerializedMap().count(ObjectName) == 0)
            return false;

        *this = serializer.deserializeObject<Settings>(ObjectName);

        return true;
    }

    bool Settings::save(const std::string& path) const
    {
        if (path.empty())
            return false;

        std::error_code ec;
        const auto folder = std::filesystem::path(path).parent_path();

        if (not folder.empty())
            std::filesystem::create_directories(folder, ec);

        if (ec)
        {
            LOG_ERROR(DOM, "Cannot create the folder of " << path << ": " << ec.message());
            return false;
        }

        pg::Serializer serializer(false);
        serializer.setFile(path);
        serializer.clear();

        writeInto(serializer);
        serializer.save();

        return pg::UniversalFileAccessor::exists(path);
    }

    void Settings::writeInto(pg::Serializer& serializer) const
    {
        serializer.serializeObject(ObjectName, *this);
    }
}
