/*----------------------------------------------------------------------------
  Vector3D: a simple vector class
 
  Copyright (c) 2009 Ge Wang
    All rights reserved.
    http://ccrma.stanford.edu/~ge/
 
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
// name: vector3d.h
// desc: vector3d class
//
// author: Ge Wang (ge@ccrma.stanford.edu)
//   date: 2009
//-----------------------------------------------------------------------------
#ifndef __VECTOR_3D_H__
#define __VECTOR_3D_H__

#include <math.h>

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




//-----------------------------------------------------------------------------
// name: class Vector3D
// desc: 3d vector
//-----------------------------------------------------------------------------
class Vector3D
{
public:
    Vector3D( ) : x(0), y(0), z(0) { }
    Vector3D( GLfloat _x, GLfloat _y, GLfloat _z ) { set( _x, _y, _z ); }
    Vector3D( const Vector3D & other ) { *this = other; }
    ~Vector3D() { }
    
public:
    void set( GLfloat _x, GLfloat _y, GLfloat _z ) { x = _x; y = _y; z = _z; }
    void setAll( GLfloat val ) { x = y = z = val; }
    
public:
    GLfloat & operator []( int index )
    { if( index == 0 ) return x; if( index == 1 ) return y; 
        if( index == 2 ) return z; return nowhere; }
    const GLfloat & operator []( int index ) const
    { if( index == 0 ) return x; if( index == 1 ) return y; 
        if( index == 2 ) return z; return zero; }
    const Vector3D & operator =( const Vector3D & rhs )
    { x = rhs.x; y = rhs.y; z = rhs.z; return *this; }
    
    Vector3D operator +( const Vector3D & rhs ) const
    { Vector3D result = *this; result += rhs; return result; }
    Vector3D operator -( const Vector3D & rhs ) const
    { Vector3D result = *this; result -= rhs; return result; }
    Vector3D operator *( GLfloat scalar ) const
    { Vector3D result = *this; result *= scalar; return result; }
    
    inline void operator +=( const Vector3D & rhs )
    { x += rhs.x; y += rhs.y; z += rhs.z; }
    inline void operator -=( const Vector3D & rhs )
    { x -= rhs.x; y -= rhs.y; z -= rhs.z; }
    inline void operator *=( GLfloat scalar )
    { x *= scalar; y *= scalar; z *= scalar; }
    
    // dot product
    inline GLfloat operator *( const Vector3D & rhs ) const
    { GLfloat result = x*rhs.x + y*rhs.y + z*rhs.z; return result; }
    // magnitude
    inline GLfloat magnitude() const
    { return ::sqrt( x*x + y*y + z*z ); }
    // normalize
    inline void normalize()
    { GLfloat mag = magnitude(); if( mag == 0 ) return; *this *= 1/mag; }
    // 2d angles
    inline GLfloat angleXY() const
    { return ::atan2( y, x ); }
    inline GLfloat angleYZ() const
    { return ::atan2( z, y ); }
    inline GLfloat angleXZ() const
    { return ::atan2( z, x ); }
    
public: // using the 3-tuple for interpolation
    inline void interp()
    { value = (goal-value)*slew + value; }
    inline void interp( GLfloat delta )
    { value = (goal-value)*slew*delta + value; }
    inline void update( GLfloat _goal )
    { goal = _goal; }
    inline void update( GLfloat _goal, GLfloat _slew )
    { goal = _goal; slew = _slew; }
    
public:
    // either use as .x, .y, .z OR .value, .goal, .slew
    union { GLfloat x; GLfloat value; };
    union { GLfloat y; GLfloat goal; };
    union { GLfloat z; GLfloat slew; };
    
public:
    static GLfloat nowhere;
    static GLfloat zero;
};













#endif
