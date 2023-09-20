// File: Vector3f.h
// Author: Thomas Havy
// Description: Simple utility class for 3D vectors and operations

#ifndef VECTOR3F_H
#define VECTOR3F_H


class Vector3f
{
    public:
        float x,y,z;

        Vector3f(float X=0, float Y=0, float Z=0);

        float magnitude(void) const;
        void normalize(void);

        static float dotProduct(const Vector3f& a, const Vector3f& b);
        static Vector3f crossProduct(const Vector3f& a, const Vector3f& b);

        Vector3f operator-() const;
        Vector3f& operator+=(const Vector3f& right);
        Vector3f& operator-=(const Vector3f& right);
        Vector3f& operator*=(const float& a);
        Vector3f& operator/=(const float& a);

        friend bool operator==(const Vector3f& a, const Vector3f& b);
        friend bool operator!=(const Vector3f& a, const Vector3f& b);
        friend Vector3f operator+(const Vector3f& a, const Vector3f& b);
        friend Vector3f operator-(const Vector3f& a, const Vector3f& b);
};

#endif // VECTOR3F_H
