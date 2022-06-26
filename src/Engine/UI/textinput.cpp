/**
 * @file textinput.cpp
 * @author Pigeon Codeur (pigeoncodeur@gmail.com)
 * @brief Implementation of the text input class
 * @version 0.1
 * @date 2022-06-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "textinput.h"

namespace pg
{
    TextInput::TextInput(const TextInputCallback& callback) : Button(this, TextInput::focus)
    {

    }

    /**
     * @brief Key function of the text input
     * 
     * @param inputHandler A pointer to the input handler
     * 
     * This function is used to handle how the key events translate into a new text.
     */
    void TextInput::changeTextCallback(Input* inputHandler, double...) 
    {
        auto previousText = text;
        // A static vector to keep track of the current key pressed
        static std::vector<int> keyPressed;

        // Convert a key to upper case if shift is pressed
        int shiftKey = inputHandler->isKeyPressed(Qt::Key_Shift) ? 0 : 32; // Number to shift from upper to lower case 
        // Check if control is pressed for keyboard accessibility
        bool ctrlKey = inputHandler->isKeyPressed(Qt::Key_Control);

        // Loop over the range of the printable characters
        for(int i = 0x41; i <= 0x5a; i++) // QtKey range from A = 0x41 to Z = 0x5a
        {
            // If a printable character is pressed then we add it to the text
            if(inputHandler->isKeyPressed(static_cast<Qt::Key>(i)) && std::find(keyPressed.begin(), keyPressed.end(), i) == keyPressed.end())
            {
                text += static_cast<char>(i + shiftKey);
                keyPressed.push_back(i);
            } 
        }

        // Put a blank character if the spacebar is pressed
        if(inputHandler->isKeyPressed(Qt::Key_Space) && std::find(keyPressed.begin(), keyPressed.end(), static_cast<int>(Qt::Key_Space)) == keyPressed.end()) 
        {
            text += " ";
            keyPressed.push_back(static_cast<int>(Qt::Key_Space));
        }

        // Remove characters when the backspace key is pressed
        if(inputHandler->isKeyPressed(Qt::Key_Backspace) && text.size() > 0 && std::find(keyPressed.begin(), keyPressed.end(), static_cast<int>(Qt::Key_Backspace)) == keyPressed.end()) 
        {
            // If control is pressed remove everything else only remove the last character
            if(ctrlKey)
                text = "";
            else
                text.pop_back();

            keyPressed.push_back(static_cast<int>(Qt::Key_Backspace));
        }

        // Clean up of the press buffer
        for(const auto& key : keyPressed)
            if(!inputHandler->isKeyPressed(static_cast<Qt::Key>(key)))
                keyPressed.erase(std::find(keyPressed.begin(), keyPressed.end(), key));

        // Set the text of the underlying sentence to the current text
        if(previousText != text)
            this->sentence->setText(text);

        // If enter key is pressed, execute the callback
        if(inputHandler->isKeyPressed(Qt::Key_Enter))
            callback(text);
    }

    void TextInput::focus(Input* inputHandler, double)
    {


    }

    void TextInput::unfocus(Input* inputHandler, double)
    {

    }

}