#include "App.h"
#include "MNFiber.h"
#include "TMatrix4x4.h"
#include "MNUserData.h"

void exposeApp( MNFiber* fiber )
{
	fiber->push_string("rootPath");
	fiber->push_string("../resource/script/");
	fiber->store_global();

	fiber->push_string("mat4"); //! ["mat4"]
	fiber->push_table();      //! ["mat4" mat4Table]
	fiber->up(1, 1);           //! [mat4Table "mat4" mat4Table]
	fiber->store_global();    //! [mat4Table]

	struct mat4
	{
		static bool _new(MNFiber* f)
		{
			TMatrix4x4* mat = (TMatrix4x4*)f->push_userdata(sizeof(TMatrix4x4));
			mat->TMatrix4x4::TMatrix4x4();
			f->up(1,0);
			f->load_stack(0);
			f->set_meta();
			return true;
		}

		static bool transpose(MNFiber* f)
		{
			MNUserData* ud = f->get(0).toUserData();
			if (!ud) return false;
			if (ud->getSize() != sizeof(TMatrix4x4)) return false;
			TMatrix4x4* mat = (TMatrix4x4*)ud->getData();
			mat->transpose(*mat);
			return false;
		}

		static bool transform(MNFiber* f)
		{
			MNUserData* ud = f->get(0).toUserData();
			if (!ud) return false;
			if (ud->getSize() != sizeof(TMatrix4x4)) return false;
			TMatrix4x4* mat = (TMatrix4x4*)ud->getData();
			float args[10];
			for (tsize i=0;i<10;++i) args[i] = f->get(i+1).toFloat();
			mat->transform(TVector3f(args[0],args[1],args[2]),TQuaternion(args[3],args[4],args[5],args[6]) ,TVector3f(args[7],args[8],args[9]));
			return false;
		}

		static bool ortho(MNFiber* fi)
		{
			MNUserData* ud = fi->get(0).toUserData();
			if (!ud) return false;
			if (ud->getSize() != sizeof(TMatrix4x4)) return false;
			TMatrix4x4* mat = (TMatrix4x4*)ud->getData();
			float l = fi->get(1).toFloat();
			float r = fi->get(2).toFloat();
			float b = fi->get(3).toFloat();
			float t = fi->get(4).toFloat();
			float n = fi->get(5).toFloat();
			float f = fi->get(6).toFloat();
			mat->ortho(l,r,b,t,n,f);
			return false;
		}

		static bool perspective(MNFiber* fi)
		{
			MNUserData* ud = fi->get(0).toUserData();
			if (!ud) return false;
			if (ud->getSize() != sizeof(TMatrix4x4)) return false;
			TMatrix4x4* mat = (TMatrix4x4*)ud->getData();
			float fov    = fi->get(1).toFloat();
			float aspect = fi->get(2).toFloat();
			float near   = fi->get(3).toFloat();
			float far    = fi->get(4).toFloat();
			mat->perspective(fov, aspect, near, far);
			return false;
		}

		static bool get(MNFiber* fi)
		{
			MNUserData* ud = fi->get(0).toUserData();
			if (!ud) return false;
			if (ud->getSize() != sizeof(TMatrix4x4)) return false;
			TMatrix4x4* mat = (TMatrix4x4*)ud->getData();
			int row = fi->get(1).toInt();
			int col = fi->get(2).toInt();
			fi->push_float(mat->m[row][col]);
			return true;
		}

		static bool mult(MNFiber* fi)
		{
			MNUserData* ud1 = fi->get(0).toUserData();
			if (!ud1) return false;
			if (ud1->getSize() != sizeof(TMatrix4x4)) return false;

			MNUserData* ud2 = fi->get(1).toUserData();
			if (!ud2) return false;
			if (ud2->getSize() != sizeof(TMatrix4x4)) return false;

			TMatrix4x4* mat1 = (TMatrix4x4*)ud1->getData();
			TMatrix4x4* mat2 = (TMatrix4x4*)ud2->getData();
			*mat1 *= *mat2;
			return false;
		}
	};

	fiber->load_stack(-1);
	fiber->push_string("type");
	fiber->push_string("mat4");
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("->");
	fiber->load_stack(-2);
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("new");
	fiber->push_closure(mat4::_new);
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("transpose");
	fiber->push_closure(mat4::transpose);
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("transform");
	fiber->push_closure(mat4::transform);
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("ortho");
	fiber->push_closure(mat4::ortho);
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("perspective");
	fiber->push_closure(mat4::perspective);
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("get");
	fiber->push_closure(mat4::get);
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("mult");
	fiber->push_closure(mat4::mult);
	fiber->store_field();
}
