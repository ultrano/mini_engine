#include "OpenGL.h"
#include "MNFiber.h"
#include "MNString.h"
#include "MNUserData.h"
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

GLuint glLoadProgram(const char* source)
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

void  glDrawRegion(int id, int srcWidth, int srcHeight, int offX, int offY, int width, int height)
{
	static tfloat ver[][3] = { {-10.0f, +10.0f, 0}, {+10.0f, +10.0f, 0}, {-10.0f, -10.0f, 0}, {+10.0f, -10.0f, 0}};
	static tushort ind[][3] = { {0,1,2}, {3,2,1}};

	tfloat x = (float)offX/(float)srcWidth;
	tfloat y = (float)offY/(float)srcHeight;
	tfloat w = (float)width/(float)srcWidth;
	tfloat h = (float)height/(float)srcHeight;
	tfloat tex[][2] = { {x,y}, {x+w,y}, {x,y+h}, {x+w,y+h}};

	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, &ver[0] );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, &tex[0] );
	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, &ind[0] );
}

struct GL
{
	static bool ClearColor(MNFiber* f)
	{
		float r = f->get(1).toFloat();
		float g = f->get(2).toFloat();
		float b = f->get(3).toFloat();
		float a = f->get(4).toFloat();
		glClearColor(r, g, b, a);
		return false;
	};

	static bool Clear(MNFiber* f)
	{
		int mask = 0;
		if (f->get(1).toBool()) mask |= GL_COLOR_BUFFER_BIT;
		if (f->get(2).toBool()) mask |= GL_DEPTH_BUFFER_BIT;
		if (f->get(3).toBool()) mask |= GL_STENCIL_BUFFER_BIT;
		if (f->get(4).toBool()) mask |= GL_ACCUM_BUFFER_BIT;
		glClear(mask);
		return false;
	};

	static bool LoadProgram(MNFiber* f)
	{
		MNString* str = f->get(1).toString();
		if (!str) return false;

		tuint programID = glLoadProgram(str->ss().c_str());
		if (programID == 0) return false;
		f->push_int(programID);
		return true;
	};

	static bool BindAttribLocation(MNFiber* f)
	{
		tuint programID = f->get(1).toInt();
		tuint index = f->get(2).toInt();
		MNString* str = f->get(3).toString();
		if (!str) return false;
		glBindAttribLocation(programID, index, str->ss().c_str());
		return false;
	};

	static bool EnableVertexAttribArray(MNFiber* f)
	{
		tuint index = f->get(1).toInt();
		glEnableVertexAttribArray(index);
		return false;
	};

	static bool GetAttribLocation(MNFiber* f)
	{
		tint programID = f->get(1).toInt();
		MNString* str = f->get(2).toString();
		if (!str) return false;
		int loc = glGetAttribLocation(programID, str->ss().c_str());
		f->push_int(loc);
		return true;
	};

	static bool GetUniformLocation(MNFiber* f)
	{
		tint programID = f->get(1).toInt();
		MNString* str = f->get(2).toString();
		if (!str) return false;
		int loc = glGetUniformLocation(programID, str->ss().c_str());
		f->push_int(loc);
		return true;
	};

	static bool UniformMatrix(MNFiber* f)
	{
		tint loc = f->get(1).toInt();
		MNUserData* ud = f->get(2).toUserData();
		if (!ud) return false;
		if (ud->getSize() < sizeof(float) * 16) return false;
		glUniformMatrix4fv(loc, 1, GL_FALSE, (float*)(ud->getData()));
		return false;
	};

	static bool GetFloatv(MNFiber* f)
	{
		tint index = f->get(1).toInt();
		MNUserData* ud = f->get(2).toUserData();
		if (!ud) return false;
		glGetFloatv(index, (float*)ud->getData());
		return false;
	};

	static bool LinkProgram(MNFiber* f)
	{
		tuint programID = f->get(1).toInt();
		glLinkProgram(programID);
		return false;
	}

	static bool MatrixMode(MNFiber* f) { glMatrixMode(f->get(1).toInt()); return false; };

	static bool UseProgram(MNFiber* f)
	{
		tuint programID = f->get(1).toInt();
		glUseProgram(programID);
		return false;
	}

	static bool LoadTexture(MNFiber* f)
	{
		MNString* str = f->get(1).toString();
		if (!str) return false;

		tint id, width, height;
		id = glLoadTexture(str->ss().c_str(), width, height);
		if (id == 0) return false;

		f->push_table(3);

		f->up(1, 0);
		f->push_string("id");
		f->push_int(id);
		f->store_field();

		f->up(1, 0);
		f->push_string("width");
		f->push_int(width);
		f->store_field();

		f->up(1, 0);
		f->push_string("height");
		f->push_int(height);
		f->store_field();
		return true;
	}

