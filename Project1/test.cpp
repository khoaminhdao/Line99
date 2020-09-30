#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

#include "OBJLoader.h"
#include "Coordinate.h"

#define PI 3.14159

float g_rotation = 0;

int loop;
int fall;
float ground_colors[1001][1001][4];
float ground_points[1001][1001][3];

float slowdown = 0.5;
float velocity = 0.0;
float zoom = -50.0;
float pan = 0.0;
float tilt = 0.0;
float hailsize = 0.1;
float accum = -10.0;

float r = 0.0;
float g = 1.0;
float b = 0.0;

float eyeX = 50.0,
    eyeY = 150.0, 
    eyeZ = 700.0;

#define MAX_PARTICLES	1000

//---------------------------snow_begin------------------

struct particles
{
	// Life
	bool alive;	// is the particle alive?
	float life;	// particle lifespan
	float fade; // decay

	// color
	float red;
	float green;
	float blue;

	// Position/direction
	float xpos;
	float ypos;
	float zpos;

	// Velocity/Direction, only goes down in y dir
	float vel;

	// Gravity
	float gravity;
};

particles par_sys[MAX_PARTICLES];

void initParticles(int i) 
{
	par_sys[i].alive = true;
	par_sys[i].life = 1.0;
	par_sys[i].fade = float(rand()%100) / 1000.0f + 0.003f;

	par_sys[i].xpos = (float) (rand() % 501) - 250;
	par_sys[i].ypos = 250.0;
	par_sys[i].zpos = (float) (rand() % 501) - 250;

	par_sys[i].red = 0.5;
	par_sys[i].green = 0.5;
	par_sys[i].blue = 1.0;

	par_sys[i].vel = velocity;
	par_sys[i].gravity = -50;
}

void drawSnow() 
{
	float x, y, z;
	for (loop = 0; loop < MAX_PARTICLES; loop=loop+2) 
	{
		if (par_sys[loop].alive == true)
		{
			//float random = (float)(rand()%3)/10;
			x = par_sys[loop].xpos;
			y = par_sys[loop].ypos;
			z = par_sys[loop].zpos + zoom;

			// Draw particles
			glColor3f(1.0, 1.0, 1.0);
			glPushMatrix();
			glTranslatef(x, y, z);
			glutSolidSphere(2, 50, 50);
			glPopMatrix();

			// Update values
			//Move
			par_sys[loop].ypos += par_sys[loop].vel / (slowdown*1000);
			par_sys[loop].vel += par_sys[loop].gravity;
			// Decay
			par_sys[loop].life -= par_sys[loop].fade;

			if (par_sys[loop].ypos <= -10)
			{
				int zi = z - zoom + 500;
				int xi = x + 10 + 500;
				ground_colors[zi][xi][0] = 1.0;
				ground_colors[zi][xi][2] = 1.0;
				ground_colors[zi][xi][3] += 1.0;
				if (ground_colors[zi][xi][3] > 1.0) {
					ground_points[xi][zi][1] += 0.1;
				}
				par_sys[loop].life = -1.0;
			}

			//Revive
			if (par_sys[loop].life < 0.0) {
				initParticles(loop);
			}
		}
	}
}

//---------------------------snow_end-------------------

void init (const char *filename)
{
    int x, z;
	GLfloat lightpos[] = { 5.0f, 10.0f, 10.0f, 1.0f };

	/* Initialize OpenGL context */
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv (GL_LIGHT0, GL_POSITION, lightpos);

    for (z = 0; z < 1001; z++) 
    {
		for (x = 0; x < 1001; x++) 
        {
			ground_points[x][z][0] = x - 10.0;
			ground_points[x][z][1] = accum;
			ground_points[x][z][2] = z - 10.0;

			ground_colors[z][x][0] = r; // red value
			ground_colors[z][x][1] = g; // green value
			ground_colors[z][x][2] = b; // blue value
			ground_colors[z][x][3] = 0.0; // acummulation factor
		}
	}

	// Initialize particles
	for (loop = 0; loop < MAX_PARTICLES; loop++)
    {
		initParticles(loop);
	}
}

