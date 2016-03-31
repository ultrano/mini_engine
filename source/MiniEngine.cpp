#include "MiniEngine.h"
#include "MNArray.h"
#include "MNBasicLib.h"
#include "MNClosure.h"
#include "MNCollectable.h"
#include "MNCommand.h"
#include "MNCompiler.h"
#include "MNCountable.h"
#include "MNFiber.h"
#include "MNFunction.h"
#include "MNGlobal.h"
#include "MNLexer.h"
#include "MNList.h"
#include "MNMemory.h"
#include "MNObject.h"
#include "MNPrimaryType.h"
#include "MNReferrer.h"
#include "MNRtti.h"
#include "MNString.h"
#include "MNTable.h"
#include "MNUserData.h"
#include "MNFileSystem.h"

#include "MNOpenGL.h"


void glInitialize()
{
    
    glClearColor(0,0,1,1);
    
    glEnable(GL_TEXTURE_2D);
    
    glEnable(GL_BLEND);
    //glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glAlphaFunc(GL_GREATER, 0);
    
    
    glViewport(0,0,320,670);
}

tuint32& glProgramID()
{
    static tuint32 programID;
    return programID;
}

void glCompileProgram(const char* source)
{
    glProgramID() = glLoadProgram(source);
    glBindAttribLocation(glProgramID(), 0, "aPos");
    glBindAttribLocation(glProgramID(), 1, "aTex");
}

void glDrawImage(tuint32 textureID, float x, float y, float width, float height)
{
    static float vertices[][3] =
    {
        {0.5f, 0.5f, 0.0f},
        {0.5f, -0.5f, 0.0f},
        {-0.5f, 0.5f, 0.0f},
        {-0.5f, -0.5f, 0.0f},
    };
    static float texCoords[][2] =
    {
        {1, 0},
        {1, 1},
        {0, 0},
        {0, 1},
    };
    static tushort triangles[][3] =
    {
        {0,1,2},
        {3,2,1},
    };
    
    ;
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (float*)&vertices[0][0] );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (float*)&texCoords[0][0] );
    
    TMatrix4x4 proj;
    proj.ortho(-500, 500, -500, 500, -1000, 1000);
    
    TMatrix4x4 mvp;
    TQuaternion quat;
    mvp.transform(TVector3(width, height, 1.0f), quat, TVector3(x, y, 0.0f));
    mvp *= proj;
    
    static tuint32 mvpLoc = glGetUniformLocation(glProgramID(), "MVP");
    glUniformMatrix4fv( mvpLoc, 1, GL_FALSE, (float*)&mvp );
    
    
    static tuint32 tex0Loc = glGetUniformLocation(glProgramID(), "TEX0");
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureID );
    glUniform1i( tex0Loc, 0 );
    
    glDrawElements( GL_TRIANGLES, 2*3, GL_UNSIGNED_SHORT, (tushort*)&triangles[0][0] );
    
}

#define MNContext (_MNContext::instance())
class _MNContext
{
public:
	static _MNContext& instance() { static _MNContext inst; return inst; }
	MNFiber* main;
};



tuint32& texID()
{
    static tuint32 id;
    return id;
}

void MNStart(const char* resourceFolder, const char* starterScript)
{
    MNResourceFolderPath(resourceFolder);
    MNContext.main = new MNFiber();
    MNContext.main->dofile("script/main.mn");
	if (starterScript != NULL) MNContext.main->dofile(starterScript);

    tstring path = MNResourceFolderPath() + "texture/background2.png";
    int width, height;
    texID() = glLoadTexture( path.c_str(), width, height);
    
    tstring shader;
    shader +=
    "uniform   mat4 MVP;\n"
    "uniform sampler2D TEX0;\n"
    "varying   vec2 vTex;\n"
    "#ifdef VERTEX_SHADER\n"
    "attribute vec4 aPos;\n"
    "attribute vec2 aTex;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = MVP * aPos;\n"
    "    vTex = aTex;\n"
    "}\n"
    "#endif\n"
    "#ifdef FRAGMENT_SHADER\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = texture2D( TEX0, vTex.st, 0.0 );\n"
    "}\n"
    "#endif";
    
    glInitialize();
    glCompileProgram(shader.c_str());

}

void MNUpdate()
{
	MNContext.main->push_string("onUpdate");
	MNContext.main->load_global();
	MNContext.main->load_stack(0);
	MNContext.main->call(1, false);
}

void MNRender()
{
    //glCullFace(GL_FRONT_AND_BACK);
    glUseProgram(glProgramID());
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glDrawImage(texID(), 0, 0, 500, 500);
}

void MNTouch(int type, int posX, int posY)
{
	MNContext.main->push_string("onTouch");
	MNContext.main->load_global();
	MNContext.main->load_stack(0);
	MNContext.main->push_integer(type);
	MNContext.main->push_integer(posX);
	MNContext.main->push_integer(posY);
	MNContext.main->call(4, false);
}