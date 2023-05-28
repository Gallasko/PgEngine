#pragma once

#ifdef __linux__
#include <SDL2/SDL.h>
#elif _WIN32
#include <SDL.h>
#endif

#include <GL/gl.h>
#include <GL/glew.h>

namespace pg
{
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

    public:
        OpenGLBuffer(OpenGLBuffer::Type type) : type(type) {}
        ~OpenGLBuffer() {}

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

    private:
        GLuint buffer;

        OpenGLBuffer::Type type;
    };

}