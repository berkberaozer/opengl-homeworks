/*********
   CTIS164 - Template Source Program
----------
STUDENT : Berk Bera Özer
SECTION : 02
HOMEWORK: 3
----------
PROBLEMS:
----------
ADDITIONAL FEATURES:
distance impact is changed, closer light source has a smaller coverage
randomized positions
star animation(shining)
increase the time speed
*********/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "vec.h"

#define WINDOW_WIDTH  1400
#define WINDOW_HEIGHT 750

#define TIMER_PERIOD  1000/60 // Period for the timer.
#define TIMER_ON         1 // 0:disable timer, 1:enable timer

#define D2R 0.0174532

#define PLANET_COUNT 8
#define STAR_COUNT 200

/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false;
int  winWidth, winHeight; // current Window width and height
bool loaded = false, initializing = false, animation = true, red = true, green = true, blue = true;
int timeSpeed = 1;

typedef struct {
    vec_t pos;
    vec_t N;
} vertex_t;

typedef struct {
    float r, g, b;
} color_t;

typedef struct {
    vec_t pos;
    bool shine;
} star_t;

typedef struct {
    vec_t pos;
    color_t color;
    int angle;
    int radius;
    int speed;
    int distance;
    bool visible;
} light_t;

typedef struct {
    vec_t pos;
    int radius;
    light_t lights[3];
} planet_t;

planet_t planets[PLANET_COUNT];
star_t stars[STAR_COUNT];

//
// to draw circle, center at (x,y)
// radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
    float angle;
    glBegin(GL_POLYGON);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
    float angle;

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void print(int x, int y, const char* string, void* font)
{
    int len, i;

    glRasterPos2f(x, y);
    len = (int)strlen(string);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, string[i]);
    }
}

// display text with variables.
// vprint(-winWidth / 2 + 10, winHeight / 2 - 20, GLUT_BITMAP_8_BY_13, "ERROR: %d", numClicks);
void vprint(int x, int y, void* font, const char* string, ...)
{
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);

    int len, i;
    glRasterPos2f(x, y);
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, str[i]);
    }
}

// vprint2(-50, 0, 0.35, "00:%02d", timeCounter);
void vprint2(int x, int y, float size, const char* string, ...) {
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(size, size, 1);

    int len, i;
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
    }
    glPopMatrix();
}

color_t mulColor(float k, color_t c) {
    color_t tmp = { k * c.r, k * c.g, k * c.b };
    return tmp;
}
color_t addColor(color_t c1, color_t c2) {
    color_t tmp = { c1.r + c2.r, c1.g + c2.g, c1.b + c2.b };
    return tmp;
}

// To add distance into calculation
// when distance is 0  => its impact is 1.0
// when distance is 350 => impact is 0.0
// Linear impact of distance on light calculation.
double distanceImpact(double d, double minimumDistance) {
    return minimumDistance/d; //if light source is perpendicular to the point impact is 1
}

color_t calculateColor(light_t source, vertex_t v, int minimumDistance) {
    vec_t L = subV(source.pos, v.pos);
    vec_t uL = unitV(L);
    float factor = dotP(uL, v.N) * distanceImpact(magV(L), minimumDistance);
    return mulColor(factor, source.color);
}

bool checkPlanetCoord(planet_t planet, int index) //checks the planet, whether it intersects with another planet/orbit or exceeds the window size
{
    for (int i = 0; i < PLANET_COUNT; i++)
        if (i != index)
            if (planets[i].pos.x != NULL) //if the planet is already initialized
                    if (sqrt(powf(planets[i].pos.x - planet.pos.x, 2) + powf(planets[i].pos.y - planet.pos.y, 2)) < planet.radius * 4.3 + planets[i].radius * 4.3 + 10)
                        return 0;

    //if exceeds the window
    if (planet.pos.x + planet.radius * 4.3 > winWidth / 2 || planet.pos.x - planet.radius * 4.3 < -winWidth / 2 || planet.pos.y + planet.radius * 4.3 > winHeight / 2 || planet.pos.y - planet.radius * 4.3 < -winHeight / 2 + 35)
        return 0;

    return 1;
}

bool checkStarCoord(star_t star, int index) //checks the star, whether it is so close the another star or not
{
    for (int i = 0; i < STAR_COUNT; i++)
        if (i != index)
            if (stars[i].pos.x != NULL)
                if (sqrt(powf(stars[i].pos.x - star.pos.x, 2) + powf(stars[i].pos.y - star.pos.y, 2)) < 7)
                    return 0;

    return 1;
}

