#include <cstdio>
#include <cstring>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

// Screen dimension constants
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

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

// Render flag
bool gRenderQuad = true;

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
    // Initialise Projection Matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Check for error
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("Error initializing OpenGL! %s\n", gluErrorString(error));
        return false;
    }

    // Initialise Modelview Matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Check for error
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("Error initializing OpenGL! %s\n", gluErrorString(error));
        return false;
    }
    
    // Initialise clear color
    glClearColor(0.f, 0.f, 0.f, 1.f);
    
    // Check for error
    error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("Error initializing OpenGL! %s\n", gluErrorString(error));
        return false;
    }
    return true;
}

void handleKeys(SDL_Keycode key)
{
    // Toggle quad
    if (key == SDLK_q) {
        gRenderQuad = !gRenderQuad;
    }
}

void render()
{
    // Clear color buffer
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Render quad
    if (gRenderQuad) {
        glBegin(GL_QUADS);
            glVertex2f(-0.5f, -0.5f);
            glVertex2f(0.5f, -0.5f);
            glVertex2f(0.5f, 0.5f);
            glVertex2f(-0.5f, 0.5f);
        glEnd();
    }
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
