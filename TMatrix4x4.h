#ifndef TMatrix4x4_h__
#define TMatrix4x4_h__

#include "TQuaternion.h"

class TMatrix4x4
{
public:

	const static TMatrix4x4 unit;

public:
	union
	{
		struct 
		{
			float m11,m12,m13,m14;
			float m21,m22,m23,m24;
			float m31,m32,m33,m34;
			float m41,m42,m43,m44;
		};
		float m[4][4];
	};

	TMatrix4x4();

	TMatrix4x4& operator = ( const TMatrix4x4& mat );
	TMatrix4x4& operator *= ( float f );
	TMatrix4x4& operator *= ( const TMatrix4x4& mat );

	void		identity();
	float		determinant() const;
	float		minorDet(unsigned char row, unsigned char col) const;
	void		inverse(TMatrix4x4& m) const;
	void		adjoint(TMatrix4x4& m) const;
	void		transpose(TMatrix4x4& t) const;
	
	TMatrix4x4& transform(const TVector3f& scale, const TQuaternion& rotate, const TVector3f& translate );
	TMatrix4x4& ortho( float left, float right, float bottom, float top, float near, float far );
	TMatrix4x4& perspective( float fov, float aspect, float near, float far );
	
	TQuaternion	row(unsigned char r) const;
	TQuaternion	col(unsigned char c) const;


};

const TMatrix4x4 operator * ( const TMatrix4x4& a, const TMatrix4x4& b );
const TVector3f  operator * ( const TVector3f& v, const TMatrix4x4& m );
const TQuaternion operator * ( const TQuaternion& v, const TMatrix4x4& m );


#endif // TMatrix4x4_h__
