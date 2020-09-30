//-----------------------------------------------------------------------------
//   name: Waterfalls.cpp
//   desc: Multiple waterfalls to visualize multiple WAVE ins (stems).
// 	   	   Modeled after Beatles Rock Band, sort of.
//  usage: Modify filenameArray, mus_file_array, and g_num_soundfiles to play
//         something other than "And Your Bird Can Sing".
//
// author: Regina Collecchia 
//   date: 11/4/13
//   uses: RtAudio by Gary Scavone, RgbImage by Samuel R. Buss
//-----------------------------------------------------------------------------

#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#include "RtAudio.h"
#include "Thread.h"
#include "Stk.h"
#include "chuck_fft.h"
#include "Waterfall.h"
#include "WvIn.h"
#include "RgbImage.h"
// #include "MFCC.h"

#if defined(__APPLE__)
	#include <GLUT/glut.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
#endif

// Stk capitalisation
#define FALSE 0
#define TRUE 1
// our datetype
#define SAMPLE double
// corresponding format for RtAudio
#define MY_FORMAT RTAUDIO_FLOAT64
// sample rate
#define MY_SRATE 48000
// number of channels
#define MY_CHANNELS 2
// zero-padding factor
#define ZPF 1
// for convenience
#define MY_PIE 3.14159265358979
#define SND_BUFFER_SIZE 512
// fft size
#define SND_FFT_SIZE ( SND_BUFFER_SIZE * 2 )
#define INC_VAL_MOUSE 1.0f
#define INC_VAL_KB .025f

using namespace std;

// width and height
GLsizei g_width = 1024;
GLsizei g_height = 720;
GLsizei g_last_width = g_width;
GLsizei g_last_height = g_height;

// fill mode
GLenum g_fillmode = GL_FILL;
// modelview stuff
GLfloat g_angle_y = 0.0f;
GLfloat g_inc = 0.0f;
GLfloat g_eye_y = 0.0f;

// Defines a point in a 3D space (coords x, y and z)
struct pt3d
{
    pt3d( GLfloat x, GLfloat y, GLfloat z ) : x(x), y(y), z(z) {};
    
    float x;
    float y;
    float z;
};

// Camera control global variables
pt3d g_look_from( 0, 0, 1);
pt3d g_look_to( 0, 0, 0 );
pt3d g_head_up( 0, 1, 0 );

// global audio buffers
float * g_audio_buffer = NULL;
float * g_stereo_buffer = NULL;
float * g_window = NULL;
float * g_fft_buffer = NULL;
unsigned int g_buffer_size = SND_BUFFER_SIZE;
unsigned int g_fft_size = SND_FFT_SIZE;

// Mutex g_mutex;

// default sample rate. using MY_SRATE in here (48k).
#if defined(__LINUX_ALSA__) || defined(__LINUX_OSS__) || defined(__LINUX_JACK__)
  GLuint g_srate = 48000;
#else
  GLuint g_srate = MY_SRATE;
#endif

// user's choices: global flags with defaults
// use window?
GLboolean g_use_window = FALSE;
// fullscreen
GLboolean g_fullscreen = FALSE;
// put a donk on it
GLboolean g_put_a_donk_on_it = FALSE;

// rotation increments
GLfloat g_inc_val_mouse = INC_VAL_MOUSE;
GLfloat g_inc_val_kb = INC_VAL_KB;

// zpos increment
GLfloat g_dz = 0.1f;

// numb3rs
double g_thresh = 5.0;
float g_fall = 0.1f;
double g_log_factor = 1;
const float deg2rad = MY_PIE / 180;
bool play_all = true;
bool play_drums = false;
bool play_guitar = false;
bool play_vocals = false;
bool play_bass = false;
bool play_tamb = false;

// number of input soundfiles: could make this more scalable, but meh
const int g_num_soundfiles = 5;
// and the appropriate number of waterfalls to represent the soundfiles
Waterfall g_wf[g_num_soundfiles];
double g_log_space[g_num_soundfiles];
double g_avg_pow[g_num_soundfiles];
// when soloed, don't show other tracks
float alphas[g_num_soundfiles];
float g_fft_gain = 2.0f;

float g_soundfile_buffer[g_num_soundfiles][SND_BUFFER_SIZE*2];

