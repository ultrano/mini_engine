//
//  TVector3.cpp
//  me_app
//
//  Created by hahasasa on 3/21/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

#include "TVector3.h"
#include "TVector2.h"
#include <math.h>

const TVector3 TVector3::axisX(1,0,0);
const TVector3 TVector3::axisY(0,1,0);
const TVector3 TVector3::axisZ(0,0,1);
const TVector3 TVector3::zero(0,0,0);
const TVector3 TVector3::one(1,1,1);


TVector3::TVector3()
{
    
}

TVector3::TVector3( const TVector3& copy ) : x(copy.x), y(copy.y), z(copy.z)
{
    
}

TVector3::TVector3( float fx, float fy, float fz ) : x(fx), y(fy), z(fz)
{
    
}

TVector3::TVector3( const TVector2& fxy, float fz ) : x(fxy.x), y(fxy.y), z(fz)
{
    
}

float TVector3::length() const
{
    return sqrtf( (x * x) + (y * y) + (z * z) );
}

float TVector3::dot( const TVector3& v ) const
{
    return (x * v.x) + (y * v.y) + (z * v.z);
}

TVector3 TVector3::cross( const TVector3& v ) const
{
    TVector3 out;
    out.x = (y * v.z) - (y * v.z);
    out.y = (z * v.x) - (x * v.z);
    out.z = (x * v.y) - (y * v.x);
    return out;
}

TVector3 TVector3::normal() const
{
    TVector3 out( 0, 0, 0 );
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

TVector3	TVector3::scale( const TVector3& v ) const
{
    return TVector3( x*v.x, y*v.y, z*v.z );
}

const TVector2& TVector3::xy() const
{
    return *((TVector2*)this);
}

void        TVector3::rotateX( float radian )
{
    float cosR = cosf( radian );
    float sinR = sinf( radian );
    float ay = (y * cosR) - (z * sinR);
    float az = (y * sinR) + (z * cosR);
    y = ay;
    z = az;
}

void        TVector3::rotateY( float radian )
{
    float cosR = cosf( radian );
    float sinR = sinf( radian );
    float ax = (z * sinR) + (x * cosR);
    float az = (z * cosR) - (x * sinR);
    x = ax;
    z = az;
}

void        TVector3::rotateZ( float radian )
{
    float cosR = cosf( radian );
    float sinR = sinf( radian );
    float ax = (x * cosR) - (y * sinR);
    float ay = (x * sinR) + (y * cosR);
    x = ax;
    y = ay;
}