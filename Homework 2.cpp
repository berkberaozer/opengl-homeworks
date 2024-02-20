/*********
   CTIS164 - Template Source Program
----------
STUDENT : Berk Bera Özer
SECTION : 02
HOMEWORK: 2
----------
PROBLEMS: 
----------
ADDITIONAL FEATURES:
grass-like background
crossbow loading animation
score system
speed of targets increase as the player hit them
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

#define WINDOW_WIDTH  1400
#define WINDOW_HEIGHT 650

#define TIMER_PERIOD  16 // Period for the timer.
#define TIMER_ON       1 // 0:disable timer, 1:enable timer

#define D2R 0.0174532

#define MAX_TARGETS 5

/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false;
int  winWidth, winHeight; // current Window width and height

bool game = false; //game state

typedef struct {
    int x;
    int y;
    bool live;
} target_t;

typedef struct {
    int x;
    int y;
    bool discharged;
} crossbow_t;

typedef struct {
    int x;
    int y;
    bool loading;
} arrow_t;

target_t targets[MAX_TARGETS]; 
crossbow_t crossbow;
arrow_t arrow;

int count = 0; //timer count
int score = 0; //player score
int bowString = 38; //variable for crossbow loading animation
int targetSpeed = 8; //speed variable

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

void drawTargets()
{
    for (int target = 0; target < MAX_TARGETS; target++) //iterate over all targets, if they are alive draw them
        if (targets[target].live == true)
        {
            glColor3ub(51, 51, 51);
            circle(targets[target].x, targets[target].y, 40);

            glColor3ub(51, 153, 255);
            circle(targets[target].x, targets[target].y, 30);

            glColor3ub(255, 51, 51);
            circle(targets[target].x, targets[target].y, 20);

            glColor3ub(255, 255, 51);
            circle(targets[target].x, targets[target].y, 10);
        }
}

void drawCrossbow()
{
    int cx = crossbow.x;
    int cy = crossbow.y;

    //upper part of the crossbow
    glColor3ub(102, 51, 0);
    glBegin(GL_POLYGON);
    glVertex2d(cx + 5, cy);
    glVertex2d(cx + 15, cy + 35);
    glVertex2d(cx + 6, cy + 50);
    glVertex2d(cx + 6, cy + 70);
    glVertex2d(cx - 6, cy + 70);
    glVertex2d(cx - 6, cy + 50);
    glVertex2d(cx - 15, cy + 35);
    glVertex2d(cx - 5, cy);
    glEnd();

    //lower part of the crossbow
    glBegin(GL_POLYGON);
    glVertex2d(cx - 5, cy);
    glVertex2d(cx - 5, cy - 25);
    glVertex2d(cx + 5, cy - 25);
    glVertex2d(cx + 5, cy);
    glEnd();

    //bow part (half-ellipse)
    glLineWidth(2.5);
    glBegin(GL_LINE_STRIP);
    for (double i = 0; i < 180 * D2R; i += 0.1)
        glVertex2d(cx + cos(i) * 50, cy + 38 + sin(i) * 32);
    glEnd();
    glLineWidth(1);

    //arrow track(groove)
    glColor3ub(185, 105, 45);
    glRectf(cx - 1, cy + 3, cx + 1, cy +70);
    
    /*BOW STRING*/
    glLineWidth(1.5);
    glColor3ub(256, 256, 256);
    glBegin(GL_LINE_STRIP);
    glVertex2d(cx + 50, cy + 38);

    if (!arrow.loading && !crossbow.discharged)
        glVertex2d(cx, cy);

    else if(arrow.loading)
        glVertex2d(cx, cy + bowString);

    glVertex2d(cx - 50, cy + 38);
    glEnd();
    glLineWidth(1);
    /************/
}

void drawArrow()
{
    int ax = arrow.x;
    int ay = arrow.y;

    glColor3ub(242, 239, 164); //main part of the arrow
    glRectf(ax - 1, ay + 72, ax + 1, ay + 2);

    glColor3ub(60, 60, 60); //arrowhead
    glBegin(GL_TRIANGLES);
    glVertex2d(ax - 5, ay + 72);
    glVertex2d(ax + 5, ay + 72);
    glVertex2d(ax, ay + 82);
    glEnd();

    glColor3ub(255, 255, 0); //fletchings
    glBegin(GL_TRIANGLES);
    glVertex2d(ax - 4, ay + 8);
    glVertex2d(ax - 1, ay + 13);
    glVertex2d(ax - 1, ay + 4);
    glEnd();    
    glBegin(GL_TRIANGLES);
    glVertex2d(ax + 4, ay + 8);
    glVertex2d(ax + 1, ay + 13);
    glVertex2d(ax + 1, ay + 4);
    glEnd();
}

