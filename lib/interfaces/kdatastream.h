#ifndef __kdatastream__h
#define __kdatastream__h

#include <qdatastream.h>

#ifdef USE_QT3
inline QDataStream & operator << (QDataStream & str, bool b)
{
    str << Q_INT8(b);
    return str;
}

inline QDataStream & operator >> (QDataStream & str, bool & b)
{
    Q_INT8 l;
    str >> l;
    b = bool(l);
    return str;
}
#endif // USE_QT3

#if QT_VERSION < 0x030200 && !defined(Q_WS_WIN) && !defined(Q_WS_MAC)
inline QDataStream & operator << (QDataStream & str, long long int ll)
{
    Q_UINT32 l1,l2;
    l1 = ll & 0xffffffffLL;
    l2 = ll >> 32;
    str << l1 << l2;
    return str;
}

inline QDataStream & operator >> (QDataStream & str, long long int&ll)
{
    Q_UINT32 l1,l2;
    str >> l1 >> l2;
    ll = ((unsigned long long int)(l2) << 32) + (long long int) l1;
    return str;
}

inline QDataStream & operator << (QDataStream & str, unsigned long long int ll)
{
    Q_UINT32 l1,l2;
    l1 = ll & 0xffffffffLL;
    l2 = ll >> 32;
    str << l1 << l2;
    return str;
}

inline QDataStream & operator >> (QDataStream & str, unsigned long long int &ll)
{
    Q_UINT32 l1,l2;
    str >> l1 >> l2;
    ll = ((unsigned long long int)(l2) << 32) + (unsigned long long int) l1;
    return str;
}
#endif

inline QDataStream & operator >> (QDataStream & str, long unsigned int &ll)
{
    return str >> ((unsigned int &)ll);
}

inline QDataStream &operator << (QDataStream &str, long unsigned int &ll)
{
    return str << ((unsigned int &)ll);
}

inline QDataStream &operator << (QDataStream &str, const long unsigned int &ll)
{
    return str << ((unsigned int &)ll);
}

#endif
