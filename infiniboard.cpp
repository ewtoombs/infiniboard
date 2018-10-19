#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

#include "helpers.hpp"

// Screen dimension constants
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SCREEN_RATIO ((float)SCREEN_WIDTH / SCREEN_HEIGHT)

float position_data[] = {
         0.f,  1.f,  0.f,
        -1.f, -1.f,  0.f,
         1.f, -1.f,  0.f
    };
#define N_VERTICES (sizeof(position_data)/sizeof(float) / 3)

// Starts up SDL, creates window, and initialises OpenGL
bool init();

// Initialises matrices and clear color
bool initGL();

// Input handler
void handleKeys(unsigned char key);

void render();

// Frees media and shuts down SDL
void close();

// The window we'll be rendering to
SDL_Window *gWindow = NULL;

// OpenGL context
SDL_GLContext gContext;

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
    gWindow = SDL_CreateWindow("infiniboard", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (gWindow == NULL) {
        printf("Window could not be created! SDL Error: %s\n",
                SDL_GetError());
        return false;
    }

    // Create context
    gContext = SDL_GL_CreateContext(gWindow);
    if (gContext == NULL) {
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
    if (!initGL()) {
        printf("Unable to initialise OpenGL!\n");
        return false;
    }
    return true;
}

bool initGL()
{
    //---- Make the VBO ----
    GLuint triangleVBO;
    glGenBuffers(1, &triangleVBO);

    // Make the new VBO active.
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);

    // Upload vertex data to the video device.  These are the vertices of a
    // triangle (counter-clockwise winding).
    glBufferData(GL_ARRAY_BUFFER, sizeof(position_data),
            position_data, GL_STATIC_DRAW);

    // Specify that our coordinate data is going into attribute index 0
    // (position_attrib), and contains three floats per vertex.
    const unsigned int position_attrib = 0;
    glVertexAttribPointer(position_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);



    // Enable attribute index 0(position_attrib) as being used
    glEnableVertexAttribArray(position_attrib);

    // Make the new VBO active. (Shouldn't I be unbinding?)
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);


    //---- Create the shader program. ----
    GLuint shaderProgram = glCreateProgram();

    // Compile and attach the vertex shader.
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    char *source = load("shader.vert");
    glShaderSource(vertexShader, 1, &source, 0);
    glCompileShader(vertexShader);
    glAttachShader(shaderProgram, vertexShader);

    // Compile and attach the fragment shader.
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    source = load("shader.frag");
    glShaderSource(fragmentShader, 1, &source, 0);
    glCompileShader(fragmentShader);
    glAttachShader(shaderProgram, fragmentShader);

    // Bind position_data to position. "position" will represent
    // position_data in the vertex shader source.
    glBindAttribLocation(shaderProgram, position_attrib, "position");

    glLinkProgram(shaderProgram);


    // Set the colour to be used in all subsequent glClear(GL_COLOR_BUFFER_BIT)
    // commands.
    glClearColor(0.f, 0.f, 0.f, 1.f);

    // Use our shader in all subsequent draw calls.
    glUseProgram(shaderProgram);

    return true;
}

void handleKeys(SDL_Keycode key)
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
    glDrawArrays(GL_TRIANGLES, 0, N_VERTICES);
}

void close()
{
    // Destroy window   
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

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
                        handleKeys(e.key.keysym.sym);
                }
            }

            render();
            
            // Update screen. This is where SDL blocks and spends most of its
            // time, waiting until it can swap buffers.
            SDL_GL_SwapWindow(gWindow);
        }
    }

    // Free resources and close SDL
    close();

    return 0;
}
