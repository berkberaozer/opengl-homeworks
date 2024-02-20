/*********
   CTIS164 - Template Source Program
----------
STUDENT : Berk Bera Özer
SECTION : 02
HOMEWORK: 1
----------
PROBLEMS:
when several cars win at the same time, the one with the lower index is considered as winner
----------
ADDITIONAL FEATURES:
exhaust and turning-like wheel animations (lug nuts drawed as a circle when moving)
randomized speed and color
align all cars button
betting and score
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

#define TIMER_PERIOD  16 // Period for the timer. 60fps 1000/16
#define TIMER_ON       1 // 0:disable timer, 1:enable timer

#define D2R 0.0174532

#define MAX_OBJECTS 10

/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false;
int  winWidth, winHeight; // current Window width and height

bool race = false; //state
int winnerIndex = -1, bet = -1, score = 0;

typedef struct { //car object
	int x;
	int y;
	int rgb[3];
	int speed;
	int num;
} car_t;

car_t cars[MAX_OBJECTS]; //cars array
int carCount = 0;

void onTimer(int v); //prototype of the onTimer function

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
		angle = 2 * PI*i / 100;
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
		angle = 2 * PI*i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void print(int x, int y, const char *string, void *font)
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
void vprint(int x, int y, void *font, const char *string, ...)
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
void vprint2(int x, int y, float size, const char *string, ...) {
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

void drawWheel(int x, int y)
{
	glColor3ub(0, 0, 0); //tyre
	circle(x, y, 20); 
	
	glColor3ub(131, 131, 131); //rim
	circle(x, y, 13);

	if (race == false) //lug nuts
	{ 
		glColor3ub(30, 30, 30);
		circle(x - 3, y + 3, 2);
		circle(x + 3, y + 3, 2);
		circle(x - 3, y - 3, 2);
		circle(x + 3, y - 3, 2);
	}
	else //lug nuts but turning-like
	{
		glColor3ub(80, 80, 80);
		circle(x, y, 5);
		glColor3ub(131, 131, 131);
		circle(x, y, 2);
	}
}

void drawCar(car_t car)
{
	int x = car.x, y = car.y;
	glColor3ub(car.rgb[0], car.rgb[1], car.rgb[2]);
	
	glRectf(x - 50, y - 17, x + 50, y + 17); //main body of the car
	glRectf(x - 75, y - 17, x + 90, y + 17);

	glLineWidth(8);
	glBegin(GL_LINE_STRIP); //upper part of the car
	glVertex2f(x - 50, y + 17);
	glVertex2f(x - 30, y + 40);
	glVertex2f(x + 30, y + 40);
	glVertex2f(x + 50, y + 17);
	glEnd();
	glLineWidth(4);
	glBegin(GL_LINES);
	glVertex2f(x, y + 40);
	glVertex2f(x, y + 17);
	glEnd();

	glPointSize(3); 
	glBegin(GL_POINTS);	//smoother curves for upper part
	glVertex2f(x - 31, y + 40);
	glVertex2f(x + 30, y + 40);
	glEnd();

	glColor3ub(0, 0, 0); //exhaust block
	glRectf(x - 67, y - 20, x - 83, y - 16);
	
	glColor3ub(204, 204, 255); //windows
	glBegin(GL_POLYGON);
	glVertex2f(x - 48, y + 16);
	glVertex2f(x - 28, y + 39);
	glVertex2f(x-2, y + 39);
	glVertex2f(x-2, y + 16);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex2f(x +2, y + 16);
	glVertex2f(x + 2, y + 39);
	glVertex2f(x + 28, y + 39);
	glVertex2f(x + 48, y + 16);
	glEnd();

	glColor3ub(204, 102, 0);	//headlamps
	glBegin(GL_TRIANGLES); 
	glVertex2f(x + 90, y + 15);
	glVertex2f(x + 90, y + 5);
	glVertex2f(x + 82, y + 5);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex2f(x - 75, y + 15);
	glVertex2f(x - 70, y + 15);
	glVertex2f(x - 70, y + 10);
	glVertex2f(x - 75, y + 10);
	glEnd();
	
	glColor3f(1, 1, 1); //car number
	if(car.num > 9)
		glRectf(x - 8, y - 10, x + 17, y + 10);
	else
		glRectf(x - 10, y - 10, x + 10, y + 10);
	glColor3ub(car.rgb[0], car.rgb[1], car.rgb[2]);
	vprint(x-5, y-5, GLUT_BITMAP_HELVETICA_18, "%d", car.num);

	drawWheel(x - 40, y - 25); //wheels
	drawWheel(x + 60, y - 25);

	if (race == true) //exhaust when moving
	{
		glColor3ub(20, 20, 20);
		glBegin(GL_POLYGON);
		glVertex2f(x-83, y-16);
		glVertex2f(x-84, y-15);
		glVertex2f(x-85, y-14);
		glVertex2f(x-86, y-13);
		glVertex2f(x-87,y-12);
		glVertex2f(x-89, y-11);
		glVertex2f(x-130,y-10);
		glVertex2f(x-120,y-9);
		glVertex2f(x - 200, y - 8);
		glVertex2f(x - 96, y - 9);
		glVertex2f(x - 93, y - 10);
		glVertex2f(x - 92, y - 11);
		glVertex2f(x - 90, y - 12);
		glVertex2f(x - 87, y - 13);
		glVertex2f(x - 84, y - 14);
		glVertex2f(x - 83, y - 15);
		glVertex2f(x - 83, y - 16);
		glEnd();
	}
		
}

void drawBackground()
{
	glClearColor(84.0/255, 80.0/255, 80.0/255, 1); //track
	glClear(GL_COLOR_BUFFER_BIT);
	
	glColor3ub(0, 0, 0);
	glRectf(-winWidth / 2 + 200, winHeight / 2, -winWidth / 2 + 260, -winHeight / 2);	//start line
	glRectf(winWidth / 2 - 60, winHeight / 2, winWidth / 2, -winHeight / 2);	//finish line
	
	bool left = false;
	for (int y = winHeight / 2; y > -winHeight / 2; y -= 30) //draw white squares in both start and finish lines
	{
		glColor3ub(255, 255, 255);
		if (left)
		{
			glRectf(-winWidth / 2 + 200, y, -winWidth / 2 + 230, y - 30);
			glRectf(winWidth / 2 + -60, y, winWidth / 2 - 30, y - 30);
		}
		else
		{
			glRectf(-winWidth / 2 + 230, y, -winWidth / 2 + 260, y - 30);
			glRectf(winWidth / 2 + -30, y, winWidth / 2, y - 30);
		}
			
		left = !left;
	}

	
	/* LABELS */
	glColor3f(0, 0, 0);
	glRectf(-85, winHeight / 2 - 50, 85, winHeight / 2);
	glRectf(85, winHeight / 2, 200, winHeight / 2 - 50);
	glRectf(-200, winHeight / 2-50, -85, winHeight / 2);
	glRectf(-85, winHeight / 2 - 50, 85, winHeight / 2 - 75);
	
	glColor3ub(128, 128, 128);
	glRectf(-82, winHeight / 2 - 47, 82, winHeight / 2-3);
	glRectf(85, winHeight / 2 - 3, 197, winHeight / 2 - 47);
	glRectf(-197, winHeight / 2 - 3, -85, winHeight / 2 - 47);
	glRectf(-82, winHeight / 2 - 50, 82, winHeight / 2 - 72);
	
	glColor3ub(51, 102, 0);
	vprint(-78, winHeight / 2 - 20, GLUT_BITMAP_HELVETICA_18, "BERK BERA OZER");
	vprint(-40, winHeight / 2 - 40, GLUT_BITMAP_HELVETICA_18, "CAR RACE");
	vprint(-188, winHeight / 2 - 20, GLUT_BITMAP_HELVETICA_18, "YOUR BET");
	vprint(105, winHeight / 2 - 20, GLUT_BITMAP_HELVETICA_18, "WINNER");

	if (bet != -1) //if there is a bet display the bet
	{
		glColor3ub(cars[bet].rgb[0], cars[bet].rgb[1], cars[bet].rgb[2]);
		vprint(-145, winHeight / 2 - 40, GLUT_BITMAP_HELVETICA_18, "%d", bet + 1);
	}
	else
	{
		glColor3ub(51, 102, 0);
		vprint(-147, winHeight / 2 - 40, GLUT_BITMAP_TIMES_ROMAN_24, "-");
	}

	if (winnerIndex != -1) //if there is a winner, display the winner and according to the player bet show a message
	{
		glColor3ub(cars[winnerIndex].rgb[0], cars[winnerIndex].rgb[1], cars[winnerIndex].rgb[2]);
		vprint(135, winHeight / 2 - 40, GLUT_BITMAP_HELVETICA_18, "%d", winnerIndex + 1);

		glColor3ub(51, 102, 0);
		if (bet != -1)
		{
			if (bet == winnerIndex)
				vprint(-50, winHeight / 2 - 68, GLUT_BITMAP_HELVETICA_18, "YOU WON!!!");

			else if (bet != -1)
				vprint(-50, winHeight / 2 - 68, GLUT_BITMAP_HELVETICA_18, "YOU LOSE!!!");
		}
	}

	if (winnerIndex == -1 || winnerIndex != -1 && bet == -1) //if there is no bet result message, show the score
	{
		glColor3ub(51, 102, 0);
		vprint(-50, winHeight / 2 - 68, GLUT_BITMAP_HELVETICA_18, "SCORE: %d", score);
	}
	/**********/

	/*INFORMATIVE MESSAGES*/
	if (!race)
	{
		glColor3ub(0, 0, 0);
		if (carCount == 0)
			vprint(-150, -winHeight / 2 + 10, GLUT_BITMAP_HELVETICA_12, "Click on the screen to create a car (maximum 10 cars)");
		else
			vprint(-170, -winHeight / 2 + 10, GLUT_BITMAP_HELVETICA_12, "F1 - Start the race | F2 - Align all cars | [0-9] - Bet on a car");
	}
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

	for (int i = 0; i < carCount; i++) //iterate over all cars and draw them
		drawCar(cars[i]);
	
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
	if (race == false) //betting, 0 -> 10.car
	{
		if (key > 48 && key < 58 && carCount >= key - 48)
			bet = key - 49;
		if (key == 48 && carCount >= 10)
			bet = 9;
	}
	
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

	int i;
	
	switch (key) {
	case GLUT_KEY_UP: up = true; break;
	case GLUT_KEY_DOWN: down = true; break;
	case GLUT_KEY_LEFT: left = true; break;
	case GLUT_KEY_RIGHT: right = true; break;
	case GLUT_KEY_F1: if (race == false) race = true; break; //start the race
	case GLUT_KEY_F2: if (race == false) for (i = 0; i < carCount; i++) cars[i].x = -winWidth/2 + 100; break; //align all cars
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
	
	if(button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && carCount < MAX_OBJECTS && race == false) //create a car and initialize it
	{
		cars[carCount].x = x - winWidth / 2;
		cars[carCount].y = winHeight / 2 - y;
		cars[carCount].rgb[0] = rand() % 255;
		cars[carCount].rgb[1] = rand() % 255;
		cars[carCount].rgb[2] = rand() % 255;
		cars[carCount].speed = 7 + rand() % 5;
		cars[carCount].num = carCount + 1;
		carCount++;
		winnerIndex = -1;
		bet = -1;
	}

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
	
	bool finished = true;

	if(race==true)
	{ 
		for (int i = 0; i < carCount; i++)
		{
			if (cars[i].x < 200 + winWidth / 2) //if there is a car that not reached the end
			{
				cars[i].x += cars[i].speed;
				finished = false;
			}
			else if (winnerIndex == -1) //if there is a car that reached the end and there is no winner yet
				winnerIndex = i;
			
		}
		
		if (finished == true) //if all cars reached the end
		{
			race = false;
			carCount = 0;
			if (bet == winnerIndex)
				score++;
			else if (score != 0)
				score--;
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

void main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Car Race");

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

	Init();

	srand(time(NULL)); //

	glutMainLoop();
}