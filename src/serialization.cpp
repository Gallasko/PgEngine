#include "serialization.h"

#include "constant.h"

namespace pg
{
    std::string Serializer::filename = "serialize.sz";

    //Serialisation of base type

    std::ostream& operator<<(std::ostream& stream, const Archive::EndOfLine& endOfLine)
    {
        stream << std::endl;
        stream << std::string(*endOfLine.indentLevel, '\t');
        
        return stream;
    }

    template<>
    void serialize(Archive& archive, const bool& value)
    {
        std::string res = value ? "true" : "false";

        archive << res << "," << archive.endl();
    }

    template<>
    void serialize(Archive& archive, const int& value)
    {
        archive << value << "," << archive.endl();
    }

    template<>
    void serialize(Archive& archive, const float& value)
    {
        archive << value << "," << archive.endl();
    }

    template<>
    void serialize(Archive& archive, const double& value)
    {
        archive << value << "," << archive.endl();
    }

    template<>
    void serialize(Archive& archive, const std::string& value)
    {
        archive << value << "," << archive.endl();
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
        
        archive << "]," << archive.endl();;

        //TODO make this automatically after a new line;

        archive << "Indicies: [ ";
        
        for(unsigned int i = 0; i < modelInfo.nbIndices; i++)
            archive << modelInfo.indices[i] << " ";
        
        archive << "]," << archive.endl();;
        
        archive.endSerialization();
    }
}