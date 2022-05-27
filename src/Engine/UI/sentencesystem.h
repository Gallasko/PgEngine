#pragma once

#include <string>
#include <vector>

#include "uisystem.h"
#include "../Loaders/fontloader.h"
#include "../Renderer/renderer.h"
#include "../constant.h"

#include "../serialization.h"

namespace pg
{
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

        SentenceText(const std::string& text, const constant::Vector4D& color1, const SentenceEffect& effect = SentenceEffect::NOEFFCT) : text(text), mainColor(color1), effect(effect) {}

        SentenceText(const std::string& text, const constant::Vector4D& color1, const constant::Vector4D& color2, const constant::Vector4D& color3 = constant::Vector4D(255.0f, 255.0f, 255.0f, 180.0f), const SentenceEffect& effect = SentenceEffect::NOEFFCT) : text(text), mainColor(color1), outline1(color2), outline2(color3), effect(effect) {}

        inline void operator=(const SentenceText &rhs)
        {
            this->text      = rhs.text;
            this->mainColor = rhs.mainColor;
            this->outline1  = rhs.outline1;
            this->outline2  = rhs.outline2;
            this->effect    = rhs.effect;
        }

        inline bool operator==(const SentenceText &rhs) const
        {
            return this->text == rhs.text && this->mainColor == rhs.mainColor && this->outline1 == rhs.outline1 && this->outline2 == rhs.outline2 && this->effect == rhs.effect;
        }

        inline bool operator!=(const SentenceText &rhs) const
        {
            return !(*this == rhs);
        }
    };

    //TODO check if in need to be static
    struct Sentence : public UiComponent, private QOpenGLFunctions
    {
        struct SentenceParameters
        {
            SentenceText text;
            float scale;
            FontLoader *font;
        };

        Sentence(const SentenceText& sentence, const float& scale, FontLoader *font);
        Sentence(const SentenceParameters& parameters);
        Sentence(const Sentence &rhs);
        ~Sentence();

        void setText(const SentenceText& sentence, FontLoader *font);

        void generateMesh();

        virtual void render(MasterRenderer* masterRenderer);

        float scale = 0.0f;

        SentenceText text;
        FontLoader *font;
        int nbChara = 0;
        
        constant::ModelInfo modelInfo;

        QOpenGLVertexArrayObject *VAO = nullptr;
        QOpenGLBuffer *VBO = nullptr;
        QOpenGLBuffer *EBO = nullptr;

        bool initialised = false;
    };

    //TODO see if a render all virtual methode is revelent and could be implemented in the base renderer for rendering multiple instance at once
    //instead of create 2 separates renderer one for the single instance rendering and the other for the multiple rendering
    //the renderList could take a vector of element and if the methode is not reimplemented could by default call render multiple time !

    // va_args can t take std::vector need to find a workaround
    /*
    struct SentenceVectorRenderer : public Renderer
    {
        using Renderer::Renderer;
        virtual ~SentenceVectorRenderer() {}

        void render(MasterRenderer* masterRenderer...);
    };
    */
}