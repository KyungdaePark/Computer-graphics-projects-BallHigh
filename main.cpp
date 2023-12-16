#include "bmpfuncs.h"
#include "ObjParser.h"
#include <GL/freeglut.h>
#include <gl/glut.h>
#include <GL/glext.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <math.h>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <mmsystem.h>
#include <windows.h>
#include <chrono>


#define M_PI 3.14159265358979323846
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 1200
#define PI 3.14159265
#pragma comment(lib,"winmm.lib")

bool isEnd = false;
bool previsEnd = false;
bool isPaused = false;
bool isStarted = false;
bool keyStates[256];
char fpsString[15];
double cameraElevation = 0.0f;
double lastTime = 0;
float ballVelocityY = 0.9f;
float ballVelocityX = 0.0f;
float ballVelocityZ = 0.0f;
float ballX, ballY, ballZ;
float ballMaxY = 0.0f;
float ballmiddleY = 0.0f;
float cameraAngle = 0.0f;
float g_nSkySize = 120;
float gravity = -0.06f;
float initialVelocity = 0.8f;
float move = 0.0f;
float sphereRadius = 0.5f;
int current_height, current_width;
int elapsedTimeAtPause = 0;
int frameCount = 0;
int g_nX, g_nY;
int lastMouseX, lastMouseY;
std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
GLuint g_nCubeTex;
GLuint StartTexture;
GLuint EndTexture;
GLuint numberTextures[10];
GLuint textureball;
GLuint texturemap;
GLuint texturemap_cone;
GLuint texturemap_cube;
GLuint texturemap_rock;
GLuint texturemap_seaweed;
GLuint texturemap_starfish;
GLuint texturemap_end;
GLuint texturemap_end2;

struct GameObject {
	ObjParser* obj;
	float x, y, z;
};

std::vector<GameObject> gameObjects;
std::vector<ObjParser*> objs;
std::vector<float> cube_x;
std::vector<float> cube_y;
std::vector<float> cube_z;
volatile double eyex, eyey, eyez;
volatile double theta = 45.0;
volatile double phi = 45.0;
volatile double upVector = cos(phi * PI / 180);
volatile int radius = 18;

void addGameObject(ObjParser*, float, float, float);
void checkOnObject();
void cubeTexture();
void displayTime(int);
void draw();
void draw_axis();
void draw_obj(ObjParser*);
void draw_obj_with_texture(ObjParser*, GLuint);
void draw_string(void*, const char*, float, float, float, float, float);
void drawskybox();
void draw_start();
void draw_end();
void draw_main();
int getElapsedTimeInSeconds();
void idle();
void init();
void keyDown(unsigned char, int, int);
void keyUp(unsigned char, int, int);
void mouse(int, int, int, int);
void mouseMotion(int, int);
void mousePassiveMotion(int, int);
void obj_parser();
void pauseGame();
void resetGame();
void resetTimer();
void resize(int, int);
void resumeGame();
void setBallTexture();
void setMapObjTexture(const char*, GLuint&);
void setMapTexture();
void timer(int);


int main(int argc, char** argv) {
	PlaySound("sounds/start.wav", NULL, SND_FILENAME | SND_ASYNC);
	resetTimer();
	/* Initialize window */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Ball High!");
	init();
	obj_parser();

	glutIdleFunc(idle);
	glutDisplayFunc(draw);
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutPassiveMotionFunc(mousePassiveMotion);
	glutTimerFunc(0, timer, 0);
	glutMainLoop();
	return 0;
}

void init() {
	ballX = 0.0;
	ballY = 3.0;
	ballZ = 0.0;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glColor3f(1.0f, 1.0f, 1.0f);
	glutReshapeFunc(resize);

	GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	glMaterialf(GL_FRONT, GL_SHININESS, 16);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
	glEnable(GL_LINE_SMOOTH);

	setMapTexture();
	setBallTexture();
	setMapObjTexture("imgs/rock.bmp", texturemap_rock);
	setMapObjTexture("imgs/sponge.bmp", texturemap_cube);
	setMapObjTexture("imgs/cone.bmp", texturemap_cone);
	setMapObjTexture("imgs/seaweed.bmp", texturemap_seaweed);
	setMapObjTexture("imgs/starfish.bmp", texturemap_starfish);
	setMapObjTexture("imgs/hole.bmp", texturemap_end);
	setMapObjTexture("imgs/hole_white.bmp", texturemap_end2);
	setMapObjTexture("imgs/start.bmp", StartTexture);
	setMapObjTexture("imgs/end_scene.bmp", EndTexture);
	printf("LOAD ALL DONE\n");


	cubeTexture();
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);
}

