
//
// 2x2, 3x3 & 4x4 Matrix lib
//
// Carl Johan Gribel 2016-2021, cjgribel@gmail.com
//

#pragma once
#ifndef MAT_H
#define MAT_H

#include <cstdio>
#include "math.h"
#include "Vec.h"

namespace linalg
{
	//
    // 2D column-major matrix
    //
    // | m11 m12 |
    // | m21 m22 |
    // 

    template<class T> class Mat2
    {
    public:

        union
        {
            T array[4];
            T mat[2][2];
            struct { T m11, m21, m12, m22; };
            struct { Vec2<T> col[2]; };
        };
        
        Mat2() { }
        
        //
        // constructor: from elements
        //
        Mat2(const T& m11, const T& m12, const T& m21, const T& m22)
            : m11(m11), m12(m12), m21(m21), m22(m22)
        { }
        
        //
        // constructor: rotation matrix
        //
        Mat2(const T& rad)
        {
            T c = cos(rad);
            T s = sin(rad);
            m11 = c; m12 = -s;
            m21 = s; m22 = c;
        }
        
        //
        // constructor: scaling matrix
        //
        Mat2(const T& scale_x, const T& scale_y)
        {
            m11 = scale_x;	m12 = 0.0;
            m21 = 0.0;		m22 = scale_y;
        }
        
        Mat2<T> Invert() const
        {
            T det = m11 * m22 - m12 * m21;
            
            return Mat2<T>(m22, -m21, -m12, m11) * (1.0/det);
        }
        
        Mat2<T> operator - ()
        {
            return Mat2<T>(-m11, -m12, -m21, -m22);
        }
        
        Mat2<T> operator * (const T& s) const
        {
            return Mat2<T>(m11*s, m12*s, m21*s, m22*s);
        }
        
        Vec2<T> operator * (const Vec2<T> &rhs) const;
        
    };
    
    
    //
    // 3D column-major matrix
    //
    template<class T> class Mat3
    {
    public:

        union
        {
            T array[9];
            T mat[3][3];
            struct { T m11, m21, m31, m12, m22, m32, m13, m23, m33; };
            struct { Vec3<T> col[3]; };
        };
        
        Mat3() { }
        
		//
		// row-major per-element constructor
		//
		Mat3(const T& _m11, const T& _m12, const T& _m13,
			 const T& _m21, const T& _m22, const T& _m23,
			 const T& _m31, const T& _m32, const T& _m33)
		{
			m11 = _m11; m12 = _m12; m13 = _m13;
			m21 = _m21; m22 = _m22; m23 = _m23;
			m31 = _m31; m32 = _m32; m33 = _m33;
		}
        
		//
        // constructor: equal diagonal elements
        //
        Mat3(const T& d) : Mat3(d,d,d) { }
        
		//
        // constructor: diagonal elements (scaling matrix)
        //
        Mat3(const T& d0, const T& d1, const T& d2)
        {
            m11 = d0;
            m22 = d1;
            m33 = d2;
            m12 = m13 = m21 = m23 = m31 = m32 = 0.0;
        }
        
        //
        // from basis vectors
        //
        Mat3(const Vec3<T>& e0, const Vec3<T>& e1, const Vec3<T>& e2)
        {
            col[0] = e0;
            col[1] = e1;
            col[2] = e2;
        }
        
        Vec3<T> Column(int i)
        {
            assert(i<3);
            return col[i];
        }
        
		//
		// Rotation theta around vector u=(x,y,z) (Euler angle & Euler axis)
		//
		//             | 0  -z  y |
		// Ru(theta) = | z   0 -x | sin(theta) + (I - u.u^T)cos(theta) + u.u^T
		//             | -y  x  0 |
		//
		// http://en.wikipedia.org/wiki/Rotation_representation#Rotation_matrix_.E2.86.94_Euler_axis.2Fangle
		// https://en.wikipedia.org/wiki/Rotation_matrix#Nested_dimensions
		//
		// notes: u should be normalized
		//
        static Mat3<T> Rotation(const T& theta, const T& x, const T& y, const T& z)
        {
            Mat3<T> R;
            T c1 = cos(theta);
            T c2 = 1.0-c1;
            T s = sin(theta);
            
            R.m11 = c1 + c2*x*x;	R.m12 = c2*x*y - s*z;	R.m13 = c2*x*z + s*y;
            R.m21 = c2*x*y + s*z;	R.m22 = c1 + c2*y*y;	R.m23 = c2*y*z - s*x;
            R.m31 = c2*x*z - s*y;	R.m32 = c2*y*z + s*x;	R.m33 = c1 + c2*z*z;
            
            return R;
        }
        
        void Transpose()
        {
            std::swap(m21, m12);
            std::swap(m31, m13);
            std::swap(m32, m23);
        }
        
        // 
        // inverse: A^(-1) = 1/det(A) * adjugate(A)
        //
        Mat3<T> Inverse() const
        {
            T det = Determinant();
            assert(det > 1e-8);
            T idet = 1.0/det;
            
            Mat3<T> M;
            M.m11 =  (m22*m33 - m32*m23);
            M.m21 = -(m21*m33 - m31*m23);
            M.m31 =  (m21*m32 - m31*m22);
            
            M.m12 = -(m12*m33 - m32*m13);
            M.m22 =  (m11*m33 - m31*m13);
            M.m32 = -(m11*m32 - m31*m12);
            
            M.m13 =  (m12*m23 - m22*m13);
            M.m23 = -(m11*m23 - m21*m13);
            M.m33 =  (m11*m22 - m21*m12);
            
            return M*idet;
        }
        
        void Set(const Mat3<T> &m)
        {
            col[0] = m.col[0];
            col[1] = m.col[1];
            col[2] = m.col[2];
        }
        
        T Determinant() const
        {
            return
                m11*m22*m33 +
                m12*m23*m31 +
                m13*m21*m32 -
                m11*m23*m32 -
                m12*m21*m33 -
                m13*m22*m31;
        }
        
		//
        // (ugly) matrix normalization
        //
        void Normalize();
        
        Mat3<T> operator * (const T& s) const
        {
            return Mat3<T>(m11*s, m12*s, m13*s,
                           m21*s, m22*s, m23*s,
                           m31*s, m32*s, m33*s);
        }
        
        Mat3<T> operator +(const Mat3<T>& m) const
        {
            return Mat3<T>(m11+m.m11, m12+m.m12, m13+m.m13,
                           m21+m.m21, m22+m.m22, m23+m.m23,
                           m31+m.m31, m32+m.m32, m33+m.m33);
        }
        
        Mat3<T> operator -(const Mat3<T>& m) const
        {
            return Mat3(m11-m.m11, m12-m.m12, m13-m.m13,
                        m21-m.m21, m22-m.m22, m23-m.m23,
                        m31-m.m31, m32-m.m32, m33-m.m33);
        }
        
        Mat3<T>& operator +=(const Mat3<T>& m)
        {
            col[0] += m.col[0];
            col[1] += m.col[1];
            col[2] += m.col[2];
            
            return *this;
        }
        
        Mat3<T>& operator *=(const T& s)
        {
            col[0] *= s;
            col[1] *= s;
            col[2] *= s;
            
            return *this;
        }
        
        Mat3<T> operator *(const Mat3<T>& m) const
        {
            return Mat3<T>(m11*m.m11 + m12*m.m21 + m13*m.m31, m11*m.m12 + m12*m.m22 + m13*m.m32, m11*m.m13 + m12*m.m23 + m13*m.m33,
                           m21*m.m11 + m22*m.m21 + m23*m.m31, m21*m.m12 + m22*m.m22 + m23*m.m32, m21*m.m13 + m22*m.m23 + m23*m.m33,
                           m31*m.m11 + m32*m.m21 + m33*m.m31, m31*m.m12 + m32*m.m22 + m33*m.m32, m31*m.m13 + m32*m.m23 + m33*m.m33);
        }
        
        Vec3<T> operator *(const Vec3<T> &v) const;
        
        //
        // thread safe debug print
        //
        void DebugPrint()
        {
            printf("%f %f %f\n%f %f %f\n%f %f %f\n",
                   m11, m12, m13, m21, m22, m23, m31, m32, m33);
        }
    };
    
