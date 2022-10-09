#ifndef XSTRING_H
#define XSTRING_H

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

#include "VxMathDefines.h"
#include "XUtil.h"

class XString;

/************************************************
{filename:XBaseString}
Summary: Wrapper around a C string (a char* or const char*).

Remarks:

    o This class does not duplicate the data it is given and does not delete
    the string it points to when destroyed.
    o It calculates and store the length, for further faster access.



See Also : XBaseString.
************************************************/
class XBaseString
{
    friend class XString;

public:
    /************************************************
    Summary: Constructors.

    Arguments:
        iString: the string to hold.
    ************************************************/
    XBaseString() : m_Buffer(0), m_Length(0), m_Allocated(0) {}

    // Ctor from a const string literal
    // Warning : the size is calculated here
    XBaseString(const char *iString)
    {
        if (iString)
        {
            m_Buffer = (char *)iString;
            m_Length = 0xffff;
            while (m_Buffer[++m_Length]);
        }
        else
        {
            m_Buffer = NULL;
            m_Length = 0;
        }
    }

    // Dtor
    ~XBaseString() {} // Nothing to do !

    /************************************************
    Summary: Returns the length of the string.
    ************************************************/
    XWORD Length() const
    {
        return m_Length;
    }

    /************************************************
    Summary: Conversion to a const char* (read only).
    ************************************************/
    const char *CStr() const
    {
        return (m_Buffer) ? m_Buffer : "";
    }

    /************************************************
    Summary: Conversion to a int.
    ************************************************/
    int ToInt() const
    {
        return (m_Buffer) ? atoi(m_Buffer) : 0;
    }

    /************************************************
    Summary: Conversion to a float.
    ************************************************/
    float ToFloat() const
    {
        return (m_Buffer) ? (float)atof(m_Buffer) : 0.0f;
    }

    /************************************************
    Summary: Conversion to a double.
    ************************************************/
    double ToDouble() const
    {
        return (m_Buffer) ? atof(m_Buffer) : 0.0;
    }

    /************************************************
    Summary: access to a character (read only)

    Arguments:
        i: index of the character to read.
    ************************************************/
    const char operator[](XWORD i) const
    {
        XASSERT(i < m_Length);
        return m_Buffer[i];
    }

    const char operator[](int i) const
    {
        XASSERT(i < m_Length);
        return m_Buffer[i];
    }

protected:
    // the string
    char *m_Buffer;
    // the length of the string
    XWORD m_Length;
    // the allocated size
    XWORD m_Allocated;
};

/************************************************
Summary: Class representation of a string (an array of character ended by a '\0').
{filename:XString}
Remarks:
    This class always duplicate the data it is given.



See Also : XBaseString.
************************************************/
class XString : public XBaseString
{
public:
    enum { NOTFOUND = 0xffff };

    // Default Ctor
    XString() : XBaseString() {}

    // Substring Ctor
    XString(const char *iString, const int iLength = 0) : XBaseString()
    {
        if (!iString || *iString == '\0')
        {
            m_Buffer = NULL;
            m_Length = 0;
            return;
        }

        m_Length = (iLength > 0) ? iLength : (XWORD)strlen(iString);
        m_Allocated = m_Length + 1;
        m_Buffer = (char *)VxNew(sizeof(char) * m_Allocated);
        strncpy(m_Buffer, iString, m_Length);
    }

    // Reserving Ctor
    explicit XString(const int iLength) : XBaseString()
    {
        if (iLength == 0)
        {
            m_Buffer = NULL;
            m_Length = 0;
            return;
        }

        m_Length = iLength - 1;
        m_Allocated = iLength;
        m_Buffer = (char *)VxNew(sizeof(char) * iLength);
        memset(m_Buffer, 0, m_Allocated);
    }

    // Copy Ctor
    XString(const XString &iSrc) : XBaseString()
    {
        if (iSrc.m_Length == 0)
        {
            m_Buffer = NULL;
            m_Length = 0;
            return;
        }

        m_Length = iSrc.m_Length;
        m_Allocated = iSrc.m_Length + 1;
        m_Buffer = (char *)VxNew(sizeof(char) * m_Allocated);
        memcpy(m_Buffer, iSrc.m_Buffer, m_Allocated);
    }