void draw_main() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);
	glViewport(0,0,current_width,current_height);
	gluLookAt(ballX - 15.0 * sin(cameraAngle), ballMaxY + 5.0 + 7.0 * sin(cameraElevation), ballZ - 15.0 * cos(cameraAngle),   // 카메라 위치 (공의 위쪽 뒤)
		ballX, ballmiddleY, ballZ,   // 공을 바라보는 위치
		0.0, 1.0, 0.0);   // 카메라의 상향 벡터
	drawskybox();

	//MAP
	glPushMatrix();
	draw_axis();
	glColor3f(1.0f, 1.0f, 1.0f);
	draw_obj_with_texture(gameObjects[1].obj, texturemap);
	glPopMatrix();


	for (int k = 2; k <= 3; k++) {
		glPushMatrix();
		glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_rock);
		glPopMatrix();
	}
	for (int k = 4; k <= 10; k++) {
		glPushMatrix();
		glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_cube);
		glPopMatrix();
	}
	for (int k = 11; k < 14; k++) {
		glPushMatrix();
		glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z);
		glRotatef(10.0 + k * 3.5f, 1.0f, 0.0f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_cone);
		glPopMatrix();
	}
	for (int k = 14; k <= 14; k++) {
		glPushMatrix();
		glTranslatef(gameObjects[k].x + move, gameObjects[k].y, gameObjects[k].z);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_cone);
		glPopMatrix();
	}
	for (int k = 15; k <= 19; k++) {
		glPushMatrix();
		glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z);
		glRotatef(60.0 + k * 15.0f, 1.0f, 0.0f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_cone);
		glPopMatrix();
	}
	for (int k = 20; k <= 22; k++) {
		glPushMatrix();
		glTranslatef(gameObjects[k].x + move, gameObjects[k].y, gameObjects[k].z);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_seaweed);
		glPopMatrix();
	}
	for (int k = 23; k <= 24; k++) {
		glPushMatrix();
		glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_seaweed);
		glPopMatrix();
	}
	for (int k = 25; k <= 27; k++) {
		glPushMatrix();
		glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_seaweed);
		glPopMatrix();
	}
	for (int k = 28; k <= 31; k++) {
		glPushMatrix();
		if (k == 30) glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z - move);
		else glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z + move);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_seaweed);
		glPopMatrix();
	}
	for (int k = 32; k <= 32; k++) {
		glPushMatrix();
		glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_seaweed);
		glPopMatrix();
	}
	for (int k = 33; k <= 36; k++) {
		glPushMatrix();
		glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_starfish);
		glPopMatrix();
	}
	for (int k = 37; k <= 42; k++) {
		glPushMatrix();
		glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_starfish);
		glPopMatrix();
	}
	for(int k=43; k<=45; k++){
		glPushMatrix();
		glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_rock);
		glPopMatrix();
	}

	for(int k=46;k<=46;k++){
		glPushMatrix();
		glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_end);
		glPopMatrix();
	}

	for(int k=47;k<=47;k++){
		glPushMatrix();
		glTranslatef(gameObjects[k].x, gameObjects[k].y, gameObjects[k].z);
		glRotatef(45.0f, 0, 1, 0);
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_obj_with_texture(gameObjects[k].obj, texturemap_end2);
		glPopMatrix();
	}


	//BALL
	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslatef(ballX, ballY, ballZ);
	draw_obj_with_texture(gameObjects[0].obj, textureball);


	// FPS를 계산하고 문자열로 만들기
	double currentTime = glutGet(GLUT_ELAPSED_TIME);
	double deltaTime = (currentTime - lastTime) / 1000.0;  // 두 프레임 사이의 시간
	int fps = 1.0 / deltaTime;  // 초당 프레임 수
	sprintf(fpsString, "FPS: %d", fps); 
	lastTime = currentTime;

	draw_string(GLUT_BITMAP_TIMES_ROMAN_24, fpsString, -0.95f, 0.95f, 0.0f, 1.0f, 0.0f);

	int elapsedSeconds = getElapsedTimeInSeconds();
	displayTime(elapsedSeconds);
}

