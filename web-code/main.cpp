// Compile with: em++ main.cpp -O2 -s USE_SDL=2 -s MAX_WEBGL_VERSION=2 -s MIN_WEBGL_VERSION=2 -o index.html

#include <SDL2/SDL.h>
#include <emscripten.h>
#include <GLES3/gl3.h> // Targets OpenGL ES 3.0 / WebGL 2.0
#include <cmath>
#include <cstdio>
#include <iostream>

// Global variables required since Emscripten uses a C-style callback for the main loop
SDL_Window* window = nullptr;
SDL_GLContext glContext;

EM_JS(int, getProgramWidth, (), {
    return Module.programWidth;
});

EM_JS(int, getProgramHeight, (), {
    return Module.programHeight;
});

EM_JS(float, get_dt, (), {
    return Module.dt;
});

EM_JS(float, get_r_multiplier, (), {
    return Module.r_multiplier;
});

float dt = get_dt();
float r_multiplier = get_r_multiplier();

// This function acts as one frame of your game/application loop
void main_loop() {
    dt = get_dt();
    // 1. Handle Events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            emscripten_cancel_main_loop();
            return;
        }
    }

    // 2. Update logic (create a shifting color wave based on time)
    static float time = 0.0f;
    static unsigned int frameCount = 0;
    time += dt;
    std::cout << "dt: " << dt << std::endl;
    std::fflush(stdout);
    float r = (std::sin(time*r_multiplier) + 1.0f) / 2.0f;
    float g = (std::cos(time) + 1.0f) / 2.0f;

    // 3. Render
    glClearColor(r, g, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Swap buffers to display the newly drawn frame
    SDL_GL_SwapWindow(window);
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
void stopProgram() {
    emscripten_cancel_main_loop();

    if (glContext) {
        SDL_GL_DeleteContext(glContext);
        glContext = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}

}

int main(int argc, char* argv[]) {
    // Initialize SDL Video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }

    // Request an OpenGL ES 3.0 Context (maps to WebGL 2)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create the window
    window = SDL_CreateWindow(
        "Colour Change Program Thing",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        getProgramWidth(), getProgramHeight(),
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!window) {
        SDL_Quit();
        return 1;
    }

    // Create the OpenGL context
    glContext = SDL_GL_CreateContext(window);
    
    // Bind the loop function to Emscripten's browser frame dispatcher.
    // 0 = browser decides frame rate (typically matches monitor refresh rate)
    // 1 = simulate infinite loop (recommended for compatibility)
    emscripten_set_main_loop(main_loop, 0, 1);

    // Cleanup (executed if the loop is broken or canceled)
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}