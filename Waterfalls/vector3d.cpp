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
// name: vector3d.cpp
// desc: vector3d class
//
// author: Ge Wang (ge@ccrma.stanford.edu)
//   date: 2010
//-----------------------------------------------------------------------------
#include "vector3d.h"


// static instantiation
GLfloat Vector3D::nowhere = 0.0f;
GLfloat Vector3D::zero = 0.0f;