    // Copy Ctor
    XString(const XBaseString &iSrc) : XBaseString()
    {
        if (iSrc.m_Length == 0)
        {
            m_Buffer = NULL;
            m_Length = 0;
            return;
        }

        m_Length = iSrc.m_Length;
        m_Allocated = iSrc.m_Length + 1;
        m_Buffer = (char *)VxNew(sizeof(char) * m_Allocated);
        memcpy(m_Buffer, iSrc.m_Buffer, m_Allocated);
    }

    // Dtor
    ~XString() { VxDelete(m_Buffer); }

    // operator =
    XString &operator=(const XString &iSrc)
    {
        if (this != &iSrc)
            Assign(iSrc.m_Buffer, iSrc.m_Length);
        return *this;
    }

    // operator =
    XString &operator=(const char *iSrc)
    {
        if (iSrc)
        {
            Assign(iSrc, strlen(iSrc));
        }
        else
        {
            m_Length = 0;
            if (m_Buffer)
                m_Buffer[0] = '\0';
        }
        return *this;
    }

    // operator =
    XString &operator=(const XBaseString &iSrc)
    {
        if (this != &iSrc)
            Assign(iSrc.m_Buffer, iSrc.m_Length);
        return *this;
    }

    // Create from a string and a length
    XString &Create(const char *iString, const int iLength = 0)
    {
        CheckSize(iLength);
        m_Length = 0;
        if (iString)
        {
            m_Length = iLength;
            strncpy(m_Buffer, iString, iLength);
        }
        return *this;
    }

    /************************************************
    Summary: Conversion to a char*.
    ************************************************/
    char *Str()
    {
        return (m_Buffer) ? m_Buffer : (char *)"";
    }

    /************************************************
    Summary: access to a character (read/write)

    Arguments:
        i: index of the character to read/write.
    ************************************************/
    char &operator[](XWORD i)
    {
        XASSERT(i < m_Length);
        return m_Buffer[i];
    }

    char &operator[](int i)
    {
        XASSERT(i < m_Length);
        return m_Buffer[i];
    }

    /************************************************
    Summary: access to a character (read only)

    Arguments:
        i: index of the character to read.
    ************************************************/
    char operator[](XWORD i) const
    {
        XASSERT(i < m_Length);
        return m_Buffer[i];
    }

    char operator[](int i) const
    {
        XASSERT(i < m_Length);
        return m_Buffer[i];
    }

    // Format the string sprintf style, ol skool ! yeahhhhh
    XString &Format(const char *iFormat, ...)
    {
        va_list args;
        va_start(args, iFormat);
        char buf[512];
        int len = vsprintf(buf, iFormat, args);
        Assign(buf, len);
        va_end(args);
        return *this;
    }

    // Capitalize all the characters of the string
    XString &ToUpper()
    {
        for (XWORD i = 0; i < m_Length; i++)
            m_Buffer[i] = toupper(m_Buffer[i]);
        return *this;
    }

    // Uncapitalize all the characters of the string
    XString &ToLower()
    {
        for (XWORD i = 0; i < m_Length; i++)
            m_Buffer[i] = tolower(m_Buffer[i]);
        return *this;
    }

    // Compare the strings.
    /*************************************************
    Summary: Compare two strings.

    Arguments:
        iStr: string to compare with the current object.

    Return Value:
        < 0 the current string is lesser than iStr,
        0 the current string is identical to iStr,
        > 0 the current string is greater than iStr.

    See also: ICompare.
    *************************************************/
    int Compare(const XBaseString &iStr) const
    {
        if (!m_Length)
            return -iStr.m_Length; // Null strings
        if (!iStr.m_Length)
            return m_Length;

        char *s1 = m_Buffer;
        char *s2 = iStr.m_Buffer;

        int Lenght4 = (m_Length > iStr.m_Length) ? (iStr.m_Length >> 2) : (m_Length >> 2);
        //--- Compare dwords by dwords....
        while ((Lenght4-- > 0) && (*(XDWORD *)s1 == *(XDWORD *)s2))
            s1 += 4, s2 += 4;

        //----- remaining bytes...
        while ((*s1 == *s2) && *s1)
            ++s1, ++s2;
        return (*s1 - *s2);
    }

    /*************************************************
    Summary: Compare the n first character of two strings.

    Arguments:
        iStr: string to compare with the current object.
        iN: n first character to compare

    Return Value:
        < 0 the current string is lesser than iStr,
        0 the current string is identical to iStr,
        > 0 the current string is greater than iStr.

    See also: ICompare.
    *************************************************/
    int NCompare(const XBaseString &iStr, const int iN) const
    {
        if (!m_Length)
            return -iStr.m_Length; // Null strings
        if (!iStr.m_Length)
            return m_Length;

        return strncmp(m_Buffer, iStr.m_Buffer, iN);
    }

