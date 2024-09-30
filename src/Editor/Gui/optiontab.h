/**
 * @file optiontab.h
 * @author Pigeon Codeur (pigeoncodeur@gmail.com)
 * @brief Definition of the option tab.
 * @version 0.1
 * @date 2022-07-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#pragma once

#include "UI/uisystem.h"
// #include "UI/listview.h"

namespace pg
{
    // Type forwarding
    struct Texture2DComponent;
    
    namespace editor
    {
        struct OptionTab : public UiComponent
        {
            // Todo set with Uisize or Uiframe
            OptionTab(int width, int height);

            void clear();

            void render(MasterRenderer *masterRenderer) override;

            void show() override;
            void hide() override;

            template <class T>
            void addTextInput(const std::string& text, T* value)
            {
                
            }

            void addTextInput(const std::string& text, const std::function<void(const std::string&)>& callback)
            {
                //std::make_shared<>
            }

            // ListView listView;
            Texture2DComponent* backgroundTexture;
        };

        template <typename... Args>
        void printOption(OptionTab* tab, Args... args);
    }
}