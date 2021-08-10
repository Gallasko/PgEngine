#pragma once

#include <unordered_map>

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
struct Renderer : protected QOpenGLFunctions
{
    Renderer() { initializeOpenGLFunctions(); }
    Renderer(QOpenGLShaderProgram *shaderProgram) : shaderProgram(shaderProgram) { initializeOpenGLFunctions(); }
    Renderer(const Renderer& renderer) : QOpenGLFunctions(), shaderProgram(renderer.shaderProgram) { initializeOpenGLFunctions(); }
    virtual ~Renderer() {}

    virtual void render(std::string...) = 0; 

    QOpenGLShaderProgram *shaderProgram = nullptr;
};

//[TODO] Multiple FBO -> 1 for a whole screen capture and other for batch rendering on a texture 
// Add Particle systeme with instancing already done / create an alternative if needed

class MasterRenderer
{
public:
    template<typename Renderer, typename... Args>
    void registerRederer(Args... args) { auto rendererName = typeid(Renderer).name(); rendererList[rendererName] = new Renderer(args...); }

    template<typename Renderer, typename... Args>
    void render(Args... args) { auto rendererName = typeid(Renderer).name(); if(rendererList.find(rendererName) != rendererList.end()) rendererList[rendererName]->render(rendererName, screenWidth, screenHeight, args...); }

    inline void setWindowSize(int width, int heigth) { screenWidth = width; screenHeight = heigth; }

private:
    std::unordered_map<std::string, Renderer*> rendererList;

    int screenWidth = 1, screenHeight = 1;  
};