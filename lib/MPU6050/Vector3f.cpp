/* *****************************************************************************
 * Filename: Vector3f.h
 *
 * Description: Implementation the Vector3f class.
 *
 * Author: Thomas Havy
 *
 * License: MIT License - Copyright (c) 2017 Thomas Havy
 * A copy of the license can be found at:
 * https://github.com/Th-Havy/Simple-MPU6050-Arduino/blob/master/LICENSE
 *
 * Changes:
 * - Created: 07-Sept-2017
 *
 * ************************************************************************* */

#include "Vector3f.h"
#include <math.h>

Vector3f::Vector3f(float X, float Y, float Z)
{
    x = X;
    y = Y;
    z = Z;
}

float Vector3f::magnitude(void) const
{
    return sqrt(x*x + y*y + z*z);
}

void Vector3f::normalize(void)
{
    *this /= magnitude();
}

float Vector3f::dotProduct(const Vector3f& a, const Vector3f& b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vector3f Vector3f::crossProduct(const Vector3f& a, const Vector3f& b)
{
    Vector3f c;

    c.x = a.y*b.z - b.y*a.z;
    c.y = a.z*b.x - b.z*a.x;
    c.z = a.x*b.y - b.x*a.y;

    return c;
}

Vector3f Vector3f::operator-() const
{
    Vector3f a;
    a.x = -x;
    a.y = -y;
    a.z = -z;

    return a;
}

Vector3f& Vector3f::operator+=(const Vector3f& right)
{
    x += right.x;
    y += right.x;
    z += right.x;

    return *this;
}

Vector3f& Vector3f::operator-=(const Vector3f& right)
{
    *this += -right;

    return *this;
}

Vector3f& Vector3f::operator*=(const float &a)
{
    x *= a;
    y *= a;
    z *= a;

    return *this;
}

Vector3f& Vector3f::operator/=(const float &a)
{
    return *this *= (1/a);
}

bool operator==(const Vector3f& a, const Vector3f& b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool operator!=(const Vector3f& a, const Vector3f& b)
{
    return !(a == b);
}

Vector3f operator+(const Vector3f& a, const Vector3f& b)
{
    Vector3f c(a);

    c += b;

    return c;
}

Vector3f operator-(const Vector3f& a, const Vector3f& b)
{
    Vector3f c(a);

    c -= b;

    return c;
}
