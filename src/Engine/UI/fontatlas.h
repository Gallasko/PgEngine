#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace pg
{
    struct GlyphInfo
    {
        glm::vec2 size{0.0f, 0.0f};    // In the units text is laid in: the raster's size over the atlas's pixel ratio
        glm::vec2 bearing{0.0f, 0.0f};
        float advance = 0.0f;          // pixels, fractional (26.6 / 64)
        glm::vec2 uvTopLeft{0.0f, 0.0f};
        glm::vec2 uvBottomRight{0.0f, 0.0f};
    };

    struct FontMetrics
    {
        float ascender = 0.0f;         // pixels above baseline
        float descender = 0.0f;        // pixels below baseline, positive
        float lineHeight = 0.0f;       // face->size->metrics.height / 64
        bool hasKerning = false;
    };

    /// A rasterised font at one pixel size: glyph table, metrics and the single-channel atlas bytes. No GL.
    class FontAtlas
    {
    public:
        static constexpr int Width = 1024;    // At a pixel ratio of 1: as many times larger as the ratio, rounded up
        static constexpr int Height = 1024;

        FontAtlas() = default;
        ~FontAtlas();

        FontAtlas(FontAtlas&& other) noexcept;
        FontAtlas& operator=(FontAtlas&& other) noexcept;

        /// Returns false (and logs) if the face cannot be opened or the atlas overflows.
        // `pixelRatio`: the glyphs are rasterised that many times larger (displayscale.h), and every
        // measure the atlas gives stays in the units of `sizePx`: a text is laid the same and drawn sharper
        bool build(FT_Library ft, const std::string& fontPath, int sizePx, const std::vector<uint32_t>& codepoints, float pixelRatio = 1.0f);

        const GlyphInfo* glyph(uint32_t codepoint) const;   // nullptr if not in the atlas
        const GlyphInfo& glyphOrNotdef(uint32_t codepoint) const;
        float kerning(uint32_t left, uint32_t right) const;  // pixels; 0 when the face has no kern table
        const FontMetrics& metrics() const { return fontMetrics; }
        const std::vector<unsigned char>& buffer() const { return atlasBuffer; }
        int sizePx() const { return pixelSize; }
        int width() const { return atlasWidth; }             // The atlas's own size, in its pixels
        int height() const { return atlasHeight; }
        float ratio() const { return rasterRatio; }          // Raster pixels to a unit of `sizePx`

    private:
        FontAtlas(const FontAtlas&) = delete;
        FontAtlas& operator=(const FontAtlas&) = delete;

        int pixelSize = 0;
        int atlasWidth = Width;
        int atlasHeight = Height;
        float rasterRatio = 1.0f;
        FontMetrics fontMetrics;
        std::vector<unsigned char> atlasBuffer;
        std::unordered_map<uint32_t, GlyphInfo> glyphs;
        GlyphInfo notdefGlyph;                                 // fallback for missing code points
        mutable std::unordered_map<uint64_t, float> kernCache; // (left << 32 | right) -> px
        FT_Face face = nullptr;                                // kept open for kerning; freed in the destructor
    };

    /// The code points every registered font rasterises: ASCII, Latin-1 Supplement, the punctuation and
    /// arrows the UI uses. One list, shared by all fonts.
    const std::vector<uint32_t>& defaultCharset();
}
