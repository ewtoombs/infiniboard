// vi:fo=qacj com=b\://

#include <stdio.h>
#include <assert.h>

#include <vector>

#include <GL/glew.h>  // needed for shaders and shit.
#include <GLFW/glfw3.h>

#include "helpers.hpp"

#include "poincare.hpp"


#define T_RENDER 10e-3

// Screen dimension constants
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 700
#define SCREEN_ZOOM 0.99f

#define GRID_SHADE 0.2f

// 16 MiB of space for drawing in should be fine until I can work out the
// details of memory management. Actually, realistically, it should be fine for
// as long as I don't have board saving and loading working, because that's the
// only conceivable way 16 MiB could ever get eaten up by drawing.
#define DRAW_SPACE (16*MiB)
#define LINE_WIDTH 0.01f

#define SCREEN_RATIO ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)

enum {  // mouse states
    IDLE,
    PAN,
    DRAW
};

void process_events_for(double t);

complex<float> screen_to_board(complex<float> s);

void error_callback(int error, const char* description);
bool init(void);
bool init_gl();
void key_callback(GLFWwindow *window, int key, int scancode,
        int action, int mods);
void cursor_position_callback(GLFWwindow *window, double sx, double sy);
void mouse_button_callback(GLFWwindow *window, int button,
        int action, int mods);
void mouse_draw_start(complex<float> p0, complex<float> p1);
void mouse_draw(complex<float> p0, complex<float> p1, complex<float> p2);
void mouse_draw_finish(void);
void refresh_foreground(void);
void render(void);
bool tasting(void);


// Globals, prefixed with g_.
GLFWwindow *g_window = NULL;

unsigned g_background_len;
GLuint g_background_vbo;

GLuint g_foreground_vbo;
unsigned g_foreground_len = 0;
unsigned g_foreground_max = DRAW_SPACE/sizeof(complex<float>);

GLuint g_poincare_program;
GLuint g_pan_uni;
GLuint g_colour_uni;
GLuint g_position_attrib;


int g_mouse_state = IDLE;
vector<vector<complex<float>>> g_lineses;  // (lol)

complex<float> g_pan = 0.f;

unsigned char g_frame_counter = 0;


// Process events for dt seconds, then return. Should almost always return in
// exactly dt seconds.
void process_events_for(double dt)
{
    for (;;) {
        double t0 = glfwGetTime();
        glfwWaitEventsTimeout(dt);
        double u = glfwGetTime() - t0;
        if (u >= dt)
            return;
        dt -= u;
    }
}


// Convert from screen coordinates to (complex) board coordinates.
complex<float> screen_to_board(complex<float> s)
{
    return (
            conj(s) - (float)SCREEN_WIDTH/2.f + (float)SCREEN_HEIGHT/2.f * 1if
           ) / ((float)SCREEN_HEIGHT/2.f) / SCREEN_ZOOM;
}

void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}
// Start up glfw, create window, and initialise the glfw- and vendor-specific
// OpenGL state.
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
    poincare::tiling(3, 7, 5, 6, &background_data, &g_background_len);

    // Upload the vertex data in background_data to the video device.
    glBindBuffer(GL_ARRAY_BUFFER, g_background_vbo);
    glBufferData(GL_ARRAY_BUFFER, g_background_len*sizeof(complex<float>),
            background_data, GL_STATIC_DRAW);


    glGenBuffers(1, &g_foreground_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_foreground_vbo);
    glBufferData(GL_ARRAY_BUFFER, g_foreground_max*sizeof(complex<float>),
            NULL, GL_DYNAMIC_DRAW);


    g_poincare_program = shader_program(
            "glsl/poincare.vert", "glsl/mono.frag");


    // Use the poincare shader program in all subsequent draw calls.
    glUseProgram(g_poincare_program);

    g_position_attrib =
        glGetAttribLocation(g_poincare_program, "position");
    // A program's vertex attribute arrays are disabled by default... wth...
    // Well, enable them, then.
    glEnableVertexAttribArray(g_position_attrib);

    g_pan_uni = glGetUniformLocation(g_poincare_program, "pan");
    g_colour_uni = glGetUniformLocation(g_poincare_program, "colour");

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
void render(void)
{
    glUniform2f(g_pan_uni, real(g_pan), imag(g_pan));


    glBindBuffer(GL_ARRAY_BUFFER, g_background_vbo);
    // Pass the currently bound VBO (g_background_vbo) to the "position" input
    // of the vertex shader.  This will associate one 2-vector out of
    // background_data with every vertex the vertex shader processes. That
    // 2-vector gets accessed by the name "position". It could be any name or
    // any data type.  It is up to the vertex shader to figure out how to turn
    // that data into a vertex position.
    glVertexAttribPointer(g_position_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Draw lines with the active shader program and its current inputs.
    glUniform4f(g_colour_uni, GRID_SHADE, GRID_SHADE, GRID_SHADE, 1.f);
    glDrawArrays(GL_LINES, 0, g_background_len);


    glBindBuffer(GL_ARRAY_BUFFER, g_foreground_vbo);
    glVertexAttribPointer(g_position_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glUniform4f(g_colour_uni, 1.f, 1.f, 1.f, 1.f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, g_foreground_len);
}


void key_callback(GLFWwindow *window, int key, int scancode,
        int action, int mods)
{
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_U && action == GLFW_PRESS && g_lineses.size() > 0)
        g_lineses.pop_back();
}
void cursor_position_callback(GLFWwindow *window, double sx, double sy)
{
    complex<float> s(sx, sy);
    complex<float> p;
    switch (g_mouse_state) {
    case PAN:
        g_pan = screen_to_board(s);
        break;
    case DRAW:
        p = screen_to_board(s);
        g_lineses.back().push_back(p);
        refresh_foreground();
        break;
    }
}
void mouse_button_callback(GLFWwindow *window, int button,
        int action, int mods)
{
    double sx, sy;
    glfwGetCursorPos(window, &sx, &sy);
    complex<float> s(sx, sy);
    switch (g_mouse_state) {
    case IDLE:
        if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_MIDDLE) {
            g_pan = screen_to_board(s);
            g_mouse_state = PAN;
        }
        if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
            complex<float> p = screen_to_board(s);
            vector<complex<float>> &&v{};
            v.push_back(p);
            g_lineses.push_back(v);
            refresh_foreground();
            g_mouse_state = DRAW;
        }
        break;
    case PAN:
        if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_MIDDLE)
            g_mouse_state = IDLE;
        break;
    case DRAW:
        if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
            // This point s is never different from the last one, acquired from
            // cursor_position_callback().  Do not bother adding s here.
            g_mouse_state = IDLE;
        }
        break;
    }
}

