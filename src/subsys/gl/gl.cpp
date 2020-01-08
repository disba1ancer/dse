/*******************************************************************************
 * DSE - disba1ancer's (graphic) engine.
 *
 * Copyright (c) 2019 disba1ancer.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *******************************************************************************/
/*
 * gl.cpp
 *
 *  Created on: 28 мар. 2019 г.
 *      Author: disba1ancer
 */

#include "gl.h"

#ifndef EDITING
IMPLEMENT_GL_FUNCTION(wglChoosePixelFormatARB, WGLCHOOSEPIXELFORMATARB);
IMPLEMENT_GL_FUNCTION(wglCreateContextAttribsARB, WGLCREATECONTEXTATTRIBSARB);
IMPLEMENT_GL_FUNCTION(glGenVertexArrays, GLGENVERTEXARRAYS);
IMPLEMENT_GL_FUNCTION(glBindVertexArray, GLBINDVERTEXARRAY);
IMPLEMENT_GL_FUNCTION(glDeleteVertexArrays, GLDELETEVERTEXARRAYS);
IMPLEMENT_GL_FUNCTION(glGenBuffers, GLGENBUFFERS);
IMPLEMENT_GL_FUNCTION(glBindBuffer, GLBINDBUFFER);
IMPLEMENT_GL_FUNCTION(glBufferData, GLBUFFERDATA);
IMPLEMENT_GL_FUNCTION(glDeleteBuffers, GLDELETEBUFFERS);
IMPLEMENT_GL_FUNCTION(glEnableVertexAttribArray, GLENABLEVERTEXATTRIBARRAY);
IMPLEMENT_GL_FUNCTION(glVertexAttribPointer, GLVERTEXATTRIBPOINTER);
IMPLEMENT_GL_FUNCTION(glDisableVertexAttribArray, GLDISABLEVERTEXATTRIBARRAY);
IMPLEMENT_GL_FUNCTION(glCreateShader, GLCREATESHADER);
IMPLEMENT_GL_FUNCTION(glShaderSource, GLSHADERSOURCE);
IMPLEMENT_GL_FUNCTION(glCompileShader, GLCOMPILESHADER);
IMPLEMENT_GL_FUNCTION(glCreateProgram, GLCREATEPROGRAM);
IMPLEMENT_GL_FUNCTION(glDeleteProgram, GLDELETEPROGRAM);
IMPLEMENT_GL_FUNCTION(glAttachShader, GLATTACHSHADER);
IMPLEMENT_GL_FUNCTION(glLinkProgram, GLLINKPROGRAM);
IMPLEMENT_GL_FUNCTION(glDeleteShader, GLDELETESHADER);
IMPLEMENT_GL_FUNCTION(glUseProgram, GLUSEPROGRAM);
IMPLEMENT_GL_FUNCTION(glGetUniformLocation, GLGETUNIFORMLOCATION);
IMPLEMENT_GL_FUNCTION(glUniformMatrix4fv, GLUNIFORMMATRIX4FV);
IMPLEMENT_GL_FUNCTION(glGenFramebuffers, GLGENFRAMEBUFFERS);
IMPLEMENT_GL_FUNCTION(glFramebufferTexture, GLFRAMEBUFFERTEXTURE);
IMPLEMENT_GL_FUNCTION(glBindFramebuffer, GLBINDFRAMEBUFFER);
IMPLEMENT_GL_FUNCTION(glBlitFramebuffer, GLBLITFRAMEBUFFER);
IMPLEMENT_GL_FUNCTION(glFramebufferRenderbuffer, GLFRAMEBUFFERRENDERBUFFER);
IMPLEMENT_GL_FUNCTION(glGetUniformfv, GLGETUNIFORMFV);
IMPLEMENT_GL_FUNCTION(glActiveTexture, GLACTIVETEXTURE);
IMPLEMENT_GL_FUNCTION(glGetAttribLocation, GLGETATTRIBLOCATION);
IMPLEMENT_GL_FUNCTION(glUniform1i, GLUNIFORM1I);
IMPLEMENT_GL_FUNCTION(glTexImage3D, GLTEXIMAGE3D);
IMPLEMENT_GL_FUNCTION(glGetShaderiv, GLGETSHADERIV);
IMPLEMENT_GL_FUNCTION(glGetShaderInfoLog, GLGETSHADERINFOLOG);
IMPLEMENT_GL_FUNCTION(glGetProgramiv, GLGETPROGRAMIV);
IMPLEMENT_GL_FUNCTION(glGetProgramInfoLog, GLGETPROGRAMINFOLOG);
IMPLEMENT_GL_FUNCTION(glBufferSubData, GLBUFFERSUBDATA);
IMPLEMENT_GL_FUNCTION(wglSwapIntervalEXT, WGLSWAPINTERVALEXT);
IMPLEMENT_GL_FUNCTION(glBindAttribLocation, GLBINDATTRIBLOCATION);
#endif
