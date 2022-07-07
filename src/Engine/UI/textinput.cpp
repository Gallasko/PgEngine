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

#include "Input/input.h"

#include "texture.h"
#include "sentencesystem.h"

namespace pg
{
    /**
     * @brief Render function of the text input
     * 
     * @tparam Template specialization of renderer for a text input object
     * @param masterRenderer A pointer to the master renderer
     * @param textInput A pointer to the text input
     */
    template<>
    void renderer(MasterRenderer* masterRenderer, TextInput* textInput)
    {
        if(textInput->texture != nullptr)
            masterRenderer->render(textInput->texture);

        if(textInput->sentence != nullptr)
            masterRenderer->render(textInput->sentence);
    }

    /**
     * @brief Construct a new Text Input object
     * 
     * @param frame The frame where the text input must be rendered
     * @param texture The texture used to render the background of the text input
     * @param fontLoader The font used for the text
     * @param callback The fonction to call when the user press enter after inputing some text
     * @param mode The mode set up for this text input
     */
    TextInput::TextInput(const UiFrame& frame, const std::string& texture, FontLoader* fontLoader, const TextInputCallback& callback, const InputMode& mode) : UiComponent(frame), callback(callback), mode(mode)
    {
        // TODO set TextureComponent with only a frame or a pointer to an UiComponent
        this->texture = new TextureComponent(this->width, this->height, texture);
        this->texture->setTopAnchor(this->top);
        this->texture->setLeftAnchor(this->left);
        
        // TODO Set sentence wihtout a fontloader call a basic font loader 
        this->sentence = new Sentence({"Text"}, 2.0f, fontLoader);
        this->sentence->setTopAnchor(this->texture->top);
        this->sentence->setTopMargin(this->texture->height / 2.0f - this->sentence->height / 2.0f);
        this->sentence->setLeftAnchor(this->left);
        this->sentence->setLeftMargin(10.0f);

        mouseInput = makeMouseArea(this, this, TextInput::focus, TextInput::unfocus);

        keyInput = makeKeyInput(this, changeTextCallback);
    }

    /**
     * @brief Destroy the Text Input object
     */
    TextInput::~TextInput()
    {
        delete texture;
        delete sentence;
    }

    /**
     * @brief Set the underlying texture of the text input
     * 
     * @param texture The name of the texture registered in the renderer
     */
    void TextInput::setTexture(const std::string& texture)
    {
        this->texture->setTexture(texture);
    }

    /**
     * @brief Override of the show function of UiComponent
     * 
     * Used to show this and the texture and sentence of the text input.
     */
    void TextInput::show()
    {
        UiComponent::show();

        texture->show();
        sentence->show();
    }

    /**
     * @brief Override of the hide function of UiComponent
     * 
     * Used to hide this and the texture and sentence of the text input.
     * 
     */
    void TextInput::hide()
    {
        UiComponent::hide();

        texture->hide();
        sentence->hide();
    }

    /**
     * @brief Override of the render function of UiComponent
     * 
     * @param masterRenderer The renderer to render this text input
     */
    void TextInput::render(MasterRenderer *masterRenderer) 
    { 
        renderer(masterRenderer, this);
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
        if(not focused)
            return;
        
        // Store the old text to change visible text only on changed text
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
        if(inputHandler->isKeyPressed(Qt::Key_Enter) || inputHandler->isKeyPressed(Qt::Key_Return))
        {
            focused = false;
            callback(text);
        }
            
    }

    /**
     * @brief Mouse callback to focus this text input
     * 
     * @param inputHandler A pointer to the input handler
     */
    void TextInput::focus(Input* inputHandler, double...)
    {
        if(inputHandler->isButtonPressed(Qt::LeftButton))
        {
            this->focused = true;
        }
    }

    /**
     * @brief Mouse callback to unfocus this text input
     * 
     * @param inputHandler A pointer to the input handler
     */
    void TextInput::unfocus(Input* inputHandler, double)
    {
        if(inputHandler->isButtonPressed(Qt::LeftButton) || inputHandler->isButtonPressed(Qt::RightButton))
        {
            this->focused = false;
        }
    }

}