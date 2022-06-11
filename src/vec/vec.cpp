
//
//	2D, 3D & 4D Vector lib
//
//  Carl Johan Gribel 2016-2021, cjgribel@gmail.com
//

#include "vec.h"
#include "mat.h"

namespace linalg
{
    
    template <class T>
    Vec4<T> Vec3<T>::XYZ0() const
    {
        return Vec4<T>(x, y, z, 0.0);
    }
    // explicit template specialisation for <float>
    template Vec4<float> Vec3<float>::XYZ0() const;
    
    template <class T>
    Vec4<T> Vec3<T>::XYZ1() const
    {
        return Vec4<T>(x, y, z, 1.0);
    }
    // explicit template specialisation for <float>
    template Vec4<float> Vec3<float>::XYZ1() const;
    
    //
    // row vector * matrix = row vector
    //
    template <class T>
    Vec3<T> Vec3<T>::operator *(const Mat3<T> &m) const
    {
        return Vec3<T>(x*m.m11 + y*m.m21 + z*m.m31,
                       x*m.m12 + y*m.m22 + z*m.m32,
                       x*m.m13 + y*m.m23 + z*m.m33);
    }
    // explicit template specialisation for <float>
    template Vec3<float> Vec3<float>::operator *(const Mat3<float> &m) const;
    
    //
    //                | a |             | ad ae af |
    // outer product: | b | | d e f | = | bd be bf |
    //                | c |             | cd ce cf |
    //
    template <class T>
    Mat3<T> Vec3<T>::OuterProduct(const Vec3<T> &v) const
    {
        return Mat3<T>(*this * v.x, *this * v.y, *this * v.z);
    }
    // explicit template specialisation for <float>
    template Mat3<float> Vec3<float>::OuterProduct(const Vec3<float> &v) const;
    
}