    // Compare the strings (ignore the case).
    int ICompare(const XBaseString &iStr) const
    {
        if (!m_Length)
            return -iStr.m_Length; // Null strings
        if (!iStr.m_Length)
            return m_Length;

        char *s1 = m_Buffer;
        char *s2 = iStr.m_Buffer;

        char c1, c2;
        do
        {
            c1 = tolower(*(s1++));
            c2 = tolower(*(s2++));
        } while (*s1 != '\0' && (c1 == c2));

        return tolower(*s1) - tolower(*s2);
    }

    ///
    // Complex operations on strings

    // removes space ( ' ', '\t' and '\n') from the start and the end of the string
    XString &Trim()
    {
        if (m_Length != 0)
        {
            while (isspace(m_Buffer[m_Length - 1]) && m_Length != 0)
                --m_Length;
            m_Buffer[m_Length] = '\0';

            if (m_Length != 0)
            {
                XWORD i = 0;
                while (isspace(m_Buffer[i]) && i != m_Length)
                    ++i;
                if (i != 0)
                {
                    m_Length -= i;
                    strncpy(m_Buffer, &m_Buffer[i], m_Length + 1);
                }
            }
        }
        return *this;
    }

    // replaces space ( ' ', '\t' and '\n') sequences by one space
    XString &Strip()
    {
        if (m_Length != 0)
        {
            XWORD i;
            XBOOL wasSpace = FALSE;
            for (i = 0; i < m_Length; i++)
            {
                if (isspace(m_Buffer[i]))
                {
                    if (wasSpace)
                        break;
                    wasSpace = TRUE;
                }
                else
                {
                    wasSpace = FALSE;
                }
            }

            for (XWORD j = i + 1; j < m_Length; j++)
            {
                if (isspace(m_Buffer[j]))
                {
                    if (!wasSpace)
                    {
                        wasSpace = TRUE;
                        m_Buffer[i++] = ' ';
                    }
                }
                else
                {
                    wasSpace = FALSE;
                    m_Buffer[i++] = m_Buffer[j];
                }
            }

            m_Length = i;
            m_Buffer[i] = '\0';
        }
        return *this;
    }

    // finds a string in the string
    XBOOL Contains(const XBaseString &iStr) const
    {
        return Find(iStr) != NOTFOUND;
    }

    // finds a character in the string
    XWORD Find(char iCar, XWORD iStart = 0) const
    {
        if (m_Length != 0)
        {
            char *str = strchr(&m_Buffer[iStart], iCar);
            if (str)
                return str - m_Buffer;
            else
                return -1;
        }
        return -1;
    }

    // finds a string in the string
    XWORD Find(const XBaseString &iStr, XWORD iStart = 0) const
    {
        if (m_Length != 0 && iStr.m_Length != 0)
        {
            char *str = strstr(&m_Buffer[iStart], iStr.m_Buffer);
            if (str)
                return str - m_Buffer;
            else
                return -1;
        }
        return -1;
    }

    // finds a character in the string
    XWORD RFind(char iCar, XWORD iStart = NOTFOUND) const
    {
        if (m_Length != 0)
        {
            XWORD i = (iStart != NOTFOUND) ? iStart : m_Length;

            char ch = m_Buffer[i];
            m_Buffer[i] = '\0';
            char *buf = strrchr(&m_Buffer[iStart], iCar);
            m_Buffer[i] = ch;
            if (buf)
                return buf - m_Buffer;
            else
                return -1;
        }
        return -1;
    }

    // creates a substring
    XString Substring(XWORD iStart, XWORD iLength = 0) const
    {
        return XString(&m_Buffer[iStart], (iLength != 0) ? iLength : m_Length - iStart);
    }

    // crops the string
    XString &Crop(XWORD iStart, XWORD iLength)
    {
        if (iStart != 0)
            strncpy(m_Buffer, &m_Buffer[iStart], iLength);
        m_Length = iLength;
        m_Buffer[iLength] = '\0';
        return *this;
    }

    // cuts the string
    XString &Cut(XWORD iStart, XWORD iLength)
    {
        strncpy(&m_Buffer[iStart], &m_Buffer[iLength + iStart], m_Length - (iLength + iStart) + 1);
        m_Length -= iLength;
        return *this;
    }