void draw_start() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glEnable(GL_DEPTH_TEST);
    gluOrtho2D(0.0f, GLUT_WINDOW_WIDTH, 0.0f, GLUT_WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0, 1.0, 1.0);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, StartTexture);

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1, 0); glVertex2f(GLUT_WINDOW_WIDTH, 0.0f);
    glTexCoord2f(1, 1); glVertex2f(GLUT_WINDOW_WIDTH, GLUT_WINDOW_HEIGHT);
    glTexCoord2f(0, 1); glVertex2f(0.0f, GLUT_WINDOW_HEIGHT);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

void RenderBitmapString(float x, float y, void *font, const char *string) {  
  char *c;
  glRasterPos2f(x, y);
  for (c = (char*)string; *c != '\0'; c++) {
    glutBitmapCharacter(font, *c);
  }
}

void draw_end() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);
    gluOrtho2D(0.0f, WINDOW_WIDTH, 0.0f, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0, 1.0, 1.0);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, EndTexture);

    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1, 0); glVertex2f(WINDOW_WIDTH, 0.0f);
    glTexCoord2f(1, 1); glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glTexCoord2f(0, 1); glVertex2f(0.0f, WINDOW_HEIGHT);
    glEnd();

    glDisable(GL_TEXTURE_2D);
	
    // 숫자 그리기
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor3f(1.0, 1.0, 1.0); // 텍스처의 원래 색상 유지

	glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);	


	int elapsedSeconds = getElapsedTimeInSeconds();
    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;

    char timeString[10];
    snprintf(timeString, sizeof(timeString), "%02d:%02d", minutes, seconds);

	glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    
	glColor3f(0.0, 0.0, 0.0); 
	RenderBitmapString(989.58, 747.3, GLUT_BITMAP_TIMES_ROMAN_24, timeString);
	
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
}



void draw(){
	if(!isStarted){
		draw_start();
	}
	if(isEnd){
		draw_end();
	}
	if(isStarted && !isEnd){
		draw_main();
	}
	glutSwapBuffers();
	glFlush();
}

void draw_axis(void)
{
	glLineWidth(1.5f);
	glBegin(GL_LINES);

	glColor4f(1.f, 0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(4.f, 0.f, 0.f);

	glColor4f(0.f, 1.f, 0.f, 1.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 4.f, 0.f);

	glColor4f(0.f, 0.f, 1.f, 1.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 0.f, 4.f);

	glEnd();
	glLineWidth(1);
}

void resize(int width, int height) {
	glViewport(0, 0, width, height);
	current_width = width;
	current_height = height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (float)width / (float)height, 1, 500);
	glMatrixMode(GL_MODELVIEW);
}

void idle() {
	glutPostRedisplay();
}

