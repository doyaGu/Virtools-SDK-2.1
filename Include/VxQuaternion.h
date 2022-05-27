/*************************************************************************/
/*	File : VxQuaternion.h												 */
/*	Author :  Romain SIDIDRIS											 */
/*																		 */
/*	Virtools SDK 														 */
/*	Copyright (c) Virtools 2000, All Rights Reserved.					 */
/*************************************************************************/
#ifndef VXQUATERNION_H
#define VXQUATERNION_H

enum QuatPart
{
    Quat_X,
    Quat_Y,
    Quat_Z,
    Quat_W
};

VX_EXPORT VxQuaternion Vx3DQuaternionSnuggle(VxQuaternion *Quat, VxVector *Scale);
VX_EXPORT VxQuaternion Vx3DQuaternionFromMatrix(const VxMatrix &Mat);
VX_EXPORT VxQuaternion Vx3DQuaternionConjugate(const VxQuaternion &Quat);
VX_EXPORT VxQuaternion Vx3DQuaternionMultiply(const VxQuaternion &QuatL, const VxQuaternion &QuatR);
VX_EXPORT VxQuaternion Vx3DQuaternionDivide(const VxQuaternion &P, const VxQuaternion &Q);
VX_EXPORT VxQuaternion Slerp(float Theta, const VxQuaternion &Quat1, const VxQuaternion &Quat2);
VX_EXPORT VxQuaternion Squad(float Theta, const VxQuaternion &Quat1, const VxQuaternion &Quat1Out, const VxQuaternion &Quat2In, const VxQuaternion &Quat2);
VX_EXPORT VxQuaternion LnDif(const VxQuaternion &P, const VxQuaternion &Q); // Ln(P/Q) = Ln(Q(-1).P)
VX_EXPORT VxQuaternion Ln(const VxQuaternion &Quat);
VX_EXPORT VxQuaternion Exp(const VxQuaternion &Quat);

/**********************************************************
{filename:VxQuaternion}
Name: VxQuaternion

Summary: Class representation of a Quaternion
Remarks:
A Quaternion is defined by 4 floats and is used to represents
an orientation in space. Its common usage is for interpolation
between two orientations through the Slerp() method.

Quaternions can be converted to VxMatrix or Euler Angles.

A VxQuaternion is defined as:

      typedef struct VxQuaternion {
           union {
                struct {
                     float x,y,z,w;
                };
                float v[4];
           };
      }

Elements can be accessed with x,y,z,w value or through the array v.



See Also : VxMatrix,VxVector,Quaternions
*********************************************************/
typedef struct VxQuaternion
{
    // Members
#if !defined(_MSC_VER)
    VxVector axis;
    float angle;
#else
    union
    {
        struct
        {
            VxVector axis;
            float angle;
        };
        struct
        {
            float x, y, z, w;
        };
        float v[4];
    };
#endif

public:
    VxQuaternion()
    {
        axis.x = axis.y = axis.z = 0;
        angle = 1.0f;
    }
    VxQuaternion(const VxVector &Vector, float Angle) { FromRotation(Vector, Angle); }
    VxQuaternion(float X, float Y, float Z, float W)
    {
        axis.x = X;
        axis.y = Y;
        axis.z = Z;
        angle = W;
    }

    VX_EXPORT void FromMatrix(const VxMatrix &Mat, BOOL MatIsUnit = TRUE, BOOL RestoreMat = TRUE);
    VX_EXPORT void ToMatrix(VxMatrix &Mat) const;
    VX_EXPORT void Multiply(const VxQuaternion &Quat);
    VX_EXPORT void FromRotation(const VxVector &Vector, float Angle);
    VX_EXPORT void ToRotation(VxVector &Vector, float &Angle);
    VX_EXPORT void FromEulerAngles(float eax, float eay, float eaz);
    VX_EXPORT void ToEulerAngles(float *eax, float *eay, float *eaz) const;
    VX_EXPORT void Normalize();

    const float &operator[](int i) const;
    float &operator[](int i);

    // Addition and subtraction
    VxQuaternion operator+(const VxQuaternion &q) const { return VxQuaternion(axis.x + q.axis.x, axis.y + q.axis.y, axis.z + q.axis.z, angle + q.angle); }
    VxQuaternion operator-(const VxQuaternion &q) const { return VxQuaternion(axis.x - q.axis.x, axis.y - q.axis.y, axis.z - q.axis.z, angle - q.angle); }
    VxQuaternion operator*(const VxQuaternion &q) const { return Vx3DQuaternionMultiply(*this, q); }
    VxQuaternion operator/(const VxQuaternion &q) const { return Vx3DQuaternionDivide(*this, q); }

    // Float operator
    friend VxQuaternion operator*(float, const VxQuaternion &);
    friend VxQuaternion operator*(const VxQuaternion &, float);
    VxQuaternion &operator*=(float s)
    {
        axis.x *= s;
        axis.y *= s;
        axis.z *= s;
        angle *= s;
        return *this;
    }

    VxQuaternion operator-() const { return (VxQuaternion(-axis.x, -axis.y, -axis.z, -angle)); }
    VxQuaternion operator+() const { return *this; }

    // Bitwise equality
    friend int operator==(const VxQuaternion &q1, const VxQuaternion &q2);
    friend int operator!=(const VxQuaternion &q1, const VxQuaternion &q2);

    friend float Magnitude(const VxQuaternion &q);
    friend float DotProduct(const VxQuaternion &p, const VxQuaternion &q);
} VxQuaternion;

inline int operator==(const VxQuaternion &q1, const VxQuaternion &q2)
{
    return (q1.axis.x == q2.axis.x && q1.axis.y == q2.axis.y && q1.axis.z == q2.axis.z && q1.angle == q2.angle);
}

inline int operator!=(const VxQuaternion &q1, const VxQuaternion &q2)
{
    return (q1.axis.x != q2.axis.x || q1.axis.y != q2.axis.y || q1.axis.z != q2.axis.z || q1.angle != q2.angle);
}

inline VxQuaternion operator*(float s, const VxQuaternion &q)
{
    return VxQuaternion(q.axis.x * s, q.axis.y * s, q.axis.z * s, q.angle * s);
}

inline VxQuaternion operator*(const VxQuaternion &q, float s)
{
    return VxQuaternion(q.axis.x * s, q.axis.y * s, q.axis.z * s, q.angle * s);
}

inline float Magnitude(const VxQuaternion &q)
{
    return (q.axis.x * q.axis.x + q.axis.y * q.axis.y + q.axis.z * q.axis.z + q.angle * q.angle);
}

inline float DotProduct(const VxQuaternion &q1, const VxQuaternion &q2)
{
    return (q1.axis.x * q2.axis.x + q1.axis.y * q2.axis.y + q1.axis.z * q2.axis.z + q1.angle * q2.angle);
}

inline const float &VxQuaternion::operator[](int i) const
{
    return *((&axis.x) + i);
}

inline float &VxQuaternion::operator[](int i)
{
    return *((&axis.x) + i);
}

#endif
