
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
    template<class T> class Vec2
    {
    public:

        union
        {
            T vec[2];
            struct { T x, y; };
        };
        
        Vec2()
        {
            x = 0.0f;
            y = 0.0f;
        }
        
        Vec2(const T& x, const T& y)
        {
            this->x = x;
            this->y = y;
        }
        
        void Set(const T &x, const T &y)
        {
            this->x = x;
            this->y = y;
        }
        
        float Dot(const Vec2<T> &u) const
        {
            return x*u.x + y*u.y;
        }
        
        //
        // length: |u| = sqrt(u.u)
        //
        float Length()
        {
            return sqrt(x*x + y*y);
        }
        
        //
        // normalization: u/|u| = u/(u.u)
        //
        Vec2<T>& Normalize()
        {
            T lengthSquared = x*x + y*y;
            
            if (lengthSquared < 1e-8 )
                Set(0.0, 0.0);
            else
            {
                T inormSquared = 1.0 / sqrt(lengthSquared);
                Set(x * inormSquared, y * inormSquared);
            }
            return *this;
        }
        
        //
        // project on v: v * u.v/v.v
        //
        Vec2<T> Project(Vec2<T> &v) const
        {
            T vLengthSquared = v.x*v.x + v.y*v.y;
            return v * (this->Dot(v) / vLengthSquared);
        }
        
        //
        // angle to a second vector
        //
        float Angle(Vec2<T> &v)
        {
            Vec2<T>	un = vec2f(*this).Normalize(),
            vn = vec2f(v).normalize();
            return acos( un.Dot(vn) );
        }
        
        Vec2<T>& operator =(const Vec2<T> &v)
        {
            x = v.x;
            y = v.y;
            return *this;
        }
        
        Vec2<T>& operator +=(const Vec2<T> &v)
        {
            x += v.x;
            y += v.y;
            return *this;
        }
        
        Vec2<T>& operator -=(const Vec2<T> &v)
        {
            x -= v.x;
            y -= v.y;
            return *this;
        }
        
        Vec2<T>& operator *=(const T &s)
        {
            x *= s;
            y *= s;
            return *this;
        }
        
        Vec2<T>& operator *=(const Vec2<T> &v)
        {
            x *= v.x;
            y *= v.y;
            return *this;
        }
        
        Vec2<T>& operator /=(const T &v)
        {
            x /= v;
            y /= v;
            return *this;
        }
        
        Vec2<T> operator -() const
        {
            return Vec2<T>(-x, -y);
        }
        
        Vec2<T> operator *(const T &s) const
        {
            return Vec2<T>(x * s, y * s);
        }

        Vec2<T> operator *(const Vec2<T> &v) const
        {
            return Vec2<T>(x * v.x, y * v.y);
        }
        
        Vec2<T> operator /(const T &v) const
        {
            T iv = 1.0 / v;
            return Vec2(x * iv, y * iv);
        }
        
        Vec2<T> operator +(const Vec2<T> &v) const
        {
            return Vec2<T>(x + v.x, y + v.y);
        }
        
        Vec2<T> operator -(const Vec2<T> &v) const
        {
            return Vec2<T>(x - v.x, y - v.y);
        }
        
        T operator %(const Vec2<T> &v) const
        {
            return x * v.y - y * v.x;
        }
        
    };
    
    template<class T>
    inline std::ostream& operator<< (std::ostream &out, const Vec2<T> &v)
    {
        return out << "(" << v.x << ", " << v.y << ")";
    }
    
    template<class T> class Vec4;
    template<class T> class Mat3;
    
    //
    // 3D vector
    //
    template<class T> class Vec3
    {
    public:

        union
        {
            T vec[3];
            struct { T x, y, z; };
        };
        
        Vec3()
        {
            x = 0.0;
            y = 0.0;
            z = 0.0;
        }
        
        Vec3(const T &x, const T &y, const T &z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
        }
        
        Vec4<T> XYZ0() const;
        
        Vec4<T> XYZ1() const;
        
        void Set(const T &x, const T &y, const T &z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
        }
        
        T Dot(const Vec3<T> &u) const
        {
            return x*u.x + y*u.y + z*u.z;
        }
        
        //
        // length: |u| = sqrt(u.u)
        //
        T Length() const
        {
            return sqrt(x*x + y*y + z*z);
        }
        
        T LengthSquared() const
        {
            return x*x + y*y + z*z;
        }
        
        //
        // normalization: u/|u| = u/(u.u)
		// divide-by-zero safe
        //
        Vec3<T>& Normalize()
        {
            T lengthSquared = LengthSquared();
            
            if (lengthSquared < 1e-8)
                Set(0.0, 0.0, 0.0);
            else
            {
                float iLengthSquared = (T)(1.0 / sqrt(lengthSquared));
                Set(x*iLengthSquared, y*iLengthSquared, z*iLengthSquared);
            }
            return *this;
        }
        
        //
        // project on v: v * u.v/v.v
        //
        Vec3<T> Project(const Vec3<T> &v) const
        {
            T vLengthSquared = v.x*v.x + v.y*v.y + v.z*v.z;
            return v * (this->Dot(v) / vLengthSquared);
        }
        
        //
        // angle to vector
        //
        T Angle(Vec3<T> &v) const
        {
            Vec3<T>	un = Vec3<T>(*this).Normalize(),
            vn = Vec3<T>(v).Normalize();
            return acos( un.Dot(vn) );
        }
        
        Vec3<T>& operator +=(const Vec3<T> &v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
        }
        
        Vec3<T>& operator -=(const Vec3<T> &v)
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            return *this;
        }
        
        Vec3<T>& operator *=(const T &s)
        {
            x *= s;
            y *= s;
            z *= s;
            return *this;
        }
        
        Vec3<T>& operator *=(const Vec3<T> &v)
        {
            x *= v.x;
            y *= v.y;
            z *= v.z;
            return *this;
        }
        
        Vec3<T>& operator /=(const T &v)
        {
            x /= v;
            y /= v;
            z /= v;
            return *this;
        }
        
        Vec3<T> operator -() const
        {
            return Vec3<T>(-x, -y, -z);
        }
        
        Vec3<T> operator *(const T& s) const
        {
            return Vec3(x*s, y*s, z*s);
        }
        
        Vec3<T> operator *(const Vec3<T>& v) const
        {
            return Vec3<T>(x*v.x, y*v.y, z*v.z);
        }
        
        Vec3<T> operator /(const T& s) const
        {
            T is = 1.0 / s;
            return Vec3<T>(x*is, y*is, z*is);
        }
        
        Vec3<T> operator +(const Vec3<T>& v) const
        {
            return Vec3<T>(x+v.x, y+v.y, z+v.z);
        }
        
        Vec3<T> operator -(const Vec3<T>& v) const
        {
            return Vec3<T>(x-v.x, y-v.y, z-v.z);
        }
        
        Vec3<T> operator %(const Vec3<T>& v) const
        {
            return Vec3<T>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
        }
        
        Vec3<T> operator *(const Mat3<T>& m) const;
        
        bool operator == (const Vec3<T>& rhs) const
        {
            return x == rhs.x && y == rhs.y && z == rhs.z;
        }
        
        Mat3<T> OuterProduct(const Vec3<T>& v) const;
        
        void DebugPrint() const
        {
            printf("(%f,%f,%f)\n", x, y, z);
        }
    };
    
    template<class T>
    inline std::ostream& operator<< (std::ostream &out, const Vec3<T> &v)
    {
        return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    }
    
    //
    // 4D vector
    //
    template<class T> class Vec4
    {
    public:
        union
        {
            T vec[4];
            struct { T x, y, z, w; };
        };
        
        Vec4()
        {
            x = y = z = w = 0;
        }
        
        Vec4(const T &x, const T &y, const T &z, const T &w)
        {
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = w;
        }
        
        Vec4(const Vec3<T> &v, const T &w)
        {
            this->x = v.x;
            this->y = v.y;
            this->z = v.z;
            this->w = w;
        }
        
        void Set(const T &x, const T &y, const T &z, const T &w){
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = w;
        }
        
        Vec2<T> XY() const
        {
            return Vec2<T>(x, y);
        }
        
        Vec3<T> XYZ() const
        {
            return Vec3<T>(x, y, z);
        }
        
        Vec4<T> operator +(const Vec4<T> &v) const
        {
            return Vec4<T>(x+v.x, y+v.y, z+v.z, w+v.w);
        }
        
        Vec4<T>& operator += (const Vec4<T>& v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            w += v.w;
            
            return *this;
        }
        
        Vec4<T> operator -(const Vec4<T> &v) const
        {
            return Vec4<T>(x-v.x, y-v.y, z-v.z, w-v.w);
        }
        
        Vec4<T> operator *(const T &s) const
        {
            return Vec4<T>(x*s, y*s, z*s, w*s);
        }
        
        bool operator == (const Vec4<T>& rhs) const;
    };
    
    template<>
    inline bool Vec4<unsigned>::operator == (const Vec4<unsigned>& rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
    }
    
    template<class T>
    inline T Dot(const Vec3<T>& u, const Vec3<T>& v)
    {
        return u.x*v.x + u.y*v.y + u.z*v.z;
    }
    
    template<class T>
    inline T Dot(const Vec4<T>& u, const Vec4<T>& v)
    {
        return u.x*v.x + u.y*v.y + u.z*v.z + u.w*v.w;
    }
    
    template<class T>
    inline Vec3<T> Normalize(const Vec3<T>& u)
    {
        T lengthSquared = u.x*u.x + u.y*u.y + u.z*u.z;
        
        if (lengthSquared < 1.0e-8)
            return Vec3<T>(0.0, 0.0, 0.0);
        else
            return u * (T)(1.0/sqrt(lengthSquared));
    }
    
    template<class T>
    inline Vec4<T> Normalize(const Vec4<T>& u)
    {
        T lengthSquared = u.x*u.x + u.y*u.y + u.z*u.z + u.w*u.w;
        
        if (lengthSquared < 1.0e-8)
            return Vec4<T>(0.0, 0.0, 0.0, 0.0);
        else
            return u * (1.0/sqrt(lengthSquared));
    }
    
    template<class T>
    inline std::ostream& operator << (std::ostream &out, const Vec4<T> &v)
    {
        return out << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    }
    
    typedef Vec2<int> int2;
    typedef Vec3<int> int3;
    typedef Vec4<int> int4;

    typedef Vec2<long> long2;
    typedef Vec3<long> long3;
    typedef Vec4<long> long4;

    typedef Vec2<float> float2;
    typedef Vec3<float> float3;
    typedef Vec4<float> float4;

    typedef Vec2<unsigned> unsigned2;
    typedef Vec3<unsigned> unsigned3;
    typedef Vec4<unsigned> unsigned4;


    typedef Vec2<int> Vec2i;
    typedef Vec3<int> Vec3i;
    typedef Vec4<int> Vec4i;

    typedef Vec2<float> Vec2f;
    typedef Vec3<float> Vec3f;
    typedef Vec4<float> Vec4f;
    
    typedef Vec2<unsigned> Vec2u;
    typedef Vec3<unsigned> Vec3u;
    typedef Vec4<unsigned> Vec4u;

    //
    // compile-time instances
    //
    const Vec2f vec2f_zero = Vec2f(0, 0);
    const Vec3f vec3f_zero = Vec3f(0, 0, 0);
    const Vec4f vec4f_zero = Vec4f(0, 0, 0, 0);
}

#endif /* VEC_H */
