#include "optiontab.h"

#include "UI/texture.h"
#include "UI/textinput.h"

#include "Renderer/renderer.h"

namespace pg
{
    template<>
    void renderer(MasterRenderer *masterRenderer, pg::editor::OptionTab* optionTab)
    {
        optionTab->backgroundTexture->render(masterRenderer);
        optionTab->listView.render(masterRenderer);
    }

    namespace editor
    {
        template<>
        void printOption(OptionTab *tab, UiComponent* component)
        {

        }

        template<>
        void printOption(OptionTab* tab, TextInput* textInput)
        {
            // tab->printOption<UiComponent*>(tab, textInput);

        }


        OptionTab::OptionTab(int width, int height) : listView(this->frame)
        {
            this->width = width;
            this->height = height;
            
            this->backgroundTexture = new TextureComponent(this->width, this->height, "TabTexture");
            this->backgroundTexture->setTopAnchor(this->top);
            this->backgroundTexture->setLeftAnchor(this->left);
            this->backgroundTexture->setRightAnchor(this->right);
            this->backgroundTexture->setBottomAnchor(this->bottom);

            listView.setTopAnchor(this->top);
            listView.setLeftAnchor(this->left);
            listView.setRightAnchor(this->right);
            listView.setBottomAnchor(this->bottom);
        }

        void OptionTab::clear()
        {
            listView.clear();
        }

        void OptionTab::render(MasterRenderer* masterRenderer)
        {
            renderer(masterRenderer, this);
        }

        void OptionTab::show()
        {
            UiComponent::show();

            listView.show();
            backgroundTexture->show();
        }

        void OptionTab::hide()
        {
            UiComponent::hide();

            listView.hide();
            backgroundTexture->hide();
        }
    }
}