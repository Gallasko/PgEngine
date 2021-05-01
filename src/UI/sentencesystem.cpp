#include "sentencesystem.h"

Sentence::Sentence(const std::string& sentence, float scale, FontLoader *font)
{
    this->scale = scale;
    setText(sentence, font);
}

void Sentence::setText(const std::string& sentence, FontLoader *font)
{
    nbChara = sentence.length();
    letters = new FontLoader::Font* [sentence.length()];

    width = 0;
    for(int i = 0; i < sentence.length(); i++)
    {
        letters[i] = font->getChara(std::string(1, sentence.at(i)));
        width += letters[i]->getWidth() * scale + 1;

        if(letters[i]->getHeight() * scale + letters[i]->getOffset() > height)
            height = letters[i]->getHeight() * scale + letters[i]->getOffset();
    }

    update();
}