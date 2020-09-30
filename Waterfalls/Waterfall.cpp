//
//  Waterfall.cpp
//  Waterfalls
//
//  Created by Gina Collecchia on 10/31/13.
//  Copyright (c) 2013 Gina Collecchia. All rights reserved.
//

#include "Waterfall.h"
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include "chuck_fft.h"

using namespace std;

// constructornator
Waterfall::Waterfall()
{
    // private values
    w_depth = 48; // number of waterfall frames displayed
    w_num_channels = 2;
    w_srate = 48000;
    w_spectrums = new Pt2D *[w_depth];
    w_space = 0.12f;
    w_gain = 1.0f;
    w_freq_scale = 1.0f;
    w_wf_id = 0;
    w_z = 0.0f;
    w_draw = NULL;
    w_wutrfall = true;
    w_window = NULL;
    w_wf_delay = (int)(w_depth * 1.0f/3.0f + 0.5f);
    w_buffer_size = 512;
    w_fft_size = 256;
    w_log_positions = NULL;
	w_freq_view = 3;
	w_backwards = false;
	w_starting = 0;
    w_index = 0;
    w_put_a_donk_on_it = false;
    w_fft_gain = 1.0f;
}

// destructoid
Waterfall::~Waterfall()
{
    
}

void Waterfall::init( int buffer_size, int fft_size, int srate, int num_channels )
{
    // set
    w_srate = srate;
    w_num_channels = num_channels;
    w_buffer_size = buffer_size;
    w_fft_size = fft_size;
    
    // allocate / initialize
	w_spectrums = new Pt2D *[w_depth];
    for( int i = 0; i < w_depth; i++ )
    {
        w_spectrums[i] = new Pt2D[w_fft_size];
        memset( w_spectrums[i], 0, sizeof(Pt2D)*w_fft_size );
    }
    
    w_draw = new bool[w_depth];
    memset( w_draw, 0, sizeof(bool)*w_depth);
	
	w_log_positions = new float[w_fft_size];
	memset( w_log_positions, 0, sizeof(float)*w_fft_size );
	
	w_window = new float[w_buffer_size];
	memset( w_window, 0, sizeof(float)*w_buffer_size );
    
}

// draw a waterfall!
void Waterfall::drawWaterfall( float * buffer, int buffer_size, int fft_size, int window_type, int index, int num_soundfiles, bool put_a_donk_on_it, float alphas, float fft_gain ) // + vector for color
{
    // indices
    int i;
    w_index = index;
    // drawing offsets
    float x = 1.5f*(w_index-2.0f)-0.5f;
    float inc = 3.6f/buffer_size;
    float y = -1.0f;
    float alpha = 1.0f;
    // donk
    const char *str = "donk";
    int len = (int)strlen(str);
    w_fft_gain = fft_gain;
	
    // set
	w_fft_size = fft_size;
	w_buffer_size = buffer_size;
    w_put_a_donk_on_it = put_a_donk_on_it;
    // float w_alphas = alphas;
    
    // set
    // w_srate = srate;
    // w_num_channels = num_channels;
    
    
    if( window_type ) // i.e., set window_type to 0 to not use a window
    {
        // make the transform window (hanning)
        make_window( w_window, (unsigned long)buffer_size );
        apply_window((float *)buffer, w_window, buffer_size);
    }
    
    // take the fft of the buffer
    rfft( (float *)buffer, fft_size/2, FFT_FORWARD );
    // cast to complex type
    complex * cbuffer = (complex *)buffer;
	
    // debug avg_pow
    // cout << w_avg_power << endl;
	x = 1.5f*(w_index-2.0f)-0.5f;
	y = -1.0f;
	
	glColor4f( 0.4f, 1.0f, 0.4f, alpha );
	glNormal3f( 0.0f, 1.0f, 0.0f );
    
    for( i = 0; i < buffer_size; i++ )
    {
        // copy x coordinate
        w_spectrums[w_wf_id][i].x = x;
        // scaled to fft_gain
        w_spectrums[w_wf_id][i].y = w_gain * w_freq_scale * 1.8f * 
			::pow( w_fft_gain * cmp_abs( cbuffer[i] ), 0.5f ) + y;
        
        // increment x
        x += inc * w_freq_view;
    }
    
    // draw the right things
    w_draw[w_wf_id] = w_wutrfall;
	if ( !w_starting ) w_draw[ (w_wf_id + w_wf_delay) % w_depth ] = true;
    
    // reset drawing variables
	x = 1.5f*(w_index-2.0f)-0.5f;
    inc = 3.6f / w_fft_size;
    
    // save current matrix state
    glPushMatrix();
    // translate to world coordinates
    glTranslatef( x, 0.0, w_z );
    // scale it by freq_view and average power
    glScalef( inc * w_freq_view, 1.0, -w_space );
    
    // now loop through each slice of the waterfall
    for ( i = 0; i < w_depth; i++ )
    {
        if ( w_wutrfall )
        {
            if( w_draw[(w_wf_id + i)%w_depth] )
            {
                Pt2D * pt = w_spectrums[(w_wf_id+i)%w_depth];
                glColor4f( (1.0f-0.2f*w_index), (0.2f*w_index), 0.4f, alphas ); // change to 4f, give it an alpha
				            
                // render the actual spectrum layer
                glBegin( GL_LINE_STRIP );
                for( int j = 0; j < fft_size/w_freq_view; j++, pt++ )
                {
					// float d = w_backwards ? w_depth - (float) i : (float) i;
                    glVertex3f( w_log_positions[j], pt->y, i );
                }
                glEnd();
                
                // back to default line width
                glLineWidth(1.0f);
            }
        }
		if( i < (w_depth-1) ) 
		{
			alpha -= 0.01f;
		} else {
			alpha = 1.0f;
		}
    }
    
    // put a donk on dat waterfall
    // would like this to "follow" the waterfall down the hatch, needs debugging
    if( w_put_a_donk_on_it )
    {
        for( int n = 0; n < w_depth; n++ )
        {
            glPushMatrix();
            glTranslatef(-(len*37)+0.3f, -(n*200), 0);
            glColor4f(0.5f, 0.9f, 0.4f, 1.0f);
            glRasterPos2d(1.3f, -1.2f); // doesn't do shit
            for( i = 0; i < len; i++ )
            {
                glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
            }
            glPopMatrix();
        }
        // reset
        w_put_a_donk_on_it = false;
    }
    
    // restore matrix state
    glPopMatrix();
    
    if( !w_wutrfall )
        w_draw[(w_wf_id+w_wf_delay) % w_depth] = false;
	
	w_wf_id--;
	w_wf_id = (w_wf_id + w_depth) % w_depth;
	if( w_wf_id == w_depth - w_wf_delay ) w_starting = 0;
    
}


//-----------------------------------------------------------------------------
// Name: compute_log_spacing( )
// Desc: ...
//-----------------------------------------------------------------------------
double Waterfall::compute_log_spacing( int fft_size, double power )
{
    int maxbin = fft_size; // for future in case we want to draw smaller range
    int minbin = 0; // what about adding this one?

    for(int i = 0; i < fft_size; i++)
    {
        // compute location
        w_log_positions[i] = ::pow((double)i/fft_size, power) * fft_size/w_freq_view;
		
        // normalize, 1 if maxbin == fft_size
        w_log_positions[i] /= pow((double)maxbin/fft_size, power);
    }

    return 1/::log(fft_size);
}


