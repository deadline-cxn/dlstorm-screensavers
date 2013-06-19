#include <windows.h>
#include <scrnsave.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glext.h>
#include <time.h>
#include "c3dcube.h"

// Declarations
C3dCube *pCube;
C3dCube *pFirstCube;
bool SetupOpenGL();
void KillGL();
void DrawGLScene();
HDC					hDC=NULL;
HGLRC				hRC=NULL;
HWND				hWnd=NULL;
HINSTANCE			hInstance;
int uTimer;

LONG WINAPI ScreenSaverProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam){
    // Handles screen saver messages
    switch(message)
    {
    case WM_CREATE:
        // Creation of the screen saver window
		hWnd = hwnd;
		SetupOpenGL();
		// Set Timer (Target 60 fps ish (1000/60 = 16)
		uTimer = SetTimer(hwnd, 1, 16, NULL);
        return 0;

    case WM_ERASEBKGND:
        // Erases the screen saver background
		// Not needed with OpenGL
        return 0;

    case WM_TIMER:
        // Handles the timer
		DrawGLScene();
        return 0;

    case WM_DESTROY:
        // Cleans up the screen saver window
		KillTimer(hwnd, uTimer);
		KillGL();
        PostQuitMessage(0);
        return 0;
    }
    return DefScreenSaverProc(hwnd,message,wparam,lparam);
}

BOOL WINAPI ScreenSaverConfigureDialog(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam){
    return true;
}

BOOL WINAPI RegisterDialogClasses(HANDLE hmodule){
    return true;
}

bool SetupOpenGL(){

    srand((unsigned)time(0));
	GLuint PixelFormat;			// Holds The Results After Searching For A Match

	int width;
	int height;

	width = GetDeviceCaps(GetDC(NULL), HORZRES);	// GetDC(NULL) returns a
	height = GetDeviceCaps(GetDC(NULL), VERTRES);	// HDC for the whole screen

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		24,											// Select Our Color Depth (bits)
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGL();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGL();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGL();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGL();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		KillGL();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	// Set Up Our Perspective GL Screen
	if (height==0){height=1;}
	glViewport(0,0,width,height);						// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,1000.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

    // Initialize cubes
    pFirstCube=NULL;
    pFirstCube=new C3dCube();
    pCube=pFirstCube;
    for(int i=0;i<1000;i++) {
            pCube->pNext=new C3dCube();
            pCube=pCube->pNext;
    }
    return TRUE;									// Success
}

void KillGL(void){

    // Delete cubes
    pCube=pFirstCube;
    while(pCube->pNext) {
        pFirstCube=pCube;
        pCube=pCube->pNext;
        delete (pFirstCube);
    }

	ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
	if (hRC){										// Do We Have A Rendering Context?
		wglMakeCurrent(NULL,NULL);					// Release The DC And RC Contexts
		wglDeleteContext(hRC);						// Delete The RC
		hRC=NULL;									// Set RC To NULL
	}
	ReleaseDC(hWnd,hDC);							// Release The DC
}

void DrawGLScene(){
    static float ri,gi,bi;
    static float cr,cg,cb;

    if((ri==0) || (cr<0)) ri=(float)rand()/((float)RAND_MAX/0.002f);
    if((gi==0) || (cg<0)) gi=(float)rand()/((float)RAND_MAX/0.002f);
    if((bi==0) || (cb<0)) bi=(float)rand()/((float)RAND_MAX/0.002f);

    if(cr>1.0f) { cr=1.0f; ri=-(float)rand()/((float)RAND_MAX/0.002f);}
    if(cg>1.0f) { cg=1.0f; gi=-(float)rand()/((float)RAND_MAX/0.002f);}
    if(cb>1.0f) { cb=1.0f; bi=-(float)rand()/((float)RAND_MAX/0.002f);}

    cr+=ri;
    cg+=gi;
    cb+=bi;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pCube=pFirstCube;
    while(pCube->pNext) {
        pCube->Update();
        pCube->Draw();
        pCube=pCube->pNext;
    }

	SwapBuffers(hDC);
}