void drawBackground()
{
    glClear(GL_COLOR_BUFFER_BIT);

    //grass background
    int green = 191, cx, cy;
    for(int x = 0; x < winWidth; x += 9)
        for (int y = 0; y < winHeight; y += 9)
        {
			green = green % 13 + 191;
            glColor3ub(0, green, 0);
			
            cx = (x - winWidth / 2);
            cy = (winHeight / 2 - y);
			
            glRectf(cx, cy, cx + 9, cy - 9);
        }

    /*TIMER AND SCORE*/
    glColor3ub(0, 0, 0);
    glRectf(170, winHeight / 2 - 50, 0, winHeight / 2);
	glRectf(-170, winHeight / 2 - 50, 0, winHeight / 2);
	
    glColor3ub(153, 76, 0);
    glRectf(167, winHeight / 2 - 47, 2, winHeight / 2 - 3);
    glRectf(-167, winHeight / 2 - 47, -2, winHeight / 2 - 3);

    glColor3ub(242, 239, 164);
    vprint(-115, winHeight / 2 - 20, GLUT_BITMAP_HELVETICA_18, "SCORE");
	if(score < 10)
	    vprint(-85, winHeight / 2 - 40, GLUT_BITMAP_HELVETICA_18, "%d", score);
    else
        vprint(-90, winHeight / 2 - 40, GLUT_BITMAP_HELVETICA_18, "%d", score);
    vprint(8, winHeight / 2 - 20, GLUT_BITMAP_HELVETICA_18, "REMAINING TIME");
    vprint(65, winHeight / 2 - 40, GLUT_BITMAP_HELVETICA_18, "%02d:%02d", count / 100, count % 100);
    /**********************/
		
    /*INFORMATIVE MESSAGES*/
    glColor3ub(0, 0, 0);
    if (count == 0)
        vprint(-80, -winHeight / 2 + 10, GLUT_BITMAP_HELVETICA_12, "Press F1 to start a new game");
    else
        vprint(-220, -winHeight / 2 + 10, GLUT_BITMAP_HELVETICA_12, "F1 - Pause/Continue | Space - Shoot an arrow | <- / -> - Move the crossbow");
    /**********************/
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

    drawBackground();
    drawCrossbow();
    if(!arrow.loading)
        drawArrow();
    drawTargets();

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
	if (key == 32 && game == true && !crossbow.discharged && !arrow.loading) //shoot the arrow with space
        crossbow.discharged = true;

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
    case GLUT_KEY_F1: //game restart/pause with F1
        game = !game;

        if (count == 0)
        {
            count = 2000;
            for (int target = 0; target < MAX_TARGETS; target++)
                targets[target].live = false;
            crossbow.x = 0;
            crossbow.y = -1 * WINDOW_HEIGHT / 2 + 70;
            crossbow.discharged = false;
            arrow.x = crossbow.x;
            arrow.y = crossbow.y + 2;
            score = 0;
        }

        break;
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
void counter(int v)
{
    glutTimerFunc(10, counter, 0);

    if (game)
        count -= 1;
    if (count == 0)
        game = false;
}

void spawnTargets(int v)
{
    glutTimerFunc(625, spawnTargets, 0);

    if (game == true) //if game is not paused, iterate over all targets and respawn them if they are not live
    {
        for (int target = 0; target < MAX_TARGETS; target++)
            if (targets[target].live == false)
            {
                targets[target].x = winWidth / 2 + 40;
                targets[target].y = winHeight / 2 - 230 + rand() % 140;

                targets[target].live = true;

                break;
            }
    }
}

void onTimer(int v) {

    glutTimerFunc(TIMER_PERIOD, onTimer, 0);
    // Write your codes here.

    if (game == true)
    { 
        for (int target = 0; target < MAX_TARGETS; target++) //move all targets, if they exceed the window "kill" them
        { 
            targets[target].x -= targetSpeed * (20 + score) / 20;
            if (targets[target].x < -1 * winWidth / 2 - 50)
                targets[target].live = false;
        }

		if (crossbow.discharged) //if the arrow was shot,
		{
			arrow.y += 25; //move the arrow

			if (arrow.y > winHeight / 2 + 50) //if the arrow exceed the window, load a new arrow
			{
                arrow.loading = true;
				crossbow.discharged = false;
				arrow.x = crossbow.x;
                arrow.y = crossbow.y;
			}

			for (int target = 0; target < MAX_TARGETS; target++) //collision detector
            {
				float dx = targets[target].x - arrow.x;
				float dy = targets[target].y - (arrow.y + 78);
				if (sqrt(dx*dx + dy*dy) < 40)
				{ 
					targets[target].live = false;
                    arrow.loading = true;
                    crossbow.discharged = false;
					arrow.x = crossbow.x;
					arrow.y = crossbow.y;
                    score++;
				}
            }
        }

        if (arrow.loading) //if the arrow is loading, decrement the y of the bowString
        {
            if (bowString > 0)
                bowString -= 1;
            else
            {
                arrow.loading = false;
                bowString = 38;
            }
        }

        if (right  && crossbow.x < winWidth /2 - 50) //move the crossbow to the right
		{ 
            crossbow.x += 10;
			if (!crossbow.discharged)
				arrow.x = crossbow.x;
		}
		if (left && crossbow.x > -winWidth / 2 + 50) //move the crossbow to the left
		{
			crossbow.x -= 10;
			if (!crossbow.discharged)
				arrow.x = crossbow.x;
		}
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
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Crossbow Range | Berk Bera Özer");

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
    glutTimerFunc(1000, spawnTargets, 0);
	glutTimerFunc(10, counter, 0);
#endif
    
    Init();

    for (int target = 0; target < MAX_TARGETS; target++)
        targets[target].live == false;

    arrow.x = 0;
    arrow.y = -1 * WINDOW_HEIGHT / 2 + 72;
	crossbow.x = 0;
	crossbow.y = -1 * WINDOW_HEIGHT / 2 + 70;

    srand(time(NULL));

    glutMainLoop();
}