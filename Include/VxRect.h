/*************************************************************************/
/*	File : VxRect.h														 */
/*	Author :  Aymeric Bard												 */
/*																		 */
/*	Virtools SDK 														 */
/*	Copyright (c) Virtools 2000, All Rights Reserved.					 */
/*************************************************************************/
#ifndef VXRECT_H
#define VXRECT_H

typedef enum VXRECT_INTERSECTION
{
    ALLOUTSIDE = 0,
    ALLINSIDE  = 1,
    PARTINSIDE = 2
} VXRECT_INTERSECTION;

/**********************************************************
{filename:VxRect}
Name: VxRect

Remarks:
A Rect is defined by 4 floats and is used to represents
a 2D region.

A VxRect is defined as:

        class VxRect
        {
        public:
            float left;
            float top;
            float right;
            float bottom;
        };

Elements can be accessed directly or by using the accessors functions
for more sophisticate access.



See Also :
*********************************************************/
class VxRect
{
public:
    // Members
#if !defined(_MSC_VER)
    Vx2DVector m_TopLeft;
    Vx2DVector m_BottomRight;
#else
    union
    {
        struct
        {
            float left;
            float top;
            float right;
            float bottom;
        };
        struct
        {
            Vx2DVector m_TopLeft;
            Vx2DVector m_BottomRight;
        };
    };
#endif

    // Methods
    VxRect(){};
    VxRect(Vx2DVector &topleft, Vx2DVector &bottomright) : m_TopLeft(topleft), m_BottomRight(bottomright) {}
    VxRect(float l, float t, float r, float b) : m_TopLeft(l, t), m_BottomRight(r, b) {}

    /************************************************
    Summary: Changes the width of a rectangle.

    Input Arguments:
        w: new width in float.
    ************************************************/
    void SetWidth(float w) { m_BottomRight.x = m_TopLeft.x + w; };

    /************************************************
    Summary: Returns the width of a rectangle.
    ************************************************/
    float GetWidth() const { return m_BottomRight.x - m_TopLeft.x; }

    /************************************************
    Summary: Changes the height of a rectangle.

    Input Arguments:
        h: new height in float.
    ************************************************/
    void SetHeight(float h) { m_BottomRight.y = m_TopLeft.y + h; }

    /************************************************
    Summary: Returns the height of a rectangle.
    ************************************************/
    float GetHeight() const { return m_BottomRight.y - m_TopLeft.y; }

    /************************************************
    Summary: Returns the horizontal center of the rect.
    ************************************************/
    float GetHCenter() const { return m_TopLeft.x + 0.5f * GetWidth(); }

    /************************************************
    Summary: Returns the vertical center of the rect.
    ************************************************/
    float GetVCenter() const { return m_TopLeft.y + 0.5f * GetHeight(); }

    /************************************************
    Summary: Changes the size of a rectangle.

    Input Arguments:
        v: new size in the form of a Vx2DVector.
    ************************************************/
    void SetSize(const Vx2DVector &v)
    {
        SetWidth(v.x);
        SetHeight(v.y);
    }

    /************************************************
    Summary: Returns the size of a rectangle.
    ************************************************/
    Vx2DVector GetSize() const { return Vx2DVector(GetWidth(), GetHeight()); }

    void SetHalfSize(const Vx2DVector &v)
    {
        Vx2DVector c = GetCenter();
        SetCenter(c, v);
    }
    Vx2DVector GetHalfSize() const { return Vx2DVector(0.5f * GetWidth(), 0.5f * GetHeight()); }

    void SetCenter(const Vx2DVector &v)
    {
        Vx2DVector hs = GetHalfSize();
        SetCenter(v, hs);
    }
    Vx2DVector GetCenter() const { return Vx2DVector(GetHCenter(), GetVCenter()); }

    void SetTopLeft(const Vx2DVector &v)
    {
        m_TopLeft = v;
    }
    const Vx2DVector &GetTopLeft() const { return m_TopLeft; }
    Vx2DVector &GetTopLeft() { return m_TopLeft; }

    void SetBottomRight(const Vx2DVector &v)
    {
        m_BottomRight = v;
    }
    const Vx2DVector &GetBottomRight() const { return m_BottomRight; }
    Vx2DVector &GetBottomRight() { return m_BottomRight; }

    /************************************************
    Summary: Sets the rectangle as a NULL rectangle
    (position (0,0) ;  size (0,0).
    ************************************************/
    void Clear() { SetCorners(0, 0, 0, 0); }

