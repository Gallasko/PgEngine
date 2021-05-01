#pragma once

#include <string>
#include <vector>

#include "uisystem.h"
#include "../Engine/fontloader.h"

struct Sentence : public UiComponent
{
    Sentence(const std::string& sentence, float scale, FontLoader *font);
    void setText(const std::string& sentence, FontLoader *font);

    FontLoader::Font* *letters;
    int nbChara = 0;
};