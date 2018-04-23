// ShaderSaver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include "SDL.h"
#include "GL\glew.h"

#define PI 3.14159265358979324

using namespace std;

// Globals.
static float R = 40.0; // Radius of circle.
static float X = 50.0; // X-coordinate of center of circle.
static float Y = 50.0; // Y-coordinate of center of circle.
static int numVertices = 5; // Number of vertices on circle.

							// Drawing routine.
void drawScene(void)
{
	float t = 0; // Angle parameter.
	int i;

	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(0.0, 0.0, 0.0);

	// Draw a line loop with vertices at equal angles apart on a circle
	// with center at (X, Y) and radius R, The vertices are colored randomly.
	glBegin(GL_LINE_LOOP);
	for (i = 0; i < numVertices; ++i)
	{
		glColor3f((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);
		glVertex3f(X + R * cos(t), Y + R * sin(t), 0.0);
		t += 2 * PI / numVertices;
	}
	glEnd();
}

// Initialization routine.
void setup(void)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 100.0, 0.0, 100.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
	cout << "Interaction:" << endl;
	cout << "Press +/- to increase/decrease the number of vertices on the circle." << endl;
	cout << "Press esc to quit." << endl;
}

void update(SDL_Window *window, bool *isClosed) {
	SDL_GL_SwapWindow(window);

	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN || (e.type == SDL_MOUSEMOTION && (abs(e.motion.xrel) > 10 || abs(e.motion.yrel) > 10)))
			*isClosed = true;
	}
}

int main(int argc, char *argv[])
{
	string title = "Square";
	if (argc < 2) {
		//return 1;
	}

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "/C") {
			//configure
			title = "Configure";
		}
		else if (arg == "/P") {
			if (i + 1 < argc) { // Make sure we aren't at the end of argv!
				//preview
				title = "Preview";
								//destination = argv[i++]; // Increment 'i' so we don't get the argument as the next argv[i].
			}
			else { // Uh-oh, there was no argument to the destination option.
				std::cerr << "--destination option requires one argument." << std::endl;
				return 1;
			}
		}
		else if (arg == "/S") {
			//Run
			title = "Run";
		}
	}
	int width = 1920;
	int height = 1080;

	bool isClosed = false;

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window *window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GLContext context = SDL_GL_CreateContext(window);

	// This makes our buffer swap syncronized with the monitor's vertical refresh
	SDL_GL_SetSwapInterval(1);

	glewExperimental = GL_TRUE;
	GLenum status = glewInit();

	if (status != GLEW_OK) {
		cerr << "Glew failed to initialise!" << endl;
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	printInteraction();

	while (!isClosed) {
		setup();
		SDL_GL_GetDrawableSize(window, &width, &height);
		resize(width, height);
		drawScene();
		update(window, &isClosed);
	}

	// Delete our OpengL context
	SDL_GL_DeleteContext(context);

	// Destroy our window
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