static GLuint textureName[g_num_soundfiles];
const char * filenameArray[g_num_soundfiles] = {
	"images/drums.bmp", 
	"images/guitar.bmp",
	"images/mic.bmp",
	"images/bass.bmp",
	"images/tamb.bmp"
};
const char * mus_file_array[g_num_soundfiles] = {
	"/Users/probraino/Desktop/bird-drums.wav",
	"/Users/probraino/Desktop/bird-guitar.wav",
	"/Users/probraino/Desktop/bird-vocals.wav",
	"/Users/probraino/Desktop/bird-bass.wav",
	"/Users/probraino/Desktop/bird-tamb.wav"
};
WvIn * g_input_music[g_num_soundfiles];



//-----------------------------------------------------------------------------
// our function prototypes
//-----------------------------------------------------------------------------
void help( );
void idleFunc( );
void displayFunc( );
void reshapeFunc( GLint width, GLint height );
void keyboardFunc( unsigned char, int, int );
void mouseFunc( int button, int state, int x, int y );
void initGfx( );
void changeLookAt( pt3d look_from, pt3d look_to, pt3d head_up );
void drawTambourine( float scale, float x, float y, float tempo, float inst_total );
void loadTextureFromFile( char * filename );
void initFiveImages( const char * filenames[] );
void initAudioFiles( const char * filenames[] );
void drawTextureQuad( int i );


//-----------------------------------------------------------------------------
// name: help()
// desc: shows me what is up
//-----------------------------------------------------------------------------

void help()
{
    fprintf( stderr, "-------------------------------------------------\n");
    fprintf( stderr, "Welcome to Stems Visualizer. These are the stems. \n");
    fprintf( stderr, "Tell me that you heard every sound there is... \n" );
    fprintf( stderr, "-------------------------------------------------\n");
    fprintf( stderr, "Keystrokes: \n" );
    fprintf( stderr, "\n" );
    fprintf( stderr, "'q' - quit \n" );
    fprintf( stderr, "'f' - toggle fullscreen mode \n" );
    fprintf( stderr, "'d' - put a donk on it, take a donk off of it \n" );
    fprintf( stderr, "'1' - solo track 1 (drums and palm mutes) \n" );
    fprintf( stderr, "'2' - solo track 2 (guitar) \n" );
    fprintf( stderr, "'3' - solo track 3 (vocals) \n" );
    fprintf( stderr, "'4' - solo track 4 (bass) \n" );
    fprintf( stderr, "'5' - solo track 5 (tambourine and the rest) \n" );
    fprintf( stderr, "'0' - play all tracks (default) \n" );
    fprintf( stderr, "'j', mousedown - spin left around the waterfall, increasingly \n" );
    fprintf( stderr, "'i' - increase the gain of the FFT by 1.0 \n" );
    fprintf( stderr, "'l' - spin right around the waterfall, increasingly \n" );
    fprintf( stderr, "'k' - decrease the gain of the FFT by 1.0 \n" );
    fprintf( stderr, "-------------------------------------------------\n");
}

//-----------------------------------------------------------------------------
// name: callme()
// desc: audio callback
//-----------------------------------------------------------------------------

