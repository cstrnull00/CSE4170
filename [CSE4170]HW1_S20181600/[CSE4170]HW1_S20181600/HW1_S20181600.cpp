#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <utility>
#include <vector>
#include <math.h>

using namespace std;

#define MAX_POINTS 128
#define PI 3.1415926
#define ROTATE_DEGREE 1

vector<pair<float, float>> vertices;
pair<float, float> center;

bool polygonized = false;
bool rotating = false;
bool rightbuttonpressed = false;

int wWidth = 800, wHeight = 600;
float prev_x = 0.0f, prev_y = 0.0f;
float r = 0.0f, g = 0.0f, b = 0.0f; // Backgroud color = Black

pair<float, float> centroid() {
	float area = 0.0f, cx = 0.0f, cy = 0.0f, x1, x2, y1, y2;

	for (int i = 0; i < vertices.size() - 1; i++) {
		int j = (i + 1) % vertices.size();

		x1 = vertices[i].first; x2 = vertices[j].first;
		y1 = vertices[i].second; y2 = vertices[j].second;

		area += x1 * y2;
		area -= y1 * x2;

		cx += ((x1 + x2) * ((x1 * y2) - (x2 * y1)));
		cy += ((y1 + y2) * ((x1 * y2) - (x2 * y1)));
	}
	area *= 3.0f;
	area = fabs(area);

	return { cx / area, cy / area };
}

pair<float, float> rotMatrix(pair<float, float> coordi) {
	float x = coordi.first;
	float y = coordi.second;

	float tmp = x;

	x = (float)cos(ROTATE_DEGREE * PI / 180) * x - (float)sin(ROTATE_DEGREE * PI / 180) * y;
	y = (float)sin(ROTATE_DEGREE * PI / 180) * tmp + (float)cos(ROTATE_DEGREE * PI / 180) * y;

	return { x, y };
}

void display(void) {
	glClearColor(r, g, b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(1.0f, 1.0f, 1.0f);
	glPointSize(5.0f);

	if (!vertices.empty()) {
		for (int i = 0; i < vertices.size(); i++) {
			glBegin(GL_POINTS);
			glVertex2f(vertices[i].first, vertices[i].second);
		}
		glEnd();

		if (vertices.size() > 1) {
			glColor3f(0.0f, 0.8f, 0.0f);
			for (int i = 0; i < vertices.size(); i++) {
				glBegin(GL_LINE_STRIP);
				glVertex2f(vertices[i].first, vertices[i].second);
			}
			glEnd();
		}
	}

	if (rotating && polygonized) {
		center = centroid();

		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_POINTS);
		glVertex2f(center.first, center.second);
	}

	glEnd();
	glFlush();
}

void timer(int value) {
	if (rotating && polygonized) {
		//pair<float, float> center = centroid();

		for (int i = 0; i < vertices.size(); i++) {
			vertices[i].first -= center.first;
			vertices[i].second -= center.second;
			vertices[i] = rotMatrix(vertices[i]);
			vertices[i].first += center.first;
			vertices[i].second += center.second;
		}
		glutPostRedisplay();
	}
	glutTimerFunc(30, timer, 1);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'r':
		if (!rotating && polygonized) {
			rotating = true;
			glutPostRedisplay();
		}
		else {
			rotating = false;
			glutPostRedisplay();
		}
		break;
	case 'p':
		if (vertices.size() > 2) {
			vertices.push_back(vertices.front());
			polygonized = true;
			glutPostRedisplay();
		}
		else fprintf(stdout, "ERROR: Choose at least 3 points!\n");
		break;
	case 'c':
		if (!rotating) {
			vertices.clear();
			polygonized = false;
			glutPostRedisplay();
		}
		break;
	case 'f':
		glutLeaveMainLoop();
		break;
	}
}

void special(int key, int x, int y) {
	if (!rotating && polygonized) {
		switch (key) {
		case GLUT_KEY_LEFT:
			for (int i = 0; i < vertices.size(); i++)
				vertices[i].first -= 0.05f;
			glutPostRedisplay();
			break;
		case GLUT_KEY_RIGHT:
			for (int i = 0; i < vertices.size(); i++)
				vertices[i].first += 0.05f;
			glutPostRedisplay();
			break;
		case GLUT_KEY_DOWN:
			for (int i = 0; i < vertices.size(); i++)
				vertices[i].second -= 0.05f;
			glutPostRedisplay();
			break;
		case GLUT_KEY_UP:
			for (int i = 0; i < vertices.size(); i++)
				vertices[i].second += 0.05f;
			glutPostRedisplay();
			break;
		}
	}
}

void mousepress(int button, int state, int x, int y) {
	int specialKey = glutGetModifiers();

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN) && (specialKey == GLUT_ACTIVE_SHIFT) && !polygonized) {
		fprintf(stdout, "*** The left mouse button was pressed at (%d, %d).\n", x, y);
		vertices.push_back({ ((float)x * 2 - (float)wWidth) / (float)wWidth , ((float)wHeight - (float)y * 2) / (float)wHeight });
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
		rightbuttonpressed = true;
		prev_x = x; prev_y = y;
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP))
		rightbuttonpressed = false;
}

void mousemove(int x, int y) {
	if (rightbuttonpressed && polygonized && !rotating) {
		//fprintf(stdout, "$$$ The right mouse button is now at (%d, %d).\n", x, y);
		for (int i = 0; i < vertices.size(); i++) {
			vertices[i].first += (x - prev_x) * 2 / (float)wWidth;
			vertices[i].second += (prev_y - y) * 2 / (float)wHeight;
		}
		prev_x = x; prev_y = y;
		glutPostRedisplay();
	}
}

void reshape(int width, int height) {
	fprintf(stdout, "### The new window size is %dx%d.\n", width, height);

	glViewport(0, 0, width, height);

	for (int i = 0; i < vertices.size(); i++) {
		vertices[i].first *= ((float)width / (float)wWidth);
		vertices[i].second *= ((float)height / (float)wHeight);
	}

	wWidth = width; wHeight = height;

	glutPostRedisplay();
}

void close(void) {
	fprintf(stdout, "\n^^^ The control is at the close callback function now.\n\n");
}

void register_callbacks(void) {
	glutTimerFunc(30, timer, 1);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mousepress);
	glutMotionFunc(mousemove);
	glutReshapeFunc(reshape);
	glutCloseFunc(close);
}

void initialize_renderer(void) {
	register_callbacks();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = TRUE;
	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char* program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 4
int main(int argc, char* argv[]) {
	char program_name[64] = "[CSE4170]HW1_S20181600";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'p', 'c', 'r', 'f'",
		"    - Special keys used: LEFT, RIGHT, UP, DOWN",
		"    - Mouse used: L-click, R-click and move",
		"    - Other operations: window size change"
	};

	glutInit(&argc, argv);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE); // <-- Be sure to use this profile for this example code!
 //	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_RGBA);

	glutInitWindowSize(wWidth, wHeight);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	// glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_EXIT); // default
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glutMainLoop();
	fprintf(stdout, "^^^ The control is at the end of main function now.\n\n");

	return 0;
}