    // replaces all the occurrence of a character by another one
    int Replace(char iSrc, char iDest)
    {
        int count = 0;
        for (XWORD i = 0; i < m_Length; ++i)
        {
            if (m_Buffer[i] == iSrc)
            {
                m_Buffer[i] = iDest;
                ++count;
            }
        }
        return count;
    }

    // replaces all the occurrence of a string by another string
    int Replace(const XBaseString &iSrc, const XBaseString &iDest)
    {
        int count = 0;
        char *src = NULL;

        if (iSrc.m_Length == iDest.m_Length)
        {
            src = strstr(m_Buffer, iSrc.m_Buffer);
            if (!src)
                return 0;

            do
            {
                strncpy(src, iDest.m_Buffer, iDest.m_Length);
                ++count;
            } while ((src = strstr(&src[iSrc.m_Length], iSrc.m_Buffer)));
            return count;
        }
        else
        {
            src = strstr(m_Buffer, iSrc.m_Buffer);
            if (src)
            {
                do
                {
                    ++count;
                } while ((src = strstr(&src[iSrc.m_Length], iSrc.m_Buffer)));
            }

            XWORD len = m_Length + count * (iDest.m_Length - iSrc.m_Length);
            char *buf = (char *)VxNew(sizeof(char) * (len + 1));

            char *s1 = m_Buffer;
            char *s2 = buf;

            src = strstr(m_Buffer, iSrc.m_Buffer);
            if (src)
            {
                do
                {
                    strncpy(s2, s1, src - s1);
                    strncpy(&s2[src - s1], iDest.m_Buffer, iDest.m_Length);
                    s2 = &s2[iDest.m_Length];
                    s1 = &src[iSrc.m_Length];
                } while ((src = strstr(s1, iSrc.m_Buffer)));
            }
            strncpy(s2, s1, (m_Buffer - s1) + 1);

            VxDelete(m_Buffer);
            m_Length = len;
            m_Buffer = buf;
            m_Allocated = len + 1;
            return count;
        }
    }

    ///
    // Comparison operators

    /************************************************
    Summary: Operators for strings comparisons.
    ************************************************/
    int operator==(const char *iStr) const
    {
        return !Compare(iStr);
    }

    int operator!=(const char *iStr) const
    {
        return Compare(iStr);
    }

    int operator<(const char *iStr) const
    {
        return Compare(iStr) < 0;
    }

    int operator<=(const char *iStr) const
    {
        return Compare(iStr) <= 0;
    }

    int operator>(const char *iStr) const
    {
        return Compare(iStr) > 0;
    }

    int operator>=(const char *iStr) const
    {
        return Compare(iStr) >= 0;
    }

    int operator-(const char *iStr) const
    {
        return Compare(iStr);
    }

    int operator==(const XBaseString &iStr) const
    {
        return !Compare(iStr);
    }

    int operator!=(const XBaseString &iStr) const
    {
        return Compare(iStr);
    }

    int operator<(const XBaseString &iStr) const
    {
        return Compare(iStr) < 0;
    }

    int operator<=(const XBaseString &iStr) const
    {
        return Compare(iStr) <= 0;
    }

    int operator>(const XBaseString &iStr) const
    {
        return Compare(iStr) > 0;
    }

    int operator>=(const XBaseString &iStr) const
    {
        return Compare(iStr) >= 0;
    }

    int operator-(const XBaseString &iStr) const
    {
        return Compare(iStr);
    }

    /************************************************
    Summary: Creates a new string that is the concatenation of the
    left and right operand.
    ************************************************/

    ///
    // Stream operators

    // Concatenation operator
    XString &operator<<(const char *iString)
    {
        if (iString)
        {
            Assign(iString, strlen(iString));
        }
        else
        {
            m_Length = 0;
            if (m_Buffer)
                m_Buffer[0] = '\0';
        }
        return *this;
    }

    // Concatenation operator
    XString &operator<<(const XBaseString &iString)
    {
        CheckSize(m_Length + iString.m_Length);
        if (iString.m_Length != 0)
        {
            strncpy(&m_Buffer[m_Length], iString.m_Buffer, iString.m_Length + 1);
            m_Length += iString.m_Length;
        }
        return *this;
    }