void obj_parser()
{
	addGameObject(new ObjParser("objs/beach_ball_resize.obj"), 0.0f, 0.0f, 0.0f); //0
	addGameObject(new ObjParser("objs/map_flat.obj"), 0.0f, 0.0f, 0.0f);
	addGameObject(new ObjParser("objs/rock.obj"), 7.0f, 4.0f, 0.0f);
	addGameObject(new ObjParser("objs/rock.obj"), 11.0f, 6.0f, 0.0f);
	addGameObject(new ObjParser("objs/map_cube.obj"), 9.0f, 8.0f, -3.0f);
	addGameObject(new ObjParser("objs/map_cube.obj"), 9.0f, 10.0f, -5.0f); //5
	addGameObject(new ObjParser("objs/map_cube.obj"), 6.0f, 13.0f, -8.0f);
	addGameObject(new ObjParser("objs/map_cube.obj"), 3.0f, 15.0f, -10.0f);
	addGameObject(new ObjParser("objs/map_cube.obj"), 1.0f, 17.0f, -13.0f);
	addGameObject(new ObjParser("objs/map_cube.obj"), -1.0f, 18.0f, -12.0f);
	addGameObject(new ObjParser("objs/map_cube.obj"), -3.0f, 21.0f, -12.0f); //10
	addGameObject(new ObjParser("objs/cone.obj"), -6.0f, 24.0f, -10.0f);
	addGameObject(new ObjParser("objs/cone.obj"), -8.0f, 27.0f, -7.0f);
	addGameObject(new ObjParser("objs/cone.obj"), -10.0f, 30.0f, -11.0f);
	addGameObject(new ObjParser("objs/cone.obj"), -12.0f, 32.0f, -14.0f); //중간에갈라지는콘, 14, movex
	addGameObject(new ObjParser("objs/cone.obj"), -4.0f, 35.0f, -12.0f); //15
	addGameObject(new ObjParser("objs/cone.obj"), -1.0f, 38.0f, -8.0f);
	addGameObject(new ObjParser("objs/cone.obj"), -4.0f, 40.0f, -4.0f);
	addGameObject(new ObjParser("objs/cone.obj"), -8.0f, 43.0f, -2.0f);
	addGameObject(new ObjParser("objs/cone.obj"), -12.0f, 45.0f, -6.0f);
	addGameObject(new ObjParser("objs/seaweed.obj"), -23.0f, 46.0f, -5.0f); //20(movex)
	addGameObject(new ObjParser("objs/seaweed.obj"), -20.0f, 46.0f, -3.0f); //(movex)
	addGameObject(new ObjParser("objs/seaweed.obj"), -17.0f, 47.0f, -1.0f); //22(movex)
	addGameObject(new ObjParser("objs/seaweed.obj"), -14.0f, 48.0f, 2.5f);
	addGameObject(new ObjParser("objs/seaweed.obj"), -11.0f, 50.0f, 4.0f); //24
	addGameObject(new ObjParser("objs/seaweed.obj"), -12.0f, 52.0f, 10.0f); //25
	addGameObject(new ObjParser("objs/seaweed.obj"), -9.0f, 53.0f, 14.0f);
	addGameObject(new ObjParser("objs/seaweed.obj"), -6.0f, 55.0f, 17.0f);
	addGameObject(new ObjParser("objs/seaweed.obj"), -5.0f, 57.0f, 20.0f); //28movez
	addGameObject(new ObjParser("objs/seaweed.obj"), -3.0f, 59.0f, 23.0f); //movez
	addGameObject(new ObjParser("objs/seaweed.obj"), 0.0f, 62.0f, 26.0f); //30 (move-z)
	addGameObject(new ObjParser("objs/seaweed.obj"), 3.0f, 65.0f, 29.0f); //31movez
	addGameObject(new ObjParser("objs/seaweed.obj"), 7.0f, 67.0f, 32.0f);
	
	addGameObject(new ObjParser("objs/starfish.obj"), 7.0f, 69.0f, 18.0f);
	addGameObject(new ObjParser("objs/starfish.obj"), 7.0f, 72.0f, 14.0f);
	addGameObject(new ObjParser("objs/starfish.obj"), 7.0f, 75.0f, 10.0f); //35
	addGameObject(new ObjParser("objs/starfish.obj"), 7.0f, 78.0f, 6.0f);	

	addGameObject(new ObjParser("objs/map_cube.obj"), -15.0f, 33.0f, -13.0f);
	addGameObject(new ObjParser("objs/map_cube.obj"), -17.0f, 35.0f, -15.0f);
	addGameObject(new ObjParser("objs/map_cube.obj"), -19.0f, 37.0f, -17.0f);
	addGameObject(new ObjParser("objs/map_cube.obj"), -21.0f, 39.0f, -19.0f); //40
	addGameObject(new ObjParser("objs/map_cube.obj"), -23.0f, 41.0f, -21.0f);
	addGameObject(new ObjParser("objs/map_cube.obj"), -25.0f, 43.0f, -23.0f);

	addGameObject(new ObjParser("objs/rock.obj"), 10.0f, 69.0f, 28.0f);
	addGameObject(new ObjParser("objs/rock.obj"), 13.0f, 72.0f, 27.0f);
	addGameObject(new ObjParser("objs/rock.obj"), 9.0f, 75.0f, 25.0f);

	addGameObject(new ObjParser("objs/endportal.obj"), 7.0f, 81.0f, 3.0f); //46, end
	addGameObject(new ObjParser("objs/endportal.obj"), -28.0f, 45.0f, -26.0f); //47, move to 0
}

