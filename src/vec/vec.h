
//
//	2D, 3D & 4D Vector lib
//
//  Carl Johan Gribel 2016-2021, cjgribel@gmail.com
//

#pragma once
#ifndef VEC_H
#define VEC_H

#include <cmath>
#include <cstdio>
#include <ostream>

namespace linalg
{
    //
    // 2D vector
    //
    template<class T> class vec2
    {
    public:
        union
        {
            T vec[2];
            struct { T x, y; };
        };
        
        vec2()
        {
            x = 0.0f;
            y = 0.0f;
        }
        
        vec2(const T& x, const T& y)
        {
            this->x = x;
            this->y = y;
        }
        
        void set(const T &x, const T &y)
        {
            this->x = x;
            this->y = y;
        }
        
        float dot(const vec2<T> &u) const
        {
            return x*u.x + y*u.y;
        }
        
        //
        // 2-norm: |u| = sqrt(u.u)
        //
        float norm2()
        {
            return sqrt(x*x + y*y);
        }
        
        //
        // normalization: u/|u| = u/(u.u)
        //
        vec2<T>& normalize()
        {
            T normSquared = x*x + y*y;
            
            if( normSquared < 1e-8 )
                set(0.0, 0.0);
            else
            {
                T inormSquared = 1.0 / sqrt(normSquared);
                set(x * inormSquared, y * inormSquared);
            }
            return *this;
        }
        
        //
        // project on v: v * u.v/v.v
        //
        vec2<T> project(vec2<T> &v) const
        {
            T vnormSquared = v.x*v.x + v.y*v.y;
            return v * (this->dot(v) / vnormSquared);
        }
        
        //
        // angle to a second vector
        //
        float angle(vec2<T> &v)
        {
            vec2<T>	un = vec2f(*this).normalize(),
            vn = vec2f(v).normalize();
            return acos( un.dot(vn) );
        }
        
        vec2<T>& operator =(const vec2<T> &v)
        {
            x = v.x;
            y = v.y;
            return *this;
        }
        
        vec2<T>& operator +=(const vec2<T> &v)
        {
            x += v.x;
            y += v.y;
            return *this;
        }
        
        vec2<T>& operator -=(const vec2<T> &v)
        {
            x -= v.x;
            y -= v.y;
            return *this;
        }
        
        vec2<T>& operator *=(const T &s)
        {
            x *= s;
            y *= s;
            return *this;
        }
        
        vec2<T>& operator *=(const vec2<T> &v)
        {
            x *= v.x;
            y *= v.y;
            return *this;
        }
        
        vec2<T>& operator /=(const T &v)
        {
            x /= v;
            y /= v;
            return *this;
        }
        
        vec2<T> operator -() const
        {
            return vec2<T>(-x, -y);
        }
        
        vec2<T> operator *(const T &s) const
        {
            return vec2<T>(x * s, y * s);
        }

        vec2<T> operator *(const vec2<T> &v) const
        {
            return vec2<T>(x * v.x, y * v.y);
        }
        
        vec2<T> operator /(const T &v) const
        {
            T iv = 1.0 / v;
            return vec2(x * iv, y * iv);
        }
        
        vec2<T> operator +(const vec2<T> &v) const
        {
            return vec2<T>(x + v.x, y + v.y);
        }
        
        vec2<T> operator -(const vec2<T> &v) const
        {
            return vec2<T>(x - v.x, y - v.y);
        }
        
        T operator %(const vec2<T> &v) const
        {
            return x * v.y - y * v.x;
        }
        
    };
    
    template<class T>
    inline std::ostream& operator<< (std::ostream &out, const vec2<T> &v)
    {
        return out << "(" << v.x << ", " << v.y << ")";
    }
    
    template<class T> class vec4;
    template<class T> class mat3;
    
    //
    // 3D vector
    //
    template<class T> class vec3
    {
    public:
        union
        {
            T vec[3];
            struct { T x, y, z; };
        };
        
        vec3()
        {
            x = 0.0;
            y = 0.0;
            z = 0.0;
        }
        
        vec3(const T &x, const T &y, const T &z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
        }
        
        vec4<T> xyz0() const;
        
        vec4<T> xyz1() const;
        
        void set(const T &x, const T &y, const T &z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
        }
        
        T dot(const vec3<T> &u) const
        {
            return x*u.x + y*u.y + z*u.z;
        }
        
        //
        // vector length (2-norm): |u| = sqrt(u.u)
        //
        T norm2() const
        {
            return sqrt(x*x + y*y + z*z);
        }
        
        T norm2squared() const
        {
            return x*x + y*y + z*z;
        }
        
        //
        // normalization: u/|u| = u/(u.u)
		// divide-by-zero safe
        //
        vec3<T>& normalize()
        {
            T normSquared = x*x + y*y + z*z;
            
            if( normSquared < 1e-8 )
                set(0.0, 0.0, 0.0);
            else
            {
                float inormSquared = (T)(1.0 / sqrt(normSquared));
                set(x*inormSquared, y*inormSquared, z*inormSquared);
            }
            return *this;
        }
        
        //
        // project on v: v * u.v/v.v
        //
        vec3<T> project(const vec3<T> &v) const
        {
            T vnormSquared = v.x*v.x + v.y*v.y + v.z*v.z;
            return v * (this->dot(v) / vnormSquared);
        }
        
        //
        // angle to vector
        //
        T angle(vec3<T> &v) const
        {
            vec3<T>	un = vec3<T>(*this).normalize(),
            vn = vec3<T>(v).normalize();
            return acos( un.dot(vn) );
        }
        
        vec3<T>& operator +=(const vec3<T> &v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
        }
        
        vec3<T>& operator -=(const vec3<T> &v)
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            return *this;
        }
        
