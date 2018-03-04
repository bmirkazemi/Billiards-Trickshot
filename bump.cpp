//modified by: Bijan Mirkazemi
//date: 2-24-18
//need help -> line 595
//
//cs335 Sample
//This program demonstrates billiard ball collisions
//Originally designed to be used with air hockey collisions
//
//program: bump.cpp
//author:  Gordon Griesel
//date:    2014, 2017
//
//-------------------------------------------
//Press arrow keys to move a ball
//Press S to slow the ball movement
//Grab and move a ball with mouse left button
//-------------------------------------------
//
//
//Depending on your Linux distribution,
//may have to install these packages:
// libx11-dev
// libglew1.6
// libglew1.6-dev
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#include <time.h>
#include <math.h>
#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"

typedef float Flt;
typedef Flt Vec[3];
//macros to manipulate vectors
#define MakeVector(x,y,z,v) (v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecNegate(a) (a)[0]=(-(a)[0]); (a)[1]=(-(a)[1]); (a)[2]=(-(a)[2]);
#define VecDot(a,b) ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecCopy(a,b) (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];
#define VecCopy2d(a,b) (b)[0]=(a)[0];(b)[1]=(a)[1];
#define VecNegate2d(a) (a)[0]=(-(a)[0]); (a)[1]=(-(a)[1]);
#define VecDot2d(a,b) ((a)[0]*(b)[0]+(a)[1]*(b)[1])
static int xres=920, yres=480;
void setup_screen_res(const int w, const int h);

class X11_wrapper {
    //X Windows variables
    Display *dpy;
    Window win;
    GLXContext glc;
    public:
    X11_wrapper() {
        Window root;
        GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
        //GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
        XVisualInfo *vi;
        Colormap cmap;
        XSetWindowAttributes swa;
        setup_screen_res(xres, yres);
        dpy = XOpenDisplay(NULL);
        if (dpy == NULL) {
            printf("\n\tcannot connect to X server\n\n");
            exit(EXIT_FAILURE);
        }
        root = DefaultRootWindow(dpy);
        vi = glXChooseVisual(dpy, 0, att);
        if (vi == NULL) {
            printf("\n\tno appropriate visual found\n\n");
            exit(EXIT_FAILURE);
        } 
        //else {
        //	// %p creates hexadecimal output like in glxinfo
        //	printf("\n\tvisual %p selected\n", (void *)vi->visualid);
        //}
        cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
        swa.colormap = cmap;
        swa.event_mask =
            ExposureMask |
            KeyPressMask |
            KeyReleaseMask |
            PointerMotionMask |
            ButtonPressMask |
            ButtonReleaseMask |
            StructureNotifyMask |
            SubstructureNotifyMask;
        win = XCreateWindow(dpy, root, 0, 0, xres, yres, 0,
                vi->depth, InputOutput, vi->visual,
                CWColormap | CWEventMask, &swa);
        set_title();
        glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
        glXMakeCurrent(dpy, win, glc);
    }
    ~X11_wrapper() {
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
    }
    void set_title(void) {
        //Set the window title bar.
        XMapWindow(dpy, win);
        XStoreName(dpy, win, "Billiard ball physics");
    }
    bool getPending() {
        return XPending(dpy);
    }
    void getNextEvent(XEvent *e) {
        XNextEvent(dpy, e);
    }
    void swapBuffers() {
        glXSwapBuffers(dpy, win);
    }
} x11;