int audio_callback( void * outputBuffer, void * inputBuffer, unsigned int numFrames, 
			double streamTime, RtAudioStreamStatus status, void * data )
{	
    // cast!
    // unused mic input
    // SAMPLE * input = (SAMPLE *)inputBuffer;
    SAMPLE * output = (SAMPLE *)outputBuffer;
		
	memset( g_audio_buffer, 0, numFrames * sizeof(float) );
    for( int f = 0; f < g_num_soundfiles; f++ )
    {
        g_avg_pow[f] = 0.0f;
    }
    
	// fill
	for( size_t i = 0; i < numFrames; i++ )
	{
		output[i*2] = 0;
		output[i*2+1] = 0;
		// for mono input
		// g_audio_buffer[i] = (input[i*2] + input[i*2+1])/2;
		
		for(int f = 0; f < g_num_soundfiles; f++)
		{
			g_soundfile_buffer[f][i] = g_input_music[f]->tick();
            if ( play_all == true )
            {
                output[i*2] += g_soundfile_buffer[f][i];
                output[i*2+1] += g_soundfile_buffer[f][i];
                alphas[f] = 1.0f;
            } else if ( play_drums == true )
            {
                output[i*2] = g_soundfile_buffer[0][i];
                output[i*2+1] = g_soundfile_buffer[0][i];
                alphas[f] = 0.2f;
                alphas[0] = 1.0f;
            } else if ( play_guitar == true )
            {
                output[i*2] = g_soundfile_buffer[1][i];
                output[i*2+1] = g_soundfile_buffer[1][i];
                alphas[f] = 0.2f;
                alphas[1] = 1.0f;
            } else if ( play_vocals == true )
            {
                output[i*2] = g_soundfile_buffer[2][i];
                output[i*2+1] = g_soundfile_buffer[2][i];
                alphas[f] = 0.2f;
                alphas[2] = 1.0f;
            } else if ( play_bass == true )
            {
                output[i*2] = g_soundfile_buffer[3][i];
                output[i*2+1] = g_soundfile_buffer[3][i];
                alphas[f] = 0.2f;
                alphas[3] = 1.0f;
            } else if ( play_tamb == true )
            {
                output[i*2] = g_soundfile_buffer[4][i];
                output[i*2+1] = g_soundfile_buffer[4][i];
                alphas[f] = 0.2f;
                alphas[4] = 1.0f;
            } else { // just in case :)
                output[i*2] += g_soundfile_buffer[f][i];
                output[i*2+1] += g_soundfile_buffer[f][i];
                alphas[f] = 1.0f;
            }
			
            // get average power for entire buffer, use it to pulse the size of the waterfall
            g_avg_pow[f] += pow(g_soundfile_buffer[f][i], 2)/(0.5f*(float)numFrames);
            
            /* if (i > 4)
            {
                g_avg_pow[f] += pow(g_soundfile_buffer[f][i], 2) + pow(g_soundfile_buffer[f][i-1], 2) + pow(g_soundfile_buffer[f][i-2], 2) + pow(g_soundfile_buffer[f][i-3], 2) + pow(g_soundfile_buffer[f][i-4], 2);
            } else {
                g_avg_pow[f] = 0.0f;
            } */
			// loop it...something like this
			// if(g_soundfile_buffer[f].isFinished())
			// 	g_soundfile_buffer[f].reset();
		}
	}
	
	// g_ready = TRUE:
    
    return 0;
}



//-----------------------------------------------------------------------------
// name: main()
// desc: entry point
//-----------------------------------------------------------------------------
int main( int argc, char ** argv )
{
    Stk::setSampleRate(MY_SRATE);
    
	RtAudio g_audio;
	// unsigned int bufferFrames = 512;
		    
    // check if audio devices exist
    if ( g_audio.getDeviceCount() < 1 ) {
        // nopes
        cout << "No audio devices found!" << endl;
        exit ( 1 );
    }
	    
    // let RtAudio print messages to stderr
    g_audio.showWarnings( true );
	
    // set input and output params
    RtAudio::StreamParameters inParams, outParams;
    inParams.deviceId = g_audio.getDefaultInputDevice(); // mic
    inParams.nChannels = 2;
    inParams.firstChannel = 0;
    outParams.deviceId = g_audio.getDefaultOutputDevice(); // speakers
    outParams.nChannels = 2;
    outParams.firstChannel = 0;

    // create stream options
    RtAudio::StreamOptions options;
    
    initAudioFiles( mus_file_array );
			
    // initialize rtaudio & set the audio callback
    try
    {
		g_fft_buffer = new float[g_fft_size * 2 * ZPF];
	
        // if filename, for i < num_sound_files..., if flag="bird"
        // input_music = new WvIn[5];
        
        // g_buffer_size = (unsigned int)input_music[0].getSize;
        g_audio_buffer = new float[g_buffer_size];
		g_stereo_buffer = new float[g_buffer_size * 2];

        for( int i = 0; i < g_num_soundfiles; i++ ) g_wf[i].init( g_buffer_size, g_fft_size, MY_SRATE, MY_CHANNELS );
        
		g_window = new float[g_buffer_size];
	
		// make the transform window
		make_window( g_window, g_buffer_size );

        // open the audio device for capture and playback
        g_audio.openStream( &outParams, &inParams, MY_FORMAT, MY_SRATE, &g_buffer_size, &audio_callback, NULL, &options );

		// start the audio stream
		g_audio.startStream();
    }
    catch( RtError& e ) {
        // error!
        cout << e.getMessage() << endl;
		exit( 1 );
        // goto cleanup;
    }
	
    // initialize GLUT
    glutInit( &argc, argv );
    // double buffer, use rgb color, enable depth buffer: makes for more fluid graphics by drawing "offscreen" first
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    // initialize the window size
    glutInitWindowSize( g_width, g_height );
    // set the window postion
    glutInitWindowPosition( 100, 100 );
    // create the window
    glutCreateWindow( "Rock Band" );
	// full screen here
	if( g_fullscreen ) glutFullScreen();
	
	initFiveImages( filenameArray );
	
    // set up the callback functions for glut
    // set the idle function - called when idle
    glutIdleFunc( idleFunc );
    // set the display function - called when redrawing
    glutDisplayFunc( displayFunc );
    // set the reshape function - called when client area changes (i.e. the window shape changes)
    glutReshapeFunc( reshapeFunc );
    // set the keyboard function - called on keyboard events
    glutKeyboardFunc( keyboardFunc );
    // set the mouse function - called on mouse stuff
    glutMouseFunc( mouseFunc );
	
    // our own initialization
    initGfx();
	
	// let GLUT handle the current thread from here
    glutMainLoop();
	
	try {
		// if we get here, stop!
		g_audio.stopStream();
	}
	catch( RtError & e ) {
		cout << e.getMessage() << endl;
	}
	
	cleanup:
	if(g_audio.isStreamOpen())
	{
		g_audio.closeStream();
	}
	
    // done
    return 0;
}

