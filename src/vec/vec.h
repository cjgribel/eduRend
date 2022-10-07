/**
 * @file vec.h
 * @brief 2D, 3D & 4D Vector lib
 * @author Carl Johan Gribel 2016-2021, cjgribel@gmail.com
*/

#pragma once
#ifndef VEC_H
#define VEC_H

#include <cmath>
#include <cstdio>
#include <ostream>

namespace linalg
{
    /**
     * @brief 2D vector
     * @tparam T Number representation to use
    */
    template<class T> 
    class vec2
    {
    public:
        union
        {
            T vec[2];
            struct { T x, y; };
        };
        
        /**
         * @brief Constructor: default zero initialization
        */
        constexpr vec2() : vec2(0.0f) {}

        /**
         * @brief Constuctor: value initialization.
         * @param value Value to set each element to.
        */
        constexpr vec2(const T &value) : vec2(value, value) {}
        
        /**
         * @brief Constructor: element initialization.
         * @param x Value for vec2::x
         * @param y Value for vec2::y
        */
        constexpr vec2(const T& x, const T& y) : x(x), y(y) {}
                
        /**
         * @brief Calculates the dot procuct between this and u
         * @param u Second vector in the dot product
         * @return this.x * u.x + this.y + u.y
        */
        constexpr float dot(const vec2<T> &u) const
        {
            return x*u.x + y*u.y;
        }
        
        /**
         * @brief Gets the length of the vector
         * @details |u| = sqrt(u.u)
         * @return length of the vector.
        */
        constexpr float length() const
        {
            return sqrt(x * x + y * y);
        }
        
        /**
         * @brief Gets the squared length of the vector
         * @return squared length of the vector.
        */
        constexpr float length_squared() const
        {
            return x * x + y * y;
        }

        /**
         * @brief Normalize the vector.
         * @details Formula is u/|u| = u/(u.u)
         * @return reference to this
        */
        constexpr vec2<T>& normalize()
        {
            T lengthSquared = length_squared();
            
            if (lengthSquared < 1e-8)
            {
                *this = vec2<T>(0.0);
            }
            else
            {
                T inormSquared = 1.0 / sqrt(lengthSquared);
                *this = vec2<T>(x * inormSquared, y * inormSquared);
            }
            return *this;
        }
        
        /**
         * @brief Project on v: v * u.v/v.v
        */
        constexpr vec2<T> project(vec2<T> &v) const
        {
            T vnormSquared = v.x*v.x + v.y*v.y;
            return v * (this->dot(v) / vnormSquared);
        }
        
        /**
         * @brief Gets the angle to v
         * @param v Target for the calculation
         * @return Angle between this and v
        */
        constexpr float angle(vec2<T> &v)
        {
            vec2<T>	un = vec2f(*this).normalize(),
            vn = vec2f(v).normalize();
            return acos( un.dot(vn) );
        }
        
        /**
         * @brief Assignment operation
         * @param v New values for the vector
         * @return Reference to this
        */
        constexpr vec2<T>& operator =(const vec2<T> &v)
        {
            x = v.x;
            y = v.y;
            return *this;
        }
        
        /**
         * @brief Element addition
         * @param v Vector to add
         * @return Reference to this
        */
        constexpr vec2<T>& operator +=(const vec2<T> &v)
        {
            x += v.x;
            y += v.y;
            return *this;
        }
        
        /**
         * @brief Element subtraction
         * @param v Vector to subtract
         * @return Reference to this
        */
        constexpr vec2<T>& operator -=(const vec2<T> &v)
        {
            x -= v.x;
            y -= v.y;
            return *this;
        }
        
        /**
         * @brief Element multiplication
         * @param s Scalar value to multiply
         * @return Reference to this
        */
        constexpr vec2<T>& operator *=(const T &s)
        {
            x *= s;
            y *= s;
            return *this;
        }
        
        /**
        * @brief Element multiplication
        * @param v Vector to multiply
        * @return Reference to this
       */
        constexpr vec2<T>& operator *=(const vec2<T> &v)
        {
            x *= v.x;
            y *= v.y;
            return *this;
        }
        
