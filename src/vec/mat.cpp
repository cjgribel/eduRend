
//
//  2x2, 3x3 & 4x4 Matrix lib
//
//  Carl Johan Gribel 2016-2021, cjgribel@gmail.com
//

#include "Mat.h"
#include "Vec.h"

namespace linalg
{
    template <class T>
    Vec2<T> Mat2<T>::operator*(const Vec2<T>& rhs) const
    {
        return Vec2<T>(m11*rhs.x + m12*rhs.y, m21*rhs.x + m22*rhs.y);
    }
    // explicit template specialisation for <float>
    template Vec2<float> Mat2<float>::operator*(const Vec2<float>& rhs) const;
    
    template <class T>
    void Mat3<T>::Normalize()
    {
        Vec3<T> r2 = Vec3<T>(m12, m22, m23), r3 = Vec3<T>(m13, m23, m33);
        r3.Normalize();
        Vec3<T> r1 = r2 % r3;
        r1.Normalize();
        r2 = r3 % r1;
        m11 = r1.x; m12 = r2.x; m13 = r3.x;
        m21 = r1.y; m22 = r2.y; m23 = r3.y;
        m31 = r1.z; m32 = r2.z; m33 = r3.z;
    }
    // explicit template specialisation for <float>
    template void Mat3<float>::Normalize();
    
    template <class T>
    Vec3<T> Mat3<T>::operator*(const Vec3<T> &v) const
    {
        return col[0]*v.x + col[1]*v.y + col[2]*v.z;
    }
    // explicit template specialisation for <float>
    template Vec3<float> Mat3<float>::operator*(const Vec3<float> &v) const;
    
    template <class T>
    Vec4<T> Mat4<T>::operator *(const Vec4<T> &v) const
    {
        return col[0]*v.x + col[1]*v.y + col[2]*v.z + col[3]*v.w;
    }
    // explicit template specialisation for <float>
    template Vec4<float> Mat4<float>::operator *(const Vec4<float> &v) const;
}