    /*************************************************
    Summary: Creates a rectangle based on two corners.

    Input Arguments:
        topleft: a Vx2DVector containing the top left corner.
        bottomright: a Vx2DVector containing the bottom right corner.
        t: top in float.
        l: left in float.
        b: bottom in float.
        r: right in float.
    See also: VxRect::SetDimension,VxRect::SetCenter
    *************************************************/
    void SetCorners(const Vx2DVector &topleft, const Vx2DVector &bottomright)
    {
        m_TopLeft = topleft;
        m_BottomRight = bottomright;
    }
    void SetCorners(float l, float t, float r, float b)
    {
        m_TopLeft.x = l;
        m_TopLeft.y = t;
        m_BottomRight.x = r;
        m_BottomRight.y = b;
    }

    /*************************************************
    Summary: Creates a rectangle based on the top left coner and the size.

    Input Arguments:
    position: a Vx2DVector containing the top left corner.
    size: a Vx2DVector containing the size.
    x: left position in float.
    y: top position in float.
    w: width in float.
    y: height in float.



    See also: VxRect::SetCorners,VxRect::SetCenter
    *************************************************/
    void SetDimension(const Vx2DVector &position, const Vx2DVector &size)
    {
        m_TopLeft.x = position.x;
        m_TopLeft.y = position.y;
        m_BottomRight.x = m_TopLeft.x + size.x;
        m_BottomRight.y = m_TopLeft.y + size.y;
    }
    void SetDimension(float x, float y, float w, float h)
    {
        m_TopLeft.x = x;
        m_TopLeft.y = y;
        m_BottomRight.x = x + w;
        m_BottomRight.y = y + h;
    }

    /*************************************************
    Summary: Creates a rectangle based on the center position and the half size.

    Input Arguments:
    position: a Vx2DVector containing the center position.
    halfsize: a Vx2DVector containing half size.
    cx: horizontal center position in float.
    cy: vertical center position in float.
    hw: half width in float.
    hy: half height in float.



    See also: VxRect::SetCorners,VxRect::SetDimension
    *************************************************/
    void SetCenter(const Vx2DVector &center, const Vx2DVector &halfsize)
    {
        m_TopLeft.x = center.x - halfsize.x;
        m_TopLeft.y = center.y - halfsize.y;
        m_BottomRight.x = center.x + halfsize.x;
        m_BottomRight.y = center.y + halfsize.y;
    }
    void SetCenter(float cx, float cy, float hw, float hh)
    {
        m_TopLeft.x = cx - hw;
        m_TopLeft.y = cy - hh;
        m_BottomRight.x = cx + hw;
        m_BottomRight.y = cy + hh;
    }

    /*************************************************
    Summary: Assign the VxRect to a CKRECT.

    Input Arguments:
    iRect: the rectangle to copy from.
    *************************************************/
    void CopyFrom(const CKRECT &iRect)
    {
        m_TopLeft.x = (float)iRect.left;
        m_TopLeft.y = (float)iRect.top;
        m_BottomRight.x = (float)iRect.right;
        m_BottomRight.y = (float)iRect.bottom;
    }

    /*************************************************
    Summary: Assign the VxRect to a CKRECT.

    Input Arguments:
    oRect: the rectangle to copy to.
    *************************************************/
    void CopyTo(CKRECT *oRect) const
    {
        XASSERT(oRect);

        oRect->left = (int)m_TopLeft.x;
        oRect->top = (int)m_TopLeft.y;
        oRect->right = (int)m_BottomRight.x;
        oRect->bottom = (int)m_BottomRight.y;
    }

    /*************************************************
    Summary: Creates a rectangle bounding two given points.

    Input Arguments:
    p1: the first Vx2DVector.
    p2: the second Vx2DVector.



    See also: VxRect::SetCorners,VxRect::SetDimension
    *************************************************/
    void Bounding(const Vx2DVector &p1, const Vx2DVector &p2)
    {
        if (p1.x < p2.x)
        {
            m_TopLeft.x = p1.x;
            m_BottomRight.x = p2.x;
        }
        else
        {
            m_TopLeft.x = p2.x;
            m_BottomRight.x = p1.x;
        }
        if (p1.y < p2.y)
        {
            m_TopLeft.y = p1.y;
            m_BottomRight.y = p2.y;
        }
        else
        {
            m_TopLeft.y = p2.y;
            m_BottomRight.y = p1.y;
        }
    }