    // Concatenation operator
    XString &operator<<(const char iValue)
    {
        CheckSize(m_Length + 1);
        m_Buffer[m_Length++] = iValue;
        m_Buffer[m_Length] = '\0';
        return *this;
    }

    // Concatenation operator
    XString &operator<<(const int iValue)
    {
        char buf[16];
        int len = sprintf(buf, "%d", iValue);
        CheckSize(m_Length + len);
        strncpy(&m_Buffer[m_Length], buf, len + 1);
        m_Length += len;
        return *this;
    }

    // Concatenation operator
    XString &operator<<(const unsigned int iValue)
    {
        char buf[16];
        int len = sprintf(buf, "%u", iValue);
        CheckSize(m_Length + len);
        strncpy(&m_Buffer[m_Length], buf, len + 1);
        m_Length += len;
        return *this;
    }

    // Concatenation operator
    XString &operator<<(const float iValue)
    {
        char buf[16];
        int len = sprintf(buf, "%f", iValue);
        CheckSize(m_Length + len);
        strncpy(&m_Buffer[m_Length], buf, len + 1);
        m_Length += len;
        return *this;
    }

    // Concatenation operator
    XString &operator<<(const void *iValue)
    {
        if (iValue)
        {
            XWORD len = (XWORD)strlen((char *)iValue);
            if (len != 0)
            {
                CheckSize(m_Length + len);
                strncpy(&m_Buffer[m_Length], (const char *)iValue, len + 1);
                m_Length += len;
            }
        }
        return *this;
    }

    ///
    // + operators

    XString operator+(const char *iString) const
    {
        XString tmp = *this;
        return tmp << iString;
    }
    XString operator+(const XBaseString &iString) const
    {
        XString tmp = *this;
        return tmp << iString;
    }
    XString operator+(const char iValue) const
    {
        XString tmp = *this;
        return tmp << iValue;
    }
    XString operator+(const int iValue) const
    {
        XString tmp = *this;
        return tmp << iValue;
    }
    XString operator+(const unsigned int iValue) const
    {
        XString tmp = *this;
        return tmp << iValue;
    }
    XString operator+(const float iValue) const
    {
        XString tmp = *this;
        return tmp << iValue;
    }

    ///
    // Capacity functions

    /************************************************
    Summary: Returns the capacity (the allocated size)
    of the string.
    See also: Reserve.
    ************************************************/
    XWORD Capacity()
    {
        return m_Allocated;
    }

    // Sets the capacity of the string
    void Reserve(XWORD iLength)
    {
        if (iLength + 1 > m_Allocated)
        {
            m_Allocated = iLength + 1;
            char *buf = (char *)VxNew(sizeof(char) * (iLength + 1));
            if (m_Length != 0)
                strncpy(buf, m_Buffer, m_Length);
            VxDelete(m_Buffer);
            m_Buffer = buf;
        }
    }

    void Resize(XWORD iLength)
    {
        Reserve(iLength);
        if (iLength != 0)
            m_Buffer[iLength] = 0;
        else if (m_Buffer)
            m_Buffer[0] = 0;
        m_Length = iLength;
    }

    ///
    // Back Compatibility functions

    XString &operator+=(const XString &v) { return (*this) << v; }

    XString &operator+=(const char *v) { return (*this) << v; }

    XString &operator+=(const char v) { return (*this) << v; }

    XString &operator+=(const int v) { return (*this) << v; }

    XString &operator+=(const float v) { return (*this) << v; }

    // conversion operator (for script purpose only) (cast operator NYI)
    // operator char*()
    // {
    // 	return (m_Buffer)?m_Buffer:(char*)"";
    // }

protected:
    // Check a string size to see if it fits in
    void CheckSize(int iLength)
    {
        if (iLength + 1 > m_Allocated)
        {
            m_Allocated = iLength + 1;
            char *buf = (char *)VxNew(sizeof(char) * (iLength + 1));
            if (m_Length != 0)
                strncpy(buf, m_Buffer, m_Length + 1);
            else
                buf[0] = '\0';
            VxDelete(m_Buffer);
            m_Buffer = buf;
        }
    }

    // Assign a string to the current string
    void Assign(const char *iBuffer, int iLength)
    {
        m_Length = 0;
        if (iLength != 0)
        {
            CheckSize(iLength);
            m_Length = iLength;
            strncpy(m_Buffer, iBuffer, iLength);
        }
        else if (m_Buffer)
        {
            m_Buffer[0] = '\0';
        }
    }
};

#endif // XSTRING_H