//#define USE_SOUND
#ifdef USE_SOUND
#include </usr/include/AL/alut.h>
class Openal {
    ALuint alSource[2];
    ALuint alBuffer[2];
    public:
    Openal() {
        //Get started right here.
        alutInit(0, NULL);
        if (alGetError() != AL_NO_ERROR) {
            printf("ERROR: starting sound.\n");
        }
        //Clear error state
        alGetError();
        //Setup the listener.
        //Forward and up vectors are used.
        float vec[6] = {0.0f,0.0f,1.0f, 0.0f,1.0f,0.0f};
        alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
        alListenerfv(AL_ORIENTATION, vec);
        alListenerf(AL_GAIN, 1.0f);
        //
        //Buffer holds the sound information.
        alBuffer[0] = alutCreateBufferFromFile("./billiard.wav");
        alBuffer[1] = alutCreateBufferFromFile("./wall.wav");
        //Source refers to the sound stored in buffer.
        for (int i=0; i<2; i++) {
            alGenSources(1, &alSource[i]);
            alSourcei(alSource[i], AL_BUFFER, alBuffer[i]);
            alSourcef(alSource[i], AL_GAIN, 1.0f);
            if (i==0) alSourcef(alSource[i], AL_GAIN, 0.4f);
            alSourcef(alSource[i], AL_PITCH,1.0f);
            alSourcei(alSource[i], AL_LOOPING,  AL_FALSE);
            if (alGetError() != AL_NO_ERROR) {
                printf("ERROR\n");
            }
        }
    }
    void playSound(int i)
    {
        alSourcePlay(alSource[i]);
    }
    ~Openal() {
        //First delete the source.
        alDeleteSources(1, &alSource[0]);
        alDeleteSources(1, &alSource[1]);
        //Delete the buffer
        alDeleteBuffers(1, &alBuffer[0]);
        alDeleteBuffers(1, &alBuffer[1]);
        //Close out OpenAL itself.
        //unsigned int alSampleSet;
        ALCcontext *Context;
        ALCdevice *Device;
        //Get active context
        Context=alcGetCurrentContext();
        //Get device for active context
        Device=alcGetContextsDevice(Context);
        //Disable context
        alcMakeContextCurrent(NULL);
        //Release context(s)
        alcDestroyContext(Context);
        //Close device
        alcCloseDevice(Device);
    }
} oal;
#endif

void playSound(int s) {
#ifdef USE_SOUND
    oal.playSound(s);
#else
    if (s) {}
#endif
}

int lbump=0;
int lbumphigh=0;

void init_holes(void);
void init_opengl(void);
void init_balls(void);
void check_resize(XEvent *e);
void check_mouse(XEvent *e);
void check_keys(XEvent *e);
void physics(void);
void render(void);

int done=0;
int leftButtonDown=0;
Vec leftButtonPos;
class Ball {
    public:
        Vec pos;
        Vec vel;
        Vec force;
        float radius;
        float mass;
} ball[7];
const int n=7;

class Hole {
    public:
        Vec pos;
        float radius;
} hole[6];
const int m=6;

bool inHole[7] = {true,true,true,true,true,true,true};

//-----------------------------------------------------------------------------
//Setup timers
const double physicsRate = 1.0 / 60.0;
const double oobillion = 1.0 / 1e9;
struct timespec timeStart, timeCurrent;
struct timespec timePause;
double physicsCountdown=0.0;
double timeSpan=0.0;
double timeDiff(struct timespec *start, struct timespec *end) {
    return (double)(end->tv_sec - start->tv_sec ) +
        (double)(end->tv_nsec - start->tv_nsec) * oobillion;
}
void timeCopy(struct timespec *dest, struct timespec *source) {
    memcpy(dest, source, sizeof(struct timespec));
}
//-----------------------------------------------------------------------------


int main(void)
{
    init_opengl();
    init_balls();
    init_holes();
    clock_gettime(CLOCK_REALTIME, &timePause);
    clock_gettime(CLOCK_REALTIME, &timeStart);
    while (!done) {
        while (x11.getPending()) {
            XEvent e;
            x11.getNextEvent(&e);
            check_resize(&e);
            check_mouse(&e);
            check_keys(&e);
        }
        clock_gettime(CLOCK_REALTIME, &timeCurrent);
        timeSpan = timeDiff(&timeStart, &timeCurrent);
        timeCopy(&timeStart, &timeCurrent);
        physicsCountdown += timeSpan;
        while (physicsCountdown >= physicsRate) {
            physics();
            physicsCountdown -= physicsRate;
        }
        render();
        x11.swapBuffers();
    }
    cleanup_fonts();
    return 0;
}