//-----------------------------------------------------------------------------
// Name: initGfx( )
// Desc: called at initialization
//-----------------------------------------------------------------------------
void initGfx()
{    
    // set clear color
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	// set shading mode to 'smooth'
	glShadeModel( GL_SMOOTH );
    // ensure correct display of polygons
    glEnable( GL_DEPTH_TEST );
	
	// set the front faces of polygons
	glFrontFace( GL_CCW );
	// set fill mode
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	
	// enable transparency
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
    // compute log spacing
    for( int i = 0; i < g_num_soundfiles; i++ )
    {
        g_log_space[i] = g_wf[i].compute_log_spacing( g_fft_size / 2, g_log_factor );
    }
	
	// start random seed
	srand( time(NULL) );
	help();
}



//-----------------------------------------------------------------------------
// Name: reshapeFunc( )
// Desc: called when window size changes, and at initialization
//-----------------------------------------------------------------------------
void reshapeFunc( GLsizei w, GLsizei h )
{
    // save the new window size
    g_width = (GLsizei)w; g_height = (GLsizei)h;
    // map the view port to the client area
    glViewport( 0, 0, (GLsizei)w, (GLsizei)h );
    // set the matrix mode to project
    glMatrixMode( GL_PROJECTION );
    // load the identity matrix
    glLoadIdentity( );
    // create the viewing frustum (like a pyramid without its head):
    // (field of view angle, aspect, znear, zfar) - last 2 are always pos and > 1.0
	gluPerspective( 45.0, (GLfloat)w / (GLfloat)h, 0.1, 60.0 );
    // set the matrix mode to modelview from projection
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity( );
    // position the view point: 3 eye/camera coord, 3 center coord, 3 UP vector
    // gluLookAt( 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );
	// could instead be:
	changeLookAt( g_look_from, g_look_to, g_head_up );

}