    /*************************************************
    Summary: Checks that the rectangle is valid. If not, turns it right.



    Remarks:
        Use this function when you create a rectangle with two
    corners without being sure that you provided the top left
    and top right corners, if this order.

    See also: VxRect::SetCorners,VxRect::SetDimension
    *************************************************/
    void Normalize()
    {
        // Check horizontally
        if (m_TopLeft.x > m_BottomRight.x)
            XSwap(m_BottomRight.x, m_TopLeft.x);

        // Check vertically
        if (m_TopLeft.y > m_BottomRight.y)
            XSwap(m_TopLeft.y, m_BottomRight.y);
    }

    /*************************************************
    Summary: Move a rectangle to a position.

    Input Arguments:
    pos: new position of the rectangle in Vx2DVector.



    See also: VxRect::Translate
    *************************************************/
    void Move(const Vx2DVector &pos)
    {
        m_BottomRight.x += (pos.x - m_TopLeft.x);
        m_BottomRight.y += (pos.y - m_TopLeft.y);
        m_TopLeft.x = pos.x;
        m_TopLeft.y = pos.y;
    }

    /*************************************************
    Summary: Translate a rectangle of an offset.

    Input Arguments:
    t: translation vector in Vx2DVector.



    See also: VxRect::Move
    *************************************************/
    void Translate(const Vx2DVector &t)
    {
        m_TopLeft.x += t.x;
        m_TopLeft.y += t.x;
        m_BottomRight.x += t.y;
        m_BottomRight.y += t.y;
    }

    void HMove(float h)
    {
        m_BottomRight.x += (h - m_TopLeft.x);
        m_TopLeft.x = h;
    }

    void VMove(float v)
    {
        m_BottomRight.y += (v - m_TopLeft.y);
        m_TopLeft.y = v;
    }

    void HTranslate(float h)
    {
        m_BottomRight.x += h;
        m_TopLeft.x += h;
    }

    void VTranslate(float v)
    {
        m_BottomRight.y += v;
        m_TopLeft.y += v;
    }

    /*************************************************
    Summary: Transform a point in homogeneous coordinates
    into a points in screen coordinates (the rect representing the screen).

    Input Arguments:
    dest: vector destination (in screen coordinates).
    srchom: src vector (in homogeneous coordinates).



    See also: VxRect::Move
    *************************************************/
    void TransformFromHomogeneous(Vx2DVector &dest, const Vx2DVector &srchom) const
    {
        dest.x = m_TopLeft.x + GetWidth() * srchom.x;
        dest.y = m_TopLeft.x + GetHeight() * srchom.y;
    }

    /*************************************************
    Summary: Scales a rectangle by a factor.

    Input Arguments:
    t: translation vector in Vx2DVector.



    See also: VxRect::Inflate
    *************************************************/
    void Scale(const Vx2DVector &s)
    {
        SetWidth(s.x * GetWidth());
        SetHeight(s.y * GetHeight());
    }

    /*************************************************
    Summary: Inflates a rectangle by an offset.

    Input Arguments:



    See also: VxRect::Scale
    *************************************************/
    void Inflate(const Vx2DVector &pt)
    {
        m_TopLeft.x -= pt.x;
        m_BottomRight.x += pt.x;
        m_TopLeft.y -= pt.y;
        m_BottomRight.y += pt.y;
    }

    /*************************************************
    Summary: Interpolates a rectangle with another one.

    Input Arguments:



    See also: VxRect::Scale
    *************************************************/
    void Interpolate(float value, const VxRect &a)
    {
        m_TopLeft.x += (a.m_TopLeft.x - m_TopLeft.x) * value;
        m_BottomRight.x += (a.m_BottomRight.x - m_BottomRight.x) * value;
        m_TopLeft.y += (a.m_TopLeft.y - m_TopLeft.y) * value;
        m_BottomRight.y += (a.m_BottomRight.y - m_BottomRight.y) * value;
    }

    /*************************************************
    Summary: Merge a rectangle with another one.

    Input Arguments:



    See also: VxRect::Scale
    *************************************************/
    void Merge(const VxRect &a)
    {
        if (a.m_TopLeft.x < m_TopLeft.x)
            m_TopLeft.x = a.m_TopLeft.x;
        if (a.m_BottomRight.x > m_BottomRight.x)
            m_BottomRight.x = a.m_BottomRight.x;
        if (a.m_TopLeft.y < m_TopLeft.y)
            m_TopLeft.y = a.m_TopLeft.y;
        if (a.m_BottomRight.y > m_BottomRight.y)
            m_BottomRight.y = a.m_BottomRight.y;
    }

