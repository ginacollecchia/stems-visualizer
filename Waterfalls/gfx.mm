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
// name: gfx.cpp
// desc: general graphics routines
//
// authors: Ge Wang and Jieun Oh
//    date: Spring 2010
//
// Stanford Mobile Phone Orchestra
//    http://mopho.stanford.edu/
//-----------------------------------------------------------------------------
#include "gfx.h"
#include <math.h>




//-----------------------------------------------------------------------------
// name: loadTexture()
// desc: loads an OpenGL ES texture
//       (from jshmrsn, macrumors forum)
//-----------------------------------------------------------------------------
bool XGfx::loadTexture( const std::string & filename, XTexture * data )
{
    // load the resource
    NSString * path = [[NSString alloc] initWithUTF8String:filename.c_str()];
    NSData * texData = [[NSData alloc] initWithContentsOfFile:path];
    NSImage * image = [[NSImage alloc] initWithData:texData];
    if (image == nil)
    {
        NSLog( @"[mo_gfx]: cannot load file: %s", filename.c_str() );
        return FALSE;
    }
    
    // set flipped
    [image setFlipped:YES];
    // get the size
    NSSize size = [image size];
    // set
    if( data ) { data->origWidth = size.width; data->origHeight = size.height; }

    // log
    NSLog( @"loading texture: %s (%.1f x %.1f)...", filename.c_str(), size.width, size.height );
    
    // resize if needed
    NSImage * resizeImage = nil;
    if( data && data->resizeWidth > 0 && data->resizeHeight > 0 )
    {
        CGFloat resizeWidth = data->resizeWidth;
        CGFloat resizeHeight = data->resizeHeight;
        resizeImage = [[NSImage alloc] initWithSize: NSMakeSize(resizeWidth, resizeHeight)];
        [resizeImage lockFocus];
        [image drawInRect: NSMakeRect(0, 0, resizeWidth, resizeHeight) fromRect: NSMakeRect(0, 0, size.width, size.height) operation: NSCompositeSourceOver fraction:1.0];
        [resizeImage unlockFocus];
    }
    else
    {
        // don't actually resize
        resizeImage = image;
    }

    // convert to CGImage
    CGImageSourceRef source;
    source = CGImageSourceCreateWithData((CFDataRef)[resizeImage TIFFRepresentation], NULL);
    CGImageRef maskRef = CGImageSourceCreateImageAtIndex(source, 0, NULL);
    
    // load image
    loadTexture( maskRef );

    // cleanup
    [image release];
    [resizeImage release];
    [texData release];
    
    return true;
}




//-----------------------------------------------------------------------------
// name: loadTexture()
// desc: loads an OpenGL texture
//       (from jshmrsn, macrumors forum)
//-----------------------------------------------------------------------------
bool XGfx::loadTexture( CGImageRef image )
{
    if (image == nil)
    {
        NSLog( @"[mo_gfx]: error: UIImage == nil..." );
        return FALSE;
    }
    
    // convert to RGBA
    GLuint width = CGImageGetWidth( image );
    GLuint height = CGImageGetHeight( image );
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    void *imageData = malloc( height * width * 4 );
    CGContextRef context = CGBitmapContextCreate( 
        imageData, width, height, 8, 4 * width, colorSpace, 
        kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
    CGColorSpaceRelease( colorSpace );
    CGContextClearRect( context, CGRectMake( 0, 0, width, height ) );
    CGContextTranslateCTM( context, 0, height - height );
    CGContextDrawImage( context, CGRectMake( 0, 0, width, height ), image );
    
    // load the texture
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, 
                  GL_RGBA, GL_UNSIGNED_BYTE, imageData );
    
    // free resource - OpenGL keeps image internally
    CGContextRelease(context);
    free(imageData);
    
    return true;
}




