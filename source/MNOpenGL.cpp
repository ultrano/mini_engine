//
//  MNOpenGL.cpp
//  me_app
//
//  Created by hahasasa on 3/2/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

#include "MNOpenGL.h"
#include "MNPrimaryType.h"
#include "stb_image.h"

#define SWLog printf

unsigned int glLoadTexture( const char* fileName, int& width, int& height )
{
    if ( !fileName ) return 0;
    
    int comp;
    
    unsigned char* data = stbi_load( fileName, &width, &height, &comp, 0 );
    
    if ( !data ) return 0;
    
    unsigned int texID[1];
    
    glGenTextures(1,&texID[0]);
    glBindTexture(GL_TEXTURE_2D,texID[0]);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    
    glTexImage2D(GL_TEXTURE_2D, 0
                 , (comp==4)? GL_RGBA : (comp==3)? GL_RGB : GL_INVALID_ENUM
                 , width, height, 0
                 , (comp==4)? GL_RGBA : (comp==3)? GL_RGB : GL_INVALID_ENUM
                 , GL_UNSIGNED_BYTE, data);
    
    GLenum err = glGetError();
    
    stbi_image_free(data);
    if ( err ) return 0;
    
    return texID[0];
}

unsigned int glLoadTextureFromMemory( const unsigned char* buf, int len, int& width, int& height )
{
    if ( !buf || !len ) return 0;
    
    int comp;
    
    unsigned char* data = stbi_load_from_memory( buf, len, &width, &height, &comp, 0 );
    
    if ( !data ) return 0;
    
    unsigned int texID[1];
    
    GLenum err = glGetError();
    glGenTextures(1,&texID[0]);
    glBindTexture(GL_TEXTURE_2D,texID[0]);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    
    glTexImage2D(GL_TEXTURE_2D, 0
                 , (comp==4)? GL_RGBA : (comp==3)? GL_RGB : GL_INVALID_ENUM
                 , width, height, 0
                 , (comp==4)? GL_RGBA : (comp==3)? GL_RGB : GL_INVALID_ENUM
                 , GL_UNSIGNED_BYTE, data);
    err = glGetError();
    
    stbi_image_free(data);
    if ( err ) return 0;
    
    return texID[0];
}

GLuint loadShader( GLenum type, const char* shaderSource )
{
    if ( shaderSource == NULL )
    {
        SWLog( "shader source is NULL" );
        return 0;
    }
    
    GLuint shaderID = glCreateShader( type );
    
    if ( shaderID == 0 )
    {
        SWLog( "create shader failed" );
        return 0;
    }
    
    tarray<const char*> sources;
    switch ( type )
    {
        case GL_VERTEX_SHADER :
            sources.push_back( "#define VERTEX_SHADER 1\n" );
            break;
        case GL_FRAGMENT_SHADER :
            sources.push_back( "#define FRAGMENT_SHADER 1\n" );
            sources.push_back( "#ifdef GL_ES\n" );
            sources.push_back( "precision mediump float;\n" );
            sources.push_back( "#endif\n" );
            break;
    }
    sources.push_back( shaderSource );
    
    glShaderSource( shaderID, sources.size(), &sources[0], NULL );
    glCompileShader( shaderID );
    
    GLint compiled = false;
    glGetShaderiv( shaderID, GL_COMPILE_STATUS, &compiled );
    
    if ( !compiled )
    {
        int infoLen = 0;
        glGetShaderiv( shaderID, GL_INFO_LOG_LENGTH, &infoLen );
        if ( infoLen > 0 )
        {
            tstring msg;
            msg.resize( infoLen );
            glGetShaderInfoLog( shaderID, infoLen, &infoLen, &msg[0] );
            
            SWLog( msg.c_str() );
        }
        glDeleteShader( shaderID );
        return 0;
    }
    
    return shaderID;
}

unsigned int glLoadProgram( const char* source )
{
    GLuint vsID = 0;
    GLuint fsID = 0;
    GLuint programID = 0;
    
    if ( (vsID = loadShader( GL_VERTEX_SHADER, source )) == 0 )
    {
        SWLog( "failed compile vertex shader" );
        return 0;
    }
    if ( (fsID = loadShader( GL_FRAGMENT_SHADER, source )) == 0 )
    {
        glDeleteShader( vsID );
        SWLog( "failed compile fragment shader" );
        return 0;
    }
    
    if ( (programID = glCreateProgram()) == 0 ) return 0;
    
    glAttachShader( programID, vsID );
    glAttachShader( programID, fsID );
    glLinkProgram( programID );
    
    GLint linked = false;
    glGetProgramiv( programID, GL_LINK_STATUS, &linked );
    if ( linked == 0 )
    {
        int infoLen = 0;
        glGetProgramiv( programID, GL_INFO_LOG_LENGTH, &infoLen );
        if ( infoLen > 0 )
        {
            tstring msg;
            msg.resize( infoLen );
            glGetProgramInfoLog( programID, infoLen, NULL, &msg[0] );
            
            SWLog( msg.c_str() );
        }
        glDeleteProgram( programID );
        programID = 0;
    }
    
    glDeleteShader( vsID );
    glDeleteShader( fsID );
    
    if ( programID == 0 ) SWLog( "failed to loading program" );
    return programID;
}