        /**
        * @brief Element devision
        * @param v Vector to divide
        * @return Reference to this
       */
        constexpr vec2<T>& operator /=(const T &v)
        {
            x /= v;
            y /= v;
            return *this;
        }
        
        /**
         * @brief Invers operator
         * @return New vector representing the inverse of this.
        */
        constexpr vec2<T> operator -() const
        {
            return vec2<T>(-x, -y);
        }
        
        /**
         * @brief Multiplication
         * @param s Right side value
         * @return Product of this * s
        */
        constexpr vec2<T> operator *(const T &s) const
        {
            return vec2<T>(x * s, y * s);
        }

        /**
         * @brief Multiplication
         * @param v Right side value
         * @return Product of this * v
        */
        constexpr vec2<T> operator *(const vec2<T> &v) const
        {
            return vec2<T>(x * v.x, y * v.y);
        }
        
        /**
         * @brief Division
         * @param v Right side value
         * @return Quotient of this / v
        */
        constexpr vec2<T> operator /(const T &v) const
        {
            T iv = 1.0 / v;
            return vec2(x * iv, y * iv);
        }
        
        /**
         * @brief Addition
         * @param v Right side value
         * @return Sum of this + v
        */
        constexpr vec2<T> operator +(const vec2<T> &v) const
        {
            return vec2<T>(x + v.x, y + v.y);
        }
        
        /**
         * @brief Subtraction
         * @param v Right side value
         * @return Difference of this - v
        */
        constexpr vec2<T> operator -(const vec2<T> &v) const
        {
            return vec2<T>(x - v.x, y - v.y);
        }
        
        constexpr T operator %(const vec2<T> &v) const
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
    
    /**
     * @brief 3D vector
     * @tparam T Number representation to use
    */
    template<class T> class vec3
    {
    public:
        union
        {
            T vec[3];
            struct { T x, y, z; };
        };
        
        constexpr vec3() : vec3(0.0f) {}
        
        constexpr vec3(const T& value) : vec3(value, value, value) {}

        constexpr vec3(const T &x, const T &y, const T &z) : x(x), y(y), z(z) {}
        
        vec4<T> xyz0() const;
        
        vec4<T> xyz1() const;
        
        void set(const T &new_x, const T &new_y, const T &new_z)
        {
            this->x = new_x;
            this->y = new_y;
            this->z = new_z;
        }
        
        T dot(const vec3<T> &u) const
        {
            return x*u.x + y*u.y + z*u.z;
        }
        
        //
        // vector length (2-norm): |u| = sqrt(u.u)
        //
        constexpr T length() const
        {
            return sqrt(x*x + y*y + z*z);
        }
        
        constexpr T length_squared() const
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
    
    /**
     * @brief 4D vector
     * @tparam T Number representation to use
    */
    template<class T> class vec4
    {
    public:
        union
        {
            T vec[4];
            struct { T x, y, z, w; };
        };
        
        constexpr vec4() : vec4(0.0f) {}
        
        constexpr vec4(const T& value) : vec4(value, value, value, value) {}

        constexpr vec4(const T &x, const T &y, const T &z, const T &w) : x(x), y(y), z(z), w(w) {}
        
        constexpr vec4(const vec3<T>& v, const T& w) : vec4(v.x, v.y, v.z, w) {}
        
        void set(const T &x, const T &y, const T &z, const T &w)
        {
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
        T length = u.length_squared();
        
        if( length < 1.0e-8 )
            return vec3<T>(0.0, 0.0, 0.0);
        else
            return u * (T)(1.0/sqrt(length));
    }
    
    template<class T>
    inline vec4<T> normalize(const vec4<T>& u)
    {
        T length = u.x*u.x + u.y*u.y + u.z*u.z + u.w*u.w;
        
        if( length < 1.0e-8 )
            return vec4<T>(0.0, 0.0, 0.0, 0.0);
        else
            return u * (1.0/sqrt(length));
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
    const vec2f vec2f_zero = vec2f(0, 0); //!< Compile-time zero initialized vec2f
    const vec3f vec3f_zero = vec3f(0, 0, 0); //!< Compile-time zero initialized vec3f
    const vec4f vec4f_zero = vec4f(0, 0, 0, 0); //!< Compile-time zero initialized vec4f
}

#endif /* VEC_H */