//-------------------------------------------------------------------------------
// name: drawTexture()
// desc: ...
//-------------------------------------------------------------------------------
void XGfx::drawTexture( const XTexture * tex )
{
    // sanity check
    if( !tex ) return;

    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    // set color
    glColor4fv( tex->color );
    // bind to texture
    glBindTexture( GL_TEXTURE_2D, tex->name );
    
    // centering
    GLfloat val = tex->diameter / 2;
    // check original dimensions
    GLfloat ratio = tex->origWidth / tex->origHeight;
    // get width and height
    GLfloat w = val, h = val;
    if( ratio < 1.0 ) { w *= ratio; }
    else { h /= ratio; }
    
    glBegin( GL_QUADS );
    glNormal3f( 0, 0, 1 );

    glTexCoord2f( 0, 0 );
    glVertex3f( -w, -h, 0 );

    glTexCoord2f( 1, 0 );
    glVertex3f( w, -h, 0 );
    
    glTexCoord2f( 1, 1  );
    glVertex3f( w, h, 0 );
    
    glTexCoord2f( 0, 1 );
    glVertex3f( -w, h, 0 );
    glEnd();

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
}




//-------------------------------------------------------------------------------
// name: drawTextureUV()
// desc: assuming texture already bind
//-------------------------------------------------------------------------------
void XGfx::drawTextureUV( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2,
                          GLfloat u1, GLfloat v1, GLfloat u2, GLfloat v2 )
{
    glBegin( GL_QUADS );
    glTexCoord2f( u1, v1 );
    glVertex3f( x1, y2, 0 );
    
    glTexCoord2f( u2, v1 );
    glVertex3f( x2, y2, 0 );
    
    glTexCoord2f( u2, v2  );
    glVertex3f( x2, y1, 0 );
    
    glTexCoord2f( u1, v2 );
    glVertex3f( x1, y1, 0 );
    glEnd();
}




//-----------------------------------------------------------------------------
// name: isPointInTriangle2D()
// desc: point in triangle test (2D)
//-----------------------------------------------------------------------------
bool XGfx::isPointInTriangle2D( const Vector3D & p, const Vector3D & a, 
                                  const Vector3D & b, const Vector3D & c )
{
    Vector3D v0 = c - a;
    Vector3D v1 = b - a;
    Vector3D v2 = p - a;
    
    // Compute dot products
    GLfloat dot00 = v0*v0;
    GLfloat dot01 = v0*v1;
    GLfloat dot02 = v0*v2;
    GLfloat dot11 = v1*v1;
    GLfloat dot12 = v1*v2;
    
    // Compute barycentric coordinates
    GLfloat invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
    GLfloat u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    GLfloat v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    
    // Check if point is in triangle
    return (u > 0) && (v > 0) && (u + v < 1);
}




//-----------------------------------------------------------------------------
// name: getCurrentTime()
// desc: get current time for simulation
//-----------------------------------------------------------------------------
double XGfx::getCurrentTime( bool fresh )
{
    if( fresh )
    {
        ourPrevTime = ourCurrTime;
        gettimeofday( &ourCurrTime, NULL );
    }
    
    return ourCurrTime.tv_sec + (double)ourCurrTime.tv_usec / 1000000;
}




//-----------------------------------------------------------------------------
// name: resetCurrentTime()
// desc: reset current time for simulation
//-----------------------------------------------------------------------------
void XGfx::resetCurrentTime()
{
    ourPrevTime.tv_sec = 0;
    ourPrevTime.tv_usec = 0;
}




//-----------------------------------------------------------------------------
// name: delta()
// desc: get current time delta for simulation
//-----------------------------------------------------------------------------
GLfloat XGfx::delta()
{
    double prev = ourPrevTime.tv_sec + (double)ourPrevTime.tv_usec / 1000000;
    double curr = ourCurrTime.tv_sec + (double)ourCurrTime.tv_usec / 1000000;
    // first 0
    return (prev == 0 ? 0.0f : (GLfloat)(curr - prev)) * ourDeltaFactor;
}




//-----------------------------------------------------------------------------
// name: setDeltaFactor()
// desc: set time delta factor for simulation
//-----------------------------------------------------------------------------
void XGfx::setDeltaFactor( GLfloat factor )
{
    ourDeltaFactor = factor;
}




// static instantiation
struct timeval XGfx::ourCurrTime;
struct timeval XGfx::ourPrevTime;
GLfloat XGfx::ourDeltaFactor = 1.0f;
