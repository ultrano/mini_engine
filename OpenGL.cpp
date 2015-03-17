#include "OpenGL.h"
#include "files\MNFiber.h"
#include "files\MNString.h"
#include "stb_image.h"

#define glLog printf
unsigned int glLoadTexture(const char* fileName, int& width, int& height)
{
	if (!fileName) return 0;

	int comp;

	unsigned char* data = stbi_load(fileName, &width, &height, &comp, 0);

	if (!data) return 0;

	unsigned int texID[1];

	glGenTextures(1, &texID[0]);
	glBindTexture(GL_TEXTURE_2D, texID[0]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0
		, (comp == 4) ? GL_RGBA : (comp == 3) ? GL_RGB : GL_INVALID_ENUM
		, width, height, 0
		, (comp == 4) ? GL_RGBA : (comp == 3) ? GL_RGB : GL_INVALID_ENUM
		, GL_UNSIGNED_BYTE, data);

	GLenum err = glGetError();

	stbi_image_free(data);
	if (err) return 0;

	return texID[0];
}

GLuint loadShader(GLenum type, const char* shaderSource)
{
	if (shaderSource == NULL)
	{
		glLog("shader source is NULL");
		return 0;
	}

	GLuint shaderID = glCreateShader(type);

	if (shaderID == 0)
	{
		glLog("create shader failed");
		return 0;
	}

	tarray<const char*> sources;
	switch (type)
	{
	case GL_VERTEX_SHADER:
		sources.push_back("#define VERTEX_SHADER 1\n");
		break;
	case GL_FRAGMENT_SHADER:
		sources.push_back("#define FRAGMENT_SHADER 1\n");
		sources.push_back("#ifdef GL_ES\n");
		sources.push_back("precision mediump float;\n");
		sources.push_back("#endif\n");
		break;
	}
	sources.push_back(shaderSource);

	glShaderSource(shaderID, sources.size(), &sources[0], NULL);
	glCompileShader(shaderID);

	GLint compiled = false;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		int infoLen = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 0)
		{
			tstring msg;
			msg.resize(infoLen);
			glGetShaderInfoLog(shaderID, infoLen, &infoLen, &msg[0]);

			glLog(msg.c_str());
		}
		glDeleteShader(shaderID);
		return 0;
	}

	return shaderID;
}

unsigned int glLoadProgram(const char* source)
{
	GLuint vsID = 0;
	GLuint fsID = 0;
	GLuint programID = 0;

	if ((vsID = loadShader(GL_VERTEX_SHADER, source)) == 0)
	{
		glLog("failed compile vertex shader");
		return 0;
	}
	if ((fsID = loadShader(GL_FRAGMENT_SHADER, source)) == 0)
	{
		glDeleteShader(vsID);
		glLog("failed compile fragment shader");
		return 0;
	}

	if ((programID = glCreateProgram()) == 0) return 0;

	glAttachShader(programID, vsID);
	glAttachShader(programID, fsID);
	glLinkProgram(programID);

	GLint linked = false;
	glGetProgramiv(programID, GL_LINK_STATUS, &linked);
	if (linked == 0)
	{
		int infoLen = 0;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen > 0)
		{
			tstring msg;
			msg.resize(infoLen);
			glGetProgramInfoLog(programID, infoLen, NULL, &msg[0]);

			glLog(msg.c_str());
		}
		glDeleteProgram(programID);
		programID = 0;
	}

	glDeleteShader(vsID);
	glDeleteShader(fsID);

	if (programID == 0) glLog("failed to loading program");
	return programID;
}

void exposeGL(MNFiber* fiber)
{
	fiber->push_string("GL"); //! ["GL"]
	fiber->push_table();      //! ["GL" table]
	fiber->up(1, 1);           //! [table "GL" table]
	fiber->store_global();    //! [gltable]

	struct ClearColor
	{
		static bool invoke(MNFiber* f)
		{
			float r = f->get(1).toFloat();
			float g = f->get(2).toFloat();
			float b = f->get(3).toFloat();
			float a = f->get(4).toFloat();
			glClearColor(r, g, b, a);
			return false;
		}
	};
	fiber->up(1, 0);
	fiber->push_string("ClearColor");
	fiber->push_closure(ClearColor::invoke);
	fiber->store_field();

	struct Clear
	{
		static bool invoke(MNFiber* f)
		{
			int mask = 0;
			if (f->get(1).toBool()) mask |= GL_COLOR_BUFFER_BIT;
			if (f->get(2).toBool()) mask |= GL_DEPTH_BUFFER_BIT;
			if (f->get(3).toBool()) mask |= GL_STENCIL_BUFFER_BIT;
			if (f->get(4).toBool()) mask |= GL_ACCUM_BUFFER_BIT;
			glClear(mask);
			return false;
		}
	};
	fiber->up(1, 0);
	fiber->push_string("Clear");
	fiber->push_closure(Clear::invoke);
	fiber->store_field();

	struct LoadProgram
	{
		static bool invoke(MNFiber* f)
		{
			MNString* str = f->get(1).toString();
			if (!str) return false;

			tuint programID = glLoadProgram(str->ss().c_str());
			if (programID == 0) return false;
			f->push_int(programID);
			return true;
		}
	};
	fiber->up(1, 0);
	fiber->push_string("LoadProgram");
	fiber->push_closure(LoadProgram::invoke);
	fiber->store_field();

	struct BindAttribLocation
	{
		static bool invoke(MNFiber* f)
		{
			tuint programID = f->get(1).toInt();
			tuint index     = f->get(2).toInt();
			MNString* str   = f->get(3).toString();
			if (!str) return false;
			glBindAttribLocation(programID, index, str->ss().c_str());
			return false;
		}
	};
	fiber->up(1, 0);
	fiber->push_string("BindAttribLocation");
	fiber->push_closure(BindAttribLocation::invoke);
	fiber->store_field();

	struct EnableVertexAttribArray
	{
		static bool invoke(MNFiber* f)
		{
			tuint index = f->get(1).toInt();
			glEnableVertexAttribArray(index);
			return false;
		}
	};
	fiber->up(1, 0);
	fiber->push_string("EnableVertexAttribArray");
	fiber->push_closure(EnableVertexAttribArray::invoke);
	fiber->store_field();

	struct LinkAndUse
	{
		static bool invoke(MNFiber* f)
		{
			tuint programID = f->get(1).toInt();
			glLinkProgram(programID);
			glUseProgram(programID);
			return false;
		}
	};
	fiber->up(1, 0);
	fiber->push_string("LinkAndUse");
	fiber->push_closure(LinkAndUse::invoke);
	fiber->store_field();

	fiber->pop(1);

	////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////

	/*
	tuint programID = glLoadProgram("");
	glBindAttribLocation(programID, 0, "pos");
	glBindAttribLocation(programID, 1, "tex");
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glLinkProgram(programID);
	glUseProgram(programID);
	glLoadIdentity();
	*/
};