	//
	// thread unsafe debug print
	//
    template<class T>
    inline std::ostream& operator<< (std::ostream &out, const Mat3<T> &m)
    {
        for (int i=0; i<3; i++)
            printf("%1.4f, %1.4f, %1.4ff\n", m.mat[0][i], m.mat[1][i], m.mat[2][i]);
        
        return out;
    }
    
	//
	// 4D column-major matrix
	//
    template<class T> class Mat4
    {
    public:

        union
        {
            T array[16];
            T mat[4][4];
            struct {
                T m11, m21, m31, m41;
                T m12, m22, m32, m42;
                T m13, m23, m33, m43;
                T m14, m24, m34, m44;
            };
            struct { Vec4<T> col[4]; };
        };
        
        constexpr Mat4() : Mat4(0) { }

        constexpr Mat4(T d) : Mat4(d, d, d, d) { }

        constexpr Mat4(const T& d0, const T& d1, const T& d2, const T& d3)
        {
            m11 = d0;  m12 = 0.0; m13 = 0.0; m14 = 0.0;
            m21 = 0.0; m22 = d1;  m23 = 0.0; m24 = 0.0;
            m31 = 0.0; m32 = 0.0; m33 = d2;  m34 = 0.0;
            m41 = 0.0; m42 = 0.0; m43 = 0.0; m44 = d3;
        }

        constexpr Mat4(const Mat3<T>& m)
        {
            m11 = m.m11; m12 = m.m12; m13 = m.m13; m14 = 0.0;
            m21 = m.m21; m22 = m.m22; m23 = m.m23; m24 = 0.0;
            m31 = m.m31; m32 = m.m32; m33 = m.m33; m34 = 0.0;
            m41 = 0.0;   m42 = 0.0;   m43 = 0.0;   m44 = 1.0;
        }

        /**
         * row-major per-element constructor
         */
        constexpr Mat4(const T& _m11, const T& _m12, const T& _m13, const T& _m14,
            const T& _m21, const T& _m22, const T& _m23, const T& _m24,
            const T& _m31, const T& _m32, const T& _m33, const T& _m34,
            const T& _m41, const T& _m42, const T& _m43, const T& _m44)
        {
            m11 = _m11; m12 = _m12; m13 = _m13; m14 = _m14;
            m21 = _m21; m22 = _m22; m23 = _m23; m24 = _m24;
            m31 = _m31; m32 = _m32; m33 = _m33; m34 = _m34;
            m41 = _m41; m42 = _m42; m43 = _m43; m44 = _m44;
        }
        
		//
		// get the upper-left submatrix
		//
        Mat3<T> Get_3x3() const
        {
            return Mat3<T>(m11, m12, m13, m21, m22, m23, m31, m32, m33);
        }
        
        void Transpose()
        {
            std::swap(m21, m12);
            std::swap(m31, m13);
            std::swap(m32, m23);
            std::swap(m41, m14);
            std::swap(m42, m24);
            std::swap(m43, m34);
        }
        
        Mat4<T> Inverse() const
        {
            T det = Determinant();
            assert(abs(det) > 1e-8);
            T idet = 1.0/det;
            
            Mat4<T> M = Mat4<T>(m23 * m34 * m42 - m24 * m33 * m42 + m24 * m32 * m43 - m22 * m34 * m43 - m23 * m32 * m44 + m22 * m33 * m44,
                                m14 * m33 * m42 - m13 * m34 * m42 - m14 * m32 * m43 + m12 * m34 * m43 + m13 * m32 * m44 - m12 * m33 * m44,
                                m13 * m24 * m42 - m14 * m23 * m42 + m14 * m22 * m43 - m12 * m24 * m43 - m13 * m22 * m44 + m12 * m23 * m44,
                                m14 * m23 * m32 - m13 * m24 * m32 - m14 * m22 * m33 + m12 * m24 * m33 + m13 * m22 * m34 - m12 * m23 * m34,
                                m24 * m33 * m41 - m23 * m34 * m41 - m24 * m31 * m43 + m21 * m34 * m43 + m23 * m31 * m44 - m21 * m33 * m44,
                                m13 * m34 * m41 - m14 * m33 * m41 + m14 * m31 * m43 - m11 * m34 * m43 - m13 * m31 * m44 + m11 * m33 * m44,
                                m14 * m23 * m41 - m13 * m24 * m41 - m14 * m21 * m43 + m11 * m24 * m43 + m13 * m21 * m44 - m11 * m23 * m44,
                                m13 * m24 * m31 - m14 * m23 * m31 + m14 * m21 * m33 - m11 * m24 * m33 - m13 * m21 * m34 + m11 * m23 * m34,
                                m22 * m34 * m41 - m24 * m32 * m41 + m24 * m31 * m42 - m21 * m34 * m42 - m22 * m31 * m44 + m21 * m32 * m44,
                                m14 * m32 * m41 - m12 * m34 * m41 - m14 * m31 * m42 + m11 * m34 * m42 + m12 * m31 * m44 - m11 * m32 * m44,
                                m12 * m24 * m41 - m14 * m22 * m41 + m14 * m21 * m42 - m11 * m24 * m42 - m12 * m21 * m44 + m11 * m22 * m44,
                                m14 * m22 * m31 - m12 * m24 * m31 - m14 * m21 * m32 + m11 * m24 * m32 + m12 * m21 * m34 - m11 * m22 * m34,
                                m23 * m32 * m41 - m22 * m33 * m41 - m23 * m31 * m42 + m21 * m33 * m42 + m22 * m31 * m43 - m21 * m32 * m43,
                                m12 * m33 * m41 - m13 * m32 * m41 + m13 * m31 * m42 - m11 * m33 * m42 - m12 * m31 * m43 + m11 * m32 * m43,
                                m13 * m22 * m41 - m12 * m23 * m41 - m13 * m21 * m42 + m11 * m23 * m42 + m12 * m21 * m43 - m11 * m22 * m43,
                                m12 * m23 * m31 - m13 * m22 * m31 + m13 * m21 * m32 - m11 * m23 * m32 - m12 * m21 * m33 + m11 * m22 * m33);
            
            return M*idet;
        }
        
        T Determinant() const
        {
            return
            m14 * m23 * m32 * m41 - m13 * m24 * m32 * m41 - m14 * m22 * m33 * m41 + m12 * m24 * m33 * m41 +
            m13 * m22 * m34 * m41 - m12 * m23 * m34 * m41 - m14 * m23 * m31 * m42 + m13 * m24 * m31 * m42 +
            m14 * m21 * m33 * m42 - m11 * m24 * m33 * m42 - m13 * m21 * m34 * m42 + m11 * m23 * m34 * m42 +
            m14 * m22 * m31 * m43 - m12 * m24 * m31 * m43 - m14 * m21 * m32 * m43 + m11 * m24 * m32 * m43 +
            m12 * m21 * m34 * m43 - m11 * m22 * m34 * m43 - m13 * m22 * m31 * m44 + m12 * m23 * m31 * m44 +
            m13 * m21 * m32 * m44 - m11 * m23 * m32 * m44 - m12 * m21 * m33 * m44 + m11 * m22 * m33 * m44;
        }
        
        void Set(const Mat4<T> &m)
        {
            col[0] = m.col[0];
            col[1] = m.col[1];
            col[2] = m.col[2];
            col[3] = m.col[3];
        }
        
        Vec4<T> Column(int i)
        {
            assert(i<4);
            return col[i];
        }

        T& operator [](unsigned i) const
        {
            return array[i];
        }
        
        T& operator [](unsigned i)
        {
            return array[i];
        }
        
        Mat4<T> operator *(const T& s) const
        {
            return Mat4<T>(m11*s, m12*s, m13*s, m14*s,
                           m21*s, m22*s, m23*s, m24*s,
                           m31*s, m32*s, m33*s, m34*s,
                           m41*s, m42*s, m43*s, m44*s);
        }
        
        Mat4<T>& operator *=(const T& s)
        {
            col[0] *= s;
            col[1] *= s;
            col[2] *= s;
            col[3] *= s;
            
            return *this;
        }
        
        Mat4<T> operator + (const Mat4<T>& m) const
        {
            Mat4<T> n = *this;
            n.col[0] += m.col[0];
            n.col[1] += m.col[1];
            n.col[2] += m.col[2];
            n.col[3] += m.col[3];
            
            return n;
        }
        
        Mat4<T> operator *(const Mat4<T>& m) const
        {
            return Mat4<T>(m11 * m.m11 + m12 * m.m21 + m13 * m.m31 + m14 * m.m41,
                           m11 * m.m12 + m12 * m.m22 + m13 * m.m32 + m14 * m.m42,
                           m11 * m.m13 + m12 * m.m23 + m13 * m.m33 + m14 * m.m43,
                           m11 * m.m14 + m12 * m.m24 + m13 * m.m34 + m14 * m.m44,
                           
                           m21 * m.m11 + m22 * m.m21 + m23 * m.m31 + m24 * m.m41,
                           m21 * m.m12 + m22 * m.m22 + m23 * m.m32 + m24 * m.m42,
                           m21 * m.m13 + m22 * m.m23 + m23 * m.m33 + m24 * m.m43,
                           m21 * m.m14 + m22 * m.m24 + m23 * m.m34 + m24 * m.m44,
                           
                           m31 * m.m11 + m32 * m.m21 + m33 * m.m31 + m34 * m.m41,
                           m31 * m.m12 + m32 * m.m22 + m33 * m.m32 + m34 * m.m42,
                           m31 * m.m13 + m32 * m.m23 + m33 * m.m33 + m34 * m.m43,
                           m31 * m.m14 + m32 * m.m24 + m33 * m.m34 + m34 * m.m44,
                           
                           m41 * m.m11 + m42 * m.m21 + m43 * m.m31 + m44 * m.m41,
                           m41 * m.m12 + m42 * m.m22 + m43 * m.m32 + m44 * m.m42,
                           m41 * m.m13 + m42 * m.m23 + m43 * m.m33 + m44 * m.m43,
                           m41 * m.m14 + m42 * m.m24 + m43 * m.m34 + m44 * m.m44);
        }
        
        Vec4<T> operator *(const Vec4<T> &v) const;
        
        static Mat4<T> Translation(const Vec3<T>& p)
        {
            return Translation(p.x, p.y, p.z);
        }
        
        static Mat4<T> Translation(const T& x, const T& y, const T& z)
        {
            Mat4<T> M;
            M.m11 = 1.0; M.m12 = 0.0; M.m13 = 0; M.m14 = x;
            M.m21 = 0.0; M.m22 = 1.0; M.m23 = 0; M.m24 = y;
            M.m31 = 0.0; M.m32 = 0.0; M.m33 = 1; M.m34 = z;
            M.m41 = 0.0; M.m42 = 0.0; M.m43 = 0; M.m44 = 1.0;
            
            return M;
        }
        
        static Mat4<T> Scaling(const T& s)
        {
            return Scaling({s,s,s});
        }
        
        static Mat4<T> Scaling(float sx, float sy, float sz)
        {
            return Mat4<T>(sx, sy, sz, 1.0);
        }
        
        static Mat4<T> Scaling(const Vec3<T> &sv)
        {
            return Mat4<T>(sv.x, sv.y, sv.z, 1.0);
        }
        
        static Mat4<T> Rotation(const T& theta, const Vec3<T> &v)
        {
            return Rotation(theta, v.x, v.y, v.z);
        }
        
        //
		// Rotation theta around vector u=(x,y,z) (Euler angle & Euler axis)
		//
        //             | 0  -z  y |
        // Ru(theta) = | z   0 -x | sin(theta) + (I - u.u^T)cos(theta) + u.u^T
        //             | -y  x  0 |
        //
		// http://en.wikipedia.org/wiki/Rotation_representation#Rotation_matrix_.E2.86.94_Euler_axis.2Fangle
        // https://en.wikipedia.org/wiki/Rotation_matrix#Nested_dimensions
        //
        // notes: u should be normalized
        //
        static Mat4<T> Rotation(const T& theta, const T& x, const T& y, const T& z)
        {
            Mat4<T> M;
            T c1 = cos(theta);
            T c2 = (T)(1.0-c1);
            T s = sin(theta);
            
            M.m11 = c1 + c2*x*x;	M.m12 = c2*x*y - s*z;	M.m13 = c2*x*z + s*y;   M.m14 = 0.0;
            M.m21 = c2*x*y + s*z;	M.m22 = c1 + c2*y*y;	M.m23 = c2*y*z - s*x;   M.m24 = 0.0;
            M.m31 = c2*x*z - s*y;	M.m32 = c2*y*z + s*x;	M.m33 = c1 + c2*z*z;;   M.m34 = 0.0;
            M.m41 = 0.0;            M.m42 = 0.0;            M.m43 = 0.0;            M.m44 = 1.0;
            
            return M;
        }
        
		//
		// Rotation from Euler angles: roll (z), yaw (y) , pitch (x)
		// Mult order, R = R_z(roll) * R_y(yaw) * R_x(pitch)
		// (first around x, then y, then z)
		//
		// Definition:
		// http://planning.cs.uiuc.edu/node102.html
		// Note: uses notation yaw (z), roll (x), pitch (y)
		//
		static Mat4<T> Rotation(const T& roll, const T& yaw, const T& pitch)
		{
			const T sina = sin(roll);
			const T cosa = cos(roll);
			const T sinb = sin(yaw);
			const T cosb = cos(yaw);
			const T sing = sin(pitch);
			const T cosg = cos(pitch);

			return Mat4<T>(	cosa*cosb, cosa*sinb*sing - sina*cosg, cosa*sinb*cosg - sina*sing, 0,
							sina*cosb, sina*sinb*sing + cosa*cosg, sina*sinb*cosg - cosa*sing, 0,
							-sinb, cosb*sing, cosb*cosg, 0,
							0, 0, 0, 1);
		}
        
        static Mat4<T> TRS(Vec3<T> vt, float theta, Vec3<T> rotv, Vec3<T> sv)
        {
            return Translation(vt) * rotation(theta, rotv) * scaling(sv);
        }
        
        static Mat4<T> ViewportMatrix(const T& w, const T& h)
        {
            return mat4f(w*0.5f,    0.0f, 0.0f, w*0.5f,
                           0.0f,  h*0.5f, 0.0f, h*0.5f,
                           0.0f,    0.0f, 0.5f,   0.5f,
                           0.0f,    0.0f, 0.0f,   1.0f);
        }

        //
        // general frustum projection matrix
        // 
        // frustum planes not necessarily symmetric in the y=0 and x=0 planes of the view frame
        //
        static Mat4<T> GL_AsymmetricProjection(const T& l, const T& r, const T& b, const T& t, const T& n, const T& f)
        {
            T n2 = 2.0f*n;
            T rl = r - l;
            T tb = t - b;
            T fn = f - n;
            
            return Mat4<T>(n2/rl,   0.0f,   (r+l)/rl,   0.0f,
                           0.0f,    n2/tb,  (t+b)/tb,   0.0f,
                           0.0f,    0.0f,   (-f- n)/fn, (-n2*f)/fn,
                           0.0f,    0.0f,   -1.0f,      0.0f);
        }
        
        //
        // GL symmetric frustum projection matrix [18]
        // 
        // frustum planes are symmetric in the y=0 and x=0 planes of the view frame
        //
        static Mat4<T> GL_SymmetricProjection(const T& r, const T& t, const T& n, const T& f)
        {
            T n2 = 2.0f*n;
            T fn = f - n;

            return Mat4<T>(n/r,   0.0f, 0.0f,       0.0f,
                           0.0f,  n/t,  0.0f,       0.0f,
                           0.0f,  0.0f, (-f- n)/fn, (-n2*f)/fn,
                           0.0f,  0.0f, -1.0f,      0.0f);
        }
        
        //
        // GL view projection matrix [18]
        //
        static Mat4<T> Projection(const T& vfov, const T& aspectr, const T& n, const T& f)
        {
            T t = n * tanf(vfov/2.0f);
			T r = t * aspectr;

            return GL_SymmetricProjection(r, t, n, f);
        }
        
        //
        // thread safe debug print
        //
        void DebugPrint()
        {
            printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n",
                   m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44);
        }

