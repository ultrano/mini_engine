//
//  TQuaternion.h
//  me_app
//
//  Created by hahasasa on 3/21/16.
//  Copyright © 2016 hahasasa. All rights reserved.
//

#ifndef TQuaternion_h
#define TQuaternion_h

#include "TVector3.h"

class TQuaternion
{
public:
    
    float x, y, z;
    float w;
    
    TQuaternion() : x(0), y(0), z(0), w(1) {};
    TQuaternion( const TQuaternion& copy ): x(copy.x), y(copy.y), z(copy.z), w(copy.w) {};
    TQuaternion( const TVector3& v, float fw) : x(v.x), y(v.y), z(v.z), w(fw) {};
    TQuaternion(float fx, float fy, float fz, float fw) : x(fx), y(fy), z(fz), w(fw) {};
    
    TQuaternion& operator  = ( const TQuaternion& q ) { vec()  = q.vec(); w  = q.w; return *this; }
    TQuaternion& operator += ( const TQuaternion& q ) { vec() += q.vec(); w += q.w; return *this; }
    TQuaternion& operator -= ( const TQuaternion& q ) { vec() -= q.vec(); w -= q.w; return *this; }
    TQuaternion& operator /= ( float f ) { vec() /= f; w /= f; return *this; }
    TQuaternion& operator *= ( float f ) { vec() *= f; w *= f; return *this; }
    TQuaternion& operator *= ( const TQuaternion& q );
    
    const TVector2&   xy() const { return *((TVector2*)this); };
    void		identify();
    TQuaternion normal();
    float		norm() const;
    TVector3&  vec() const;
    void		conjugate(TQuaternion& q) const;
    void		inverse(TQuaternion& q) const;
    void        toEuler( TVector3& euler ) const;
    TQuaternion& rotate(const TVector3& axis, float radian);
    TQuaternion& rotate( float radianX, float radianY, float radianZ );
    TQuaternion& rotate( const TVector3& radian );
};

inline TQuaternion operator + ( const TQuaternion& a, const TQuaternion& b ) { return TQuaternion(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
inline TQuaternion operator - ( const TQuaternion& a, const TQuaternion& b ) { return TQuaternion(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
TQuaternion operator * ( const TQuaternion& a, const TQuaternion& b );
TVector3   operator * ( const TVector3& a, const TQuaternion& b );


#endif /* TQuaternion_h */