void setup_screen_res(const int w, const int h)
{
    xres = w;
    yres = h;
}

void reshape_window(int width, int height)
{
    //window has been resized.
    setup_screen_res(width, height);
    //
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glOrtho(0, xres, 0, yres, -1, 1);
    x11.set_title();
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, xres, yres);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //This sets 2D mode (no perspective)
    glOrtho(0, xres, 0, yres, -1, 1);
    //Clear the screen
    glClearColor(0, 0.46, 0, 1.0);
    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

#define sphereVolume(r) (r)*(r)*(r)*3.14159265358979*4.0/3.0;

void init_holes(void) 
{
    float rad = 30.0;
    //bottom left corner
    hole[0].pos[0] = 5;
    hole[0].pos[1] = 5;
    hole[0].radius = 30.0;	
    //top left corner	
    hole[1].pos[0] = 5;
    hole[1].pos[1] = yres-5;
    hole[1].radius = rad;	
    //bottom right corner
    hole[2].pos[0] = xres-5;
    hole[2].pos[1] = 5;
    hole[2].radius = rad;	
    //top right corner
    hole[3].pos[0] = xres-5;
    hole[3].pos[1] = yres-5;
    hole[3].radius = rad;	
    //bottom middle
    hole[4].pos[0] = xres/2;
    hole[4].pos[1] = 5;
    hole[4].radius = rad;	
    //top middle
    hole[5].pos[0] = xres/2;
    hole[5].pos[1] = yres-5;
    hole[5].radius = rad;	
}

void init_balls(void)
{	
    float rad = 13.0;
    //cue ball
    ball[0].pos[0] = xres-125;
    ball[0].pos[1] = yres/2;
    ball[0].vel[0] = 0.0;
    ball[0].vel[1] = 0.0;
    ball[0].force[0] = 0.0;
    ball[0].force[1] = 0.0;
    ball[0].radius = rad;
    ball[0].mass = sphereVolume(ball[0].radius);
    //
    //red ball
    ball[1].pos[0] = (xres/2) + 56;
    ball[1].pos[1] = (yres/2) + 30;
    ball[1].vel[0] = 0.0;
    ball[1].vel[1] = 0.0;
    ball[1].force[0] = 0.0;
    ball[1].force[1] = 0.0;
    ball[1].radius = rad;
    ball[1].mass = sphereVolume(ball[1].radius);
    //
    //blue ball
    ball[2].pos[0] = (xres/2) + 80;
    ball[2].pos[1] = (yres/2) + 18;
    ball[2].vel[0] = 0.0;
    ball[2].vel[1] = 0.0;
    ball[2].force[0] = 0.0;
    ball[2].force[1] = 0.0;
    ball[2].radius = rad;
    ball[2].mass = sphereVolume(ball[2].radius);
    //
    //orange ball
    ball[3].pos[0] = (xres/2) + 102;
    ball[3].pos[1] = (yres/2) + 32;
    ball[3].vel[0] = 0.0;
    ball[3].vel[1] = 0.0;
    ball[3].force[0] = 0.0;
    ball[3].force[1] = 0.0;
    ball[3].radius = rad;
    ball[3].mass = sphereVolume(ball[3].radius);
    //
    //black ball
    ball[4].pos[0] = xres/2 + 56;
    ball[4].pos[1] = yres/2 - 30;
    ball[4].vel[0] = 0.0;
    ball[4].vel[1] = 0.0;
    ball[4].force[0] = 0.0;
    ball[4].force[1] = 0.0;
    ball[4].radius = rad;
    ball[4].mass = sphereVolume(ball[4].radius);
    //
    //purple ball
    ball[5].pos[0] = (xres/2) + 80;
    ball[5].pos[1] = (yres/2) - 18;
    ball[5].vel[0] = 0.0;
    ball[5].vel[1] = 0.0;
    ball[5].force[0] = 0.0;
    ball[5].force[1] = 0.0;
    ball[5].radius = rad;
    ball[5].mass = sphereVolume(ball[4].radius);
    //yellow ball
    ball[6].pos[0] = (xres/2) + 102;
    ball[6].pos[1] = (yres/2) - 32;
    ball[6].vel[0] = 0.0;
    ball[6].vel[1] = 0.0;
    ball[6].force[0] = 0.0;
    ball[6].force[1] = 0.0;
    ball[6].radius = rad;
    ball[6].mass = sphereVolume(ball[6].radius);
}

