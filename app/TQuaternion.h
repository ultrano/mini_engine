#ifndef TQuaternion_h__
#define TQuaternion_h__

#include "TVector3f.h"

class TQuaternion
{
public:

	float x, y, z;
	float w;

	TQuaternion() : x(0), y(0), z(0), w(1) {};
	TQuaternion( const TQuaternion& copy ): x(copy.x), y(copy.y), z(copy.z), w(copy.w) {};
	TQuaternion( const TVector3f& v, float fw) : x(v.x), y(v.y), z(v.z), w(fw) {};
	TQuaternion(float fx, float fy, float fz, float fw) : x(fx), y(fy), z(fz), w(fw) {};

	TQuaternion& operator  = ( const TQuaternion& q ) { vec()  = q.vec(); w  = q.w; return *this; }
	TQuaternion& operator += ( const TQuaternion& q ) { vec() += q.vec(); w += q.w; return *this; }
	TQuaternion& operator -= ( const TQuaternion& q ) { vec() -= q.vec(); w -= q.w; return *this; }
	TQuaternion& operator /= ( float f ) { vec() /= f; w /= f; return *this; }
	TQuaternion& operator *= ( float f ) { vec() *= f; w *= f; return *this; }
	TQuaternion& operator *= ( const TQuaternion& q );
	
	const TVector2f&   xy() const { return *((TVector2f*)this); };
	void		identify();
	TQuaternion normal();
	float		norm() const;
	TVector3f&  vec() const;
	void		conjugate(TQuaternion& q) const;
	void		inverse(TQuaternion& q) const;
	void        toEuler( TVector3f& euler ) const;
	TQuaternion& rotate(const TVector3f& axis, float radian);
	TQuaternion& rotate( float radianX, float radianY, float radianZ );
	TQuaternion& rotate( const TVector3f& radian );
};

inline TQuaternion operator + ( const TQuaternion& a, const TQuaternion& b ) { return TQuaternion(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
inline TQuaternion operator - ( const TQuaternion& a, const TQuaternion& b ) { return TQuaternion(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
TQuaternion operator * ( const TQuaternion& a, const TQuaternion& b );
TVector3f   operator * ( const TVector3f& a, const TQuaternion& b );

#endif // TQuaternion_h__
