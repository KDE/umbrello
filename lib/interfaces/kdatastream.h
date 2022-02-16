#ifndef __kdatastream__h
#define __kdatastream__h

#include <qdatastream.h>

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
