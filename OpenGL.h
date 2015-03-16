#ifndef _H_OpenGL
#define _H_OpenGL

inline void exposeGL(MNFiber* fiber)
{
	fiber->push_string("GL"); //! ["GL"]
	fiber->push_table();      //! ["GL" table]
	fiber->up(1,1);           //! [table "GL" table]
	fiber->store_global();    //! [gltable]

	struct ClearColor
	{
		static bool invoke(MNFiber* f)
		{
			float r = f->get(1).toFloat();
			float g = f->get(2).toFloat();
			float b = f->get(3).toFloat();
			float a = f->get(4).toFloat();
			glClearColor(r,g,b,a);
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

	fiber->pop(1);
};

#endif