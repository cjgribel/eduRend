/**
 * @file math.h
 * @brief Contains mathmatical functions
 * @author Carl Johan Gribel 2016-2021, cjgribel@gmail.com
*/

#pragma once
#ifndef MATH_H
#define MATH_H

#include <stdlib.h>
//#include <cmath>
#include <algorithm>

#ifndef DEBUG
#ifndef NDEBUG
#define NDEBUG          /* NDEBUG will disable assert */
#endif
#endif
#include <cassert>

//!< Define for PI.
#define fPI			3.141592653f

//!< Define for floating point infinity.
#define fINF		3.4028234e38

//!< Define for floating point negative infinity.
#define fNINF		-3.4028234e38

//!< Define for multiplication from Degrees to Radians.
#define fTO_RAD		(fPI/180.0f)

//!<Define for multiplication from Radians to Degrees.
#define fTO_DEG		(180.0f/fPI)

//!< Get closest whole integer that is smaller than or equal to x.
#define simplefloor(x) ((double)((long)(x)-((x)<0.0)))

/**
 * @brief Generates a random floating point number between min and max.
 * @param min Minimum value to randomize.
 * @param max Maximum value to randomize.
 * @return Randomized value between min and max
*/
inline float rnd(const float &min, const float &max) { return min + (float)rand()/RAND_MAX*(max-min); }

/**
 * @brief Linearly interpolates between a and b
 * @details Formula used is a*(1.0f-x) + b*x
 * @tparam T Number representation to use.
 * @param a Start value.
 * @param b Target value.
 * @param x Interpolation value.
 * @return Value that is between a and b dependent on x.
*/
template<typename T>
inline T lerp(const T &a, const T &b, float x) { return a*(1.0f-x) + b*x; }

/**
 * @brief Clamps a between min and max.
 * @tparam T Number representation to use.
 * @param a Value to be clamped.
 * @param min Minumum value.
 * @param max Maximum value.
 * @return [min <= a <= max]
*/
template<typename T>
inline T clamp(const T &a, const T &min, const T &max) { return std::max<T>(min, std::min<T>(max, a)); }

template<typename T>
inline T smoothstep(const T &x, const T &a, const T &b)
{
    if (x < a)
        return 0.0;
    if (x >= b)
        return 1.0;
    
    T t = (x-a)/(b-a);
    return t*t*(3.0-2.0*t);
}

/* handles negative numbers correclty
 */
inline float mod(float a, float b)
{
    const int n = (int)(a/b);
    a -= n*b;
    if (a < 0)
        a += b;

    return a;
}

inline float gammacorrect(const float &gamma, const float &x) { return powf(x, 1.0f/gamma); }


#endif /* MATH_H */