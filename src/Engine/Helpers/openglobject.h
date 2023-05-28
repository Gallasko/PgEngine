#pragma once

#ifdef __linux__
#include <SDL2/SDL.h>
#elif _WIN32
#include <SDL.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>

#include <glm.hpp>

#include <string>

namespace pg
{
    class OpenGLShaderProgram
    {
    public:
        unsigned int ID;
        // constructor generates the shader on the fly
        // ------------------------------------------------------------------------
        OpenGLShaderProgram(const char* vertexPath, const char* fragmentPath);
        // activate the shader
        // ------------------------------------------------------------------------
        void bind() const;
        // activate the shader
        // ------------------------------------------------------------------------
        void release() const;
        // utility uniform functions
        // ------------------------------------------------------------------------
        void setUniformValue(const std::string &name, bool value) const;
        // ------------------------------------------------------------------------
        void setUniformValue(const std::string &name, int value) const;
        // ------------------------------------------------------------------------
        void setUniformValue(const std::string &name, float value) const;
        // ------------------------------------------------------------------------
        void setUniformValue(const std::string &name, const glm::vec2 &value) const;
        void setUniformValue(const std::string &name, float x, float y) const;
        // ------------------------------------------------------------------------
        void setUniformValue(const std::string &name, const glm::vec3 &value) const;
        void setUniformValue(const std::string &name, float x, float y, float z) const;
        // ------------------------------------------------------------------------
        void setUniformValue(const std::string &name, const glm::vec4 &value) const;
        void setUniformValue(const std::string &name, float x, float y, float z, float w) const;
        // ------------------------------------------------------------------------
        void setUniformValue(const std::string &name, const glm::mat2 &mat) const;
        // ------------------------------------------------------------------------
        void setUniformValue(const std::string &name, const glm::mat3 &mat) const;
        // ------------------------------------------------------------------------
        void setUniformValue(const std::string &name, const glm::mat4 &mat) const;
    private:
        // utility function for checking shader compilation/linking errors.
        // ------------------------------------------------------------------------
        void checkCompileErrors(GLuint shader, std::string type);
    };

    class OpenGLVertexArrayObject
    {
    public:
        OpenGLVertexArrayObject() {}
        ~OpenGLVertexArrayObject() {}

        inline void create()
        {
            /* Allocate and assign a Vertex Array Object to our handle */
            glGenVertexArrays(1, &vao);
        }

        inline void bind()
        {
            /* Bind our Vertex Array Object as the current used object */
            glBindVertexArray(vao);
        }

        inline void release()
        {
            glDeleteVertexArrays(1, &vao);
        }

    private:
        GLuint vao;
    };

    class OpenGLBuffer
    {
    public:
        enum Type
        {
            VertexBuffer        = GL_ARRAY_BUFFER,
            IndexBuffer         = GL_ELEMENT_ARRAY_BUFFER,
            PixelPackBuffer     = GL_PIXEL_PACK_BUFFER,
            PixelUnpackBuffer   = GL_PIXEL_UNPACK_BUFFER
        };

        enum UsagePattern
        {
            StreamDraw          = GL_STREAM_DRAW,
            StreamRead          = GL_STREAM_READ,
            StreamCopy          = GL_STREAM_COPY,
            StaticDraw          = GL_STATIC_DRAW,
            StaticRead          = GL_STATIC_READ,
            StaticCopy          = GL_STATIC_COPY,
            DynamicDraw         = GL_DYNAMIC_DRAW,
            DynamicRead         = GL_DYNAMIC_READ,
            DynamicCopy         = GL_DYNAMIC_COPY
        };

        enum Access
        {
            ReadOnly            = GL_READ_ONLY,
            WriteOnly           = GL_WRITE_ONLY,
            ReadWrite           = GL_READ_WRITE
        };

        enum RangeAccessFlag
        {
            RangeRead             = GL_MAP_READ_BIT,
            RangeWrite            = GL_MAP_WRITE_BIT,
            RangeInvalidate       = GL_MAP_INVALIDATE_RANGE_BIT,
            RangeInvalidateBuffer = GL_MAP_INVALIDATE_BUFFER_BIT,
            RangeFlushExplicit    = GL_MAP_FLUSH_EXPLICIT_BIT,
            RangeUnsynchronized   = GL_MAP_UNSYNCHRONIZED_BIT
        };

    public:
        OpenGLBuffer(OpenGLBuffer::Type type) : type(type), usagePattern(UsagePattern::StaticDraw) {}
        ~OpenGLBuffer() {}

        inline OpenGLBuffer::UsagePattern getUsagePattern() const { return usagePattern; }

        void setUsagePattern(OpenGLBuffer::UsagePattern value)
        {
            usagePattern = value;
        }

        inline void create()
        {
            /* Allocate and assign a Vertex Array Object to our handle */
            glGenBuffers(1, &buffer);
        }

        inline void bind()
        {
            /* Bind our Vertex Array Object as the current used object */
            glBindBuffer(type, buffer);
        }

        inline void release()
        {
            glDeleteBuffers(1, &buffer);
        }

        void allocate(const void *data, int count)
        {
            glBindBuffer(type, buffer);
            glBufferData(type, count, data, usagePattern);
        }

        inline void allocate(int count) { allocate(nullptr, count); }

    private:
        GLuint buffer;

        OpenGLBuffer::Type type;
        OpenGLBuffer::UsagePattern usagePattern;
    };

}