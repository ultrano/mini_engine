//
//  TMatrix4x4.cpp
//  me_app
//
//  Created by hahasasa on 3/21/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

#include "TMatrix4x4.h"
#include <string.h>
#include <math.h>

const TMatrix4x4 TMatrix4x4::unit;

TMatrix4x4::TMatrix4x4() : m11(1), m12(0), m13(0), m14(0)
, m21(0), m22(1), m23(0), m24(0)
, m31(0), m32(0), m33(1), m34(0)
, m41(0), m42(0), m43(0), m44(1)
{
    
}

TMatrix4x4& TMatrix4x4::operator=( const TMatrix4x4& mat )
{
    memcpy( this, &mat, sizeof(TMatrix4x4) );
    return *this;
}

TMatrix4x4& TMatrix4x4::operator*=( float f )
{
    for ( int i = 0 ; i < 16 ; ++i ) ((float*)this)[i] *= f;
    
    return *this;
}

TMatrix4x4& TMatrix4x4::operator*=( const TMatrix4x4& mat )
{
    float t1,t2,t3,t4;
    
    for ( int r = 0 ; r < 4 ; ++r )
    {
        t1 = m[r][0]; t2 = m[r][1]; t3 = m[r][2]; t4 = m[r][3];
        m[r][0] = (t1*mat.m11) + (t2*mat.m21) + (t3*mat.m31) + (t4*mat.m41);
        m[r][1] = (t1*mat.m12) + (t2*mat.m22) + (t3*mat.m32) + (t4*mat.m42);
        m[r][2] = (t1*mat.m13) + (t2*mat.m23) + (t3*mat.m33) + (t4*mat.m43);
        m[r][3] = (t1*mat.m14) + (t2*mat.m24) + (t3*mat.m34) + (t4*mat.m44);
    }
    
    return *this;
}

const TMatrix4x4 operator*( const TMatrix4x4& a, const TMatrix4x4& b )
{
    TMatrix4x4 ret;
    float t1,t2,t3,t4;
    
    for ( int r = 0 ; r < 4 ; ++r )
    {
        t1 = a.m[r][0]; t2 = a.m[r][1]; t3 = a.m[r][2]; t4 = a.m[r][3];
        ret.m[r][0] = (t1*b.m11) + (t2*b.m21) + (t3*b.m31) + (t4*b.m41);
        ret.m[r][1] = (t1*b.m12) + (t2*b.m22) + (t3*b.m32) + (t4*b.m42);
        ret.m[r][2] = (t1*b.m13) + (t2*b.m23) + (t3*b.m33) + (t4*b.m43);
        ret.m[r][3] = (t1*b.m14) + (t2*b.m24) + (t3*b.m34) + (t4*b.m44);
    }
    return ret;
}

const TVector3 operator*( const TVector3& v, const TMatrix4x4& m )
{
    float w = (v.x*m.m14 + v.y*m.m24 + v.z*m.m34) + m.m44;
    if ( w == 0 ) w = __FLT_EPSILON__;
    return TVector3
    ( (v.x*m.m11 + v.y*m.m21 + v.z*m.m31 + m.m41)/w
     , (v.x*m.m12 + v.y*m.m22 + v.z*m.m32 + m.m42)/w
     , (v.x*m.m13 + v.y*m.m23 + v.z*m.m33 + m.m43)/w );
}

const TQuaternion operator*( const TQuaternion& v, const TMatrix4x4& m )
{
    return TQuaternion
    ( (v.x*m.m11 + v.y*m.m21 + v.z*m.m31 + v.w*m.m41)
     , (v.x*m.m12 + v.y*m.m22 + v.z*m.m32 + v.w*m.m42)
     , (v.x*m.m13 + v.y*m.m23 + v.z*m.m33 + v.w*m.m43)
     , (v.x*m.m14 + v.y*m.m24 + v.z*m.m34 + v.w*m.m44));
}

float matDet44( unsigned square, float* m )
{
    if ( square == 1 ) return *m;
    float ret = 0;
    for ( unsigned i = 0 ; i < square ; ++i )
    {
        int k = 0;
        float sub[9] = {0};
        for ( unsigned r = 1 ; r < square ; ++r )
        {
            for ( unsigned c = 0 ; c < square ; ++c )
            {
                if ( i == c ) continue;
                sub[k++] = (m[(r*square) + c]);
            }
        }
        float cof = ((i%2)? -1:+1) * matDet44(square-1, &sub[0]);
        ret += m[i] * cof;
    }
    return ret;
}

