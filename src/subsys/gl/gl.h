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
 * gl.h
 *
 *  Created on: 28 мар. 2019 г.
 *      Author: disba1ancer
 */

#ifndef DSEGL_GL_H_
#define DSEGL_GL_H_

#include "ImportedFunction.h"
#include <GL/gl.h>
#ifdef EDITING
#define GL_GLEXT_PROTOTYPES
#define WGL_WGLEXT_PROTOTYPES
#endif
#include <GL/glext.h>

#ifdef _WIN32
#include <GL/wglext.h>
#endif

#ifndef EDITING
IMPORT_GL_FUNCTION(glGenVertexArrays, GLGENVERTEXARRAYS);
IMPORT_GL_FUNCTION(glBindVertexArray, GLBINDVERTEXARRAY);
IMPORT_GL_FUNCTION(glDeleteVertexArrays, GLDELETEVERTEXARRAYS);
IMPORT_GL_FUNCTION(glGenBuffers, GLGENBUFFERS);
IMPORT_GL_FUNCTION(glBindBuffer, GLBINDBUFFER);
IMPORT_GL_FUNCTION(glBufferData, GLBUFFERDATA);
IMPORT_GL_FUNCTION(glDeleteBuffers, GLDELETEBUFFERS);
IMPORT_GL_FUNCTION(glEnableVertexAttribArray, GLENABLEVERTEXATTRIBARRAY);
IMPORT_GL_FUNCTION(glVertexAttribPointer, GLVERTEXATTRIBPOINTER);
IMPORT_GL_FUNCTION(glDisableVertexAttribArray, GLDISABLEVERTEXATTRIBARRAY);
IMPORT_GL_FUNCTION(glCreateShader, GLCREATESHADER);
IMPORT_GL_FUNCTION(glShaderSource, GLSHADERSOURCE);
IMPORT_GL_FUNCTION(glCompileShader, GLCOMPILESHADER);
IMPORT_GL_FUNCTION(glCreateProgram, GLCREATEPROGRAM);
IMPORT_GL_FUNCTION(glDeleteProgram, GLDELETEPROGRAM);
IMPORT_GL_FUNCTION(glAttachShader, GLATTACHSHADER);
IMPORT_GL_FUNCTION(glLinkProgram, GLLINKPROGRAM);
IMPORT_GL_FUNCTION(glDeleteShader, GLDELETESHADER);
IMPORT_GL_FUNCTION(glUseProgram, GLUSEPROGRAM);
IMPORT_GL_FUNCTION(glGetUniformLocation, GLGETUNIFORMLOCATION);
IMPORT_GL_FUNCTION(glUniformMatrix4fv, GLUNIFORMMATRIX4FV);
IMPORT_GL_FUNCTION(glGenFramebuffers, GLGENFRAMEBUFFERS);
IMPORT_GL_FUNCTION(glFramebufferTexture, GLFRAMEBUFFERTEXTURE);
IMPORT_GL_FUNCTION(glBindFramebuffer, GLBINDFRAMEBUFFER);
IMPORT_GL_FUNCTION(glBlitFramebuffer, GLBLITFRAMEBUFFER);
IMPORT_GL_FUNCTION(glFramebufferRenderbuffer, GLFRAMEBUFFERRENDERBUFFER);
IMPORT_GL_FUNCTION(glGetUniformfv, GLGETUNIFORMFV);
IMPORT_GL_FUNCTION(glActiveTexture, GLACTIVETEXTURE);
IMPORT_GL_FUNCTION(glGetAttribLocation, GLGETATTRIBLOCATION);
IMPORT_GL_FUNCTION(glUniform1i, GLUNIFORM1I);
IMPORT_GL_FUNCTION(glTexImage3D, GLTEXIMAGE3D);
IMPORT_GL_FUNCTION(glGetShaderiv, GLGETSHADERIV);
IMPORT_GL_FUNCTION(glGetShaderInfoLog, GLGETSHADERINFOLOG);
IMPORT_GL_FUNCTION(glGetProgramiv, GLGETPROGRAMIV);
IMPORT_GL_FUNCTION(glGetProgramInfoLog, GLGETPROGRAMINFOLOG);
IMPORT_GL_FUNCTION(glBufferSubData, GLBUFFERSUBDATA);
IMPORT_GL_FUNCTION(glBindAttribLocation, GLBINDATTRIBLOCATION);
IMPORT_GL_FUNCTION(glGetStringi, GLGETSTRINGI);
IMPORT_GL_FUNCTION(glVertexAttribDivisor, GLVERTEXATTRIBDIVISOR);
IMPORT_GL_FUNCTION(glVertexAttribDivisorARB, GLVERTEXATTRIBDIVISORARB);
IMPORT_GL_FUNCTION(glDrawElementsBaseVertex, GLDRAWELEMENTSBASEVERTEX);
IMPORT_GL_FUNCTION(glUniform2f, GLUNIFORM2F);
IMPORT_GL_FUNCTION(glGetFramebufferAttachmentParameteriv, GLGETFRAMEBUFFERATTACHMENTPARAMETERIV);
IMPORT_GL_FUNCTION(glBindFragDataLocation, GLBINDFRAGDATALOCATION);
IMPORT_GL_FUNCTION(glUniform3fv, GLUNIFORM3FV);
IMPORT_GL_FUNCTION(glUniform4fv, GLUNIFORM4FV);
IMPORT_GL_FUNCTION(glUniform1f, GLUNIFORM1F);

#ifdef _WIN32
IMPORT_GL_FUNCTION(wglChoosePixelFormatARB, WGLCHOOSEPIXELFORMATARB);
IMPORT_GL_FUNCTION(wglCreateContextAttribsARB, WGLCREATECONTEXTATTRIBSARB);
IMPORT_GL_FUNCTION(wglSwapIntervalEXT, WGLSWAPINTERVALEXT);
IMPORT_GL_FUNCTION(wglGetExtensionsStringARB, WGLGETEXTENSIONSSTRINGARB);
#endif

#endif

#endif /* DSEGL_GL_H_ */