int initializePlanetCoordinats() //randomize planet coords
{
    int step;

    for (int i = 0; i < PLANET_COUNT; i++)
    {
        
        planets[i].pos.x = rand() % winWidth;
        planets[i].pos.x = planets[i].pos.x - winWidth / 2;
        planets[i].pos.y = rand() % winHeight;
        planets[i].pos.y = winHeight / 2 - planets[i].pos.y;

        step = 0;
        while (!checkPlanetCoord(planets[i], i)) //if randomized pos is not suitable, randomize and check again up to 30 times
        {
            planets[i].pos.x = rand() % winWidth;
            planets[i].pos.x = planets[i].pos.x - winWidth / 2;
            planets[i].pos.y = rand() % winHeight;
            planets[i].pos.y = winHeight / 2 - planets[i].pos.y;

            step++;
            if (step > 30)
                return 0;
        }
    }

    return 1;
}

void initializePlanets() //randomize planet and light source values(pos, radius, angle, speed, direction etc.)
{
    int step;
    int neg = 1;

    for (int i = 0; i < PLANET_COUNT; i++)
    {
        planets[i].pos.x = NULL;
        planets[i].radius = 20 + rand() % 20;

        for (int s = 0; s < 3; s++)
        {
            planets[i].lights[s].angle = rand() % 359;
            planets[i].lights[s].radius = planets[i].radius * 0.3 + rand() % 4;
            planets[i].lights[s].visible = true;

            if (rand() % 2 == 0)
                neg = 1;
            else
                neg = -1;

            planets[i].lights[s].speed = neg * (1 + rand() % 2); //direction

            switch (s) //orbit distance
            {
            case 0:
                planets[i].lights[s].color.r = 1;
                planets[i].lights[s].distance = planets[i].radius * 2;
                break;
            case 1:
                planets[i].lights[s].color.g = 1;
                planets[i].lights[s].distance = planets[i].radius * 3;
                break;
            case 2:
                planets[i].lights[s].color.b = 1;
                planets[i].lights[s].distance = planets[i].radius * 4;
            }
        }
    }

    //after initializing all the values except pos, initialize planet coords
    step = 0;
    while (!initializePlanetCoordinats())
    {
        step += 1;
        if (step > 100) //in some cases, when the radius of the most of the planets is the greatest value, it can take a very long time to place the planet, in that case randomize the radiuses again
        { 
            initializePlanets();
            step = 0;
        }
    }

    loaded = 1; //if every initializing is complete, game is loaded
}

void initializeStars() //randomize star positions
{

    for (int s = 0; s < STAR_COUNT; s++)
    {
        stars[s].pos.x = NULL;
        stars[s].shine = false;

        stars[s].pos.x = rand() % winWidth;
        stars[s].pos.x = stars[s].pos.x - winWidth / 2;
        stars[s].pos.y = rand() % winHeight;
        stars[s].pos.y = winHeight / 2 - stars[s].pos.y;

        while (!checkStarCoord(stars[s], s))
        {
            stars[s].pos.x = rand() % winWidth;
            stars[s].pos.x = stars[s].pos.x - winWidth / 2;
            stars[s].pos.y = rand() % winHeight;
            stars[s].pos.y = winHeight / 2 - stars[s].pos.y;
        }
    }
}

void drawPlanet(planet_t planet)
{
    glLineWidth(2); //orbits
    glColor3f(0.2, 0.2, 0.2);
    circle_wire(planet.pos.x, planet.pos.y, planet.radius + planet.radius);
    circle_wire(planet.pos.x, planet.pos.y, planet.radius + planet.radius * 2);
    circle_wire(planet.pos.x, planet.pos.y, planet.radius + planet.radius * 3);
    glLineWidth(2);

    for (int s = 0; s < 3; s++) //light sources
        if (planet.lights[s].visible)
        {
            glColor3f(planet.lights[s].color.r, planet.lights[s].color.g, planet.lights[s].color.b);

            planet.lights[s].pos = { planet.pos.x + planet.lights[s].distance * cos(D2R * planet.lights[s].angle), planet.pos.y + planet.lights[s].distance * sin(D2R * planet.lights[s].angle) };

            circle(planet.lights[s].pos.x, planet.lights[s].pos.y, planet.lights[s].radius);
        }

    glBegin(GL_TRIANGLE_FAN); //planet
    glColor3f(0, 0, 0);
    glVertex2f(planet.pos.x, planet.pos.y);

    for (int angle = 0; angle <= 360; angle++)
    {
        vertex_t P = { {planet.pos.x + planet.radius * cos(D2R * angle), planet.pos.y + planet.radius * sin(D2R * angle)}, {cos(D2R * angle), sin(D2R * angle)} };

        color_t res = { 0, 0, 0 };
        for (int i = 0; i < 3; i++)
            if (planet.lights[i].visible)
                res = addColor(res, calculateColor(planet.lights[i], P, planet.lights[i].distance - planet.radius));

        glColor3f(res.r, res.g, res.b);
        glVertex2f(P.pos.x, P.pos.y);
    }

    glEnd();

}

