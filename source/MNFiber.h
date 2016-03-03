#ifndef _H_MNFiber
#define _H_MNFiber

#include "MNCollectable.h"

class MNClosure;
class UpLink;
class MNFiber : public MNCollectable
{
	MN_RTTI(MNFiber, MNCollectable);

public:

	enum Status { Stop = 0, Suspend = 1, Resume = 2 };

	class CallInfo : public MNMemory
	{
	public:
		CallInfo*	prev;
		MNClosure*  closure;
		tbyte*		pc;
		tint		begin;
		tint		end;
		bool        ret;
	};

	MNFiber(MNGlobal* global = NULL);
	~MNFiber();

	MNGlobal* global() const;

	//MNValue globalTable() const;

	bool compileFile(MNObject& func, const tstring& path);
	bool dofile(const tstring& path);

	void            setAt(tint32 idx, const MNObject& val);
	const MNObject& getAt(tint32 idx) const;

	void            set(tint32 idx, const MNObject& val);
	const MNObject& get(tint32 idx) const;
	
	const MNObject& getConst(tsize idx) const;

	void            setUpval(tuint32 idx, const MNObject& val);
	const MNObject& getUpval(tuint32 idx);

	CallInfo*       enterCall(tuint nargs, bool ret);
	CallInfo*       returnCall(bool retOnTop);
	tint32          excuteCall();

	UpLink*         openLink(tint32 index);
	void            closeLinks(tint32 level);

	tsize           localSize() const;
	tsize			stackSize() const;

	void            setStatus(tbyte status);
	tbyte           getStatus() const;

	//! command methods.
	void  push_null();
	void  push_integer(tinteger val);
	void  push_real(treal val);
	void  push_string(const tstring& val);
	void  push_pointer(void* val);
	void  push_bool(tboolean val);
	void  push_closure(NativeFunc val);
	void  push_table(tsize size = 0);
	void  push_array(tsize size = 0);
	void  push_const(tsize idx);
	void* push_userdata(tsize size);
	void  push(const MNObject& val);
	void  pop(tuint32 count);

	void load_stack(tint32 idx);
	void store_stack(tint32 idx);

	bool load_raw_field();
	bool store_raw_field(bool insert = true);

	void load_field();
	void store_field(bool insert = true);
		 
	void load_global();
	void store_global();

	void set_meta();
	void get_meta();
		
	void up(tsize n, tsize x);
	void swap();
		
	void equals();
	void less_than();
	void and_with();
	void or_with();

	void tostring();

	void inc();
	void dec();
	void neg();
	void add();
	void sub();
	void mul();
	void div();
	void mod();

	void call(tsize nargs, bool ret);
private:

	virtual void travelMark();

private:
	friend class MNGlobal;
	MNGlobal* m_global;
	tarray<MNObject> m_stack;
	tlist<UpLink*> m_openLinks;
	CallInfo* m_info;
	tbyte m_status;
};

#endif