void setMapTexture(void) {
	int imgWidth, imgHeight, channels;
	uchar* img = readImageData("imgs/sand_texture_2_original.bmp", &imgWidth, &imgHeight, &channels);

	int texNum = 1;
	glGenTextures(texNum, &texturemap);
	glBindTexture(GL_TEXTURE_2D, texturemap);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	printf("map rendered\n");
}

void setBallTexture(void) {
	int imgWidth, imgHeight, channels;
	uchar* img = readImageData("imgs/beach_ball.bmp", &imgWidth, &imgHeight, &channels);

	int texNum = 1;
	glGenTextures(texNum, &textureball);
	glBindTexture(GL_TEXTURE_2D, textureball);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	printf("ball rendered\n");
}

void setMapObjTexture(const char* filename, GLuint& texure_obj) {
	int imgWidth, imgHeight, channels;
	uchar* img = readImageData(filename, &imgWidth, &imgHeight, &channels);

	int texNum = 1;
	glGenTextures(texNum, &texure_obj);
	glBindTexture(GL_TEXTURE_2D, texure_obj);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	printf(filename);
	printf(" - map_obj rendered\n");
}

bool move_positive = true;
void timer(int value) {
	if(!previsEnd && isEnd){
		PlaySound("sounds/end.wav", NULL, SND_FILENAME | SND_ASYNC);
		previsEnd = true;
	}
	if(isPaused){
		glutTimerFunc(16, timer, 0);
		return;
	}
	checkOnObject();
	const float speed = 0.15f; 
	float moveidx = 0.1f;

	if (move >= 5) move_positive = false;
	if (move <= -5) move_positive = true;
	if (move_positive) move += moveidx;
	else move -= moveidx;
	ballX += ballVelocityX;
	ballY += ballVelocityY;
	ballZ += ballVelocityZ;

	if (keyStates['w']) {
		ballX += speed * sin(cameraAngle);
		ballZ += speed * cos(cameraAngle);
	}
	if (keyStates['a']) {
		ballX += speed * cos(cameraAngle);
		ballZ -= speed * sin(cameraAngle);
	}
	if (keyStates['s']) {
		ballX -= speed * sin(cameraAngle);
		ballZ -= speed * cos(cameraAngle);
	}
	if (keyStates['d']) {
		ballX -= speed * cos(cameraAngle);
		ballZ += speed * sin(cameraAngle);
	}
	ballVelocityY += gravity;
	if (ballY > ballMaxY) {
		ballMaxY = ballY;
	}
	if (ballMaxY - ballY > 9.0f) {
		ballMaxY = ballY + 5.0f;
	}
	ballmiddleY = ballMaxY - 4.0f;

	glutPostRedisplay();
	glutTimerFunc(16, timer, 0);  // 60fps로 업데이트
}

float calculateDistance(float ballX, float ballY, float ballZ, float objX, float objY, float objZ) {
	return std::sqrt(std::pow(ballX - objX, 2) + std::pow(ballY - objY, 2) + std::pow(ballZ - objZ, 2));
}

