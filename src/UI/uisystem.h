#pragma once

#include <algorithm>
#include <vector>

struct UiComponent
{
    bool visible = true;

    int x = 0;
    int y = 0;
    int z = 0; // stack indice

    int width = 0;
    int height = 0;
    float scale = 1.0f;

    UiComponent *topAnchor = nullptr;
    UiComponent *rightAnchor = nullptr;
    UiComponent *bottomAnchor = nullptr;
    UiComponent *leftAnchor = nullptr;

    int topMargin = 0;
    int rightMargin = 0;
    int bottomMargin = 0;
    int leftMargin = 0;

    std::vector<UiComponent*> children;

    void inline setX(int value) { x = value; update(); }
    void inline setY(int value) { y = value; update(); }
    void inline setZ(int value) { z = value; update(); }
    void inline setWidth(int value) { width = value; update(); }
    void inline setHeight(int value) { height = value; update(); }
    void inline setTopMargin(int value) { topMargin = value; update(); }
    void inline setRightMargin(int value) { rightMargin = value; update(); }
    void inline setBottomMargin(int value) { bottomMargin = value; update(); }
    void inline setLeftMargin(int value) { leftMargin = value; update(); }

    void inline setTopAnchor(UiComponent* component) { topAnchor = component; auto it = std::find(component->children.begin(), component->children.end(), this); if(it == component->children.end()) component->children.push_back(this); update(); }
    void inline setRightAnchor(UiComponent* component) { rightAnchor = component; auto it = std::find(component->children.begin(), component->children.end(), this); if(it == component->children.end()) component->children.push_back(this); update(); }
    void inline setBottomAnchor(UiComponent* component) { bottomAnchor = component; auto it = std::find(component->children.begin(), component->children.end(), this); if(it == component->children.end()) component->children.push_back(this); update(); }
    void inline setLeftAnchor(UiComponent* component) { leftAnchor = component; auto it = std::find(component->children.begin(), component->children.end(), this); if(it == component->children.end()) component->children.push_back(this); update(); }
    
    void update();
};
