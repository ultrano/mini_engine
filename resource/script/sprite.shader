uniform   mat4 MVP;
uniform sampler2D TEX0;

varying   vec2 vTex;

#ifdef VERTEX_SHADER

attribute vec4 aPos;
attribute vec2 aTex;

void main()
{
gl_Position = MATRIX_MVP * aPos;
vTex = aTex;
}

#endif

#ifdef FRAGMENT_SHADER

void main()
{
gl_FragColor = texture2D( TEX0, vTex );
}

#endif