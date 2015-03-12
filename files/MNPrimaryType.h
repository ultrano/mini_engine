#ifndef _H_MNPrimaryType
#define _H_MNPrimaryType

#include <string>
#include <list>
#include <map>
#include <set>
#include <vector>

typedef std::string    tstring;
typedef std::wstring   twstring;

typedef bool           tboolean;

typedef float          tfloat;
typedef double         tdouble;

typedef tfloat         tfloat32;
typedef double         tdouble64;

typedef tfloat32       treal32;
typedef tdouble64      treal64;

typedef short          tshort;
typedef unsigned short tushort;

typedef char           tchar;
typedef unsigned char  tbyte;

typedef int            tint;
typedef tchar          tint8;
typedef tshort         tint16;
typedef tint           tint32;
typedef long long int  tint64;

typedef size_t         tsize;
typedef unsigned int   tuint;
typedef tbyte          tuint8;
typedef tushort        tuint16;
typedef tuint          tuint32;
typedef unsigned long long int tuint64;

typedef tuint32        thash32;
typedef tuint64        thash64;


#define tlist  std::list
#define tset   std::set
#define tarray std::vector
#define tpair  std::pair
#define ttable std::map

class THashString
{
public:
	THashString();
	THashString(const tstring& str);
	static thash32 makeHash(const tstring& str);
	static thash32 makeHash(const char* str, tsize len);

	inline tsize size() { return m_str.size(); }
	inline thash32 hash() const { return m_hash; }
	inline const tstring& str() const { return m_str; }
	inline const tchar* c_str() const { return m_str.c_str(); }
	inline THashString& operator =(const tstring& str) { m_str = str; m_hash = makeHash(str); return *this; }
	inline bool	operator ==(const THashString& sstr) const { return (m_hash == sstr.m_hash && m_str == sstr.m_str); };
	inline bool	operator !=(const THashString& sstr) const { return (m_hash != sstr.m_hash); };
private:
	thash32 m_hash;
	tstring m_str;
};

typedef THashString thashstring;

template<typename T>
class TFlag
{
public:

	T flags;

	TFlag():flags(0){};
	inline void	set(tuint index,bool check) { if (check) flags |= 1<<index; else flags &= ~(1<<index); }
	inline bool get(tuint index) const { return (( flags & ( 1 << index ) ) != 0); }
	inline void	clear( bool check = false ) { flags = check? -1 : 0; }
	inline bool	operator ==( const TFlag<T>& flag ) { return flag.flags == flags; };
	inline bool	operator !=( const TFlag<T>& flag ) { return flag.flags != flags; };
};

typedef	TFlag<tuint8>	tflag8;
typedef	TFlag<tuint16>	tflag16;
typedef	TFlag<tuint32>	tflag32;
typedef	TFlag<tuint64>	tflag64;

enum TObjectType : tuint8
{
	Referrer = (1 << 0),

	//! primitive types
	Null      = (1<<1),
	Pointer   = (2<<1),
	Int       = (3<<1),
	Float     = (4<<1),
	Boolean   = (5<<1),
	CFunction = (6<<1),

	//! 
	String   = (7<<1)  | Referrer,
	Fiber    = (8<<1)  | Referrer,
	Closure  = (9<<1)  | Referrer,
	Table    = (10<<1) | Referrer,
	Array    = (11<<1) | Referrer,
	Function = (12<<1) | Referrer,
};

enum TObjectProp
{
	WeekRef = 0,
};

enum TMeta
{
	Add,
	Sub,
	Mul,
	Div,
	Mod,
	Equals,
	GetField,
	SetField,
	Last,
};

inline const tchar* meta2Str(tsize meta)
{
	if (meta >= TMeta::Last) return NULL;

	static const tchar* metaStr[] =
	{
		"+", "-", "*", "/", "%", "==", "->", "-<",
	};
	return metaStr[meta];
}

enum TCommand : tbyte
{
	cmd_none,
	cmd_push_null,
	cmd_push_int, //! >> tint
	cmd_push_float, //! >> tfloat
	cmd_push_true,
	cmd_push_false,
	cmd_push_closure,
	cmd_push_table,
	cmd_push_array,
	cmd_pop1,
	cmd_pop2,
	cmd_popn,//! >> tbyte
	cmd_load_const,  //! >> tuint16
	cmd_load_upval,  //! >> tuint16
	cmd_store_upval, //! >> tuint16
	cmd_load_method, //! [object key] -> [closure object]
	cmd_load_this,   //! load_stack(0)
	cmd_load_stack,  //! >> tuint16
	cmd_store_stack, //! >> tuint16
	cmd_load_field,
	cmd_store_field,
	cmd_insert_field,
	cmd_load_global, //! >> tuint16
	cmd_store_global, //! >> tuint16
	cmd_set_meta,
	cmd_get_meta,
	cmd_up,//! >> tbyte, tbyte
	cmd_up1,
	cmd_up1_x2,
	cmd_up2,
	cmd_swap,
	cmd_eq,
	cmd_neq,
	cmd_lt,
	cmd_gt,
	cmd_leq,
	cmd_geq,
	cmd_and,
	cmd_or,
	cmd_tostring,
	cmd_add,
	cmd_sub,
	cmd_mul,
	cmd_div,
	cmd_mod,
	cmd_jmp, //! >> tint16
	cmd_fjp, //! >> tint16
	cmd_call,//! >> 1byte
	cmd_call_void,//! >> 1byte
	cmd_return,
	cmd_return_void,
	cmd_yield,
	cmd_yield_void,
	cmd_close_links, //! >> tuint16
};

class MNFiber;
typedef bool(*TCFunction)(MNFiber* s); // Native C Function

#endif