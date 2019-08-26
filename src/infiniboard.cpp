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
void refresh_background(void);
void refresh_foreground(void);
void render(void);
bool tasting(void);


// Globals, prefixed with g_.
GLFWwindow *g_window = NULL;

unsigned g_background_len;
GLuint g_background_vbo;
unsigned g_p = 3, g_q = 7, g_res = 5, g_niter = 6;

GLuint g_foreground_vbo;
unsigned g_foreground_len = 0;
unsigned g_foreground_max = DRAW_SPACE/sizeof(complex<float>);

GLuint g_poincare_program;
GLuint g_pan_uni;
GLuint g_colour_uni;
GLuint g_position_attrib;


int g_mouse_state = IDLE;
// This is more or less the board's foreground state. This is a list of curves,
// each curve being approximated by a series of points. There is a one-to-one
// correspondence (currently) between mouse positions while drawing and points
// in the curve.
vector<vector<complex<float>>> g_curves;

complex<float> g_pan = 0.f;
// The point in board space (in the reference configuration) where the mouse is
// during the start of a pan operation.
complex<float> g_pan_start = 0.f;

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
    glfwWindowHint(GLFW_SAMPLES, 8);
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
    refresh_background();

    // Make the foreground VBO.
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

    if (key == GLFW_KEY_U && action == GLFW_PRESS && g_curves.size() > 0) {
        g_curves.pop_back();
        refresh_foreground();
    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        g_p++;
        refresh_background();
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        g_q++;
        refresh_background();
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        g_res++;
        refresh_background();
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        g_niter++;
        refresh_background();
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS &&
            2*((g_p - 1) + g_q) < (g_p - 1)*g_q) {
        g_p--;
        refresh_background();
    }
    if (key == GLFW_KEY_X && action == GLFW_PRESS &&
            2*(g_p + (g_q - 1)) < g_p*(g_q - 1)) {
        g_q--;
        refresh_background();
    }
    if (key == GLFW_KEY_C && action == GLFW_PRESS && g_res > 2) {
        g_res--;
        refresh_background();
    }
    if (key == GLFW_KEY_V && action == GLFW_PRESS && g_niter > 1) {
        g_niter--;
        refresh_background();
    }
}
void cursor_position_callback(GLFWwindow *window, double sx, double sy)
{
    complex<float> s(sx, sy);
    complex<float> p;
    switch (g_mouse_state) {
    case PAN:
    {
        complex<float> p = g_pan_start, q = screen_to_board(s);
        float mod2p = norm(p), mod2q = norm(q);
        g_pan = ((1 - mod2p)*q - (1 - mod2q)*p) / (1 - mod2p*mod2q);
    }
        break;
    case DRAW:
        p = screen_to_board(s);
        g_curves.back().push_back(poincare::S(-g_pan, p));
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
            g_pan_start = poincare::S(-g_pan, screen_to_board(s));
            g_mouse_state = PAN;
        }
        if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
            complex<float> p = screen_to_board(s);
            vector<complex<float>> &&v{};
            v.push_back(poincare::S(-g_pan, p));
            g_curves.push_back(v);
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
    // TODO: A huge amount of the foreground is the same as it was before, in a
    // predictable way. So almost this entire function doesn't actually have to
    // get run. It gets very buggy, though, only trying to refresh the parts
    // that have changed. I'll only do it if it becomes a performance problem.
    complex<float> shape[] = {
         3.f + 4if,
         4.f + 3if,
        -3.f - 4if,
        -4.f - 3if,
        };
    for (unsigned i = 0; i < 4; i++)
        shape[i] *= LINE_WIDTH/10;
    // Yes, rendered gets allocated every time, but there's probably not a
    // point in reusing a previous allocation under any circumstances. I'll
    // only consider it if it isn't fast enough. The same goes for reserving
    // space---something I actually could do pretty easily but still won't in
    // case something else changes.
    vector<complex<float>> rendered;
    for (auto& curve : g_curves) {
        unsigned N = curve.size();
        // Record the location of this curve's first point and reserve room so
        // that it can be repeated. See "stitching" below.
        unsigned first_stitch_i = rendered.size();
        rendered.push_back(0);  
        for (unsigned i = 0; i < N - 1; i++) {
            // The first N-1 points require actual lines from one to the next.
            complex<float> r0 = curve[i],
                           r1 = curve[i + 1];

            // Zoom the point shape according to where the point is located.
            complex<float> shape0[4];
            for (unsigned j = 0; j < 4; j++)
                // norm is actually the modulus squared. Nice, C++.
                shape0[j] = shape[j]*(1 - norm(r0));
            complex<float> shape1[4];
            for (unsigned j = 0; j < 4; j++)
                shape1[j] = shape[j]*(1 - norm(r1));

            rendered.push_back(r0 + shape0[0]);
            rendered.push_back(r0 + shape0[1]);
            rendered.push_back(r1 + shape1[1]);
            rendered.push_back(r0 + shape0[2]);
            rendered.push_back(r1 + shape1[2]);
            rendered.push_back(r0 + shape0[3]);
            rendered.push_back(r1 + shape1[3]);
            rendered.push_back(r0 + shape0[0]);
        }
        // The last point requires a cap.
        complex<float> r0 = curve[N - 1];
        complex<float> shape0[4];
        for (unsigned i = 0; i < 4; i++)
            shape0[i] = shape[i]*(1 - norm(r0));
        rendered.push_back(r0 + shape0[0]);
        rendered.push_back(r0 + shape0[1]);
        rendered.push_back(r0 + shape0[3]);
        rendered.push_back(r0 + shape0[2]);

        // Stitching: Repeat the first and last vertices of every curve so that
        // two zero-area triangles are "drawn" from the end of one curve to the
        // beginning of the next.  Do this so that the entire foreground can be
        // drawn in a single OpenGL draw call.
        rendered[first_stitch_i] = rendered[first_stitch_i + 1];
        rendered.push_back(rendered.back());
    }

    // set g_foreground_len.
    g_foreground_len = rendered.size();
    assert(g_foreground_len <= g_foreground_max);
    glBindBuffer(GL_ARRAY_BUFFER, g_foreground_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
            rendered.size()*8, rendered.data());
}

void refresh_background(void)
{
    // Make the vertex data.
    complex<float> *background_data;
    poincare::tiling(g_p, g_q, g_res, g_niter,
            &background_data, &g_background_len);

    // Upload the vertex data in background_data to the video device.
    glBindBuffer(GL_ARRAY_BUFFER, g_background_vbo);
    glBufferData(GL_ARRAY_BUFFER, g_background_len*sizeof(complex<float>),
            background_data, GL_DYNAMIC_DRAW);
    free(background_data);
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