void checkOnObject() {
	bool isOnObject = false;
	for (int i = 2; i < gameObjects.size(); i++) {
		float obj_minx = gameObjects[i].obj->getMinX() + gameObjects[i].x;
		float obj_miny = gameObjects[i].obj->getMinY() + gameObjects[i].y;
		float obj_minz = gameObjects[i].obj->getMinZ() + gameObjects[i].z;
		float obj_maxx = gameObjects[i].obj->getMaxX() + gameObjects[i].x;
		float obj_maxy = gameObjects[i].obj->getMaxY() + gameObjects[i].y;
		float obj_maxz = gameObjects[i].obj->getMaxZ() + gameObjects[i].z;
		if (i == 14) {
			obj_minx += move;
			obj_maxx += move;
		}
		if (i >= 20 && i <= 22) {
			obj_minx += move;
			obj_maxx += move;
		}
		if (i > 28 && i <= 31) {
			if (i == 30) {
				obj_minz += (move) * (-1);
				obj_maxz += (move) * (-1);
			}
			else {
				obj_minz += move;
				obj_maxz += move;
			}
		}
		if (ballX + sphereRadius >= obj_minx && ballX - sphereRadius <= obj_maxx
			&& ballY + sphereRadius >= obj_miny && ballY - sphereRadius <= obj_maxy
			&& ballZ + sphereRadius >= obj_minz && ballZ - sphereRadius <= obj_maxz) {
			if(isStarted && !isEnd && !isPaused) PlaySound("sounds/normalCollision.wav", NULL, SND_FILENAME | SND_ASYNC);
			ballVelocityY = initialVelocity;
			if(i==46){
				isEnd = true;
			}
			if(i==47){
				PlaySound("sounds/wrong.wav", NULL, SND_FILENAME | SND_ASYNC);
				ballX = 20.0f;
				ballY = 200.0f;
				ballZ = 20.0f;
			}
			isOnObject = true;
			break;
		}
	}
	// 만약 어떤 오브젝트 위에도 없다면 바닥으로 떨어뜨림
	if (!isOnObject) {
		// 바닥에 닿으면 튕기게 설정
		if (ballY - sphereRadius <= 0.0f) {
			ballY = sphereRadius;
			if(isStarted && !isEnd && !isPaused) PlaySound("sounds/normalCollision.wav", NULL, SND_FILENAME | SND_ASYNC);
			ballVelocityY = initialVelocity;  // 튕길 때 초기 속도로 설정
		}
	}
}

void mouse(int button, int state, int x, int y) {
}

void mouseMotion(int x, int y) {
	if(isPaused) return;
}

void mousePassiveMotion(int x, int y) {
	if(isPaused) return;
	// 마우스의 이동 거리 계산
	int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;

	if (x != centerX || y != centerY) {
        glutWarpPointer(centerX, centerY);
    }

	int dx = x - centerX;
	int dy = y - centerY;

	// 카메라 각도 업데이트
	cameraAngle -= dx * 0.0005f;  
	cameraElevation += dy * 0.001f;  

	cameraElevation = std::max(M_PI * (-1) / 2.0, std::min(M_PI / 2.0, cameraElevation));

	if (!isPaused) {
		glutWarpPointer(centerX, centerY);
		glutSetCursor(GLUT_CURSOR_NONE);
	}

	glutPostRedisplay();
}

void keyDown(unsigned char key, int x, int y) {
	keyStates[key] = true; // 키가 눌린 상태로 설정
	if(!isStarted){
		if (key == 13){
			isStarted = true;
			resetGame();
		}
		else if(key == 'q'){
			exit(0);
		}
	}
	if (key == 27) { 
        isPaused = !isPaused;

        if (!isPaused) {
            auto currentTime = std::chrono::steady_clock::now();
            startTime = currentTime - std::chrono::seconds(elapsedTimeAtPause);
            glutSetCursor(GLUT_CURSOR_NONE); // 마우스 커서 숨기기
        } else {
            elapsedTimeAtPause = getElapsedTimeInSeconds();
            glutSetCursor(GLUT_CURSOR_INHERIT); // 마우스 커서 보이기
        }
        return;
    }

    if (isPaused) {
        return;
    }
	if (key == 'r' || key == 'R') {
		resetGame();
	}
}

// 키가 떼어졌을 때 호출되는 함수
void keyUp(unsigned char key, int x, int y) {
	keyStates[key] = false; // 키가 떼어진 상태로 설정
}

void draw_string(void* font, const char* str, float x_position, float y_position, float red, float green, float blue)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	GLfloat emission[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat emission_off[] = { 0.0, 0.0, 0.0, 1.0 };

	gluOrtho2D(-1, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glColor3f(red, green, blue);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);


	glRasterPos3f(x_position, y_position, 0);
	for (unsigned int i = 0; i < strlen(str); i++)
		glutBitmapCharacter(font, str[i]);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission_off);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
}


