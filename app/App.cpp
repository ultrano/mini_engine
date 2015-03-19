#include "App.h"
#include "MNFiber.h"
#include "TMatrix4x4.h"
#include "MNUserData.h"
#include "MNString.h"

void exposeApp( MNFiber* fiber )
{
	fiber->push_string("rootPath");
	fiber->push_string("../resource/script/");
	fiber->store_global();

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

	fiber->push_string("mat4"); //! ["mat4"]
	fiber->push_table();      //! ["mat4" mat4Table]
	fiber->up(1, 1);           //! [mat4Table "mat4" mat4Table]
	fiber->store_global();    //! [mat4Table]

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

	fiber->pop(1);

	struct vec3
	{
		static bool _new(MNFiber* fiber)
		{
			TVector3f* vec = (TVector3f*)fiber->push_userdata(sizeof(TVector3f));
			float x = fiber->get(1).toFloat();
			float y = fiber->get(2).toFloat();
			float z = fiber->get(3).toFloat();
			vec->TVector3f::TVector3f(x,y,z);
			fiber->up(1,0);
			fiber->load_stack(0);
			fiber->set_meta();
			return true;
		}

		static bool length(MNFiber* fiber)
		{
			MNUserData* ud = fiber->get(0).toUserData();
			if (!ud) return false;
			if (ud->getSize() != sizeof(TVector3f)) return false;
			TVector3f* vec = (TVector3f*)ud->getData();
			fiber->push_float(vec->length());
			return true;
		}

		static bool lengthXYZ(MNFiber* fiber)
		{
			MNUserData* ud = fiber->get(0).toUserData();
			if (!ud) return false;
			if (ud->getSize() != sizeof(TVector3f)) return false;
			TVector3f* vec = (TVector3f*)ud->getData();
			float x = fiber->get(1).toFloat();
			float y = fiber->get(2).toFloat();
			float z = fiber->get(3).toFloat();
			fiber->push_float((*vec-TVector3f(x,y,z)).length());
			return true;
		}

		static bool dot(MNFiber* fiber)
		{
			MNUserData* ud = fiber->get(0).toUserData();
			if (!ud) return false;
			if (ud->getSize() != sizeof(TVector3f)) return false;
			TVector3f* arg1 = (TVector3f*)ud->getData();

			MNUserData* ud2 = fiber->get(1).toUserData();
			if (!ud2) return false;
			if (ud2->getSize() != sizeof(TVector3f)) return false;
			TVector3f* arg2 = (TVector3f*)ud2->getData();

			fiber->push_float(arg1->dot(*arg2));
			return true;
		}

		static bool cross(MNFiber* fiber)
		{
			MNUserData* ud = fiber->get(0).toUserData();
			if (!ud) return false;
			if (ud->getSize() != sizeof(TVector3f)) return false;
			TVector3f* arg1 = (TVector3f*)ud->getData();

			MNUserData* ud2 = fiber->get(1).toUserData();
			if (!ud2) return false;
			if (ud2->getSize() != sizeof(TVector3f)) return false;
			TVector3f* arg2 = (TVector3f*)ud2->getData();

			TVector3f* vec = (TVector3f*)fiber->push_userdata(sizeof(TVector3f));
			vec->TVector3f::TVector3f(arg1->cross(*arg2));
			return true;
		}

		static bool normalize(MNFiber* fiber)
		{
			MNUserData* ud = fiber->get(0).toUserData();
			if (!ud) return false;
			if (ud->getSize() != sizeof(TVector3f)) return false;
			TVector3f* arg1 = (TVector3f*)ud->getData();
			*arg1 = arg1->normal();
			return false;
		}

		static bool scale(MNFiber* fiber)
		{
			MNUserData* ud = fiber->get(0).toUserData();
			if (!ud) return false;
			if (ud->getSize() != sizeof(TVector3f)) return false;
			TVector3f* arg1 = (TVector3f*)ud->getData();

			MNUserData* ud2 = fiber->get(1).toUserData();
			if (!ud2) return false;
			if (ud2->getSize() != sizeof(TVector3f)) return false;
			TVector3f* arg2 = (TVector3f*)ud2->getData();

			*arg1 = arg1->scale(*arg2);
			return false;
		}

		static bool getField(MNFiber* fiber)
		{
			MNUserData* ud = fiber->get(0).toUserData();
			if (!ud) return false;
			if (ud->getSize() != sizeof(TVector3f)) return false;
			TVector3f* arg1 = (TVector3f*)ud->getData();

			static thashstring nameX("x"), nameY("y"), nameZ("z");

			float val;
			MNString* name = fiber->get(1).toString();
			if (!name) return false;
			else if (name->ss() == nameX) val = arg1->x;
			else if (name->ss() == nameY) val = arg1->y;
			else if (name->ss() == nameZ) val = arg1->z;
			else
			{
				fiber->load_stack(0);
				fiber->get_meta();
				fiber->load_stack(1);
				fiber->load_field();
				return true;
			}
			fiber->push_float(val);
			return true;
		}

		static bool setField(MNFiber* fiber)
		{
			MNUserData* ud = fiber->get(0).toUserData();
			if (!ud) return false;
			if (ud->getSize() != sizeof(TVector3f)) return false;
			TVector3f* arg1 = (TVector3f*)ud->getData();

			static thashstring nameX("x"), nameY("y"), nameZ("z");

			MNString* name = fiber->get(1).toString();
			float val = fiber->get(2).toFloat();
			if (!name) return false;
			else if (name->ss() == nameX) arg1->x = val;
			else if (name->ss() == nameY) arg1->y = val;
			else if (name->ss() == nameZ) arg1->z = val;
			return true;
		}
	};

	fiber->push_string("vec3"); //! ["vec3"]
	fiber->push_table();      //! ["vec3" vec3Table]
	fiber->up(1, 1);           //! [vec3Table "vec3" vec3Table]
	fiber->store_global();    //! [vec3Table]

	fiber->load_stack(-1);
	fiber->push_string("new");
	fiber->push_closure(vec3::_new);
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("cross");
	fiber->push_closure(vec3::cross);
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("dot");
	fiber->push_closure(vec3::dot);
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("->");
	fiber->push_closure(vec3::getField);
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("-<");
	fiber->push_closure(vec3::setField);
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("length");
	fiber->push_closure(vec3::length);
	fiber->store_field();


	fiber->load_stack(-1);
	fiber->push_string("lengthXYZ");
	fiber->push_closure(vec3::lengthXYZ);
	fiber->store_field();
	fiber->load_stack(-1);
	fiber->push_string("normalize");
	fiber->push_closure(vec3::normalize);
	fiber->store_field();

	fiber->load_stack(-1);
	fiber->push_string("scale");
	fiber->push_closure(vec3::scale);
	fiber->store_field();

	fiber->pop(1);

	struct common
	{
		static bool random(MNFiber* fiber)
		{
			fiber->push_int(rand());
			return true;
		}
	};

	fiber->push_string("random");
	fiber->push_closure(common::random);
	fiber->store_global();
}