void check_resize(XEvent *e)
{
    //The ConfigureNotify is sent by the
    //server if the window is resized.
    if (e->type != ConfigureNotify)
        return;
    XConfigureEvent xce = e->xconfigure;
    if (xce.width != xres || xce.height != yres) {
        //Window size did change.
        reshape_window(xce.width, xce.height);
    }
}

void check_mouse(XEvent *e)
{
    //Did the mouse move?
    //Was a mouse button clicked?
    static int savex = 0;
    static int savey = 0;
    //
    if (e->type == ButtonRelease) {
        leftButtonDown=0;
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //Left button is down
            leftButtonDown = 1;
            leftButtonPos[0] = (Flt)e->xbutton.x;
            leftButtonPos[1] = (Flt)(yres - e->xbutton.y);
        }
        if (e->xbutton.button==3) {
            //Right button is down
        }
    }
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
        //Mouse moved
        savex = e->xbutton.x;
        savey = e->xbutton.y;
        if (leftButtonDown) {
            leftButtonPos[0] = (Flt)e->xbutton.x;
            leftButtonPos[1] = (Flt)(yres - e->xbutton.y);
        }
    }
}

void check_keys(XEvent *e)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        switch(key) {
            case XK_Left:
                ball[0].vel[0] -= 1.0;
                break;
            case XK_Right:
                ball[0].vel[0] += 1.0;
                break;
            case XK_Up:
                ball[0].vel[1] += 1.0;
                break;
            case XK_Down:
                ball[0].vel[1] -= 1.0;
                break;
            case XK_x: 
                ball[0].vel[0] -= 7.0;
                break;
            case XK_s:
                //press s to slow the balls
                ball[0].vel[0] *= 0.5;
                ball[0].vel[1] *= 0.5;
                ball[1].vel[0] *= 0.5;
                ball[1].vel[1] *= 0.5;
                break;
	    case XK_n:
		init_balls();
		break;
            case XK_Escape:
                done=1;
                break;
        }
    }
}

void VecNormalize(Vec v)
{
    Flt dist = v[0]*v[0]+v[1]*v[1]+v[2]*v[2];
    if (dist==0.0)
        return;
    Flt len = 1.0 / sqrt(dist);
    v[0] *= len;
    v[1] *= len;
    v[2] *= len;
}

void VecNormalize2d(Vec v)
{
    Flt dist = v[0]*v[0]+v[1]*v[1];
    if (dist==0.0)
        return;
    Flt len = 1.0 / sqrt(dist);
    v[0] *= len;
    v[1] *= len;
    //v[2] *= len;
}

