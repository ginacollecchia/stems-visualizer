/*----------------------------------------------------------------------------
  X: General API for audio/graphics/interaction programming
    (created as part of Converge Visualizer)
 
  Copyright (c) 2010 Ge Wang and Jieun Oh
    All rights reserved.
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  U.S.A.
-----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// name: gfx.h
// desc: general graphics routines
//
// authors: Ge Wang
//    date: Spring 2010
//
// Stanford Mobile Phone Orchestra
//    http://mopho.stanford.edu/
//-----------------------------------------------------------------------------
#ifndef __GFX_H__
#define __GFX_H__

#include "vector3d.h"
#include <sys/time.h>
#include <string>
#include <Cocoa/Cocoa.h>




//-----------------------------------------------------------------------------
// name: struct CvTexture
// desc: image data from loading and image
//-----------------------------------------------------------------------------
struct XTexture
{
    // texture data
    GLuint  name;
    GLfloat diameter;

    // color
    GLfloat color[4];

    // image data
    CGFloat origWidth;
    CGFloat origHeight;
    CGFloat resizeWidth;
    CGFloat resizeHeight;

    // constructor
    XTexture()
    : name(0), diameter(1), 
      origWidth(0), origHeight(0), resizeWidth(0), resizeHeight(0)
    { color[0] = color[1] = color[2] = color[3] = 1.0f; }
    
    // copy constructor
    XTexture( const XTexture & lhs )
    {
        name = lhs.name; diameter = lhs.diameter;
        origWidth = lhs.origWidth; origHeight = lhs.origHeight;
        resizeWidth = lhs.resizeWidth; resizeHeight = lhs.resizeHeight;
    }
    
    // destructor
    ~XTexture()
    { if( name != 0 ) glDeleteTextures( 1, &name ); name = 0; }
};




//-----------------------------------------------------------------------------
// name: class CvGfx
// desc: CV graphics functions
//-----------------------------------------------------------------------------
class XGfx
{
public:
    // load texture (call this with a texture bound)
    static bool loadTexture( const std::string & name, XTexture * data = NULL );
    // load texture from a UIImage
    static bool loadTexture( CGImageRef image );
    
    // draw texture
    static void drawTexture( const XTexture * image );
    // draw texture
    static void drawTextureUV( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2,
                               GLfloat u1, GLfloat v1, GLfloat u2, GLfloat v2 );

    // point in triangle test (2D)
    static bool isPointInTriangle2D( const Vector3D & pt, const Vector3D & a, 
                                     const Vector3D & b, const Vector3D & c );
    
    // get current time
    static double getCurrentTime( bool fresh );
    // reset current time tracking
    static void resetCurrentTime();
    // get delta
    static GLfloat delta();
    // set delta factor
    static void setDeltaFactor( GLfloat factor );

public:
    static struct timeval ourPrevTime;
    static struct timeval ourCurrTime;
    static GLfloat ourDeltaFactor;
};




#endif
