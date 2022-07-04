#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>
#include <thread>

#include <math.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


#define W_WIDTH 1920
#define W_HEIGHT 1080
#define FIELD_SIZE 45

using namespace std;

static double SCALE_CUBE = 1;
static unsigned char scene_field_fg[FIELD_SIZE][FIELD_SIZE][FIELD_SIZE];
static unsigned char scene_field_bg[FIELD_SIZE][FIELD_SIZE][FIELD_SIZE];

static double axiosZ = 0;
static int cnt_neighbors;
static int tick_render = 0;
static const int max_tick_render = 30;

static const GLfloat array_vertexes_cube[] = {
    0,0,0,  0,1,0,  1,1,0,  1,0,0,
    0,0,1,  0,1,1,  1,1,1,  1,0,1,
};
static const GLuint array_indexes_cube[] = {
    0,1,2,  2,3,0,
    4,5,6,  6,7,4,
    0,4,7,  7,3,0,
    1,5,6,  6,2,1,
    3,7,6,  6,2,3,
    0,4,5,  5,1,0
};
static GLuint vertex_VBO;
static GLuint index_EBO;

static void __key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwDestroyWindow(window);
}
static void __scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if(yoffset > 0)
        SCALE_CUBE += 0.025;
    else
        SCALE_CUBE -= 0.025;
}
static void __init_scene()
{
    float scaleX = W_WIDTH / (float)W_HEIGHT;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0,0,W_WIDTH,W_HEIGHT);

    glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-1*scaleX,1*scaleX,  -1,1,  1, FIELD_SIZE*3);
    glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

    glGenBuffers(1, &vertex_VBO);
    glGenBuffers(1, &index_EBO);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(array_vertexes_cube),
        array_vertexes_cube,
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(array_indexes_cube),
        array_indexes_cube,
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    srand(time(NULL));
    int cnt_cells = FIELD_SIZE*FIELD_SIZE*FIELD_SIZE/8;

    for(int n = 0; n < cnt_cells; n++)
    {
        int i = rand() % FIELD_SIZE;
        int j = rand() % FIELD_SIZE;
        int k = rand() % FIELD_SIZE;

        if(!scene_field_fg[i][j][k])
            scene_field_fg[i][j][k] = 1;
        else
            k--;
    }
}
static void __draw_cube()
{
    #define DEFAULT_MODE
    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
        glVertexPointer(3, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    #ifdef BLICK_MODE
    glColor4f(1,0.85,0, (max_tick_render - tick_render)/(float)max_tick_render*0.8 + 0.2);
    #elif defined TRANSPARENT_MODE
    glColor4f(1,0.85,0, 0);
    #elif defined DEFAULT_MODE
    glColor3f(1,0.85,0);
    #endif // DEFAULT_MODE


    glDrawElements(GL_TRIANGLES, sizeof(array_indexes_cube)/sizeof(GLuint),
                   GL_UNSIGNED_INT, array_indexes_cube);

    glColor3f(0,0,0);
    glDrawElements(GL_LINE_LOOP, sizeof(array_indexes_cube)/sizeof(GLuint),
                   GL_UNSIGNED_INT, array_indexes_cube);

    glDisableClientState(GL_VERTEX_ARRAY);
}
static void __draw_border()
{
    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
        glVertexPointer(3, GL_FLOAT, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glColor3f(0,0,0);
    glDrawElements(GL_LINE_STRIP, sizeof(array_indexes_cube)/sizeof(GLuint),
                   GL_UNSIGNED_INT, array_indexes_cube);

    glDisableClientState(GL_VERTEX_ARRAY);
}
static int __cnt_neighbors(int i, int j, int k)
{
    int cnt = 0;

    for(int _i = i - 1; _i <= i + 1; _i++)
    {
        for(int _j = j - 1; _j <= j + 1; _j++)
        {
            for(int _k = k - 1; _k <= k + 1; _k++)
            {
                if(_i == i && _j == j && _k == k)
                    continue;

                if(scene_field_fg   [(_i >= 0) ? (_i%FIELD_SIZE) : (FIELD_SIZE-1)]
                                    [(_j >= 0) ? (_j%FIELD_SIZE) : (FIELD_SIZE-1)]
                                    [(_k >= 0) ? (_k%FIELD_SIZE) : (FIELD_SIZE-1)])
                    cnt++;
            }
        }
    }

    return cnt;
}
static void __draw_cells()
{
    for(int i = 0; i < FIELD_SIZE; i++)
    {
        for(register int j = 0; j < FIELD_SIZE; j++)
        {
            for(register int k = 0; k < FIELD_SIZE; k++)
            {
                if(scene_field_fg[i][j][k])
                {
                    glPushMatrix();
                        glScalef(SCALE_CUBE, SCALE_CUBE, SCALE_CUBE);
                        glTranslatef(i,j,k);
                        __draw_cube();
                    glPopMatrix();
                }
            }
        }
    }
}
static void __calc_new_generation()
{
    if(tick_render == max_tick_render) {
        for(int i = 0; i < FIELD_SIZE; i++)
        {
            for(register int j = 0; j < FIELD_SIZE; j++)
            {
                for(register int k = 0; k < FIELD_SIZE; k++)
                {
                    cnt_neighbors = __cnt_neighbors(i,j,k);

                    if(scene_field_fg[i][j][k])                     // life cell
                    {
                        if(cnt_neighbors == 4 || cnt_neighbors == 5)// stay live
                            scene_field_bg[i][j][k] = 1;
                        else                                        // die cell
                            scene_field_bg[i][j][k] = 0;
                    }
                    else                                            // die cell
                    {
                        if(cnt_neighbors == 5)                      // life cell
                            scene_field_bg[i][j][k] = 1;
                        else
                            scene_field_bg[i][j][k] = 0;            // stay die
                    }
                }
            }
        }
    }
}
static void __draw_scene()
{
    glPushMatrix();

    /* calculate new generation by Rule (4555) */
    thread thread_calc(__calc_new_generation);

    /* scene setting with comfortable view */
    glRotatef(-60, 1,0,0);
    glRotatef(axiosZ, 0,0,1);
        float translateX = sin(axiosZ*M_PI/180.0)*FIELD_SIZE;
        float translateY = cos(axiosZ*M_PI/180.0)*FIELD_SIZE;
    glTranslatef(translateX, translateY, 0);
    glTranslatef(-FIELD_SIZE*SCALE_CUBE/2, -FIELD_SIZE*SCALE_CUBE/2, -FIELD_SIZE*SCALE_CUBE*1.1);

    /* draw cells */
    __draw_cells();

    /* draw border around field */
    glScalef(SCALE_CUBE*FIELD_SIZE, SCALE_CUBE*FIELD_SIZE, SCALE_CUBE*FIELD_SIZE);
    __draw_border();

    /* swap field buffers */
    thread_calc.join();
    if(tick_render == max_tick_render)
    {
        memcpy(scene_field_fg, scene_field_bg, sizeof(scene_field_bg));
        tick_render = 0;
    }

    /* rotate scene */
    axiosZ += 0.5;
    if(axiosZ >= 360) axiosZ = 0;

    tick_render++;
    glPopMatrix();
}



int main(void)
{
    static const int PERIOD_RENDER_NSEC = 20000000;
    static struct timespec req = {0,PERIOD_RENDER_NSEC};
    static GLFWwindow * window;

    assert(glfwInit());

    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, "GOL", glfwGetPrimaryMonitor(), NULL);
    if (!window) {glfwTerminate(); return -1;}

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, __key_callback);
    glfwSetScrollCallback(window, __scroll_callback);

    __init_scene();

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1,1,1,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto start = chrono::steady_clock::now();
            __draw_scene();
        auto end = chrono::steady_clock::now();

        glfwSwapBuffers(window);
        glfwPollEvents();

        auto nsec = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
        if(PERIOD_RENDER_NSEC - nsec > 0)
        {
            req.tv_nsec = PERIOD_RENDER_NSEC - nsec;
            nanosleep(&req,NULL);
        }
    }

    glfwTerminate();
    return 0;
}