	static bool BindTexture(MNFiber* f)
	{
		tuint loc = f->get(1).toInt();
		tuint texID = f->get(2).toInt();
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texID);
		glUniform1i(loc, 0);
		return false;
	}

	static bool PushMatrix(MNFiber* f) { glPushMatrix(); return false; };
	static bool PopMatrix(MNFiber* f) { glPopMatrix(); return false; };
	static bool LoadIdentity(MNFiber* f) { glLoadIdentity(); return true; };
	static bool LoadMatrix(MNFiber* f)
	{
		float* mat = NULL;
		MNUserData* ud = f->get(1).toUserData();
		if (!ud) return false;
		if (ud->getSize() < sizeof(float) * 16) return false;
		mat = (float*)ud->getData();
		glLoadMatrixf(mat);
		return false;
	}
	static bool Translate(MNFiber* f)
	{
		glTranslatef(f->get(1).toFloat(), f->get(2).toFloat(), f->get(3).toFloat());
		return false;
	}
	static bool Rotate(MNFiber* f)
	{
		glRotatef(f->get(1).toFloat(), f->get(2).toFloat(), f->get(3).toFloat(), f->get(4).toFloat());
		return false;
	}
	static bool Scale(MNFiber* f)
	{
		glScalef(f->get(1).toFloat(), f->get(2).toFloat(), f->get(3).toFloat());
		return false;
	}
	static bool Ortho(MNFiber* fi)
	{
		tint l = fi->get(1).toInt();
		tint r = fi->get(2).toInt();
		tint b = fi->get(3).toInt();
		tint t = fi->get(4).toInt();
		tint n = fi->get(5).toInt();
		tint f = fi->get(6).toInt();
		glOrtho(l, r, b, t, n, f);
		return false;
	}
	static bool Viewport(MNFiber* fi)
	{
		tint x = fi->get(1).toInt();
		tint y = fi->get(2).toInt();
		tint w = fi->get(3).toInt();
		tint h = fi->get(4).toInt();
		glViewport(x, y, w, h);
		return false;
	}
	static bool DrawRegion(MNFiber* fi)
	{
		tint id = fi->get(1).toInt();
		tint w = fi->get(2).toInt();
		tint h = fi->get(3).toInt();
		tint offX = fi->get(4).toInt();
		tint offY = fi->get(5).toInt();
		tint offW = fi->get(6).toInt();
		tint offH = fi->get(7).toInt();

		glDrawRegion(id, w, h, offX, offY, offW, offH);
		return false;
	}
};
void exposeGL(MNFiber* fiber)
{
	fiber->push_string("GL"); //! ["GL"]
	fiber->push_table();      //! ["GL" table]
	fiber->up(1, 1);           //! [table "GL" table]
	fiber->store_global();    //! [gltable]

	fiber->up(1, 0);
	fiber->push_string("ClearColor");
	fiber->push_closure(GL::ClearColor);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("Clear");
	fiber->push_closure(GL::Clear);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("LoadProgram");
	fiber->push_closure(GL::LoadProgram);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("BindAttribLocation");
	fiber->push_closure(GL::BindAttribLocation);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("EnableVertexAttribArray");
	fiber->push_closure(GL::EnableVertexAttribArray);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("GetAttribLocation");
	fiber->push_closure(GL::GetAttribLocation);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("GetUniformLocation");
	fiber->push_closure(GL::GetUniformLocation);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("UniformMatrix");
	fiber->push_closure(GL::UniformMatrix);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("MatrixMode");
	fiber->push_closure(GL::MatrixMode);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("GetFloatv");
	fiber->push_closure(GL::GetFloatv);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("LinkProgram");
	fiber->push_closure(GL::LinkProgram);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("UseProgram");
	fiber->push_closure(GL::UseProgram);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("LoadTexture");
	fiber->push_closure(GL::LoadTexture);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("BindTexture");
	fiber->push_closure(GL::BindTexture);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("PushMatrix");
	fiber->push_closure(GL::PushMatrix);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("PopMatrix");
	fiber->push_closure(GL::PopMatrix);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("LoadIdentity");
	fiber->push_closure(GL::LoadIdentity);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("LoadMatrix");
	fiber->push_closure(GL::LoadMatrix);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("Translate");
	fiber->push_closure(GL::Translate);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("Rotate");
	fiber->push_closure(GL::Rotate);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("Scale");
	fiber->push_closure(GL::Scale);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("Ortho");
	fiber->push_closure(GL::Ortho);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("Viewport");
	fiber->push_closure(GL::Viewport);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("DrawRegion");
	fiber->push_closure(GL::DrawRegion);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("MODELVIEW");
	fiber->push_int(GL_MODELVIEW);
	fiber->store_field();

	fiber->up(1, 0);
	fiber->push_string("MODELVIEW_MATRIX");
	fiber->push_int(GL_MODELVIEW_MATRIX);
	fiber->store_field();

	fiber->pop(1);

	////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////

	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	//glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
