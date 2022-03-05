#include "serialization.h"

#include <fstream>

#include "logger.h"
#include "constant.h"

namespace pg
{
    namespace
    {
        const char * DOM = "Serializer";
    }

    //Serialisation of base type

    template<>
    void serialize(Archive& archive, const bool& value)
    {
        std::string res = value ? "true" : "false";

        archive << "bool {" << res << "}" << archive.endl();
    }

    template<>
    void serialize(Archive& archive, const int& value)
    {
        archive << "int {" << value << "}" << archive.endl();
    }

    template<>
    void serialize(Archive& archive, const float& value)
    {
        archive << "float {" << value << "}" << archive.endl();
    }

    template<>
    void serialize(Archive& archive, const double& value)
    {
        archive << "double {" << value << "}" << archive.endl();
    }

    template<>
    void serialize(Archive& archive, const std::string& value)
    {
        archive << "string {" << value << "}" << archive.endl();
    }

    template<>
    void serialize(Archive& archive, const constant::Vector2D& vec2D)
    {
        archive.startSerialization("Vector 2D");

        serialize(archive, "x", vec2D.x);
        serialize(archive, "y", vec2D.y);

        archive.endSerialization();
    }

    template<>
    void serialize(Archive& archive, const constant::Vector3D& vec3D)
    {
        archive.startSerialization("Vector 3D");

        serialize(archive, "x", vec3D.x);
        serialize(archive, "y", vec3D.y);
        serialize(archive, "z", vec3D.z);

        archive.endSerialization();
    }

    template<>
    void serialize(Archive& archive, const constant::Vector4D& vec4D)
    {
        archive.startSerialization("Vector 4D");

        serialize(archive, "x", vec4D.x);
        serialize(archive, "y", vec4D.y);
        serialize(archive, "z", vec4D.z);
        serialize(archive, "w", vec4D.w);

        archive.endSerialization();
    }

    template<>
    void serialize(Archive& archive, const constant::ModelInfo& modelInfo)
    {
        archive.startSerialization("Model Info");

        archive << "Verticies: [ ";
        
        for(unsigned int i = 0; i < modelInfo.nbVertices; i++)
            archive << modelInfo.vertices[i] << " ";
        
        archive << "]" << archive.endl();;

        //TODO make this automatically after a new line;

        archive << "Indicies: [ ";
        
        for(unsigned int i = 0; i < modelInfo.nbIndices; i++)
            archive << modelInfo.indices[i] << " ";
        
        archive << "]" << archive.endl();
        
        archive.endSerialization();
    }

    void Archive::startSerialization(const std::string& className)
    {
        LOG_THIS_MEMBER(DOM);

        *this << className << " {" << endOfLine;
        indentLevel++;

        requestComma = false;
    }

    void Archive::endSerialization()
    {
        LOG_THIS_MEMBER(DOM);

        requestComma = false;

        indentLevel--;
        *this << "}" << endOfLine;
    }

    Serializer::~Serializer()
    {
        std::lock_guard<std::mutex> lock(mutex);

        registerToFile();
    }

    void Serializer::registerSerialized(const std::string& objectName, const std::stringstream& serializedString)
    {
        LOG_THIS_MEMBER(DOM);
        
        std::lock_guard<std::mutex> lock(mutex);
        serializedMap[objectName] = serializedString.str();

        registerToFile();
    }

    void Serializer::registerToFile() const
    {
        LOG_THIS_MEMBER(DOM);

        std::ofstream file;
        
        file.open(filename);

        if(file.is_open())
        {
            for(const auto& serializedString : serializedMap)
                file << serializedString.first << ": " << serializedString.second;

            file.close();
        }
        else
        {
            LOG_ERROR(DOM, "Serializer can't open serialize file: " + filename);
        }
    }
}