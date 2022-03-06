#include "serialization.h"

#include <fstream>

#include "logger.h"
#include "constant.h"

#include <iostream>
#include <sstream> 

namespace pg
{
    namespace
    {
        const char * DOM = "Serializer";

        size_t nbLeadingSpaces(const std::string& str, const std::string& whitespace = " \t")
        {
            const auto strBegin = str.find_first_not_of(whitespace);
            if (strBegin == std::string::npos)
                return -1; // no content

            return strBegin;
        }

        std::string trim(const std::string& str, const std::string& whitespace = " \t")
        {
            const auto strBegin = str.find_first_not_of(whitespace);
            if (strBegin == std::string::npos)
                return ""; // no content

            const auto strEnd = str.find_last_not_of(whitespace);
            const auto strRange = strEnd - strBegin + 1;

            return str.substr(strBegin, strRange);
        }
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
    void serialize(Archive& archive, const size_t& value)
    {
        archive << "size_t {" << value << "}" << archive.endl();
    }

    template<>
    void serialize(Archive& archive, const std::string& value)
    {
        // Todo if the string contains end of line characters -> make sure to correctly indent the string in the archive
        // by spliting the input string by end of line and then reconstructing the end of line with archive.endl()
        archive.startSerialization("String");

        serialize(archive, "size", value.size());
        archive << "string " << " {" << value << "}" << archive.endl();

        archive.endSerialization();
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
        
        archive << "]" << archive.endl();

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

    void UnserializedObject::parseString()
    {
        LOG_THIS_MEMBER(DOM);

        std::string currentLine;
        size_t classIndent = 0;
        size_t classChildIndent = 0;

        std::string nextLine;
        size_t currentIndent = 0;
        size_t nextIndent = 0;

        std::istringstream iss(serializedString);

        std::string tempObjectName = ""; 
        //std::string tempClassName = ""; 
        std::string delimiter = ": ";
        std::string tempSerializedString = "";
        size_t pos;
        
        // First line should always be the beginning of a class declaration
        bool startOfClass = false;
        bool lockupClass = false;

        // First class of the serialized string is for the current class
        //bool firstClass = true;

        bool errorHappened = false;

        // Check if their is at least one line in the serialized string
        if(!std::getline(iss, currentLine))
        {
            LOG_ERROR(DOM, "Serialized string is empty for object: '" + objectName + "'");

            isNullObject = true;
            return;
        }
        else
        {
            LOG_INFO(DOM, currentLine);

            // Get the current indent of the class
            classIndent = nbLeadingSpaces(currentLine);
            // Then calculate the indent of the children
            classChildIndent = nbLeadingSpaces("\t" + currentLine);

            // Check if the name given to the constructor match to the class in the serialized string
            pos = currentLine.find(delimiter);

            // If ":" is not found for the class declaration then it is a unnamed one
            if(pos == std::string::npos)
                tempObjectName = "";
            else // Else the name of the object is the first part of the string
                tempObjectName = currentLine.substr(0, pos);

            // If the object name doesn't match we throw an error
            if(tempObjectName != objectName)
            {
                LOG_ERROR(DOM, "Error happened when passing data: Current objectName '" + objectName + "' doesn't match the serialized string passed '" + tempObjectName + "'");
                
                isNullObject = true;
                return;
            }

            // Read the next line (For a class their should always be at least 2 lines, one for constructor and the last "}" to indicate the end of the class)
            if(!std::getline(iss, currentLine))
            {
                LOG_ERROR(DOM, "Serialized string doesn't end correctly for object: '" + objectName + "'");

                isNullObject = true;
                return;
            }

            LOG_INFO(DOM, currentLine);

            // If the second line of the string is "{" then it means that the class doesn't have any children so we can return early.
            const auto trimmed = trim(currentLine);
            if(trimmed == "}")
            {
                LOG_INFO(DOM, "Object: " + objectName + " doesn't have any children in serialization !");
                
                return;
            }

        }
            
        while(std::getline(iss, nextLine))
        {
            LOG_INFO(DOM, "Current line: " + currentLine + " with class indent: " + std::to_string(classIndent) + ", and current indent: " + std::to_string(currentIndent));

            nextIndent = nbLeadingSpaces(nextLine);
            if(nextIndent > classChildIndent and startOfClass == false and lockupClass == false)
                startOfClass = true;

            if(startOfClass)
            {
                // Trim the current line first when retriving the object name to not get whitespace issues.
                //tempObjectName = trim(currentLine);

                // Check if the name given to the constructor match to the class in the serialized string
                pos = currentLine.find(delimiter);

                // If ":" is not found for the class declaration then it is a unnamed one
                if(pos == std::string::npos)
                    tempObjectName = "";
                else // Else the name of the object is the first part of the string
                    tempObjectName = currentLine.substr(0, pos);

                LOG_INFO(DOM, "Child Object name: '" + tempObjectName + "'");

                // Todo: check if we need to retrive the class name from the serialized object

                // Look up for another ":" if found the serialized string is ill formated
                pos = currentLine.find(delimiter, pos + delimiter.length());
                if(pos != std::string::npos)
                {
                    LOG_ERROR(DOM, "Class string is ill formated, current line possess multiple ':' '" + currentLine + "'");

                    errorHappened = true;
                    break;
                }

                tempSerializedString = currentLine + "\n";

                startOfClass = false;
                lockupClass = true;
            }
            else
            {
                if(lockupClass)
                {
                    tempSerializedString += currentLine + "\n";

                    const auto trimmed = trim(currentLine);
                    if((classChildIndent == currentIndent) and (trimmed == "}" or trimmed =="},"))
                    {
                        LOG_INFO(DOM, "Create a new child object with the name: '" + tempObjectName + "'");

                        children.emplace_back(tempSerializedString, tempObjectName);
                        lockupClass = false;
                    }
                }
            }

            currentLine = nextLine;
            currentIndent = nextIndent;
        }

        if(errorHappened)
        {
            LOG_ERROR(DOM, "Error happened when parsing serialized string for object: '" + objectName + "'");
            isNullObject = true;
        }
    }

    Serializer::Serializer(const std::string& filename) : filename(filename)
    {
        LOG_THIS_MEMBER(DOM);

        std::ifstream file;
        
        file.open(filename);

        if(file.is_open())
        {
            std::string line;
            std::string serializedString;

            std::string objectName = ""; 
            std::string delimiter = ": ";
            size_t pos;
            
            // First line should always be the beginning of a class declaration
            bool startOfClass = true;

            bool errorHappened = false;

            while(std::getline(file, line))
            {
                std::cout << line << std::endl;

                if(startOfClass)
                {
                    pos = line.find(delimiter);

                    if(pos == std::string::npos)
                    {
                        errorHappened = true;
                        break;
                    }
                    
                    objectName = line.substr(0, pos);
                    std::cout << objectName << std::endl;

                    pos = line.find(delimiter, pos + delimiter.length());

                    if(pos != std::string::npos)
                    {
                        errorHappened = true;
                        break;
                    }

                    serializedString = line + '\n';
                    startOfClass = false;
                }
                else
                {
                    serializedString += line + '\n';

                    if(line == "}")
                    {
                        serializedMap[objectName] = serializedString;
                        startOfClass = true;
                    }
                        
                }
            }

            if(errorHappened)
                LOG_ERROR(DOM, "Error happened when parsing: " + filename);
        }
        else
        {
            LOG_ERROR(DOM, "Serializer can't open serialize file: " + filename);
        }
    }

    Serializer::~Serializer()
    {
        LOG_THIS_MEMBER(DOM);

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