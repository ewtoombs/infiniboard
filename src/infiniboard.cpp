// vi:fo=qacj com=b\://

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <GL/glew.h>  // needed for shaders and shit.
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

#include "helpers.hpp"


// Screen dimension constants
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SCREEN_RATIO ((float)SCREEN_WIDTH / SCREEN_HEIGHT)

float position_data[] = {
         1,  1,    -1,  1,
        -1,  1,    -1, -1,
        -1, -1,     1, -1,
         1, -1,     1,  1
    };
#define N_VERTICES (sizeof(position_data)/sizeof(float) / 2)

// Starts up SDL, creates window, and initialises the SDL- and vendor-specific
// OpenGL state.
bool init();
// Initialises the generic OpenGL state.
bool init_gl();
// Input handler.
void handle_keys(unsigned char key);
// Per-frame actions.
void render();
// Frees media and shuts down SDL.
void close();


// The window we'll be rendering to.
SDL_Window *g_window = NULL;



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

    // Upload the vertex data in position_data to the video device.
    glBufferData(GL_ARRAY_BUFFER, sizeof(position_data),
            position_data, GL_STATIC_DRAW);

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
    GLuint shader_program = glCreateProgram();

    // Pass position_attrib to the "position" input of the vertex shader.  This
    // will associate one 2-vector out of position_data with every vertex the
    // vertex shader processes. That 2-vector gets accessed by the name
    // "position". It could be any name or any data type. It is up to the
    // vertex shader to figure out how to turn that data into a vertex
    // position. In this simple example, it's just a one-to-one mapping.
    glBindAttribLocation(shader_program, position_attrib, "position");

    compile_shaders("shader.vert", "shader.frag", shader_program);

    glLinkProgram(shader_program);


    // Set the colour to be used in all subsequent glClear(GL_COLOR_BUFFER_BIT)
    // commands.
    glClearColor(0, 0, 0, 1);


    // Use the new VBO in subsequent draw calls.
    glBindBuffer(GL_ARRAY_BUFFER, position_vbo);

    // Enable attribute index 0(position_attrib) as being used
    glEnableVertexAttribArray(position_attrib);

    // Use our shader in all subsequent draw calls.
    glUseProgram(shader_program);


    // For all subsequent draw calls, pass SCREEN_RATIO into the uniform vertex
    // shader input, screen_ratio.
    glUniform1f(glGetUniformLocation(shader_program, "screen_ratio"),
            SCREEN_RATIO);


    return true;
}

void handle_keys(SDL_Keycode key)
{
    if (key == SDLK_q) {
        SDL_Event e;
        e.type = SDL_QUIT;
        e.quit.timestamp = time(NULL);
        SDL_PushEvent(&e);
    }
}

void render()
{
    // Clear the screen with the current glClearColor.
    glClear(GL_COLOR_BUFFER_BIT);
    // Draw with the active shader.
    glDrawArrays(GL_LINES, 0, N_VERTICES);
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
        bool quit = false;
        while (!quit) {
            for (;;) {
                // Handle events on queue
                SDL_Event e;
                if (SDL_PollEvent(&e) == 0)
                    break;
                // User requests quit
                if (e.type == SDL_QUIT) {
                    puts("Queue received SDL_QUIT.");
                    quit = true;
                } else if (e.type == SDL_KEYDOWN) {
                    // Handle keypress
                    if (!e.key.repeat)
                        handle_keys(e.key.keysym.sym);
                }
            }

            render();
            
            // Update screen. This is where SDL blocks and spends most of its
            // time, waiting until it can swap buffers.
            SDL_GL_SwapWindow(g_window);
        }
    }

    // Free resources and close SDL
    close();

    return 0;
}