//-----------------------------------------------------------------------------
// Name: keyboardFunc( )
// Desc: key event
//-----------------------------------------------------------------------------
void keyboardFunc( unsigned char key, int x, int y )
{
    switch( key )
    {
        case 'Q':
        case 'q':
            fprintf( stderr, "goodbyeeeee...i love youuuu... \n");
            exit( 1 );
            break;
        case 'F':
        case 'f':
            if( !g_fullscreen )
            {
                g_last_width = g_width;
                g_last_height = g_height;
                glutFullScreen();
            }
            else
                glutReshapeWindow( g_last_width, g_last_height );
            g_fullscreen = !g_fullscreen;
            g_put_a_donk_on_it = !g_put_a_donk_on_it; // not sure why that's happening, but this is a 'fix'.
        case 'D':
        case 'd':
            (g_put_a_donk_on_it) ? g_put_a_donk_on_it = false : g_put_a_donk_on_it = true;
            break;
        case 'N':
        case 'n':
            // g_window_type = 'hann';
            break;
        case 'M':
        case 'm':
            // g_window_type = 'hamming';
            break;
        case '1':
            // solo track 1
            play_all = false;
            play_guitar = false;
            play_bass = false;
            play_tamb = false;
            play_vocals = false;
            play_drums = true;
            break;
        case '2':
            // solo track 2
            play_all = false;
            play_bass = false;
            play_tamb = false;
            play_vocals = false;
            play_drums = false;
            play_guitar = true;
            break;
        case '3':
            // solo track 3
            play_all = false;
            play_guitar = false;
            play_bass = false;
            play_tamb = false;
            play_drums = false;
            play_vocals = true;
            break;
        case '4':
            // solo track 4
            play_all = false;
            play_guitar = false;
            play_tamb = false;
            play_vocals = false;
            play_drums = false;
            play_bass = true;
            break;
        case '5':
            // solo track 5
            play_all = false;
            play_guitar = false;
            play_bass = false;
            play_vocals = false;
            play_drums = false;
            play_tamb = true;
            break;
        case '0':
            // play and show all tracks
            play_guitar = false;
            play_bass = false;
            play_tamb = false;
            play_vocals = false;
            play_drums = false;
            play_all = true;
            break;
        case 'j':
            // spin left
            g_inc += g_inc_val_mouse;
            break;
        case 'k':
            // decrease gain of FFT
            if( g_fft_gain > 1.0f ) g_fft_gain -= 1.0f;
            break;
        case 'l':
            // spin right
            g_inc -= g_inc_val_mouse;
            break;
        case 'i':
            // increase gain of FFT
            if( g_fft_gain < 20.0f ) g_fft_gain += 1.0f;
            break;
    }
    
    // mark for rendering
    glutPostRedisplay( );
}




//-----------------------------------------------------------------------------
// Name: mouseFunc( )
// Desc: handles mouse stuff
//-----------------------------------------------------------------------------
void mouseFunc( int button, int state, int x, int y )
{
    if( button == GLUT_LEFT_BUTTON )
    {
        // rotate
        if( state == GLUT_DOWN )
            g_inc -= g_inc_val_mouse;
        else
            g_inc += g_inc_val_mouse;
    }
    else if ( button == GLUT_RIGHT_BUTTON )
    {
        if( state == GLUT_DOWN )
            g_inc += g_inc_val_mouse;
        else
            g_inc -= g_inc_val_mouse;
    }
    else
        g_inc = 0.0f;

    // mark for rendering
    glutPostRedisplay( );
}




//-----------------------------------------------------------------------------
// Name: idleFunc( )
// Desc: callback from GLUT
//-----------------------------------------------------------------------------
void idleFunc( )
{
    // render the scene
    glutPostRedisplay( );
}



//-----------------------------------------------------------------------------
// Name: displayFunc( )
// Desc: callback function to display errthing
//-----------------------------------------------------------------------------
// Clear, Flush, and SwapBuffer are essential calls
void displayFunc( ) 
{
	
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glEnable( GL_TEXTURE_2D );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
    // set the matrix mode to modelview from projection
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity( );
    // position the view point: 3 eye/camera coord, 3 center coord, 3 UP vector
    gluLookAt( 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );

    // old function
	// void drawTambourine( float scale, float x, float y, float tempo, float inst_total )
	// drawTambourine( 0.3f, 0.1f, 0.0f, 100.0f, 5.0 );
	// glPushAttrib(GL_CURRENT_BIT);

    glPushMatrix(); //
    // rotate about y axis
    glRotatef( g_angle_y += g_inc, 0.0f, 1.0f, 0.0f );
    
	// color "filter" of the images 
	glColor3f(1,1,1);
				
	// modified from FourTextures.cpp / RgbImage.cpp by Samuel R. Buss
	glPushMatrix();
	    glTranslatef( -2.5f, 1.8f, -0.5f );
        glScalef(0.25f * (1 + g_avg_pow[0]), 0.25f * (1 + g_avg_pow[0]), 0.0f * (1 + g_avg_pow[0]));
	    drawTextureQuad ( 0 );	// contains a pushpop
    glPopMatrix();
	
    glPushMatrix();
	    glTranslatef( -1.25f, 1.8f, -0.5f );
        glScalef(0.25f * (1 + g_avg_pow[1]), 0.25f * (1 + g_avg_pow[1]), 0.0f * (1 + g_avg_pow[1]));
        drawTextureQuad ( 1 );
    glPopMatrix();
	
    glPushMatrix();
	    glTranslatef( 0.0f, 1.8f, -0.5f );
        glScalef(0.25f * (1 + g_avg_pow[2]), 0.25f * (1 + g_avg_pow[2]), 0.0f * (1 + g_avg_pow[2]));
	    drawTextureQuad ( 2 );
    glPopMatrix();

    glPushMatrix();
	    glTranslatef( 1.25f, 1.8f, -0.5f );
        glScalef(0.25f * (1 + g_avg_pow[3]), 0.25f * (1 + g_avg_pow[3]), 0.0f * (1 + g_avg_pow[3]));
	    drawTextureQuad ( 3 );
    glPopMatrix();

    glPushMatrix();
	    glTranslatef( 2.5f, 1.8f, -0.5f );
		glScalef(0.25f * (1 + g_avg_pow[4]), 0.25f * (1 + g_avg_pow[4]), 0.0f * (1 + g_avg_pow[4]));
	    drawTextureQuad ( 4 );
    glPopMatrix();
    
	
	// essential for displaying wutrfall correctly
	glDisable(GL_TEXTURE_2D);
	
	// plot the waterfalls
    for( int f = 0; f < g_num_soundfiles; f++ )
	{
        // yeeeuh chase em down
        g_wf[f].drawWaterfall( g_soundfile_buffer[f], g_buffer_size, g_fft_size, 1, f, g_num_soundfiles, g_put_a_donk_on_it, alphas[f], g_fft_gain );
    }

    glPopMatrix();
	
    // flush: done with the current offscreen buffer
    glFlush();
	
    // swap double buffer
    glutSwapBuffers();
}


