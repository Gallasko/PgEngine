#pragma once

#include <string>
#include <vector>

#include "uisystem.h"
#include "Loaders/fontloader.h"
#include "Renderer/renderer.h"
#include "constant.h"

#include "serialization.h"

namespace pg
{
    enum class SentenceEffect : uint8_t
    {
        NOEFFCT     = 0x00,
        TEXTWIGGLE  = 0x01,
        COLORSCROLL = 0x10,

        NUM_TYPES
    };

    struct SentenceMesh : public Mesh
    {
        SentenceMesh() : Mesh() { LOG_THIS_MEMBER("Sentence Mesh"); modelInfo = constant::SquareInfo{}; }
        ~SentenceMesh() { LOG_THIS_MEMBER("Sentence Mesh"); }

        void generateMesh();
    };

    struct SentenceText
    {
        std::string text = "";
        float scale = 2.0f;
        constant::Vector4D mainColor = constant::Vector4D(255.0f, 255.0f, 255.0f, 255.0f);
        constant::Vector4D outline1  = constant::Vector4D(  0.0f,   0.0f,   0.0f, 205.0f);
        constant::Vector4D outline2  = constant::Vector4D(255.0f, 255.0f, 255.0f, 180.0f);

        SentenceEffect effect = SentenceEffect::NOEFFCT;

        UiSize textWidth = 0.0f, textHeight = 0.0f;

        SentenceText() {}
        SentenceText(const SentenceText& other) : 
            text(other.text),
            scale(other.scale),
            mainColor(other.mainColor),
            outline1(other.outline1),
            outline2(other.outline2),
            effect(other.effect),
            textWidth(other.textWidth),
            textHeight(other.textHeight)
            {}

        SentenceText(const std::string& text) : text(text) {}
        SentenceText(const std::string& text, float scale, const constant::Vector4D& color1, const SentenceEffect& effect = SentenceEffect::NOEFFCT) : text(text), scale(scale), mainColor(color1), effect(effect) {}
        SentenceText(const std::string& text, float scale, const constant::Vector4D& color1, const constant::Vector4D& color2, const constant::Vector4D& color3 = constant::Vector4D(255.0f, 255.0f, 255.0f, 180.0f), const SentenceEffect& effect = SentenceEffect::NOEFFCT) : text(text), scale(scale), mainColor(color1), outline1(color2), outline2(color3), effect(effect) {}

        inline void operator=(const SentenceText &rhs)
        {
            this->text       = rhs.text;
            this->scale      = rhs.scale;
            this->mainColor  = rhs.mainColor;
            this->outline1   = rhs.outline1;
            this->outline2   = rhs.outline2;
            this->effect     = rhs.effect;
            this->textWidth  = rhs.textWidth;
            this->textHeight = rhs.textHeight;
        }

        inline bool operator==(const SentenceText &rhs) const
        {
            return this->text == rhs.text && this->scale == rhs.scale && this->mainColor == rhs.mainColor && this->outline1 == rhs.outline1 && this->outline2 == rhs.outline2 && this->effect == rhs.effect;
        }

        inline bool operator!=(const SentenceText &rhs) const
        {
            return !(*this == rhs);
        }
    };

    template <>
    void serialize(Archive& archive, const SentenceText& value);

    struct OnTextChanged
    {
        _unique_id entityId;
        std::string newText;
    };

    struct SentenceSystem : public AbstractRenderer, System<Own<SentenceText>, Ref<UiComponent>, Listener<OnTextChanged>, Listener<UiComponentChangeEvent>, NamedSystem, InitSys, StoragePolicy>
    {
        SentenceSystem(MasterRenderer *renderer, FontLoader *font) : AbstractRenderer(renderer, RenderStage::Render), font(font) { }

        virtual std::string getSystemName() const override { return "Sentence System"; }

        virtual void init() override;

        virtual void onEvent(const OnTextChanged& event) override;

        virtual void onEvent(const UiComponentChangeEvent& event) override;

        virtual void render() override;

        Mesh* getSentenceMesh(SentenceText& sentence, FontLoader *font);

        FontLoader *font;
    };

    /** Helper that create an entity with an Ui component and a Texture component */
    CompList<UiComponent, SentenceText> makeSentence(EntitySystem *ecs, float x, float y, const SentenceText& text);
}