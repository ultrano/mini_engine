#ifndef _H_MNRtti
#define _H_MNRtti

/** @biref save the RTTI(Real Time Type Info).
*/
class MNRtti
{
public:

	const char* name;  //< name of type, it would be used on native for checking type
	//const char* alias; //< alias of type, it would be used on script for checking type
	const MNRtti* super; //< info of super type

	MNRtti(const char* n) : name(n), super(0) {}
	MNRtti(const char* n, const MNRtti* s) : name(n), super(s) {}
};

/* @brief declare RTTI start point */
#define MN_RTTI_ROOT(typeName) public:\
    virtual const MNRtti* queryRtti() const { return getRtti(); }; \
    static const MNRtti* getRtti() { static MNRtti rtti(#typeName); return &rtti;} \
    typedef typeName __this;\
private:

/* @brief write type name and super type name */

#ifdef _MSC_VER
#define MN_RTTI(typeName, superType) public:\
	virtual const MNRtti* queryRtti() const { return getRtti(); }; \
	static const MNRtti* getRtti() { static MNRtti rtti(#typeName, superType::getRtti()); return &rtti;} \
	typedef typeName __this;\
private:
#else
#define MN_RTTI(typeName, superType) public:\
	virtual const MNRtti* queryRtti() const { return getRtti(); }; \
	static const MNRtti* getRtti() { static MNRtti rtti(#typeName, superType::getRtti()); return &rtti;} \
	typedef typeName __this;\
	typedef superType __super;\
private:
#endif
/**
@brief using like (static_cast, reinterpret_cast)
static_cast<TypeA>( a );
reinterpret_cast<TypeB>( b );
mnrtti_cast<TypeC>( c ); << using like this.
%%% MNRtti will fail to compile if there isn't. %%%
*/
template<typename T1, typename T2>
inline T1* mnrtti_cast(T2* cls)
{
	if (!cls) return NULL;
	const MNRtti* rtti = cls->queryRtti();
	while (rtti)
	{
		if (rtti == T1::getRtti()) return (T1*)cls;
		rtti = rtti->super;
	}
	return NULL;
}

#endif