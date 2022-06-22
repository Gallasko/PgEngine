#pragma once

#include "UI/uisystem.h"
#include "UI/listview.h"

namespace pg
{
    namespace editor
    {
        struct OptionTab : public UiComponent
        {
            OptionTab(int width, int height);

            void render(MasterRenderer *masterRenderer) override;

            void show() override;
            void hide() override;

            ListView listView;
            TextureComponent* backgroundTexture;
        };

        template<typename... Args>
        void printOption(OptionTab* tab, const Args&... args);
    }
}