//
//  TVector3.h
//  me_app
//
//  Created by hahasasa on 3/21/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

#ifndef TVector3_h
#define TVector3_h

#include "MNMemory.h"

class TVector2;
class TVector3 : MNMemory
{
public:
    static const TVector3 axisX;
    static const TVector3 axisY;
    static const TVector3 axisZ;
    static const TVector3 zero;
    static const TVector3 one;
public:
    
    float x,y,z;
    
    TVector3();;
    TVector3( const TVector3& copy );;
    TVector3(float fx, float fy, float fz);;
    TVector3( const TVector2& fxy, float fz );;
    
    float		length() const;
    float		dot( const TVector3& v ) const;
    TVector3   cross( const TVector3& v ) const;
    TVector3	normal() const;
    TVector3	scale( const TVector3& v ) const;
    const TVector2&   xy() const;;
    
    void        rotateX( float radian );
    void        rotateY( float radian );
    void        rotateZ( float radian );
    
    TVector3  operator - () const { return TVector3(-x, -y, -z); }
    TVector3  operator +( const TVector3& pt ) const { return TVector3( x + pt.x, y + pt.y, z + pt.z ); }
    TVector3  operator -( const TVector3& pt ) const {return TVector3( x - pt.x, y - pt.y, z - pt.z ); }
    TVector3  operator *( float scalar ) const { return TVector3( x * scalar, y * scalar, z * scalar ); }
    TVector3  operator /( float scalar ) const { return TVector3( x / scalar, y / scalar, z / scalar ); }
    
    TVector3& operator += ( const TVector3& a ) { x += a.x; y += a.y; z += a.z; return *this; }
    TVector3& operator -= ( const TVector3& a ) { x -= a.x; y -= a.y; z -= a.z; return *this; }
    TVector3& operator *= ( float f ) { x *= f; y *= f; z *= f; return *this; }
    TVector3& operator /= ( float f ) { x /= f; y /= f; z /= f; return *this; }
    
    bool	  operator == ( const TVector3& a ) const { return ((x==a.x)&&(y==a.y)&&(z==a.z)); };
    bool	  operator != ( const TVector3& a ) const { return !(*this == a);};
    
    TVector3& operator = ( const TVector3& a ) { x = a.x; y = a.y; z = a.z;  return *this; }
    
};

inline TVector3 operator * ( float f, const TVector3& a ) { return TVector3(a.x * f, a.y * f, a.z * f); }

#endif /* TVector3_h */