void physics(void)
{
    //Update positions
    if (leftButtonDown) {
        //Special physics applied here.
        //Physics employs a penalty system.
        ball[1].vel[0] += ball[1].force[0];
        ball[1].vel[1] += ball[1].force[1];
        ball[1].pos[0] += ball[1].vel[0];
        ball[1].pos[1] += ball[1].vel[1];
        ball[1].force[0] =
            ball[1].force[1] = 0.0;
        ball[0].pos[0] = leftButtonPos[0];
        ball[0].pos[1] = leftButtonPos[1];
        Vec v;
        v[0] = ball[1].pos[0] - ball[0].pos[0];
        v[1] = ball[1].pos[1] - ball[0].pos[1];
        Flt distance = sqrt(v[0]*v[0] + v[1]*v[1]);
        if (distance < (ball[0].radius + ball[1].radius)) {
            ball[1].force[0] = v[0]*0.01;
            ball[1].force[1] = v[1]*0.01;
        }
        ball[1].vel[0] *= 0.99;
        ball[1].vel[1] *= 0.99;
        //
        //bounce off window sides...
        if (ball[1].pos[0] < ball[1].radius) {
            ball[1].pos[0] = ball[1].radius + 0.1;
            if (ball[1].vel[0] < 0.0)
                ball[1].vel[0] = -ball[1].vel[0];
            playSound(1);
        }
        if (ball[1].pos[0] >= (Flt)xres-ball[1].radius) {
            ball[1].pos[0] = (Flt)xres-ball[1].radius - 0.1;
            if (ball[1].vel[0] > 0.0)
                ball[1].vel[0] = -ball[1].vel[0];
            playSound(1);
        }
        if (ball[1].pos[1] < ball[1].radius) {
            ball[1].pos[1] = ball[1].radius + 0.1;
            if (ball[1].vel[1] < 0.0)
                ball[1].vel[1] = -ball[1].vel[1];
            playSound(1);
        }
        if (ball[1].pos[1] >= (Flt)yres-ball[1].radius) {
            ball[1].pos[1] = (Flt)yres-ball[1].radius - 0.1;
            if (ball[1].vel[1] > 0.0)
                ball[1].vel[1] = -ball[1].vel[1];
            playSound(1);
        }
        return;
    }
    //Different physics applied here...
    //100% elastic collisions. 
    for (int i=0; i<n; i++) {
        ball[i].pos[0] += ball[i].vel[0];
        ball[i].pos[1] += ball[i].vel[1];
    }
    //check for collision here
    Flt distance, speed, dist2;
    Flt movi[2], movj[2];
    Flt massFactor, massi, massj;
    Vec vcontact[2];
    int vx, vy;
    Vec vmove[2];
    Flt dot0, dot1;

    for (int i=0; i<m; i++) {
        for (int j=0; j<n; j++) {
			vx = hole[i].pos[0] - ball[j].pos[0]; 
			vy = hole[i].pos[1] - ball[j].pos[1];
			dist2 = sqrt(vx*vx + vy*vy);
			if (dist2 < (hole[i].radius + ball[j].radius)) {
				//ball is in the hole!
				//ball[j].vel[0] = 0.2;
				//ball[j].vel[1] = 0.2;
				//inHole[j] = false;
				ball[j].pos[0] = 1000;
				ball[j].vel[0] = 0.0;
				ball[j].vel[1] = 0.0;
			}
        }
    }

    for (int i=0; i<n-1; i++) {
        for (int j=i+1; j<n; j++) {
            //vx = ball[i].pos[0] - ball[j].pos[0];
            //vy = ball[i].pos[1] - ball[j].pos[1];
            vcontact[0][0] = ball[i].pos[0] - ball[j].pos[0];
            vcontact[0][1] = ball[i].pos[1] - ball[j].pos[1];
            //vcontact[0][2] = 0.0;
            distance = sqrt(vcontact[0][0]*vcontact[0][0] +
                    vcontact[0][1]*vcontact[0][1]);
            if (distance < (ball[i].radius + ball[j].radius)) {
                //We have a collision!
                playSound(0);
                //vector from center to center.
                VecNormalize2d(vcontact[0]);
                VecCopy2d(vcontact[0], vcontact[1]);
                VecNegate2d(vcontact[1]);
                movi[0] = ball[i].vel[0];
                movi[1] = ball[i].vel[1];
                movj[0] = ball[j].vel[0];
                movj[1] = ball[j].vel[1];
                vmove[0][0] = movi[0];
                vmove[0][1] = movi[1];
                VecNormalize2d(vmove[0]);
                vmove[1][0] = movj[0];
                vmove[1][1] = movj[1];
                VecNormalize2d(vmove[1]);
                //Determine how direct the hit was...
                dot0 = VecDot2d(vcontact[0], vmove[0]);
                dot1 = VecDot2d(vcontact[1], vmove[1]);
                //Find the closing (relative) speed of the objects...
                speed =
                    sqrtf(movi[0]*movi[0] + movi[1]*movi[1]) * dot0 +
                    sqrtf(movj[0]*movj[0] + movj[1]*movj[1]) * dot1;
                if (speed < 0.0) {
                    //Normalize the mass of each object...
                    massFactor = 2.0 / (ball[i].mass + ball[j].mass);
                    massi = ball[i].mass * massFactor;
                    massj = ball[j].mass * massFactor;
                    ball[j].vel[0] += vcontact[0][0] * speed * massi;
                    ball[j].vel[1] += vcontact[0][1] * speed * massi;
                    ball[i].vel[0] += vcontact[1][0] * speed * massj;
                    ball[i].vel[1] += vcontact[1][1] * speed * massj;
                }
            }
        }
    }
    //Check for collision with window edges
    for (int i=0; i<n; i++) {
        if (ball[i].pos[0] < ball[i].radius && ball[i].vel[0] <= 0.0) {
            ball[i].vel[0] = -ball[i].vel[0];
            playSound(1);
        }
        if (ball[i].pos[0] >= (Flt)xres-ball[i].radius &&
                ball[i].vel[0] >= 0.0) {
            ball[i].vel[0] = -ball[i].vel[0];
            playSound(1);
        }
        if (ball[i].pos[1] < ball[i].radius && ball[i].vel[1] <= 0.0) {
            ball[i].vel[1] = -ball[i].vel[1];
            playSound(1);
        }
        if (ball[i].pos[1] >= (Flt)yres-ball[i].radius &&
                ball[i].vel[1] >= 0.0) {
            ball[i].vel[1] = -ball[i].vel[1];
            playSound(1);
        }
    }
}