void draw_obj(ObjParser* objParser)
{
	glBegin(GL_TRIANGLES);
	for (unsigned int n = 0; n < objParser->getFaceSize(); n += 3) {
		glNormal3f(objParser->normal[objParser->normalIdx[n] - 1].x,
			objParser->normal[objParser->normalIdx[n] - 1].y,
			objParser->normal[objParser->normalIdx[n] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n] - 1].x,
			objParser->vertices[objParser->vertexIdx[n] - 1].y,
			objParser->vertices[objParser->vertexIdx[n] - 1].z);

		glNormal3f(objParser->normal[objParser->normalIdx[n + 1] - 1].x,
			objParser->normal[objParser->normalIdx[n + 1] - 1].y,
			objParser->normal[objParser->normalIdx[n + 1] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 1] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].z);

		glNormal3f(objParser->normal[objParser->normalIdx[n + 2] - 1].x,
			objParser->normal[objParser->normalIdx[n + 2] - 1].y,
			objParser->normal[objParser->normalIdx[n + 2] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 2] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].z);
	}
	glEnd();
}

void draw_obj_with_texture(ObjParser* objParser, GLuint names)
{
	glDisable(GL_BLEND);
	// glEnable(GL_TEXTURE_2D);	// texture �� ������ ���� enable
	glBindTexture(GL_TEXTURE_2D, names);
	glBegin(GL_TRIANGLES);
	for (unsigned int n = 0; n < objParser->getFaceSize(); n += 3) {
		glTexCoord2f(objParser->textures[objParser->textureIdx[n] - 1].x,
			objParser->textures[objParser->textureIdx[n] - 1].y);
		glNormal3f(objParser->normal[objParser->normalIdx[n] - 1].x,
			objParser->normal[objParser->normalIdx[n] - 1].y,
			objParser->normal[objParser->normalIdx[n] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n] - 1].x,
			objParser->vertices[objParser->vertexIdx[n] - 1].y,
			objParser->vertices[objParser->vertexIdx[n] - 1].z);

		glTexCoord2f(objParser->textures[objParser->textureIdx[n + 1] - 1].x,
			objParser->textures[objParser->textureIdx[n + 1] - 1].y);
		glNormal3f(objParser->normal[objParser->normalIdx[n + 1] - 1].x,
			objParser->normal[objParser->normalIdx[n + 1] - 1].y,
			objParser->normal[objParser->normalIdx[n + 1] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 1] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].z);

		glTexCoord2f(objParser->textures[objParser->textureIdx[n + 2] - 1].x,
			objParser->textures[objParser->textureIdx[n + 2] - 1].y);
		glNormal3f(objParser->normal[objParser->normalIdx[n + 2] - 1].x,
			objParser->normal[objParser->normalIdx[n + 2] - 1].y,
			objParser->normal[objParser->normalIdx[n + 2] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 2] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].z);
	}
	glEnd();
}

void cubeTexture() {
	glGenTextures(1, &g_nCubeTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, g_nCubeTex);
	int width, height, channels;
	uchar* img0 = readImageData("imgs/lamaca/posx.bmp", &width, &height, &channels);
	uchar* img1 = readImageData("imgs/lamaca/negx.bmp", &width, &height, &channels);
	uchar* img2 = readImageData("imgs/lamaca/posy.bmp", &width, &height, &channels);
	uchar* img3 = readImageData("imgs/lamaca/negy.bmp", &width, &height, &channels);
	uchar* img4 = readImageData("imgs/lamaca/negz.bmp", &width, &height, &channels);
	uchar* img5 = readImageData("imgs/lamaca/posz.bmp", &width, &height, &channels);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE, img0);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE, img1);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE, img2);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE, img3);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE, img4);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width,
		height, 0, GL_RGB, GL_UNSIGNED_BYTE, img5);

	glBindTexture(GL_TEXTURE_CUBE_MAP, g_nCubeTex);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

}

