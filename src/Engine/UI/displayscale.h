#pragma once

namespace pg
{
    /**
     * @brief How many pixels of the screen one unit of the page is.
     *
     * 1 on an ordinary screen. On a dense one (a 4K monitor at 200 %, a phone) the window is still
     * measured in the units the page is laid in, and drawn on that many times more pixels: what is
     * rasterised ahead of the drawing (the font atlases, the icon atlases) is rasterised that many
     * times larger and drawn at the same size, so that it is as sharp as the screen is.
     *
     * Set once, before the fonts and the icons are registered. Nothing reads it afterwards: an
     * atlas keeps the ratio it was built at.
     */
    inline float& displayPixelRatio()
    {
        static float ratio = 1.0f;

        return ratio;
    }

    /**
     * @brief How many pixels the window is drawn on for one of its units, as the window says it.
     *
     * Kept by Window::resize from the drawable's size. What speaks to OpenGL in pixels and is
     * given in the page's units is multiplied by it: the scissor of a clip.
     */
    inline float& drawablePixelRatio()
    {
        static float ratio = 1.0f;

        return ratio;
    }
}