void reshape (int w, int h)
{
	if (h == 0)
		h = 1;

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, w/(GLdouble)h, 0.1, 1000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutPostRedisplay();
}

void display ()
{
    int i, j;
	float x, y, z;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	//gluLookAt(50.0, 150.0, 700.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glTranslatef(0.0f, 0.0f, -00.0f);

	/* Draw the model */

	glPushMatrix();
   
	//glRotatef(g_rotation,0,1,0);
	glRotatef(90,0,1,0);
	
	char *filename = "tree.obj";
	if (!ReadOBJModel(filename, &objfile))
		exit(EXIT_FAILURE);
	glTranslatef(100.0, 0.0, 100.0);
	RenderOBJModel(&objfile);
	glPopMatrix();

	glPushMatrix();
    
	//glRotatef(g_rotation, 0, 1, 0);
	//glRotatef(90, 0, 1, 0);
    
	filename = "screen.obj";
	if (!ReadOBJModel (filename, &objfile))
		exit (EXIT_FAILURE);
   
	RenderOBJModel(&objfile);

	glPopMatrix();

    glPushMatrix();
    glColor3f(r, g, b);

	glBegin(GL_QUADS);
	// along z - y const
		for (i = -10; i+1 < 11; i++) 
        {
			// along x - y const
			for (j = -10; j+1 < 11; j++) 
            {
				glColor3fv(ground_colors[i+10][j+10]);
				glVertex3f(ground_points[j+10][i+10][0], 
					ground_points[j+10][i+10][1], 
					ground_points[j+10][i+10][2] + zoom);

				glColor3fv(ground_colors[i+10][j+1+10]);
				glVertex3f(ground_points[j+1+10][i+10][0],
					ground_points[j+1+10][i+10][1],
					ground_points[j+1+10][i+10][2] + zoom);

				glColor3fv(ground_colors[i+1+10][j+1+10]);
				glVertex3f(ground_points[j+1+10][i+1+10][0],
					ground_points[j+1+10][i+1+10][1],
					ground_points[j+1+10][i+1+10][2] + zoom);

				glColor3fv(ground_colors[i+1+10][j+10]);
				glVertex3f(ground_points[j+10][i+1+10][0],
					ground_points[j+10][i+1+10][1],
					ground_points[j+10][i+1+10][2] + zoom);
			}
		}
	glEnd();
    glPopMatrix();

    glPushMatrix();
    //glScalef(10.0, 10.0, 10.0);
	drawSnow();
    glPopMatrix();

	glutSwapBuffers();
}

void keyboard (unsigned char key, int x, int y)
{
    float tempX = eyeX;
    float tempZ = eyeZ;
    float degree = 2.0;

	/* Escape */
    switch (key)
    {
	case 27:
		exit (0);

    case 'a':
        eyeX = tempX * cos(degree * PI/180.0) - tempZ * sin(degree * PI/180.0);
        eyeZ = tempX * sin(degree * PI/180.0) + tempZ * cos(degree * PI/180.0);
        break;

    case 'd':
        eyeX = tempX * cos(-degree * PI/180.0) - tempZ * sin(-degree * PI/180.0);
        eyeZ = tempX * sin(-degree * PI/180.0) + tempZ * cos(-degree * PI/180.0);
        break;
    default:
        return;
    }
}

void timer(int)
{
    //drawCoordinate();
	glutPostRedisplay();
	glutTimerFunc(1000/60, timer, 0);
	//g_rotation++;
}

int main (int argc, char *argv[])
{
	/*if (argc < 2)
	{
		fprintf (stderr, "usage: %s <cottage_obj.obj>\n", argv[0]);
		system("pause");
		return -1;
	}*/

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutCreateWindow("Snow");

	atexit (cleanup);
	init(argv[1]);

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(display);
	glutTimerFunc(0, timer, 0);
	glutMainLoop();

	return 0;
}