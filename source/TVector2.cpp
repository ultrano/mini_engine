//
//  TVector2.cpp
//  me_app
//
//  Created by hahasasa on 3/21/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//


#include "TVector2.h"
#include "TVector3.h"
#include <math.h>

const TVector2 TVector2::axisX(1,0);
const TVector2 TVector2::axisY(0,1);
const TVector2 TVector2::zero(0,0);
const TVector2 TVector2::one(1,1);

TVector2::TVector2()
{
    
}

TVector2::TVector2( float kx, float ky )
: x( kx )
, y( ky )
{
    
}

TVector2::TVector2( const TVector3& vec3 )
: x( vec3.x )
, y( vec3.y )
{
    
}

float TVector2::length() const
{
    return sqrtf( x*x + y*y );
    // or
    // return distWith( 0, 0 );
}

float TVector2::dist( float kx, float ky ) const
{
    kx = x - kx;
    ky = y - ky;
    return sqrtf( ( kx * kx ) + ( ky * ky ) );
}

float TVector2::dist( const TVector2 &pt ) const
{
    return dist( pt.x, pt.y );
}

float TVector2::dot( float kx, float ky ) const
{
    return ( x*kx + y*ky );
}

float TVector2::dot( const TVector2 &pt ) const
{
    return dot( pt.x, pt.y );
}

float TVector2::cross( float kx, float ky ) const
{
    return ( x*ky - y*kx );
}

float TVector2::cross( const TVector2 &pt ) const
{
    return cross( pt.x, pt.y );
}

TVector2 TVector2::cross( float kz ) const
{
    return TVector2( y*kz, -x*kz );
}

TVector2   TVector2::normal() const
{
    TVector2 pt( 0, 0 );
    float len = length();
    if ( len != 0.0f )
    {
        pt.x = x / len;
        pt.y = y / len;
    }
    else
    {
        //! 길이가 0인 벡터에 대한 단위화는 보장하지 않는다. (사전에 체크)
        
        //SWLog( "zero length vector" );
    }
    return pt;
}

TVector2 TVector2::scale( float kx, float ky ) const
{
    return TVector2( x*kx, y*ky );
}

TVector2 TVector2::scale( const TVector2& pt ) const
{
    return TVector2( x*pt.x, y*pt.y );
}

void TVector2::rotateZ( float radian )
{
    float cosR = cosf( radian );
    float sinR = sinf( radian );
    float ax = (x * cosR) - (y * sinR);
    float ay = (x * sinR) + (y * cosR);
    x = ax;
    y = ay;
}