void TMatrix4x4::identity()
{
    m11=(1); m12=(0); m13=(0); m14=(0);
    m21=(0); m22=(1); m23=(0); m24=(0);
    m31=(0); m32=(0); m33=(1); m34=(0);
    m41=(0); m42=(0); m43=(0); m44=(1);
}

float TMatrix4x4::determinant() const
{
    return matDet44(4, (float*)this);
}

float TMatrix4x4::minorDet( unsigned char row, unsigned char col ) const
{
    int i = 0;
    float minor[3 * 3] = {0};
    for ( int r = 0 ; r < 4 ; ++r )
    {
        if ( r == row ) continue;
        for ( int c = 0 ; c < 4 ; ++c )
        {
            if ( c == col ) continue;
            minor[i++] = ((float*)this)[(r*4) + c];
        }
    }
    return matDet44(3,minor);
}

void TMatrix4x4::inverse( TMatrix4x4& m ) const
{
    adjoint(m);
    float det = determinant();
    if ( det != 0.0f ) m *= 1.0f/det;
}

void TMatrix4x4::adjoint( TMatrix4x4& m ) const
{
    for ( int r = 0 ; r < 4 ; ++r )
    {
        for ( int c = 0 ; c < 4 ; ++c )
        {
            m.m[c][r] = (((r+c)%2)? -1:+1) * minorDet( r, c );
        }
    }
}

void TMatrix4x4::transpose(TMatrix4x4& t) const
{
    for ( int r = 0 ; r < 4 ; ++r )
    {
        for ( int c = 0 ; c < r ; ++c )
        {
            if( c == r ) continue;
            
            float f = m[r][c];
            t.m[r][c] = m[c][r];
            t.m[c][r] = f;
        }
    }
}

TMatrix4x4& TMatrix4x4::transform( const TVector3& scale, const TQuaternion& rot, const TVector3& trans )
{
    m11 = (1- 2*(rot.y*rot.y + rot.z*rot.z))*scale.x;	m12 = (2*(rot.x*rot.y + rot.w*rot.z))*scale.x;		m13 = (2*(rot.x*rot.z - rot.w*rot.y))*scale.x;
    m21 = (2*(rot.x*rot.y - rot.w*rot.z))*scale.y;		m22 = (1- 2*(rot.z*rot.z + rot.x*rot.x))*scale.y;	m23 = (2*(rot.y*rot.z + rot.w*rot.x))*scale.y;
    m31 = (2*(rot.x*rot.z + rot.w*rot.y))*scale.z;		m32 = (2*(rot.y*rot.z - rot.w*rot.x))*scale.z;		m33 = (1- 2*(rot.x*rot.x + rot.y*rot.y))*scale.z;
    
    m14 = 0;
    m24 = 0;
    m34 = 0;
    m44 = 1;
    
    m41 = trans.x;  m42 = trans.y;  m43 = trans.z;
    
    return *this;
}

TMatrix4x4& TMatrix4x4::ortho( float left, float right, float bottom, float top, float near, float far )
{
    identity();
    
    float w = right - left;
    float h = top - bottom;
    float d = far - near;
    
    float x = (right + left)/2.0f;
    float y = (bottom + top)/2.0f;
    float z = (near + far)/2.0f;
    
    m11 = 2.0f/w;
    m22 = 2.0f/h;
    m33 = 2.0f/d;
    m41 = -(2*x)/w;
    m42 = -(2*y)/h;
    m43 = -(2*z)/d;
    
    return *this;
}

TMatrix4x4& TMatrix4x4::perspective( float fov, float aspect, float near, float far )
{
    identity();
    
    float cot = 1.0f/tanf( fov / 2.0f );
    float cotW = (cot) * aspect;
    float cotH = (cot);
    float d = far - near;
    
    m11 = cotW;
    m22 = cotH;
    m33 = (far+near)/d;
    m34 = 1;
    m43 = -(2*far*near)/d;
    m44 = 0;
    
    return *this;
}

TQuaternion TMatrix4x4::row( unsigned char r ) const
{
    if ( r < 4 ) return TQuaternion( m[r][0], m[r][1], m[r][2], m[r][3] );
    return TQuaternion();
}

TQuaternion TMatrix4x4::col( unsigned char c ) const
{
    if ( c < 4 ) return TQuaternion( m[0][c], m[1][c], m[2][c], m[3][c] );
    return TQuaternion();
}