void drawBall(Flt rad)
{
    int i;
    static int firsttime=1;
    static Flt verts[32][2];
    static int n=32;
    if (firsttime) {
        Flt ang=0.0;
        Flt inc = 3.14159 * 2.0 / (Flt)n;
        for (i=0; i<n; i++) {
            verts[i][0] = sin(ang);
            verts[i][1] = cos(ang);
            ang += inc;
        }
        firsttime=0;
    }
    glBegin(GL_TRIANGLE_FAN);
    for (i=0; i<n; i++) {
        glVertex2f(verts[i][0]*rad, verts[i][1]*rad);
    }
    glEnd();
}


void render(void)
{
    Rect r;
    glClear(GL_COLOR_BUFFER_BIT);
    //
    //draw holes
    for(int i=0; i<m; i++) {
        glColor3ub(0,0,0);
        glPushMatrix();
        glTranslatef(hole[i].pos[0], hole[i].pos[1], hole[i].pos[2]);
        drawBall(hole[i].radius);
        glPopMatrix();
    }

	//draw balls
	//que ball
	glColor3ub(255,255,255);
	glPushMatrix();
	glTranslatef(ball[0].pos[0], ball[0].pos[1], ball[0].pos[2]);
	drawBall(ball[0].radius);
	glPopMatrix();
	//red ball
	if (inHole[1]) {
	  glColor3ub(150,0,0);
	  glPushMatrix();
	  glTranslatef(ball[1].pos[0], ball[1].pos[1], ball[1].pos[2]);
	  drawBall(ball[1].radius);
	  glColor3ub(255,255,255);
	  drawBall(7.0);
	  glPopMatrix();
	  //r.center = 1;
	  r.left = ball[1].pos[0];
	  r.bot  = ball[1].pos[1]-5;
	  ggprint8b(&r, 16, 0x00000000, "3");
	}
	//blue ball
	if (inHole[2]) {
	  glColor3ub(8,83,204);
	  glPushMatrix();
	  glTranslatef(ball[2].pos[0], ball[2].pos[1], ball[2].pos[2]);
	  drawBall(ball[2].radius);
	  glColor3ub(255,255,255);
	  drawBall(7.0);
	  glPopMatrix();
	  r.left = ball[2].pos[0];
	  r.bot  = ball[2].pos[1]-5;
	  ggprint8b(&r, 16, 0x00000000, "2");
	}
	//orange ball
	if (inHole[3]) {
	  glColor3ub(219,133,4);
	  glPushMatrix();
	  glTranslatef(ball[3].pos[0], ball[3].pos[1], ball[3].pos[2]);
	  drawBall(ball[3].radius);
	  glColor3ub(255,255,255);
	  drawBall(7.0);
	  glPopMatrix();
	  r.left = ball[3].pos[0];
	  r.bot  = ball[3].pos[1]-5;
	  ggprint8b(&r, 16, 0x00000000, "5");
	}
	//black ball
	if (inHole[4]) {
	  glColor3ub(0,0,0);
	  glPushMatrix();
	  glTranslatef(ball[4].pos[0], ball[4].pos[1], ball[4].pos[2]);
	  drawBall(ball[4].radius);
	  glColor3ub(255,255,255);
	  drawBall(7.0);
	  glPopMatrix();
	  r.left = ball[4].pos[0];
	  r.bot  = ball[4].pos[1]-5;
	  ggprint8b(&r, 16, 0x00000000, "8");
	}
	//purple ball
	if (inHole[5]) {
	  glColor3ub(132,25,198);
	  glPushMatrix();
	  glTranslatef(ball[5].pos[0], ball[5].pos[1], ball[5].pos[2]);
	  drawBall(ball[5].radius);
	  glColor3ub(255,255,255);
	  drawBall(7.0);
	  glPopMatrix();
	  r.left = ball[5].pos[0];
	  r.bot  = ball[5].pos[1]-5;
	  ggprint8b(&r, 16, 0x00000000, "4");
	}
	//yellow ball
	if (inHole[6]) {
	  glColor3ub(249,249,13);
	  glPushMatrix();
	  glTranslatef(ball[6].pos[0], ball[6].pos[1], ball[6].pos[2]);
	  drawBall(ball[5].radius);
	  glColor3ub(255,255,255);
	  drawBall(7.0);
	  glPopMatrix();
	  r.left = ball[6].pos[0];
	  r.bot  = ball[6].pos[1]-5;
	  ggprint8b(&r, 16, 0x00000000, "1");
	}
	//
	r.bot = yres/2;
	r.left = 10;
	r.center = 0;
	ggprint8b(&r, 16, 0x0000000, "n for new balls");
	ggprint8b(&r, 16, 0x0000000, "x to hit cue ball");
	//ggprint8b(&r, 16, 0x0000000, "S - Slow down movement");
	//ggprint8b(&r, 16, 0x0000000, "hold down 1, 2, 3, 4");
	//char ts[16];
	//sprintf(ts, "%i", lbumphigh);
	//ggprint8b(&r, 16, 0x00ff000, ts);
	/*
	   r.center = 1;
	   r.left = ball[1].pos[0];
	   r.bot  = ball[1].pos[1]-4;
	   ggprint8b(&r, 16, 0x00000000, "3");
	   r.left = ball[2].pos[0];
	   r.bot  = ball[2].pos[1]-4;
	   ggprint8b(&r, 16, 0x00000000, "2");
	   r.left = ball[3].pos[0];
	   r.bot  = ball[3].pos[1]-4;
	   ggprint8b(&r, 16, 0x00000000, "5");
	   r.left = ball[4].pos[0];
	   r.bot  = ball[4].pos[1]-4;
	   ggprint8b(&r, 16, 0xffffffff, "8");
	   r.left = ball[5].pos[0];
	   r.bot  = ball[5].pos[1]-4;
	   ggprint8b(&r, 16, 0x00000000, "4");
	   r.left = ball[6].pos[0];
	   r.bot  = ball[6].pos[1]-4;
	   ggprint8b(&r, 16, 0x00000000, "1");
	   */
}



