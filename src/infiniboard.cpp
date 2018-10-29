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
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCREEN_ZOOM 0.99f

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
Uint32 push_render_event(Uint32 interval, void *param);
bool tasting(void);


// Globals, prefixed with g_.
SDL_Window *g_window = NULL;
unsigned g_nvertices = 0;
GLuint g_shader_program;
complex<float> g_pan = 0.f;
Uint32 g_render_event;
unsigned g_frame_counter = 0;


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

void push_render_event(void *param)
{
    SDL_Event e;
    e.type = g_render_event;
    SDL_PushEvent(&e);
}
// Give the stew a taste every once in a while.
bool tasting(void)
{
    return g_frame_counter  == 0;
}
int main(int argc, char *argv[])
{
    cout.precision(16);  // Show me all of the digits by default.

    // Start up SDL and create window
    if (!init()) {
        printf("Failed to initialise!\n");
    } else {
        g_render_event = SDL_RegisterEvents(1);
        push_render_event(NULL);

        timer_t render_timer = create_callback_timer(push_render_event, NULL);

        SDL_DisplayMode m;
        SDL_GetCurrentDisplayMode(0, &m);
        double T = 1. / (double)m.refresh_rate;
        printf("T = %5fms\n", T*1000.);

        double t_last_frame = dtime();
        for (;;) {
            // Wait indefinitely until next event.
            double u;
            if (tasting())
                u = dtime();
            SDL_Event e;
            SDL_WaitEvent(&e);
            if (tasting())
                printf("WaitEvent takes %5fms.\n", (dtime() - u)*1000.);

            // User requests quit.
            if (e.type == SDL_QUIT) {
                puts("Queue received SDL_QUIT.");
                break;
            }

            quit_listener(&e);
            pan_listener(&e);

            if (e.type == g_render_event) {
                // We have awoken! It is only 4 milliseconds before the next
                // vsync, and we have got a frame to render!
                double t = dtime();
                if (tasting())
                    printf("Time since last frame: %5fms\n",
                            (t - t_last_frame)*1000.);
                t_last_frame = t;


                // Do all OpenGL drawing commands. 
                if (tasting())
                    t = dtime();
                render();
                glFinish();
                if (tasting())
                    printf("Render takes %5fms.\n", (dtime() - t)*1000.);

                if (tasting())
                    t = dtime();
                // Tell OpenGL that all subsequent openGL commands are to
                // happen after the next buffer swap. Almost never swaps
                // buffers, and in fact, will return immediately, no matter
                // what happens. Will only actually swap buffers if the draw
                // command took longer than the amount of time we gave it to
                // finish.
                SDL_GL_SwapWindow(g_window);
                // Clear the screen with the current glClearColor. OpenGL will
                // block here if the buffers haven't been swapped yet, which is
                // almost always. This command is put here to ensure the
                // buffers are indeed swapped before continuing!
                glClear(GL_COLOR_BUFFER_BIT);
                if (tasting())
                    printf("Swap takes %5fms.\n", (dtime() - t)*1000.);

                // OK, the vsync has like /juuuust/ happened. The buffers have
                // just been swapped for suresiez. Schedule next render just
                // before the next vsync.  Allow some time for rendering.
                timer_settime_d(render_timer, T - 4e-3);
                // Use for testing instead of timer_settime_d(). Schedule next
                // render event immediately, without waiting for all input
                // events.  Mouse lag should increase considerably and all of
                // the idle time should get transferred into the buffer swap.
                //push_render_event(NULL);


                g_frame_counter++;
                g_frame_counter &= 0xff;
            }
        }
    }

    // Free resources and close SDL
    close();

    return 0;
}