void drawskybox() {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, g_nCubeTex);
	glBegin(GL_QUADS);
	// px
	glTexCoord3d(1.0, -1.0, -1.0); glVertex3f(g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(1.0, -1.0, 1.0); glVertex3f(g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(1.0, 1.0, 1.0); glVertex3f(g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(1.0, 1.0, -1.0); glVertex3f(g_nSkySize, g_nSkySize, -g_nSkySize);
	//nx
	glTexCoord3d(-1.0, -1.0, -1.0); glVertex3f(-g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(-1.0, -1.0, 1.0); glVertex3f(-g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, 1.0); glVertex3f(-g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, -1.0); glVertex3f(-g_nSkySize, g_nSkySize, -g_nSkySize);
	//py
	glTexCoord3d(1.0, 1.0, 1.0); glVertex3f(g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, 1.0); glVertex3f(-g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, -1.0); glVertex3f(-g_nSkySize, g_nSkySize, -g_nSkySize);
	glTexCoord3d(1.0, 1.0, -1.0); glVertex3f(g_nSkySize, g_nSkySize, -g_nSkySize);
	//ny
	glTexCoord3d(1.0, -1.0, 1.0); glVertex3f(g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, -1.0, 1.0); glVertex3f(-g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, -1.0, -1.0); glVertex3f(-g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(1.0, -1.0, -1.0); glVertex3f(g_nSkySize, -g_nSkySize, -g_nSkySize);
	//pz
	glTexCoord3d(1.0, -1.0, 1.0); glVertex3f(g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, -1.0, 1.0); glVertex3f(-g_nSkySize, -g_nSkySize, g_nSkySize);
	glTexCoord3d(-1.0, 1.0, 1.0); glVertex3f(-g_nSkySize, g_nSkySize, g_nSkySize);
	glTexCoord3d(1.0, 1.0, 1.0); glVertex3f(g_nSkySize, g_nSkySize, g_nSkySize);
	//nz
	glTexCoord3d(1.0, -1.0, -1.0); glVertex3f(g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(-1.0, -1.0, -1.0); glVertex3f(-g_nSkySize, -g_nSkySize, -g_nSkySize);
	glTexCoord3d(-1.0, 1.0, -1.0); glVertex3f(-g_nSkySize, g_nSkySize, -g_nSkySize);
	glTexCoord3d(1.0, 1.0, -1.0); glVertex3f(g_nSkySize, g_nSkySize, -g_nSkySize);
	glEnd();

	glPopAttrib();
}

void displayTime(int elapsedSeconds) {
	int minutes = elapsedSeconds / 60;
	int seconds = elapsedSeconds % 60;

	char timeString[6]; 
	snprintf(timeString, sizeof(timeString), "%02d:%02d", minutes, seconds);
	draw_string(GLUT_BITMAP_TIMES_ROMAN_24, timeString, 0.85f, 0.95f, 0.0f, 1.0f, 0.0f);
}


void addGameObject(ObjParser* obj, float x, float y, float z) {
	GameObject gameObject;
	gameObject.obj = obj;
	gameObject.x = x;
	gameObject.y = y;
	gameObject.z = z;
	gameObjects.push_back(gameObject);
}

void resetGame() {
	startTime = std::chrono::steady_clock::now();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)current_width / (float)current_height, 1, 500);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	init();
	isEnd = false;
	previsEnd = false;
}

void resetTimer() {
	startTime = std::chrono::steady_clock::now();
}
void pauseGame() {
    if (!isPaused) { 
        isPaused = true;
        elapsedTimeAtPause = getElapsedTimeInSeconds(); 
        glutSetCursor(GLUT_CURSOR_INHERIT);
    }
}

void resumeGame() {
    if (isPaused) { 
        isPaused = false;
        auto currentTime = std::chrono::steady_clock::now();
        startTime = currentTime - std::chrono::seconds(elapsedTimeAtPause); 
        glutSetCursor(GLUT_CURSOR_NONE);
    }
}

int getElapsedTimeInSeconds() {
    if (isPaused || isEnd) {
        return elapsedTimeAtPause;
    }
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
    elapsedTimeAtPause = elapsed;
    return static_cast<int>(elapsed);
}

