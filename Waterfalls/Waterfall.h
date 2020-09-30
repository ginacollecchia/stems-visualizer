//
//  Waterfall.h
//  Waterfalls
//
//  Created by Gina Collecchia on 10/31/13.
//  Copyright (c) 2013 Gina Collecchia. All rights reserved.
//

#ifndef Waterfalls_Waterfall_h
#define Waterfalls_Waterfall_h


// libsndfile
/* #if defined(__USE_SNDFILE_NATIVE__)
    #include <sndfile.h>
#else
    #include "util_sndfile.h"
#endif */

// OpenGL
#if defined(__APPLE__)
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

// process related
#if defined(__OS_WINDOWS__)
#include <process.h>
// usleep
#ifndef usleep
    #define usleep(x) Sleep( (x/1000 <= 0 ? 1 : x/1000) )
#endif
#else
    #include <unistd.h>
#endif


// datatype
#define SAMPLE double

//-----------------------------------------------------------
// name: class Waterfall
// desc: generates a waterfall of FFTs with custom settings.
//       input buffer can be either file or mic input.
//-----------------------------------------------------------
class Waterfall
{
    
public:
    Waterfall();
    ~Waterfall();

public:
    // initialize... necessary?
    void init( int buffer_size, int fft_size, int srate, int num_channels );
    // draw a waterfall!
    void drawWaterfall( float * buffer, int buffer_size, int fft_size, int window_type, int index, int num_soundfiles, bool put_a_donk_on_it, float alphas, float fft_gain );
	double compute_log_spacing( int fft_size, double power );

private:
    // waterfall index
    unsigned int w_wf_id;
    // channels
    int w_num_channels;
    // a point in 2D space
    struct Pt2D { float x; float y; };
    // array of fft buffers
    Pt2D ** w_spectrums;
    // number of those buffers
    unsigned int w_depth;
    // z-coordinate, probably
    float w_z;
    // distance between spectra
    float w_space;
    // sample rate
    int w_srate;
    // gain
    float w_gain;
    // stretch that x-axis
    float w_freq_scale;
    // should we draw a given spectrum?
    bool * w_draw;
    // should we draw a waterfall?
    bool w_wutrfall;
    // window buffer
    float * w_window;
    // hmm
    unsigned int w_wf_delay;
    // buffer size
    int w_buffer_size;
    // fft size
    int w_fft_size;
    // precompute positions for log spacing
    float * w_log_positions;
	// frequency scale
	int w_freq_view;
	// backwards view?
	bool w_backwards;
	// don't think i use this; a memory enhancement
	int w_starting;
    // which waterfall?
    int w_index;
    // how many waterfalls? (for spacing)
    int w_num_soundfiles;
    // pulse dat waterfall
    double w_avg_power;
	// write "DONK"
    bool w_put_a_donk_on_it;
    // spectrum alpha
    float w_alphas;
    // height of FFT
    float w_fft_gain;
};

#endif
