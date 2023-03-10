/**
 * @file textinput.h
 * @author Pigeon Codeur (pigeoncodeur@gmail.com)
 * @brief Definition of the text input class.
 * @version 0.1
 * @date 2022-06-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include <string>
#include <functional>

#include "uisystem.h"
#include "Renderer/renderer.h"
#include "Input/inputcomponent.h"

namespace pg
{
    // Type forwarding
    class Input;
    class TextureComponent;
    // class Sentence;
    class FontLoader;

    class TextInput : public UiComponent
    {
        // Typedefs
    public:
        /** Definition of a text input callback function */
        typedef std::function<void(const std::string&)> TextInputCallback;

        /**
         * @brief Enum defining what type of character can be typed in the text input
         * 
         * Text: Can hold any ascii characters
         * Numeric: Accept only digits
         * 
         */
        enum class InputMode
        {
            TEXT,
            NUMERIC
        };
    
        // Public Interface
    public:
        TextInput(const UiFrame& frame, const std::string& texture, FontLoader* fontLoader, const TextInputCallback& onAccept, const TextInputCallback& onChange = nullptr, const InputMode& mode = InputMode::TEXT);
        // TextInput(const TextInput& other);
        virtual ~TextInput();

        void setTexture(const std::string& texture);

        void show() override;
        void hide() override;

        inline void render(MasterRenderer *masterRenderer) override;

        // Inline functions
    public:
        /**
         * @brief Set the Mode of the text input
         * 
         * @param mode The mode to set to the text input
         */
        inline void setMode(const InputMode& mode) { this->mode = mode; }
        
        // Private Interface
    private:
        friend void renderer<>(MasterRenderer *masterRenderer, TextInput *textInput);

        void changeTextCallback(Input* inputHandler, double...);

        void focus(Input* inputHandler, double...);
        void unfocus(Input* inputHandler, double);

        // Private Variables
    private:
        /** Hold the accept callback function */
        TextInputCallback onAccept;

        /** Hold the accept callback function */
        TextInputCallback onChange;

        /** Hold the mouse input handler */
        MouseComponent* mouseInput;

        /** Hold the key input handler */
        KeyComponent* keyInput;

        /** Hold the texture of the text input */
        TextureComponent *texture;
        
        /** Hold the sentence print to the screen */
        // Sentence *sentence;

        /** Actual text inside the text input */
        std::string text;

        /** Boolean to indicate whether the text input is focused or not */
        bool focused = false;

        /** Hold what type of character does the text input accept */
        InputMode mode = InputMode::TEXT;
    };
}
