#pragma once

#ifdef HAVE_OPENGL

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include <SDL_opengles2.h>
#else
#include <GL/gl3w.h>
#include <SDL_opengl.h>
#endif

#include "Video.h"
#include "SDL20.h"

struct SDLGLContextDeleter { void operator()(SDL_GLContext context) { SDL_GL_DeleteContext(context); } };
using unique_sdl_gl_context = unique_resource<SDL_GLContext, nullptr, SDLGLContextDeleter>;

struct GLProgramDeleter { void operator()(GLuint program) { glDeleteProgram(program); } };
using unique_gl_program = unique_resource<GLuint, 0, GLProgramDeleter>;

struct GLTextureDeleter { void operator()(GLuint texture) { glDeleteTextures(1, &texture); } };
using unique_gl_texture = unique_resource<GLuint, 0, GLTextureDeleter>;

struct GLFramebufferDeleter { void operator()(GLuint fbo) { glDeleteFramebuffers(1, &fbo); } };
using unique_gl_framebuffer = unique_resource<GLuint, 0, GLFramebufferDeleter>;

struct GLVertexArrayDeleter { void operator()(GLuint vao) { glDeleteVertexArrays(1, &vao); } };
using unique_gl_vertexarray = unique_resource<GLuint, 0, GLVertexArrayDeleter>;

struct GLVertexBufferDeleter { void operator()(GLuint vbo) { glDeleteBuffers(1, &vbo); } };
using unique_gl_vertexbuffer = unique_resource<GLuint, 0, GLVertexBufferDeleter>;


class SDL_GL3 final : public IVideoBase
{
public:
    SDL_GL3() = default;
    ~SDL_GL3();

public:
    bool Init() override;
    Rect DisplayRect() const override;
    void ResizeWindow(int height) const override;
    std::pair<int, int> MouseRelative() override;
    void OptionsChanged() override;
    void Update(const FrameBuffer& fb) override;

protected:
    void UpdatePalette();
    void ResizeSource(int width, int height);
    void ResizeTarget(int width, int height);
    void ResizeIntermediate(bool smooth);
    bool DrawChanges(const FrameBuffer& fb);
    void Render();
    GLuint MakeProgram(const std::string& vs_code, const std::string& fs_code);
    void SaveWindowPosition();
    void RestoreWindowPosition();

private:
    unique_sdl_window m_window;
    unique_sdl_gl_context m_context;

    unique_gl_program m_palette_program{};
    unique_gl_program m_aspect_program{};
    unique_gl_program m_blend_program{};

    GLint m_uniform_tex_output{};

    unique_gl_texture m_texture_palette{};
    unique_gl_texture m_texture_screen{};
    unique_gl_texture m_texture_scaled{};
    unique_gl_texture m_texture_output{};
    unique_gl_texture m_texture_prev_output{};

    unique_gl_vertexarray m_vao{};
    unique_gl_framebuffer m_fbo{};        // FBO for palette pass (→ m_texture_scaled)
    unique_gl_framebuffer m_fbo_blend{};  // FBO for blend pass (→ ping-pong output)

    SDL_Rect m_rSource{};
    SDL_Rect m_rIntermediate{};
    SDL_Rect m_rTarget{};
    SDL_Rect m_rDisplay{};

    bool m_smooth{ true };
};

#endif // HAVE_OPENGL