void refresh_foreground(void)
{
    complex<float> shape[] = {
         3.f + 4if,
         4.f + 3if,
        -3.f - 4if,
        -4.f - 3if,
        };
    for (unsigned i = 0; i < 4; i++)
        shape[i] *= LINE_WIDTH/10;
    vector<complex<float>> rendered;
    for (auto& lines : g_lineses) {
        unsigned N = lines.size();
        // Skip the first point.
        unsigned first_stitch_i = rendered.size();
        rendered.push_back(0);  // Save room for stitching.
        for (unsigned i = 0; i < N - 1; i++) {
            // The first N-1 points require actual lines from one to the next.
            complex<float> r0 = lines[i],
                           r1 = lines[i + 1];
            rendered.push_back(r0 + shape[0]);
            rendered.push_back(r0 + shape[1]);
            rendered.push_back(r1 + shape[1]);
            rendered.push_back(r0 + shape[2]);
            rendered.push_back(r1 + shape[2]);
            rendered.push_back(r0 + shape[3]);
            rendered.push_back(r1 + shape[3]);
            rendered.push_back(r0 + shape[0]);
        }
        // The last point requires a cap.
        complex<float> r0 = lines[N - 1];
        rendered.push_back(r0 + shape[0]);
        rendered.push_back(r0 + shape[1]);
        rendered.push_back(r0 + shape[3]);
        rendered.push_back(r0 + shape[2]);

        // Repeat the first and last vertices so that two zero-area triangles
        // are "drawn" from the previous line to this one.
        rendered[first_stitch_i] = rendered[first_stitch_i + 1];
        rendered.push_back(rendered.back());
    }

    for (auto& p : rendered)
        p = poincare::S(-g_pan, p);

    // set g_foreground_len.
    g_foreground_len = rendered.size();
    assert(g_foreground_len <= g_foreground_max);
    glBindBuffer(GL_ARRAY_BUFFER, g_foreground_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
            rendered.size()*8, rendered.data());
}

// Give the stew a taste every once in a while.
bool tasting(void)
{
    return g_frame_counter == 0;
}
int main(int argc, char *argv[])
{
    // Start up glfw and create window.
    if (!init()) {
        printf("Failed to initialise!\n");
    } else {
        const GLFWvidmode *m = glfwGetVideoMode(glfwGetPrimaryMonitor());
        double T = 1. / (double)m->refreshRate;
        printf("T = %.3fms\n", T*1000.);

        double t_last_frame = glfwGetTime();
        while (!glfwWindowShouldClose(g_window)) {  // once per frame.
            double t;
            if (tasting())
                t = glfwGetTime();
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
            glFinish();  // Also, actually do the thing, like right meow.
            double t1 = glfwGetTime();
            if (tasting())
                printf("Time waiting for vsync: %.3fms.\n",
                        (t1 - t)*1000.);
            if (tasting())
                printf("Frame duration: %.3fms\n\n",
                        (t1 - t_last_frame)*1000.);
            t_last_frame = t1;
            g_frame_counter++;

            //---------------- ***VSYNC*** ----------------

            // OK, the vsync has like /juuuust/ happened. The buffers have just
            // been swapped for suresiez.  Process events for T - T_RENDER, so
            // that as many events as possible are used to determine the
            // content of the next frame.
            if (tasting())
                t = glfwGetTime();
            process_events_for(T - T_RENDER);
            if (tasting())
                printf("processEventsFor takes %.3fms.\n", (glfwGetTime() - t)*1000.);

            // We have awoken! It is only T_RENDER seconds before the next
            // vsync, and we have got a frame to render!  Do all OpenGL drawing
            // commands. 
            if (tasting())
                t = glfwGetTime();
            render();
            glFinish();
            if (tasting())
                printf("Draw takes %.3fms.\n", (glfwGetTime() - t)*1000.);
        }
    }

    // Destroy window
    glfwDestroyWindow(g_window);

    glfwTerminate();

    return 0;
}