        vec3<T>& operator *=(const T &s)
        {
            x *= s;
            y *= s;
            z *= s;
            return *this;
        }
        
        vec3<T>& operator *=(const vec3<T> &v)
        {
            x *= v.x;
            y *= v.y;
            z *= v.z;
            return *this;
        }
        
        vec3<T>& operator /=(const T &v)
        {
            x /= v;
            y /= v;
            z /= v;
            return *this;
        }
        
        vec3<T> operator -() const
        {
            return vec3<T>(-x, -y, -z);
        }
        
        vec3<T> operator *(const T& s) const
        {
            return vec3(x*s, y*s, z*s);
        }
        
        vec3<T> operator *(const vec3<T>& v) const
        {
            return vec3<T>(x*v.x, y*v.y, z*v.z);
        }
        
        vec3<T> operator /(const T& s) const
        {
            T is = 1.0 / s;
            return vec3<T>(x*is, y*is, z*is);
        }
        
        vec3<T> operator +(const vec3<T>& v) const
        {
            return vec3<T>(x+v.x, y+v.y, z+v.z);
        }
        
        vec3<T> operator -(const vec3<T>& v) const
        {
            return vec3<T>(x-v.x, y-v.y, z-v.z);
        }
        
        vec3<T> operator %(const vec3<T>& v) const
        {
            return vec3<T>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
        }
        
        vec3<T> operator *(const mat3<T>& m) const;
        
        bool operator == (const vec3<T>& rhs) const
        {
            return x == rhs.x && y == rhs.y && z == rhs.z;
        }
        
        mat3<T> outer_product(const vec3<T>& v) const;
        
        void debugPrint() const
        {
            printf("(%f,%f,%f)\n", x, y, z);
        }
    };
    
    template<class T>
    inline std::ostream& operator<< (std::ostream &out, const vec3<T> &v)
    {
        return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    }
    
    //
    // 4D vector
    //
    template<class T> class vec4
    {
    public:
        union
        {
            T vec[4];
            struct { T x, y, z, w; };
        };
        
        vec4()
        {
            x = y = z = w = 0;
        }
        
        vec4(const T &x, const T &y, const T &z, const T &w)
        {
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = w;
        }
        
        vec4(const vec3<T> &v, const T &w)
        {
            this->x = v.x;
            this->y = v.y;
            this->z = v.z;
            this->w = w;
        }
        
        void set(const T &x, const T &y, const T &z, const T &w){
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = w;
        }
        
        vec2<T> xy() const
        {
            return vec2<T>(x, y);
        }
        
        vec3<T> xyz() const
        {
            return vec3<T>(x, y, z);
        }
        
        vec4<T> operator +(const vec4<T> &v) const
        {
            return vec4<T>(x+v.x, y+v.y, z+v.z, w+v.w);
        }
        
        vec4<T>& operator += (const vec4<T>& v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            w += v.w;
            
            return *this;
        }
        
        vec4<T> operator -(const vec4<T> &v) const
        {
            return vec4<T>(x-v.x, y-v.y, z-v.z, w-v.w);
        }
        
        vec4<T> operator *(const T &s) const
        {
            return vec4<T>(x*s, y*s, z*s, w*s);
        }
        
        bool operator == (const vec4<T>& rhs) const;
    };
    
    template<>
    inline bool vec4<unsigned>::operator == (const vec4<unsigned>& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    }
    
    template<class T>
    inline T dot(const vec3<T>& u, const vec3<T>& v)
    {
        return u.x*v.x + u.y*v.y + u.z*v.z;
    }
    
    template<class T>
    inline T dot(const vec4<T>& u, const vec4<T>& v)
    {
        return u.x*v.x + u.y*v.y + u.z*v.z + u.w*v.w;
    }
    
    template<class T>
    inline vec3<T> normalize(const vec3<T>& u)
    {
        T norm2 = u.x*u.x + u.y*u.y + u.z*u.z;
        
        if( norm2 < 1.0e-8 )
            return vec3<T>(0.0, 0.0, 0.0);
        else
            return u * (T)(1.0/sqrt(norm2));
    }
    
    template<class T>
    inline vec4<T> normalize(const vec4<T>& u)
    {
        T norm2 = u.x*u.x + u.y*u.y + u.z*u.z + u.w*u.w;
        
        if( norm2 < 1.0e-8 )
            return vec4<T>(0.0, 0.0, 0.0, 0.0);
        else
            return u * (1.0/sqrt(norm2));
    }
    
    template<class T>
    inline std::ostream& operator << (std::ostream &out, const vec4<T> &v)
    {
        return out << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    }
    
    typedef vec2<float> float2;
    typedef vec3<float> float3;
    typedef vec4<float> float4;
    
    typedef vec2<float> vec2f;
    typedef vec3<float> vec3f;
    typedef vec4<float> vec4f;
    
    typedef vec2<int> int2;
    typedef vec3<int> int3;
    typedef vec4<int> int4;
    
    typedef vec2<int> vec2i;
    typedef vec3<int> vec3i;
    typedef vec4<int> vec4i;
    
    typedef vec2<long> long2;
    typedef vec3<long> long3;
    typedef vec4<long> long4;
    
    typedef vec2<unsigned> unsigned2;
    typedef vec3<unsigned> unsigned3;
    typedef vec4<unsigned> unsigned4;
    typedef vec2<unsigned> vec2ui;
    typedef vec3<unsigned> vec3ui;
    typedef vec4<unsigned> vec4ui;
    
   
    //
    // compile-time instances
    //
    const vec2f vec2f_zero = vec2f(0, 0);
    const vec3f vec3f_zero = vec3f(0, 0, 0);
    const vec4f vec4f_zero = vec4f(0, 0, 0, 0);
}

#endif /* VEC_H */
