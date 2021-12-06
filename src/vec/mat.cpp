
//
//  2x2, 3x3 & 4x4 Matrix lib
//
//  Carl Johan Gribel 2016-2021, cjgribel@gmail.com
//

#include "mat.h"
#include "vec.h"

namespace linalg
{
    template <class T>
    vec2<T> mat2<T>::operator*(const vec2<T>& rhs) const
    {
        return vec2<T>(m11*rhs.x + m12*rhs.y, m21*rhs.x + m22*rhs.y);
    }
    // explicit template specialisation for <float>
    template vec2<float> mat2<float>::operator*(const vec2<float>& rhs) const;
    
    template <class T>
    void mat3<T>::normalize()
    {
        vec3<T> r2 = vec3<T>(m12, m22, m23), r3 = vec3<T>(m13, m23, m33);
        r3.normalize();
        vec3<T> r1 = r2 % r3;
        r1.normalize();
        r2 = r3 % r1;
        m11 = r1.x; m12 = r2.x; m13 = r3.x;
        m21 = r1.y; m22 = r2.y; m23 = r3.y;
        m31 = r1.z; m32 = r2.z; m33 = r3.z;
    }
    // explicit template specialisation for <float>
    template void mat3<float>::normalize();
    
    template <class T>
    vec3<T> mat3<T>::operator*(const vec3<T> &v) const
    {
        return col[0]*v.x + col[1]*v.y + col[2]*v.z;
    }
    // explicit template specialisation for <float>
    template vec3<float> mat3<float>::operator*(const vec3<float> &v) const;
    
    template <class T>
    vec4<T> mat4<T>::operator *(const vec4<T> &v) const
    {
        return col[0]*v.x + col[1]*v.y + col[2]*v.z + col[3]*v.w;
    }
    // explicit template specialisation for <float>
    template vec4<float> mat4<float>::operator *(const vec4<float> &v) const;
}