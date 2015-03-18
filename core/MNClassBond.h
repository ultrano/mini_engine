#ifndef _H_MNClassBond
#define _H_MNClassBond


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct class_name
{
	static const tstring& name(const tstring& n)
	{
		static tstring m_name(n);
		return m_name;
	}
};

class user_object
{
public:
	user_object(void* o) : object(0) {};
	virtual ~user_object() {};
	void* object;
};

template<typename T>
class user_class : public user_object
{
public:
	~user_class() { delete (T*)object; }

	user_class() : user_object(new T()) {}

	template<typename A1>
	user_class(A1& a1) : user_object(new T(a1)) {}

	template<typename A1, typename A2>
	user_class(A1& a1, A2& a2) : user_object(new T(a1, a2)) {}
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

template<typename T>
bool user_constructor(MNFiber* s)
{
	MNUserData* userData = s->newUserData();
	new (userData->newData(sizeof(user_class<T>))) user_class<T>();

	MNObject meta = s->newString(class_name<T>::name());
	meta = s->globalTable().get(metaName);

	userData->setMeta(meta);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

template<typename T>
inline T void2val(void* p) { return *(T*)p; }

inline user_object* void2user(void* p) { return (user_object*)p; }

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

template<typename R, typename T, typename A1 = void, typename A2 = void>
struct native_function
{
	typedef R(T::*Method)(A1, A2);
	static bool invoke(MNFiber* state)
	{
		T* self = (T*)state->get(0).toObject();
		if (MNUserData* userData = mnrtti_cast<MNUserData>(self)) self = (T*)void2user(userData->getData())->object;
		state->push((self->*void2val<Method>(state->getUpval(0).to<MNUserData>()->getData()))(state->get(1), state->get(2)));
		return true;
	}
};

template<typename R, typename T, typename A1>
struct native_function<R, T, A1>
{
	typedef R(T::*Method)(A1);
	static bool invoke(MNFiber* state)
	{
		T* self = (T*)state->get(0).toObject();
		if (MNUserData* userData = mnrtti_cast<MNUserData>(self)) self = (T*)void2user(userData->getData())->object;
		state->push((self->*void2val<Method>(state->getUpval(0).to<MNUserData>()->getData()))(state->get(1)));
		return true;
	}
};

template<typename R, typename T>
struct native_function<R, T>
{
	typedef R(T::*Method)();
	static bool invoke(MNFiber* state)
	{
		T* self = (T*)state->get(0).toObject();
		if (MNUserData* userData = mnrtti_cast<MNUserData>(self)) self = (T*)void2user(userData->getData())->object;
		state->push((self->*void2val<Method>(state->getUpval(0).to<MNUserData>()->getData()))());
		return true;
	}
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

template<typename T, typename A1, typename A2>
struct native_function<void, T, A1, A2>
{
	typedef void(T::*Method)(A1, A2);
	static bool invoke(MNFiber* state)
	{
		T* self = (T*)state->get(0).toObject();
		if (MNUserData* userData = mnrtti_cast<MNUserData>(self)) self = (T*)void2user(userData->getData())->object;
		(self->*void2val<Method>(state->getUpval(0).to<MNUserData>()->getData()))(state->get(1), state->get(2));
		return false;
	}
};

template<typename T, typename A1>
struct native_function<void, T, A1>
{
	typedef void(T::*Method)(A1);
	static bool invoke(MNFiber* state)
	{
		T* self = (T*)state->get(0).toObject();
		if (MNUserData* userData = mnrtti_cast<MNUserData>(self)) self = (T*)void2user(userData->getData())->object;
		(self->*void2val<Method>(state->getUpval(0).to<MNUserData>()->getData()))(state->get(1));
		return false;
	}
};

template<typename T>
struct native_function<void, T>
{
	typedef void(T::*Method)();
	static bool invoke(MNFiber* state)
	{
		T* self = (T*)state->get(0).toObject();
		if (MNUserData* userData = mnrtti_cast<MNUserData>(self)) self = (T*)void2user(userData->getData())->object;
		(self->*void2val<Method>(state->getUpval(0).to<MNUserData>()->getData()))();
		return false;
	}
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

template<typename R, typename T, typename A1, typename A2>
void setMethod(MNClosure* closure, R(T::*method)(A1, A2))
{
	closure->setFunc(native_function<R, T, A1, A2>::invoke);
}

template<typename R, typename T, typename A1, typename A2>
void setMethod(MNClosure* closure, R(T::*method)(A1, A2) const)
{
	closure->setFunc(native_function<R, T, A1, A2>::invoke);
}

template<typename R, typename T, typename A1>
void setMethod(MNClosure* closure, R(T::*method)(A1))
{
	closure->setFunc(native_function<R, T, A1>::invoke);
}

template<typename R, typename T, typename A1>
void setMethod(MNClosure* closure, R(T::*method)(A1) const)
{
	closure->setFunc(native_function<R, T, A1>::invoke);
}

template<typename R, typename T>
void setMethod(MNClosure* closure, R(T::*method)())
{
	closure->setFunc(native_function<R, T>::invoke);
}

template<typename R, typename T>
void setMethod(MNClosure* closure, R(T::*method)() const)
{
	closure->setFunc(native_function<R, T>::invoke);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

template<typename F>
void bindMethod(MNClosure* closure, F func)
{
	MNUserData* userData = closure->state()->newUserData();
	new(userData->newData(sizeof(F))) F(func);
	closure->addLink(new UpLink(MNObject(userData)));
	setMethod(closure, func);
}

template<typename F>
void bindMeta(MNGCObject* object, const tstring& field, F func)
{
	if (!object) return;
	MNTable* table = object->getMeta().to<MNTable>();
	if (!table)
	{
		table = object->state()->newTable();
		object->setMeta(table);
	}
	MNString* key = table->state()->newString(field);
	MNClosure* val = table->state()->newClosure();
	bindMethod(val, func);
	table->insert(key, val);
}

template<typename F>
void fillMeta(MNGCObject* object, const tstring& field, F func)
{
	if (!object) return;
	MNTable* table = mnrtti_cast<MNTable>(object);
	MNString* key = table->state()->newString(field);
	MNClosure* val = table->state()->newClosure();
	bindMethod(val, func);
	table->insert(MNObject((MNGCObject*)key, false), MNObject((MNGCObject*)val, false));
}

#endif