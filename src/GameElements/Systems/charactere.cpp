#include "charactere.h"

#include "../../Engine/randomnumbergenerator.h"
#include "namegen.h"

Character::CharacterUi::CharacterUi(Character *chara, pg::FontLoader *fontLoader) : chara(chara)
{
    displayName = new pg::Sentence({chara->getName()}, 2.0f, fontLoader);
}

Character::CharacterUi::CharacterUi(const Character::CharacterUi& other) : CharacterUi(other.chara, other.displayName->font)
{

}

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

    return Character(info);
}
