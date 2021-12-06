
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
    vec4<T> vec3<T>::xyz0() const
    {
        return vec4<T>(x, y, z, 0.0);
    }
    // explicit template specialisation for <float>
    template vec4<float> vec3<float>::xyz0() const;
    
    template <class T>
    vec4<T> vec3<T>::xyz1() const
    {
        return vec4<T>(x, y, z, 1.0);
    }
    // explicit template specialisation for <float>
    template vec4<float> vec3<float>::xyz1() const;
    
    //
    // row vector * matrix = row vector
    //
    template <class T>
    vec3<T> vec3<T>::operator *(const mat3<T> &m) const
    {
        return vec3<T>(x*m.m11 + y*m.m21 + z*m.m31,
                       x*m.m12 + y*m.m22 + z*m.m32,
                       x*m.m13 + y*m.m23 + z*m.m33);
    }
    // explicit template specialisation for <float>
    template vec3<float> vec3<float>::operator *(const mat3<float> &m) const;
    
    //
    //                | a |             | ad ae af |
    // outer product: | b | | d e f | = | bd be bf |
    //                | c |             | cd ce cf |
    //
    template <class T>
    mat3<T> vec3<T>::outer_product(const vec3<T> &v) const
    {
        return mat3<T>(*this * v.x, *this * v.y, *this * v.z);
    }
    // explicit template specialisation for <float>
    template mat3<float> vec3<float>::outer_product(const vec3<float> &v) const;
    
}
