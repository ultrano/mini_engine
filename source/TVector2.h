//
//  TVector2.h
//  me_app
//
//  Created by hahasasa on 3/21/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

#ifndef TVector2_h
#define TVector2_h

#include "MNMemory.h"

class TVector3;
// 2D 공간 좌표 Class
class TVector2 : public MNMemory
{
public:
    static const TVector2 axisX;
    static const TVector2 axisY;
    static const TVector2 zero;
    static const TVector2 one;
public:
    
    //! 2D 공간 좌표
    float x, y;
    
    //! 생성자
    TVector2();
    TVector2( float kx, float ky );
    TVector2( const TVector3& vec3 );
    
    //! length from origin
    float length() const;
    
    //! distance with other
    float dist( float kx, float ky ) const;
    float dist( const TVector2& pt ) const;
    
    //! dot product
    float dot( float kx, float ky ) const;
    float dot( const TVector2& pt ) const;
    
    //! cross product
    float cross( const TVector2& pt ) const;
    float cross( float kx, float ky ) const;
    TVector2 cross( float kz ) const;
    
    //! 단위화 값 (길이가 0인 벡터에 대한 단위화는 보장하지 않는다. (사전에 체크))
    TVector2 normal() const;
    
    //! multiplies two vectors component-wise
    TVector2 scale( float kx, float ky ) const;
    TVector2 scale( const TVector2& pt ) const;
    
    //! z-axis rotate
    void rotateZ( float radian );
    
    //! 사칙 & 논리 연산 오버로드.
    TVector2  operator -() const { return TVector2( -x, -y ); }
    TVector2  operator +( const TVector2& pt ) const { return TVector2( x + pt.x, y + pt.y ); }
    TVector2  operator -( const TVector2& pt ) const {return TVector2( x - pt.x, y - pt.y ); }
    TVector2  operator *( float scalar ) const { return TVector2( x * scalar, y * scalar ); }
    TVector2  operator /( float scalar ) const { return TVector2( x / scalar, y / scalar ); }
    
    TVector2& operator +=( const TVector2& pt ) { x += pt.x; y += pt.y; return *this; }
    TVector2& operator -=( const TVector2& pt ) { x -= pt.x; y -= pt.y; return *this; }
    TVector2& operator *= ( float f ) { x *= f; y *= f; return *this; }
    TVector2& operator /= ( float f ) { x /= f; y /= f; return *this; }
    
    bool        operator ==( const TVector2& pt ) const { return ( ( x == pt.x ) && ( y == pt.y ) ); }
    bool        operator !=( const TVector2& pt ) const { return ( ( x != pt.x ) && ( y != pt.y ) ); }
    
    TVector2& operator = ( const TVector2& a ) { x = a.x; y = a.y; return *this; }
    
};

inline TVector2 operator * ( float f, const TVector2& a ) { return TVector2( a.x * f, a.y * f ); }

#endif /* TVector2_h */