    /*************************************************
    Summary: Tests a rectangle over a clipping rectangle.

    Input Arguments:
    cliprect: clipping rectangle.

    Return Value:
        ALLOUTSIDE if the two rectangles are distinct.
        PARTINSIDE if the two rectangles are crossing.
        ALLINSIDE  if the tested rectangle is inside the clipping rectangle.



    See also: VxRect::IsOutside
    *************************************************/
    int IsInside(const VxRect &cliprect) const
    {
        // entirely clipped
        if (m_TopLeft.x >= cliprect.m_BottomRight.x)
            return ALLOUTSIDE;
        if (m_BottomRight.x < cliprect.m_TopLeft.x)
            return ALLOUTSIDE;
        if (m_TopLeft.y >= cliprect.m_BottomRight.y)
            return ALLOUTSIDE;
        if (m_BottomRight.y < cliprect.m_TopLeft.y)
            return ALLOUTSIDE;

        // partially or not clipped
        if (m_TopLeft.x < cliprect.m_TopLeft.x)
            return PARTINSIDE;
        if (m_BottomRight.x > cliprect.m_BottomRight.x)
            return PARTINSIDE;
        if (m_TopLeft.y < cliprect.m_TopLeft.y)
            return PARTINSIDE;
        if (m_BottomRight.y > cliprect.m_BottomRight.y)
            return PARTINSIDE;

        return ALLINSIDE;
    }

    /*************************************************
    Summary: Tests a rectangle over a clipping rectangle.

    Input Arguments:
    cliprect: clipping rectangle.

    Return Value:
        TRUE if the rectangle is outside the clipping one,
    FALSE otherwise.



    See also: VxRect::IsInside
    *************************************************/
    BOOL IsOutside(const VxRect &cliprect) const
    {
        // entirely clipped
        if (m_TopLeft.x >= cliprect.m_BottomRight.x)
            return TRUE;
        if (m_BottomRight.x < cliprect.m_TopLeft.x)
            return TRUE;
        if (m_TopLeft.y >= cliprect.m_BottomRight.y)
            return TRUE;
        if (m_BottomRight.y < cliprect.m_TopLeft.y)
            return TRUE;

        return FALSE;
    }

    /*************************************************
    Summary: Tests a point for rectangle interiority.

    Input Arguments:
    pt: Vx2DVector to test.

    Return Value:
        TRUE if the point is inside, FALSE otherwise.



    See also: VxRect::IsOutside
    *************************************************/
    BOOL IsInside(const Vx2DVector &pt) const
    {
        if (pt.x < m_TopLeft.x)
            return FALSE;
        if (pt.x > m_BottomRight.x)
            return FALSE;
        if (pt.y < m_TopLeft.y)
            return FALSE;
        if (pt.y > m_BottomRight.y)
            return FALSE;
        return TRUE;
    }

    /*************************************************
    Summary: Tests a rectangle for validity.

    Return Value:
        TRUE if the rectangle is NULL, FALSE otherwise.



    See also: VxRect::IsEmpty, VxRect::IsOutside, VxRect::Clear
    *************************************************/
    BOOL IsNull() const { return (m_TopLeft.x == 0 && m_BottomRight.x == 0 && m_BottomRight.y == 0 && m_TopLeft.y == 0); }

    /*************************************************
    Summary: Tests if a rectangle is Empty (width==0 or height==0)

    Return Value:
        TRUE if the rectangle is Empty, FALSE otherwise.



    See also: VxRect::IsNull, VxRect::IsOutside, VxRect::Clear
    *************************************************/
    BOOL IsEmpty() const { return ((m_TopLeft.x == m_BottomRight.x) || (m_BottomRight.y == m_TopLeft.y)); }

    /*************************************************
    Summary: Clips a rectangle over a clipping rectangle.

    Input Arguments:
    cliprect: clipping rectangle.

    Return Value:
        TRUE if the rectangle is visible (partially clipped or not),
    FALSE otherwise.



    See also: VxRect::IsOutside,VxRect::IsInside
    *************************************************/
    BOOL Clip(const VxRect &cliprect)
    {
        // entirely clipped
        if (IsOutside(cliprect))
            return FALSE;

        // partially or not clipped
        if (m_TopLeft.x < cliprect.m_TopLeft.x)
            m_TopLeft.x = cliprect.m_TopLeft.x;
        if (m_BottomRight.x > cliprect.m_BottomRight.x)
            m_BottomRight.x = cliprect.m_BottomRight.x;
        if (m_TopLeft.y < cliprect.m_TopLeft.y)
            m_TopLeft.y = cliprect.m_TopLeft.y;
        if (m_BottomRight.y > cliprect.m_BottomRight.y)
            m_BottomRight.y = cliprect.m_BottomRight.y;

        return TRUE;
    }