void drawStar(star_t star)
{
    int x = star.pos.x;
    int y = star.pos.y;

    glBegin(GL_POLYGON);
    glVertex2d(x + 2, y);
    glVertex2d(x + 3, y + 2);
    glVertex2d(x + 1, y + 2);
    glVertex2d(x, y + 3);
    glVertex2d(x - 1, y + 2);
    glVertex2d(x - 3, y + 2);
    glVertex2d(x - 2, y);
    glVertex2d(x - 3, y - 2);
    glVertex2d(x - 1, y - 2);
    glVertex2d(x, y - 3);
    glVertex2d(x + 1, y - 2);
    glVertex2d(x + 3, y - 2);
    glEnd();
}

void drawBackground()
{
    glClearColor(0.05, 0.05, 0.05, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    for (int s = 0; s < STAR_COUNT; s++) //draw stars
    {
        if (stars[s].shine) //if star is shining, its color is brighter
            glColor3f(1, 1, 1);
        else
            glColor3f(0.7, 0.7, 0.7);

        drawStar(stars[s]);
    }

    glColor3f(0.05, 0.05, 0.05);
    glRectf(-winWidth / 2, winHeight / 2, -winWidth / 2 + 130, winHeight / 2 - 35);
    glColor3f(1, 1, 1);

    vprint(-winWidth / 2 + 5, winHeight / 2 - 15, GLUT_BITMAP_9_BY_15, "Berk Bera Ozer");
    vprint(-winWidth / 2 + 15, winHeight / 2 - 30, GLUT_BITMAP_9_BY_15, "HOMEWORK #3");

    glColor3f(0.2, 0.2, 0.2);
    glRectf(-winWidth / 2, -winHeight / 2, winWidth / 2, -winHeight / 2 + 30);

    glColor3f(1, 1, 1);
    vprint(-winWidth / 2 + 170, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "F1(Red): ");
    if(red)
        vprint(-winWidth / 2 + 245, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "ON");
    else
        vprint(-winWidth / 2 + 245, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "OFF");

    vprint(-winWidth / 2 + 320, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "F2(Green): ");
    if (green)
        vprint(-winWidth / 2 + 410, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "ON");
    else
        vprint(-winWidth / 2 + 410, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "OFF");

    vprint(-winWidth / 2 + 480, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "F3(Blue): ");
    if (blue)
        vprint(-winWidth / 2 + 565, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "ON");
    else
        vprint(-winWidth / 2 + 565, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "OFF");

    vprint(-winWidth / 2 + 640, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "F4(Animation): ");
    if (animation)
        vprint(-winWidth / 2 + 770, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "ON");
    else
        vprint(-winWidth / 2 + 770, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "OFF");

    vprint(-winWidth / 2 + 850, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "F5: Restart");

    vprint(-winWidth / 2 + 1000, -winHeight / 2 + 10, GLUT_BITMAP_9_BY_15, "F6(Time Speed): ");
    
    int timeCount = log2(timeSpeed); //print rectangles as the time speed indicator
    int xPos = 1150;
    for (int i = 0; i <= timeCount; i++)
    {
        glRectf(-winWidth / 2 + xPos, -winHeight / 2 + 10, -winWidth / 2 + xPos + 10, -winHeight / 2 + 20);
        xPos += 20;
    }

}

void loadingScreen() //initialize the game and inform the player
{
    drawBackground();

    glColor3f(1, 1, 1);
    vprint(0, 0, GLUT_BITMAP_8_BY_13, "LOADING");

    glutSwapBuffers();

    if (!initializing)
    {
        initializing = 1;
        initializePlanets();
        initializeStars();
        loaded = 1;
        initializing = 0;
    }
}

//
// To display onto window using OpenGL commands
//
void display() {
    //
    // clear window to black
    //
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!loaded) //if game is not loaded, load
        loadingScreen();
    else
    {
        drawBackground();

        for (int i = 0; i < PLANET_COUNT; i++)
            drawPlanet(planets[i]);
    }

    glutSwapBuffers();
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void onKeyDown(unsigned char key, int x, int y)
{
    // exit when ESC is pressed.
    if (key == 27)
        exit(0);

    // to refresh the window it calls display() function
    glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
    // exit when ESC is pressed.
    if (key == 27)
        exit(0);

    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyDown(int key, int x, int y)
{
    // Write your codes here.
    switch (key) {
    case GLUT_KEY_UP: up = true; break;
    case GLUT_KEY_DOWN: down = true; break;
    case GLUT_KEY_LEFT: left = true; break;
    case GLUT_KEY_RIGHT: right = true; break;
    case GLUT_KEY_F1: 
        red = !red; 
        for (int p = 0; p < PLANET_COUNT; p++)
            planets[p].lights[0].visible = !planets[p].lights[0].visible; 
        break;
    case GLUT_KEY_F2: 
        green = !green; 
        for (int p = 0; p < PLANET_COUNT; p++)
            planets[p].lights[1].visible = !planets[p].lights[1].visible;
        break;
    case GLUT_KEY_F3: 
        blue = !blue; 
        for (int p = 0; p < PLANET_COUNT; p++)
            planets[p].lights[2].visible = !planets[p].lights[2].visible;
        break;
    case GLUT_KEY_F4: animation = !animation; break;
    case GLUT_KEY_F5: loaded = false; break;
    case GLUT_KEY_F6: timeSpeed = (timeSpeed * 2) % 64; if (timeSpeed == 0) timeSpeed = 1; break;
    }
    
    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyUp(int key, int x, int y)
{
    // Write your codes here.
    switch (key) {
    case GLUT_KEY_UP: up = false; break;
    case GLUT_KEY_DOWN: down = false; break;
    case GLUT_KEY_LEFT: left = false; break;
    case GLUT_KEY_RIGHT: right = false; break;
    }

    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
void onClick(int button, int stat, int x, int y)
{
    // Write your codes here.



    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void onResize(int w, int h)
{
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    display(); // refresh window.
}

void onMoveDown(int x, int y) {
    // Write your codes here.



    // to refresh the window it calls display() function   
    glutPostRedisplay();
}

// GLUT to OpenGL coordinate conversion:
//   x2 = x1 - winWidth / 2
//   y2 = winHeight / 2 - y1
void onMove(int x, int y) {
    // Write your codes here.



    // to refresh the window it calls display() function
    glutPostRedisplay();
}

#if TIMER_ON == 1
void onTimer(int v) {

    glutTimerFunc(TIMER_PERIOD, onTimer, 0);
    // Write your codes here.

    if (animation) //if animation is on
    {
        for (int p = 0; p < PLANET_COUNT; p++) //increase the angle of the light sources
            for (int s = 0; s < 3; s++)
                planets[p].lights[s].angle = (planets[p].lights[s].angle + planets[p].lights[s].speed * timeSpeed) % 359;

        for (int s = 0; s < STAR_COUNT; s++) //shine or unshine some stars
            if (rand() % 137/timeSpeed == 0)
                stars[s].shine = !stars[s].shine;
    }

    // to refresh the window it calls display() function
    glutPostRedisplay(); // display()

}
#endif

void Init() {

    // Smoothing shapes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(30, 30);
    glutCreateWindow("RGB Galaxy");

    glutDisplayFunc(display);
    glutReshapeFunc(onResize);

    //
    // keyboard registration
    //
    glutKeyboardFunc(onKeyDown);
    glutSpecialFunc(onSpecialKeyDown);

    glutKeyboardUpFunc(onKeyUp);
    glutSpecialUpFunc(onSpecialKeyUp);

    //
    // mouse registration
    //
    glutMouseFunc(onClick);
    glutMotionFunc(onMoveDown);
    glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
    // timer event
    glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

    srand(time(NULL));
    Init();

    glutMainLoop();
}