//-----------------------------------------------------------------------------
// name: changeLookAt()
// desc: changes the point of view
//-----------------------------------------------------------------------------
void changeLookAt( pt3d look_from, pt3d look_to, pt3d head_up )
{
    gluLookAt(  look_from.x, look_from.y, look_from.z, 
                look_to.x, look_to.y, look_to.z, 
                head_up.x, head_up.y, head_up.z );
}


//-----------------------------------------------------------------------------
// name: loadTextureFromFile (from FourTextures.cpp / RgbImage.cpp by Samuel R. Buss)
// desc: load image file callback
//-----------------------------------------------------------------------------

void loadTextureFromFile( const char * filename )
{    
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);

	RgbImage theTexMap( filename );

	// Pixel alignment: each row is word aligned.  Word alignment is the default. 
	// glPixelStorei(GL_UNPACK_ALIGNMENT, 4);		

	// Set the interpolation settings to best quality.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB,
					 theTexMap.GetNumCols(), theTexMap.GetNumRows(),
					 GL_RGB, GL_UNSIGNED_BYTE, theTexMap.ImageData() );

}


//-----------------------------------------------------------------------------
// name: initFiveImages() (from FourTextures.cpp / RgbImage.cpp by Samuel R. Buss)
// desc: Load five textures, by repeatedly calling loadTextureFromFile().
//-----------------------------------------------------------------------------

void initFiveImages( const char * filenames[] )
{
	glGenTextures( 5, textureName );	// Load five texture names into array
	for( int i = 0; i < 5; i++ ) {
		glBindTexture(GL_TEXTURE_2D, textureName[i]);	// Texture #i is active now
        loadTextureFromFile( filenames[i] );			// Load texture #i
	}
}


//-----------------------------------------------------------------------------
// name: initAudioFiles
//-----------------------------------------------------------------------------

void initAudioFiles( const char * mus_file_names[] )
{
	for( int f = 0; f < g_num_soundfiles; f++ )
	{
		g_input_music[f] = new WvIn(mus_file_names[f], 0, 0);
        g_input_music[f]->normalize(1);
	}
}


//-----------------------------------------------------------------------------
// name: drawTextureQuad(i) (from FourTextures.cpp / RgbImage.cpp by Samuel R. Buss)
// desc: display the ith texture
//-----------------------------------------------------------------------------

void drawTextureQuad( int i ) 
{
   glBindTexture(GL_TEXTURE_2D, textureName[i]);

   glBegin(GL_QUADS);
   glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, 1.0, 0.0);
   glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 1.0, 0.0);
   glTexCoord2f(1.0, 0.0); glVertex3f(1.0, -1.0, 0.0);
   glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0, 0.0);
   glEnd();
}
