#pragma once

#include <string>
#include <vector>

#include "uisystem.h"
#include "../Engine/fontloader.h"

#include <QOpenGLFramebufferObject>

struct Sentence : public UiComponent
{
    Sentence(const std::string& sentence, float scale, FontLoader *font);
    void setText(const std::string& sentence, FontLoader *font);

    std::string text = "";

    FontLoader::Font* *letters;
    int nbChara = 0;

    unsigned int texture = 0;
    //unsigned int frameBuffer = 0;
    QOpenGLFramebufferObject *frameBuffer = nullptr;
    bool initialized = false;
};