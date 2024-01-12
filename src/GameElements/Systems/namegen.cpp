// #include "namegen.h"

// #include "../../Engine/logger.h"
// #include "../../Engine/Maths/randomnumbergenerator.h"
// #include "../../Engine/Files/fileparser.h"

// namespace
// {
//     const char * DOM = "Name Generation";
// }

// using pg::RandomNumberGenerator;

// std::string NameGenerator::getRandomName(const Gender& gender) const
// {
//     LOG_THIS_MEMBER(DOM);
    
//     std::string name = "";

//     switch(gender)
//     {
//         case Gender::FEMALE:
//             name = femaleList.at(RandomNumberGenerator::generator()->generateNumber() % femaleList.size()).name;
//             break;

//         case Gender::MALE:
//         default:
//             name = maleList.at(RandomNumberGenerator::generator()->generateNumber() % maleList.size()).name;
//             break;
//     }

//     name += " " + surnameList.at(RandomNumberGenerator::generator()->generateNumber() % surnameList.size()).name;

//     return name;
// }

// NameGenerator::NameGenerator()
// {
//     //TODO make this lookup from somewhere
//     parseFiles("names/");
// }

// void NameGenerator::parseFiles(const std::string &path)
// {
//     LOG_THIS_MEMBER(DOM);

//     std::string line;
//     std::string country;

//     std::vector<pg::TextFile> folder = pg::ResourceAccessor::openTextFolder(path);

//     for(auto file : folder)
//     {
//         pg::FileParser parser(file);        

//         country = parser.getNextLine();

//         if(country == "")
//         {
//             LOG_ERROR(DOM, "Error parsing: '" + file.filepath + "', file is empty");
//             continue;
//         }

//         line = parser.getNextLine();

//         if(line == "")
//         {
//             LOG_ERROR(DOM, "Error parsing: '" + file.filepath + "', file is missing the gender type");
//             continue;
//         }

//         std::vector<Name> *nameVector;

//         if(line == "female")
//             nameVector = &femaleList;
//         else if(line == "male")
//             nameVector = &maleList;
//         else if(line == "any")
//             nameVector = &surnameList;
//         else
//         {
//             LOG_ERROR(DOM, "Error in parsing the gender of the file: " + file.filepath);
//             continue;
//         }

//         parser.addCallback(std::regex{".*"},[&](const std::string& line) { nameVector->emplace_back(country, line); } );

//         parser.run();
//     }
// }
