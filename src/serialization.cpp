#include "serialization.h"

#include "constant.h"

namespace pg
{
    std::string Serializer::filename = "serialize.sz";

    //Serialisation of base type

    template<>
    void serialize(Archive& archive, const bool& value)
    {
        std::string res = value ? "true" : "false";

        archive << res << ",\n";
    }

    template<>
    void serialize(Archive& archive, const int& value)
    {
        archive << value << ",\n";
    }

    template<>
    void serialize(Archive& archive, const float& value)
    {
        archive << value << ",\n";
    }

    template<>
    void serialize(Archive& archive, const double& value)
    {
        archive << value << ",\n";
    }

    template<>
    void serialize(Archive& archive, const std::string& value)
    {
        archive << value << ",\n";
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
        
        archive << "],\n";

        //TODO make this automatically after a new line;
        archive.indent();

        archive << "Indicies: [ ";
        
        for(unsigned int i = 0; i < modelInfo.nbIndices; i++)
            archive << modelInfo.indices[i] << " ";
        
        archive << "],\n";
        
        archive.endSerialization();
    }
}