		static void DebugPrint(const Mat4<T>& m)
		{
			printf("%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n",
				m.m11, m.m12, m.m13, m.m14, m.m21, m.m22, m.m23, m.m24, m.m31, m.m32, m.m33, m.m34, m.m41, m.m42, m.m43, m.m44);
		}
        
    };
    
	//
	// thread unsafe debug print
	//
    template<class T>
    inline std::ostream& operator<< (std::ostream &out, const Mat4<T> &m)
    {
        for (int i=0; i<4; i++)
            printf("%f, %f, %f, %f\n", m.mat[0][i], m.mat[1][i], m.mat[2][i], m.mat[3][i]);
        
        return out;
    }

    template<class T>
    inline Mat4<T> Transpose(const Mat4<T>& m)
    {
		Mat4<T> n = m;
		n.Transpose();
		return n;
    }
    
    typedef Mat2<float> Mat2f;
    typedef Mat3<float> Mat3f;
    typedef Mat4<float> Mat4f;
    
    //
    // compile-time instances
    //
    const Mat2f mat2f_zero = Mat2f(0.0f);
    const Mat3f mat3f_zero = Mat3f(0.0f);
    const Mat4f mat4f_zero = Mat4f(0.0f);
    const Mat2f mat2f_identity = Mat2f(1.0f);
    const Mat3f mat3f_identity = Mat3f(1.0f);
    const Mat4f mat4f_identity = Mat4f(1.0f);
}

#endif /* MAT_H */
