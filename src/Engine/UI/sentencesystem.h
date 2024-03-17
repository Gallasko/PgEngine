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

    struct OnTextChanged
    {
        _unique_id entityId;
        std::string newText;
    };

    struct Letter
    {
        Letter(_unique_id id, const FontLoader::Font * const font) : id(id), font(font) { }
        Letter(const Letter& rhs) : id(rhs.id), font(rhs.font) { }

        void operator=(const Letter& rhs)
        {
            id = rhs.id;
            font = rhs.font;
        }

        _unique_id id;
        const FontLoader::Font * font;
    };

    struct SentenceText : public Ctor, public Dtor
    {
        std::string text = "";
        float scale = 2.0f;
        constant::Vector4D mainColor = constant::Vector4D(255.0f, 255.0f, 255.0f, 255.0f);
        constant::Vector4D outline1  = constant::Vector4D(  0.0f,   0.0f,   0.0f, 205.0f);
        constant::Vector4D outline2  = constant::Vector4D(255.0f, 255.0f, 255.0f, 180.0f);

        SentenceEffect effect = SentenceEffect::NOEFFCT;

        UiSize textWidth, textHeight;

        std::vector<Letter> letters;

        _unique_id entityId = 0;

        EntitySystem *ecsRef = nullptr;

        SentenceText() {}
        SentenceText(const SentenceText& other) : 
            text(other.text),
            scale(other.scale),
            mainColor(other.mainColor),
            outline1(other.outline1),
            outline2(other.outline2),
            effect(other.effect),
            textWidth(other.textWidth),
            textHeight(other.textHeight),
            entityId(other.entityId),
            ecsRef(other.ecsRef)
            {}

        SentenceText(const std::string& text) : text(text) {}
        SentenceText(const std::string& text, float scale, const constant::Vector4D& color1, const SentenceEffect& effect = SentenceEffect::NOEFFCT) : text(text), scale(scale), mainColor(color1), effect(effect) {}
        SentenceText(const std::string& text, float scale, const constant::Vector4D& color1, const constant::Vector4D& color2, const constant::Vector4D& color3 = constant::Vector4D(255.0f, 255.0f, 255.0f, 180.0f), const SentenceEffect& effect = SentenceEffect::NOEFFCT) : text(text), scale(scale), mainColor(color1), outline1(color2), outline2(color3), effect(effect) {}

        virtual void onCreation(EntityRef entity) override
        {
            ecsRef = entity->world();

            entityId = entity->id;
        }

        virtual void onDeletion(EntityRef entity) override;

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
            this->letters    = rhs.letters;
            this->entityId   = rhs.entityId;
            this->ecsRef     = rhs.ecsRef;
        }

        inline bool operator==(const SentenceText &rhs) const
        {
            return this->text == rhs.text && this->scale == rhs.scale && this->mainColor == rhs.mainColor && this->outline1 == rhs.outline1 && this->outline2 == rhs.outline2 && this->effect == rhs.effect;
        }

        inline bool operator!=(const SentenceText &rhs) const
        {
            return !(*this == rhs);
        }

        inline void setText(const std::string &text)
        {
            if (ecsRef)
                ecsRef->sendEvent(OnTextChanged{entityId, text});
            else
                this->text = text;
        }
    };

    template <>
    void serialize(Archive& archive, const SentenceText& value);

    struct SentenceSystem : public AbstractInstanceRenderer, System<Own<SentenceText>, Ref<UiComponent>, Listener<OnTextChanged>, Listener<UiComponentChangeEvent>, NamedSystem, InitSys, StoragePolicy>
    {
        struct SimpleSquareMesh : public Mesh
        {
            SimpleSquareMesh() : Mesh()
            { 
                LOG_THIS_MEMBER("Shape 2D Mesh");
                modelInfo.vertices = new float[12];
				//              x                     y                              z  
				modelInfo.vertices[0] =   0.0f; modelInfo.vertices[1] =   0.0f; modelInfo.vertices[2] =  1.0f;
				modelInfo.vertices[3] =   1.0f; modelInfo.vertices[4] =   0.0f; modelInfo.vertices[5] =  1.0f;
				modelInfo.vertices[6] =   0.0f; modelInfo.vertices[7] =  -1.0f; modelInfo.vertices[8] =  1.0f;
				modelInfo.vertices[9] =   1.0f; modelInfo.vertices[10] = -1.0f; modelInfo.vertices[11] = 1.0f;

				modelInfo.indices = new unsigned int[6];
				modelInfo.indices[0] = 0; modelInfo.indices[1] = 1; modelInfo.indices[2] = 2;
				modelInfo.indices[3] = 1; modelInfo.indices[4] = 2; modelInfo.indices[5] = 3;

				modelInfo.nbVertices = 12;
				modelInfo.nbIndices = 6;
            }
            virtual ~SimpleSquareMesh();

            void generateMesh();

            OpenGLBuffer *instanceVBO = nullptr;
        };

        SentenceSystem(MasterRenderer *renderer, FontLoader *font) : AbstractInstanceRenderer(renderer, RenderStage::Render, 22), font(font) { }

        virtual std::string getSystemName() const override { return "Sentence System"; }

        virtual void init() override;

        virtual void onEvent(const OnTextChanged& event) override;

        virtual void onEvent(const UiComponentChangeEvent& event) override;

        virtual void render() override;

        void addElement(const CompRef<UiComponent>& ui, const CompRef<SentenceText>& obj);

        void generateLetters(SentenceText& sentence, FontLoader *font);

        SimpleSquareMesh basicSquareMesh;
        bool squareMeshInitialized = false;

        FontLoader *font;

        std::atomic<_unique_id> nextLetterId = {0};
    };

    /** Helper that create an entity with an Ui component and a Texture component */
    CompList<UiComponent, SentenceText> makeSentence(EntitySystem *ecs, float x, float y, const SentenceText& text);
}