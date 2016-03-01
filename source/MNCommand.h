//#ifndef _H_MNCommand
//#define _H_MNCommand
//
//#include "MNMemory.h"
//#include "MNPrimaryType.h"
//
//class MNFiber;
//class MNCommand : public MNMemory
//{
//public:
//	MNCommand(MNFiber* fiber) : m_fiber(fiber) {};
//	~MNCommand() {};
//
//	tint32       to_int(tint32 idx, tint32 def = 0);
//	tfloat       to_float(tint32 idx, tfloat def = 0.0f);
//	const tchar* to_string(tint32 idx, const tchar* def = NULL);
//	void*        to_pointer(tint32 idx, void* def = NULL);
//	tboolean     to_bool(tint32 idx, tboolean def = false);
//
//	void push_null();
//	void push_int(tint32 val);
//	void push_float(tfloat val);
//	void push_string(const tstring& val);
//	void push_pointer(void* val);
//	void push_bool(tboolean val);
//	void push_closure(TFunction val);
//	void push_table();
//	void push_array();
//
//	void pop(tsize count);
//
//	void load_stack(tint32 idx);
//	void store_stack(tint32 idx);
//
//	void load_field();
//	void store_field();
//
//	void load_global();
//	void store_global();
//
//	void set_meta();
//	void get_meta();
//
//	void up(tsize n, tsize x);
//	void swap();
//
//	void equals();
//	void tostring();
//
//	void add();
//	void sub();
//	void mul();
//	void div();
//	void mod();
//
//	void call(tsize nargs, bool ret);
//
//private:
//	MNFiber* m_fiber;
//};
//
//#endif