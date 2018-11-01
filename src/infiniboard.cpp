// vi:fo=qacj com=b\://

#include <stdio.h>

#include <GL/glew.h>  // needed for shaders and shit.
#include <GLFW/glfw3.h>

#include "helpers.hpp"

#include "poincare.hpp"


// Screen dimension constants
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCREEN_ZOOM 0.99f

#define SCREEN_RATIO ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)

enum {
    IDLE,
    PAN,
    DRAW
};

void processEventsFor(double t);

complex<float> screen_to_board(double x, double y);

void error_callback(int error, const char* description);
bool init(void);
bool init_gl();
void key_callback(GLFWwindow *window, int key, int scancode,
        int action, int mods);
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow *window, int button,
        int action, int mods);
void draw(void);
bool tasting(void);


// Globals, prefixed with g_.
GLFWwindow *g_window = NULL;
unsigned g_nvertices = 0;
GLuint g_background_vbo;

GLuint g_poincare_program;
GLuint g_pan_uni;
GLuint g_position_attrib;

complex<float> g_pan = 0.f;
int g_tool = IDLE;
unsigned char g_frame_counter = 0;


// Process events for dt seconds, then return. Should almost always return in
// exactly dt seconds.
void processEventsFor(double dt)
{
    double t0 = glfwGetTime();
    for (;;) {
        glfwWaitEventsTimeout(dt);
        double t1 = glfwGetTime();
        double u = t1 - t0;
        if (u >= dt)
            return;
        dt -= u;
    }
}


// Convert from screen coordinates to (complex) board coordinates.
complex<float> screen_to_board(double x, double y)
{
    return
        (
            (float)x - (float)SCREEN_WIDTH/2.f -
            ((float)y - (float)SCREEN_HEIGHT/2.f) * 1if
        ) / ((float)SCREEN_HEIGHT/2.f) / SCREEN_ZOOM;
}

// Starts up glfw, creates window, and initialises the glfw- and
// vendor-specific OpenGL state.
void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}
bool init(void)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        return false;


    // Make the window.

    // Set OpenGL version.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    g_window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "infiniboard",
            NULL, NULL);
    if (g_window == NULL)
        return false;
    glfwSetKeyCallback(g_window, key_callback);
    glfwSetCursorPosCallback(g_window, cursor_position_callback);
    glfwSetMouseButtonCallback(g_window, mouse_button_callback);
    glfwMakeContextCurrent(g_window);


    // Do the arcane OpenGL badness nobody but GLEW properly understands.
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        printf("Error initialising GLEW! %s\n",
                glewGetErrorString(glewError));
    }

    // Turn on double buffering and vsync. 1 is the maximum time in frames to
    // wait before swapping buffers.
    glfwSwapInterval(1);

    // Initialise OpenGL.
    if (!init_gl()) {
        printf("Unable to initialise OpenGL!\n");
        return false;
    }

    return true;
}

// Initialises the generic OpenGL state.
bool init_gl()
{
    //---- Make the background VBO. ----
    glGenBuffers(1, &g_background_vbo);

    // Make the vertex data.
    complex<float> *background_data;
    poincare::tiling(3, 7, 5, 6, &background_data, &g_nvertices);

    // Upload the vertex data in background_data to the video device.
    glBindBuffer(GL_ARRAY_BUFFER, g_background_vbo);
    glBufferData(GL_ARRAY_BUFFER, g_nvertices*sizeof(complex<float>),
            background_data, GL_STATIC_DRAW);

    // g_background_vbo is ready.


    //---- Create the poincare shader program. ----
    g_poincare_program = glCreateProgram();
    compile_shaders("glsl/poincare.vert", "glsl/white.frag",
            g_poincare_program);
    glLinkProgram(g_poincare_program);


    // Use the poincare shader program in all subsequent draw calls.
    glUseProgram(g_poincare_program);

    g_position_attrib =
        glGetAttribLocation(g_poincare_program, "position");
    // A program's vertex attribute arrays are disabled by default... wth...
    // Well, enable them, then.
    glEnableVertexAttribArray(g_position_attrib);

    g_pan_uni = glGetUniformLocation(g_poincare_program, "pan");

    // For all subsequent draw calls, pass SCREEN_RATIO into the uniform vertex
    // shader input, screen_ratio.
    glUniform1f(glGetUniformLocation(g_poincare_program, "screen_ratio"),
            SCREEN_RATIO);
    glUniform1f(glGetUniformLocation(g_poincare_program, "screen_zoom"),
            SCREEN_ZOOM);


    // Set the colour to be used in all subsequent glClear(GL_COLOR_BUFFER_BIT)
    // commands.
    glClearColor(0, 0, 0, 1);


    return true;
}

