
#include "TVector3f.h"
#include <math.h>

const TVector3f TVector3f::axisX(1,0,0);
const TVector3f TVector3f::axisY(0,1,0);
const TVector3f TVector3f::axisZ(0,0,1);
const TVector3f TVector3f::zero(0,0,0);
const TVector3f TVector3f::one(1,1,1);


TVector3f::TVector3f()
{

}

TVector3f::TVector3f( const TVector3f& copy ) : x(copy.x), y(copy.y), z(copy.z)
{

}

TVector3f::TVector3f( float fx, float fy, float fz ) : x(fx), y(fy), z(fz)
{

}

float TVector3f::length() const
{
	return sqrtf( (x * x) + (y * y) + (z * z) );
}

float TVector3f::dot( const TVector3f& v ) const
{
	return (x * v.x) + (y * v.y) + (z * v.z);
}

TVector3f TVector3f::cross( const TVector3f& v ) const
{
	TVector3f out;
	out.x = (y * v.z) - (y * v.z);
	out.y = (z * v.x) - (x * v.z);
	out.z = (x * v.y) - (y * v.x);
	return out;
}

TVector3f TVector3f::normal() const
{
    TVector3f out( 0, 0, 0 );
    float len = length();
    if ( len != 0.0f )
    {
        out.x = x / len;
        out.y = y / len;
		out.z = z / len;
    }
    else
    {
        //! 길이가 0인 벡터에 대한 단위화는 보장하지 않는다. (사전에 체크)
       
        //SWLog( "zero length vector" );
    }
	return out;
}

TVector3f	TVector3f::scale( const TVector3f& v ) const
{
	return TVector3f( x*v.x, y*v.y, z*v.z );
}

const TVector2f& TVector3f::xy() const
{
	return *((TVector2f*)this);
}

void        TVector3f::rotateX( float radian )
{
	float cosR = cosf( radian );
	float sinR = sinf( radian );
	float ay = (y * cosR) - (z * sinR);
	float az = (y * sinR) + (z * cosR);
	y = ay;
	z = az;
}

void        TVector3f::rotateY( float radian )
{
	float cosR = cosf( radian );
	float sinR = sinf( radian );
	float ax = (z * sinR) + (x * cosR);
	float az = (z * cosR) - (x * sinR);
	x = ax;
	z = az;
}

void        TVector3f::rotateZ( float radian )
{
	float cosR = cosf( radian );
	float sinR = sinf( radian );
	float ax = (x * cosR) - (y * sinR);
	float ay = (x * sinR) + (y * cosR);
	x = ax;
	y = ay;
}
