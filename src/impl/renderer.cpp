/**
 * @author 60Trees_ (github.com/60Trees)
 * @version 1.0.0
 */

#include <base/renderer.hpp>
#include <base/input_handler.hpp>
#include <base/fps_handler.hpp>

#include <iostream>
#include <vector>

#include <opengl.hpp>

/// Put the setup code in here
struct WorkingRenderer : Renderer {
    void init() override {
        tassertmsg(SDL_Init(SDL_INIT_VIDEO), SDL_GetError());

        set_opengl_version();

        window = SDL_CreateWindow(parent->name.c_str(), 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        tassertmsg(window, SDL_GetError());

        glcontext = SDL_GL_CreateContext(window);
        tassertmsg(glcontext, SDL_GetError());

        SDL_GetWindowSizeInPixels(window, &window_size.x, &window_size.y);

#ifndef __EMSCRIPTEN__
        int loaded = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
        std::cout << "glad load result: " << loaded << "\n";
        if (!loaded) {
            std::cerr << "GLAD FAILED TO LOAD\n";
            std::abort();
        }
#endif

#ifdef DO_GL_DEBUG
        opengl::enable_debugging();
#endif

        {
            GLint count = 0;
            glGetIntegerv(GL_NUM_EXTENSIONS, &count);

            gl_extensions.clear();
            gl_extensions.reserve(count);

            for (GLint i = 0; i < count; ++i) {
                gl_extensions.emplace(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)));
            }
        }

        welcome_message();
    }

    void loop() override {
        glClearColor(1.f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window);
    }

    void welcome_message() {
        std::cout << "OpenGL " << glGetString(GL_VERSION) << ":\n";
        std::cout << "- Vendor: " << glGetString(GL_VENDOR);
        if (std::string_view(reinterpret_cast<const char*>(glGetString(GL_VENDOR))) == "Mozilla")
            std::cout << " (firefox is the best)\n";
        else
            std::cout << '\n';
        std::cout << "- Renderer: " << glGetString(GL_RENDERER) << '\n';
        std::cout << "- GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
        std::cout << "- Has " << gl_extensions.size() << " extensions.\n";
    }

    void quit() override {
        if (glcontext) {
            SDL_GL_DestroyContext(glcontext);
            glcontext = nullptr;
        }

        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }

        SDL_Quit();
    }

    ~WorkingRenderer() override { quit(); }

    bool should_resize = true;
    glm::vec<2, int> window_size;
};
struct WindowResizeHandler : InputHandler {
    void digest_event(const SDL_Event& e) {
        if (e.type != SDL_EVENT_WINDOW_RESIZED) return;
        auto& r = parent->get<WorkingRenderer>();
        r.should_resize = true;
        r.window_size.x = e.window.data1;
        r.window_size.y = e.window.data2;
    }
} REGISTER_MOD(WindowResizeHandler);

#define UNIFORM_DEF(name, func)                  \
    Uniform {                                    \
        name, [](Uniform& u, typeof(*this)& r) { \
            const auto b = u.base.id;            \
            func;                                \
        }                                        \
    }
#define ADD_UNIFORM(name, func) uniforms.push_back(UNIFORM_DEF(name, func))

/// Put the game code in here
struct GameRenderer : WorkingRenderer {
    FpsHandler* fps;

    struct Uniform {
        std::string name;
        void (*update)(Uniform&, GameRenderer&);
        opengl::ShaderUniform base;
    };

    // clang-format off
    std::vector<float> vertices = {
       -1.0f, -1.0f,  0.0f,
        1.0f, -1.0f,  0.0f,
        1.0f,  1.0f,  0.0f,
        1.0f,  1.0f,  0.0f,
       -1.0f,  1.0f,  0.0f,
       -1.0f, -1.0f,  0.0f,
    };
    // clang-format on

    std::vector<Uniform> uniforms;

    opengl::ShaderProgram shaderprogram;
    opengl::VBO vbo;
    opengl::VAO vao;

    void init() override {
        WorkingRenderer::init();

        fps = &parent->get<FpsHandler>();

        // #ifndef __EMSCRIPTEN__
        //         if (has_extension("GL_ARB_gpu_shader_int64") || has_extension("GL_AMD_gpu_shader_int64"))
        //             ADD_UNIFORM("u_time", glUniform1ui64NV(b, r.fps->ticks));
        //         else
        // #endif
        ADD_UNIFORM("u_time", glUniform1ui(b, static_cast<unsigned int>(r.fps->ticks)));

        ADD_UNIFORM("u_windowsize", glUniform2i(b, r.window_size.x, r.window_size.y));

        vao.create();
        vao.bind();

        vbo.create();
        vbo.bind();
        vbo.data<float>(vertices, GL_STATIC_DRAW);

        vao.vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        vao.enableAttribute(0);

        opengl::Shader vert{GL_VERTEX_SHADER};
        vert.create();
        vert.source_from_path("assets/vert.glsl");
        vert.compile();

        opengl::Shader frag{GL_FRAGMENT_SHADER};
        frag.create();
        frag.source_from_path("assets/frag.glsl");
        frag.compile();

        shaderprogram.create();

        shaderprogram.attach(vert);
        shaderprogram.attach(frag);

        shaderprogram.link();

        for (auto& uniform : uniforms) {
            uniform.base.id = shaderprogram.id;
            uniform.base.init(uniform.name.c_str());
        }
    }
    void loop() override {
        typeof(*this)& hi = *this;
        glClearColor(1.f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderprogram.use();

        for (auto& uniform : uniforms) uniform.update(uniform, *this);

        vao.bind();

        glDrawArrays(GL_TRIANGLES, 0, 3);

        if (should_resize) {
            glViewport(0, 0, window_size.x, window_size.y);
            should_resize = false;
        }
        SDL_GL_SwapWindow(window);
    }

    void quit() override { WorkingRenderer::quit(); }
} REGISTER_MOD(GameRenderer);