// Per-frame actions.
void draw(void)
{
    // Pass g_background_vbo to the "position" input of the vertex shader.
    // This will associate one 2-vector out of background_data with every
    // vertex the vertex shader processes. That 2-vector gets accessed by the
    // name "position". It could be any name or any data type. It is up to the
    // vertex shader to figure out how to turn that data into a vertex
    // position.
    glBindBuffer(GL_ARRAY_BUFFER, g_background_vbo);
    glVertexAttribPointer(g_position_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glUniform2f(g_pan_uni, real(g_pan), imag(g_pan));

    // Draw with the active shader program and its current inputs.
    glDrawArrays(GL_LINES, 0, g_nvertices);
}


void key_callback(GLFWwindow *window, int key, int scancode,
        int action, int mods)
{
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (g_tool == PAN)
        g_pan = screen_to_board(xpos, ypos);
}
void mouse_button_callback(GLFWwindow *window, int button,
        int action, int mods)
{
    if (g_tool == IDLE) {
        if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_MIDDLE) {
            g_tool = PAN;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            g_pan = screen_to_board(xpos, ypos);
        }
    } else {
        if (action == GLFW_RELEASE)
            g_tool = IDLE;
    }
}


// Give the stew a taste every once in a while.
bool tasting(void)
{
    return g_frame_counter == 0;
}
int main(int argc, char *argv[])
{
    cout.precision(16);  // Show me all of the digits by default.

    // Start up glfw and create window.
    if (!init()) {
        printf("Failed to initialise!\n");
    } else {
        const GLFWvidmode *m = glfwGetVideoMode(glfwGetPrimaryMonitor());
        double T = 1. / (double)m->refreshRate;
        printf("T = %5fms\n", T*1000.);

        double t_last_frame = dtime();
        while (!glfwWindowShouldClose(g_window)) {  // once per frame.
            double t_draw = 4e-3;
            double t;
            if (tasting())
                t = dtime();
            // Tell OpenGL that all subsequent OpenGL commands are to happen
            // after the next buffer swap. This will almost never actually swap
            // the buffers, and in fact, will return immediately, no matter
            // what happens.  This will only actually swap buffers if the
            // drawing took longer than the amount of time we gave it to finish
            // and we have missed the vsync.
            glfwSwapBuffers(g_window);
            // Clear the screen with the current glClearColor. OpenGL will
            // block here if the buffers haven't been swapped yet, which is
            // almost always. This command is put here to ensure the buffers
            // are indeed swapped before continuing!
            glClear(GL_COLOR_BUFFER_BIT);
            double t1 = dtime();
            if (tasting())
                printf("Time waiting for vsync: %5fms.\n",
                        (t1 - t)*1000.);
            if (tasting())
                printf("Frame duration: %5fms\n\n",
                        (t1 - t_last_frame)*1000.);
            t_last_frame = t1;
            g_frame_counter++;

            //---------------- ***VSYNC*** ----------------

            // OK, the vsync has like /juuuust/ happened. The buffers have just
            // been swapped for suresiez.  Process events for T - t_draw, so
            // that as many events as possible are used to determine the
            // content of the next frame.
            if (tasting())
                t = dtime();
            processEventsFor(T - t_draw);
            if (tasting())
                printf("processEventsFor takes %5fms.\n", (dtime() - t)*1000.);

            // We have awoken! It is only t_draw seconds before the next
            // vsync, and we have got a frame to render!  Do all OpenGL drawing
            // commands. 
            if (tasting())
                t = dtime();
            draw();
            glFinish();
            if (tasting())
                printf("Draw takes %5fms.\n", (dtime() - t)*1000.);
        }
    }

    // Destroy window
    glfwDestroyWindow(g_window);

    glfwTerminate();

    return 0;
}
