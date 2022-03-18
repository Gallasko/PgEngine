#include "charactere.h"

#include "../../Engine/randomnumbergenerator.h"
#include "namegen.h"

Character Character::createCharacter(const std::string& name)
{
    static unsigned int id = 0;

    std::string characterName;

    int gender = RandomNumberGenerator::generator()->generateNumber() % 2;

    if(name == "")
        characterName = NameGenerator::generator()->getRandomName(gender == 0 ? NameGenerator::Gender::MALE : NameGenerator::Gender::FEMALE);
    else
        characterName = name;

    CharacterInfo info = {id++, characterName};
}
