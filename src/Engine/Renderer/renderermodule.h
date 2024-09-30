#pragma once

#include "renderer.h"

#include "Interpreter/pginterpreter.h"

namespace pg
{
    class RegisterShaderFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(MasterRenderer *renderer)
        {
            setArity(3, 3);

            masterRenderer = renderer;
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto name = args.front()->getElement();
            args.pop();

            auto vsPath = args.front()->getElement();
            args.pop();

            auto fsPath = args.front()->getElement();
            args.pop();

            if(not name.isLitteral() and not fsPath.isLitteral() and not vsPath.isLitteral())
            {
                LOG_ERROR("Register Shader Function", "Received wrong kind of parameters");
                return nullptr;
            }

            LOG_INFO("Register Shader Function", "Register Shader: " << name.toString());

            masterRenderer->registerShader(name.toString(), vsPath.toString(), fsPath.toString());

            return nullptr; 
        }

        MasterRenderer *masterRenderer;
    };

    class RegisterTextureFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(MasterRenderer *renderer)
        {
            // Todo make the type of the texture as an optional arg
            // setArity(2, 3);
            setArity(2, 3);

            masterRenderer = renderer;
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto name = args.front()->getElement();
            args.pop();

            auto path = args.front()->getElement();
            args.pop();

            // Todo add an argument to specify the type of texture loaded, e.g.: RGBA, RGB, ...
            // if(not args.empty())
            // auto type = args.front()->getElement();
            // args.pop();
            // TextureType tType;
            // if (type == "RGBA") tType == "RGBA";

            // if(not name.isLitteral() and not path.isLitteral() and not type.isLitteral())
            if(not name.isLitteral() and not path.isLitteral())
            {
                LOG_ERROR("Register Shader Function", "Received wrong kind of parameters");
                return nullptr;
            }

            masterRenderer->registerTexture(name.toString(), path.toString().c_str());

            return nullptr; 
        }

        MasterRenderer *masterRenderer;
    };

    class RegisterAtlasTextureFunction : public Function
    {
        using Function::Function;
    public:
        void setUp(MasterRenderer *renderer)
        {
            // Todo make the type of the texture as an optional arg
            // setArity(2, 3);
            setArity(3, 4);

            masterRenderer = renderer;
        }

        virtual ValuablePtr call(ValuableQueue& args) override
        {
            auto name = args.front()->getElement();
            args.pop();

            auto texturePath = args.front()->getElement();
            args.pop();

            auto atlasPath = args.front()->getElement();
            args.pop();

            // Todo add an argument to specify the type of texture loaded, e.g.: RGBA, RGB, ...
            // if(not args.empty())
            // auto type = args.front()->getElement();
            // args.pop();
            // TextureType tType;
            // if (type == "RGBA") tType == "RGBA";

            // if(not name.isLitteral() and not path.isLitteral() and not type.isLitteral())
            if(not name.isLitteral() and not texturePath.isLitteral() and not atlasPath.isLitteral())
            {
                LOG_ERROR("Register Shader Function", "Received wrong kind of parameters");
                return nullptr;
            }

            masterRenderer->registerAtlasTexture(name.toString(), texturePath.toString().c_str(), atlasPath.toString().c_str());

            return nullptr; 
        }

        MasterRenderer *masterRenderer;
    };

    struct RendererModule : public SysModule
    {
        RendererModule(MasterRenderer *masterRenderer)
        {            
            addSystemFunction<RegisterShaderFunction>("loadShader", masterRenderer);
            addSystemFunction<RegisterTextureFunction>("loadTexture", masterRenderer);
            addSystemFunction<RegisterAtlasTextureFunction>("loadAtlasTexture", masterRenderer);
        }

        MasterRenderer *masterRenderer;
    };

}