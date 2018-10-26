// vi:fo=qacj com=b\://

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <GL/glew.h>  // needed for shaders and shit.
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

#include "helpers.hpp"

#include "poincare.hpp"


// Screen dimension constants
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SCREEN_ZOOM 0.7f

#define SCREEN_RATIO ((float)SCREEN_WIDTH / SCREEN_HEIGHT)

// Convert from screen coordinates to (complex) board coordinates.
complex<float> screen_to_board(Sint32 x, Sint32 y);

// Starts up SDL, creates window, and initialises the SDL- and vendor-specific
// OpenGL state.
bool init();
// Initialises the generic OpenGL state.
bool init_gl();
void quit_listener(SDL_Event *e);
void pan_listener(SDL_Event *e);
// Per-frame actions.
void render();
// Frees media and shuts down SDL.
void close();


// The window we'll be rendering to.
SDL_Window *g_window = NULL;
unsigned g_nvertices = 0;
GLuint g_shader_program;
complex<float> g_pan = 0.f;


complex<float> screen_to_board(Sint32 x, Sint32 y)
{
    return
        (
            (float)(x - SCREEN_WIDTH/2) -
            (float)(y - SCREEN_HEIGHT/2) * 1if
        ) / (float)(SCREEN_HEIGHT/2) / SCREEN_ZOOM;
}

bool init()
{
    // Initialise SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) < 0) {
        printf("SDL could not initialise! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    // Use OpenGL 2.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    // Create window
    g_window = SDL_CreateWindow("infiniboard", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (g_window == NULL) {
        printf("Window could not be created! SDL Error: %s\n",
                SDL_GetError());
        return false;
    }

    // Create context
    SDL_GLContext context = SDL_GL_CreateContext(g_window);
    if (context == NULL) {
        printf("OpenGL context could not be created! SDL Error: %s\n",
                SDL_GetError());
        return false;
    }

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        printf("Error initialising GLEW! %s\n",
                glewGetErrorString(glewError));
    }

    // Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n",
                SDL_GetError());
    }

    // Initialise OpenGL
    if (!init_gl()) {
        printf("Unable to initialise OpenGL!\n");
        return false;
    }
    return true;
}

bool init_gl()
{
    //---- Make the VBO and the containing vertex attribute array. ----
    GLuint position_vbo;
    glGenBuffers(1, &position_vbo);

    // Make the new VBO active.
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo);

    // Make the vertex data.
    complex<float> *background_data;
    poincare::tiling(4, 5, 5, 4, &background_data, &g_nvertices);

    // Upload the vertex data in background_data to the video device.
    glBufferData(GL_ARRAY_BUFFER, g_nvertices*sizeof(complex<float>),
            background_data, GL_STATIC_DRAW);

    // Define a vertex attribute array as follows. Each element of the array is
    // a 2-dimensional vector of GL_FLOATS. The underlying data is the
    // currently bound buffer (position_vbo). Store this defining information
    // in position_attrib (index 0). These arrays are used as inputs and
    // outputs for vertex shaders.
    const unsigned int position_attrib = 0;
    glVertexAttribPointer(position_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // position_vbo and position_attrib are ready. Unbind the VBO.
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    //---- Create the shader program. ----
    g_shader_program = glCreateProgram();

    // Pass position_attrib to the "position" input of the vertex shader.  This
    // will associate one 2-vector out of background_data with every vertex the
    // vertex shader processes. That 2-vector gets accessed by the name
    // "position". It could be any name or any data type. It is up to the
    // vertex shader to figure out how to turn that data into a vertex
    // position.
    glBindAttribLocation(g_shader_program, position_attrib, "position");

    compile_shaders("glsl/shader.vert", "glsl/shader.frag", g_shader_program);

    glLinkProgram(g_shader_program);


    // Set the colour to be used in all subsequent glClear(GL_COLOR_BUFFER_BIT)
    // commands.
    glClearColor(0, 0, 0, 1);


    // Use the new VBO in subsequent draw calls.
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo);

    // Enable attribute index 0(position_attrib) as being used
    glEnableVertexAttribArray(position_attrib);

    // Use our shader in all subsequent draw calls.
    glUseProgram(g_shader_program);


    // For all subsequent draw calls, pass SCREEN_RATIO into the uniform vertex
    // shader input, screen_ratio.
    glUniform1f(glGetUniformLocation(g_shader_program, "screen_ratio"),
            SCREEN_RATIO);
    glUniform1f(glGetUniformLocation(g_shader_program, "screen_zoom"),
            SCREEN_ZOOM);


    return true;
}

void quit_listener(SDL_Event *e)
{
    if (e->type == SDL_KEYDOWN) {
        if (!e->key.repeat && e->key.keysym.sym == SDLK_q) {
            SDL_Event qe;
            qe.type = SDL_QUIT;
            qe.quit.timestamp = time(NULL);
            SDL_PushEvent(&qe);
        }
    }
}
void pan_listener(SDL_Event *e)
{
    static bool dragging = false;
    if (!dragging) {
        if (e->type == SDL_MOUSEBUTTONDOWN) {
            dragging = true;
            g_pan = screen_to_board(e->button.x, e->button.y);
        }
    } else {
        if (e->type == SDL_MOUSEMOTION)
            g_pan = screen_to_board(e->motion.x, e->motion.y);
        if (e->type == SDL_MOUSEBUTTONUP)
            dragging = false;
    }
}


void render()
{
    glUniform2f(glGetUniformLocation(g_shader_program, "pan"),
            real(g_pan), imag(g_pan));

    // Clear the screen with the current glClearColor.
    glClear(GL_COLOR_BUFFER_BIT);
    // Draw with the active shader.
    glDrawArrays(GL_LINES, 0, g_nvertices);
}

void close()
{
    // Destroy window
    SDL_DestroyWindow(g_window);

    // Quit SDL subsystems
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    // Start up SDL and create window
    if (!init()) {
        printf("Failed to initialise!\n");
    } else {
        for (;;) {
            for (;;) {
                // Handle events on queue
                SDL_Event e;
                if (SDL_PollEvent(&e) == 0)
                    break;
                // User requests quit
                if (e.type == SDL_QUIT) {
                    puts("Queue received SDL_QUIT.");
                    goto break_outer;
                }
                quit_listener(&e);
                pan_listener(&e);
            }

            render();

            // Update screen. This is where SDL blocks and spends most of its
            // time, waiting until it can swap buffers.
            SDL_GL_SwapWindow(g_window);
        }
    }
break_outer:

    // Free resources and close SDL
    close();

    return 0;
}
