#pragma once

#include <string>
#include <vector>

#include "uisystem.h"
#include "../Engine/fontloader.h"
#include "../constant.h"

enum class SentenceEffect : uint8_t
{
    NOEFFCT     = 0x00,
    TEXTWIGGLE  = 0x01,
    COLORSCROLL = 0x10,

    NUM_TYPES
};

struct SentenceText
{
    std::string text = "";
    constant::Vector4D mainColor = constant::Vector4D(255.0f, 255.0f, 255.0f, 255.0f);
    constant::Vector4D outline1  = constant::Vector4D(  0.0f,   0.0f,   0.0f, 205.0f);
    constant::Vector4D outline2  = constant::Vector4D(255.0f, 255.0f, 255.0f, 180.0f);

    SentenceEffect effect = SentenceEffect::NOEFFCT;

    SentenceText() {}

    SentenceText(const std::string& text) : text(text) {}

    SentenceText(const std::string& text, constant::Vector4D color1, SentenceEffect effect = SentenceEffect::NOEFFCT) : text(text), mainColor(color1), effect(effect) {}

    SentenceText(const std::string& text, constant::Vector4D color1, constant::Vector4D color2, constant::Vector4D color3 = constant::Vector4D(255.0f, 255.0f, 255.0f, 180.0f), SentenceEffect effect = SentenceEffect::NOEFFCT) : text(text), mainColor(color1), outline1(color2), outline2(color3), effect(effect) {}

    inline void operator=(const SentenceText &rhs)
    {
        this->text      = rhs.text;
        this->mainColor = rhs.mainColor;
        this->outline1  = rhs.outline1;
        this->outline2  = rhs.outline2;
        this->effect    = rhs.effect;
    }

    inline bool operator==(const SentenceText &rhs)
    {
        return this->text == rhs.text && this->mainColor == rhs.mainColor && this->outline1 == rhs.outline1 && this->outline2 == rhs.outline2 && this->effect == rhs.effect;
    }

    inline bool operator!=(const SentenceText &rhs)
    {
        return !(*this == rhs);
    }
};

struct Sentence : public UiComponent, private QOpenGLFunctions
{
    Sentence(const SentenceText& sentence, float scale, FontLoader *font);
    Sentence(const Sentence &rhs);
    ~Sentence();
    void setText(const SentenceText& sentence, FontLoader *font);

    void generateMesh();

    SentenceText text;
    int nbChara = 0;

    constant::ModelInfo modelInfo;

    QOpenGLVertexArrayObject *VAO = nullptr;
	QOpenGLBuffer *VBO = nullptr;
	QOpenGLBuffer *EBO = nullptr;

    bool initialised = false;
};