    /*************************************************
    Summary: Clips a point over a rectangle.

    Input Arguments:
    pt: point to clip
    excluderightbottom: should the bottom and right line be considered as valid values for the point.



    See also: VxRect::IsOutside,VxRect::IsInside
    *************************************************/
    void Clip(Vx2DVector &pt, BOOL excluderightbottom = TRUE) const
    {
        if (pt.x < m_TopLeft.x)
            pt.x = m_TopLeft.x;
        else
        {
            if (pt.x >= m_BottomRight.x)
            {
                if (excluderightbottom)
                    pt.x = m_BottomRight.x - 1;
                else
                    pt.x = m_BottomRight.x;
            }
        }
        if (pt.y < m_TopLeft.y)
            pt.y = m_TopLeft.y;
        else
        {
            if (pt.y >= m_BottomRight.y)
            {
                if (excluderightbottom)
                    pt.y = m_BottomRight.y - 1;
                else
                    pt.y = m_BottomRight.y;
            }
        }
    }

    /*************************************************
    Summary: Transforms a rectangle from a screen referentiel to another.

    Input Arguments:
    destscreen: destination referential screen defined by a rectangle.
    srcscreen: source referential screen defined by a rectangle.
    destscreen: destination referential screen size.
    srcscreen: source referential screen size.



    See also: VxRect::TransformToHomogeneous
    *************************************************/
    VX_EXPORT void Transform(const VxRect &destScreen, const VxRect &srcScreen);
    VX_EXPORT void Transform(const Vx2DVector &destScreenSize, const Vx2DVector &srcScreenSize);

    /*************************************************
    Summary: Transforms a screen coordinate rectangle to an homogeneous rectangle.

    Input Arguments:
    screen: Vx2DVector screen size.



    See also: VxRect::Transform
    *************************************************/
    VX_EXPORT void TransformToHomogeneous(const VxRect &screen);

    /*************************************************
    Summary: Transforms an homogeneous rectangle to a screen coordinate rectangle.

    Input Arguments:
    screen: Vx2DVector screen size.



    See also: VxRect::Transform
    *************************************************/
    VX_EXPORT void TransformFromHomogeneous(const VxRect &screen);

    // Operator with 2DVectors

    VxRect &operator+=(const Vx2DVector &t)
    {
        m_TopLeft.x += t.x;
        m_BottomRight.x += t.x;
        m_TopLeft.y += t.y;
        m_BottomRight.y += t.y;
        return *this;
    }
    VxRect &operator-=(const Vx2DVector &t)
    {
        m_TopLeft.x -= t.x;
        m_BottomRight.x -= t.x;
        m_TopLeft.y -= t.y;
        m_BottomRight.y -= t.y;
        return *this;
    }
    VxRect &operator*=(const Vx2DVector &t)
    {
        m_TopLeft.x *= t.x;
        m_BottomRight.x *= t.x;
        m_TopLeft.y *= t.y;
        m_BottomRight.y *= t.y;
        return *this;
    }
    VxRect &operator/=(const Vx2DVector &t)
    {
        float x = 1.0f / t.x;
        float y = 1.0f / t.y;
        m_TopLeft.x *= x;
        m_BottomRight.x *= x;
        m_TopLeft.y *= y;
        m_BottomRight.y *= y;
        return *this;
    }

    //
    friend int operator==(const VxRect &r1, const VxRect &r2);

    //
    friend int operator!=(const VxRect &r1, const VxRect &r2);
};

//
inline int operator==(const VxRect &r1, const VxRect &r2)
{
    return (r1.m_TopLeft.x == r2.m_TopLeft.x && r1.m_BottomRight.x == r2.m_BottomRight.x && r1.m_TopLeft.y == r2.m_TopLeft.y && r1.m_BottomRight.y == r2.m_BottomRight.y);
}

//
inline int operator!=(const VxRect &r1, const VxRect &r2)
{
    return !(r1 == r2);
}

#endif
