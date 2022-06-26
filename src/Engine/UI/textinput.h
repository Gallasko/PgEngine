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

#include "button.h"

namespace pg
{
    class TextInput : public Button
    {
        // Typedefs
    public:
        /** Definition of a text input callback function */
        typedef std::function<void(const std::string&)> TextInputCallback;
    
        // Public Interface
    public:
        TextInput(const TextInputCallback& callback);

        // Private Interface
    private:
        void changeTextCallback(Input* inputHandler, double...);

        void focus(Input* inputHandler, double);
        void unfocus(Input* inputHandler, double);

        // Private Variables
    private:
        /** Hold the callback function */
        TextInputCallback callback;

        /** Actual text inside the text input */
        std::string text;

        /** Boolean to indicate whether the text input is focused or not */
        bool focused = false;
    };
}
