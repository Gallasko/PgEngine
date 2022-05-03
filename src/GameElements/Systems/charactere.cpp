#include "charactere.h"

#include "../../Engine/Maths/randomnumbergenerator.h"
#include "namegen.h"

namespace pg
{
    template<>
    void renderer(MasterRenderer* renderer, CharacterUi* chara)
    {

    }
}

CharacterUi::CharacterUi(Character *chara, pg::FontLoader *fontLoader) : pg::UiComponent(), chara(chara)
{
    displayName = new pg::Sentence({chara->getName()}, 2.0f, fontLoader);
}

CharacterUi::CharacterUi(const CharacterUi& other) : CharacterUi(other.chara, other.displayName->font)
{

}

void CharacterUi::render(pg::MasterRenderer* masterRenderer)
{
   pg::renderer(masterRenderer, this);
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
